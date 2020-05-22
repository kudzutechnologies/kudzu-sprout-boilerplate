#include "Modules/ModuleUART0.hpp"
#include "ModuleNMEAParser.hpp"
#include "ModuleManager.hpp"
#include "Sherlock.hpp"
// #include "NMEAUtils.hpp"
#include <ctype.h>
#include "esp_system.h"
#include "esp_log.h"

/**
 * @brief GPS parser library runtime structure
 *
 */
typedef struct {
    uint8_t item_pos;                              /*!< Current position in item */
    uint8_t item_num;                              /*!< Current item number */
    uint8_t asterisk;                              /*!< Asterisk detected flag */
    uint8_t crc;                                   /*!< Calculated CRC value */
    uint8_t parsed_statement;                      /*!< OR'd of statements that have been parsed */
    uint8_t sat_num;                               /*!< Satellite number */
    uint8_t sat_count;                             /*!< Satellite count */
    uint8_t cur_statement;                         /*!< Current statement ID */
    uint32_t all_statements;                       /*!< All statements mask */
    char item_str[NMEA_MAX_STATEMENT_ITEM_LENGTH]; /*!< Current item */
    gps_t parent;                                  /*!< Parent class */
    // uint8_t *buffer;                               /*!< Runtime buffer */
} esp_gps_t;

/**
 * Parses a NMEA command from the given string buffer into a tokenized
 * structure
 */
void parseNMEACommand(const char * buf, size_t len, NMEAStatement *statement, esp_gps_t *esp_gps);

/**
 * Module Singleton
 */
_ModuleNMEAParser ModuleNMEAParser;

/**
 * Module configuration
 */
static const ModuleConfig config = {
	.name = "drv.nmeaparser",
	.title = "UART0 NMEA Parser",
    .category = MODULE_CATEGORY_DRIVER,
	.nv_size = 0,
	.nv_version = 0,
	.runlevels = { /* On demand */ },
	.activate = DEFAULT_ACTIVE,
	.depends = {
		&ModuleUART0
	}
};

/**
 * Configuration forwarding
 */
static const char *TAG = config.name;
const ModuleConfig &_ModuleNMEAParser::getModuleConfig() { return config; }

_ModuleNMEAParser::_ModuleNMEAParser()
	: Module(), nmea_timer(NULL), suspended(false)
{
}

#define NMEA_PARSER_RUNTIME_BUFFER_SIZE 1024/2//(CONFIG_NMEA_PARSER_RING_BUFFER_SIZE / 2)

#define CONFIG_NMEA_STATEMENT_GGA 1
#define CONFIG_NMEA_STATEMENT_GSV 1
#define CONFIG_NMEA_STATEMENT_GLL 1
#define CONFIG_NMEA_STATEMENT_GSA 1
#define CONFIG_NMEA_STATEMENT_VTG 1
#define CONFIG_NMEA_STATEMENT_RMC 1


/**
 * @brief parse latitude or longitude
 *              format of latitude in NMEA is ddmm.sss and longitude is dddmm.sss
 * @param esp_gps esp_gps_t type object
 * @return float Latitude or Longitude value (unit: degree)
 */
static float parse_lat_long(esp_gps_t *esp_gps)
{
    float ll = strtof(esp_gps->item_str, NULL);
    int deg = ((int)ll) / 100;
    float min = ll - (deg * 100);
    ll = deg + min / 60.0f;
    return ll;
}

/**
 * @brief Converter two continuous numeric character into a uint8_t number
 *
 * @param digit_char numeric character
 * @return uint8_t result of converting
 */
static inline uint8_t convert_two_digit2number(const char *digit_char)
{
    return 10 * (digit_char[0] - '0') + (digit_char[1] - '0');
}

/**
 * @brief Parse UTC time in GPS statements
 *
 * @param esp_gps esp_gps_t type object
 */
static void parse_utc_time(esp_gps_t *esp_gps)
{
    esp_gps->parent.tim.hour = convert_two_digit2number(esp_gps->item_str + 0);
    esp_gps->parent.tim.minute = convert_two_digit2number(esp_gps->item_str + 2);
    esp_gps->parent.tim.second = convert_two_digit2number(esp_gps->item_str + 4);
    if (esp_gps->item_str[6] == '.') {
        uint16_t tmp = 0;
        uint8_t i = 7;
        while (esp_gps->item_str[i]) {
            tmp = 10 * tmp + esp_gps->item_str[i] - '0';
            i++;
        }
        esp_gps->parent.tim.thousand = tmp;
    }
}

#if CONFIG_NMEA_STATEMENT_GGA
/**
 * @brief Parse GGA statements
 *
 * @param esp_gps esp_gps_t type object
 */
static void parse_gga(esp_gps_t *esp_gps)
{
    /* Process GGA statement */
    switch (esp_gps->item_num) {
    case 1: /* Process UTC time */
        parse_utc_time(esp_gps);
        break;
    case 2: /* Latitude */
        esp_gps->parent.latitude = parse_lat_long(esp_gps);
        break;
    case 3: /* Latitude north(1)/south(-1) information */
        if (esp_gps->item_str[0] == 'S' || esp_gps->item_str[0] == 's') {
            esp_gps->parent.latitude *= -1;
        }
        break;
    case 4: /* Longitude */
        esp_gps->parent.longitude = parse_lat_long(esp_gps);
        break;
    case 5: /* Longitude east(1)/west(-1) information */
        if (esp_gps->item_str[0] == 'W' || esp_gps->item_str[0] == 'w') {
            esp_gps->parent.longitude *= -1;
        }
        break;
    case 6: /* Fix status */
        esp_gps->parent.fix = (gps_fix_t)strtol(esp_gps->item_str, NULL, 10);
        break;
    case 7: /* Satellites in use */
        esp_gps->parent.sats_in_use = (uint8_t)strtol(esp_gps->item_str, NULL, 10);
        break;
    case 8: /* HDOP */
        esp_gps->parent.dop_h = strtof(esp_gps->item_str, NULL);
        break;
    case 9: /* Altitude */
        esp_gps->parent.altitude = strtof(esp_gps->item_str, NULL);
        break;
    case 11: /* Altitude above ellipsoid */
        esp_gps->parent.altitude += strtof(esp_gps->item_str, NULL);
        break;
    default:
        break;
    }
}
#endif

#if CONFIG_NMEA_STATEMENT_GSA
/**
 * @brief Parse GSA statements
 *
 * @param esp_gps esp_gps_t type object
 */
static void parse_gsa(esp_gps_t *esp_gps)
{
    /* Process GSA statement */
    switch (esp_gps->item_num) {
    case 2: /* Process fix mode */
        esp_gps->parent.fix_mode = (gps_fix_mode_t)strtol(esp_gps->item_str, NULL, 10);
        break;
    case 15: /* Process PDOP */
        esp_gps->parent.dop_p = strtof(esp_gps->item_str, NULL);
        break;
    case 16: /* Process HDOP */
        esp_gps->parent.dop_h = strtof(esp_gps->item_str, NULL);
        break;
    case 17: /* Process VDOP */
        esp_gps->parent.dop_v = strtof(esp_gps->item_str, NULL);
        break;
    default:
        /* Parse satellite IDs */
        if (esp_gps->item_num >= 3 && esp_gps->item_num <= 14) {
            esp_gps->parent.sats_id_in_use[esp_gps->item_num - 3] = (uint8_t)strtol(esp_gps->item_str, NULL, 10);
        }
        break;
    }
}
#endif

#if CONFIG_NMEA_STATEMENT_GSV
/**
 * @brief Parse GSV statements
 *
 * @param esp_gps esp_gps_t type object
 */
static void parse_gsv(esp_gps_t *esp_gps)
{
    /* Process GSV statement */
    switch (esp_gps->item_num) {
    case 1: /* total GSV numbers */
        esp_gps->sat_count = (uint8_t)strtol(esp_gps->item_str, NULL, 10);
        break;
    case 2: /* Current GSV statement number */
        esp_gps->sat_num = (uint8_t)strtol(esp_gps->item_str, NULL, 10);
        break;
    case 3: /* Process satellites in view */
        esp_gps->parent.sats_in_view = (uint8_t)strtol(esp_gps->item_str, NULL, 10);
        break;
    default:
        if (esp_gps->item_num >= 4 && esp_gps->item_num <= 19) {
            uint8_t item_num = esp_gps->item_num - 4; /* Normalize item number from 4-19 to 0-15 */
            uint8_t index;
            uint32_t value;
            index = 4 * (esp_gps->sat_num - 1) + item_num / 4; /* Get array index */
            if (index < GPS_MAX_SATELLITES_IN_VIEW) {
                value = strtol(esp_gps->item_str, NULL, 10);
                switch (item_num % 4) {
                case 0:
                    esp_gps->parent.sats_desc_in_view[index].num = (uint8_t)value;
                    break;
                case 1:
                    esp_gps->parent.sats_desc_in_view[index].elevation = (uint8_t)value;
                    break;
                case 2:
                    esp_gps->parent.sats_desc_in_view[index].azimuth = (uint16_t)value;
                    break;
                case 3:
                    esp_gps->parent.sats_desc_in_view[index].snr = (uint8_t)value;
                    break;
                default:
                    break;
                }
            }
        }
        break;
    }
}
#endif

#if CONFIG_NMEA_STATEMENT_RMC
/**
 * @brief Parse RMC statements
 *
 * @param esp_gps esp_gps_t type object
 */
static void parse_rmc(esp_gps_t *esp_gps)
{
    /* Process GPRMC statement */
    switch (esp_gps->item_num) {
    case 1:/* Process UTC time */
        parse_utc_time(esp_gps);
        break;
    case 2: /* Process valid status */
        esp_gps->parent.valid = (esp_gps->item_str[0] == 'A');
        break;
    case 3:/* Latitude */
        esp_gps->parent.latitude = parse_lat_long(esp_gps);
        break;
    case 4: /* Latitude north(1)/south(-1) information */
        if (esp_gps->item_str[0] == 'S' || esp_gps->item_str[0] == 's') {
            esp_gps->parent.latitude *= -1;
        }
        break;
    case 5: /* Longitude */
        esp_gps->parent.longitude = parse_lat_long(esp_gps);
        break;
    case 6: /* Longitude east(1)/west(-1) information */
        if (esp_gps->item_str[0] == 'W' || esp_gps->item_str[0] == 'w') {
            esp_gps->parent.longitude *= -1;
        }
        break;
    case 7: /* Process ground speed in unit m/s */
        esp_gps->parent.speed = strtof(esp_gps->item_str, NULL) * 1.852;
        break;
    case 8: /* Process true course over ground */
        esp_gps->parent.cog = strtof(esp_gps->item_str, NULL);
        break;
    case 9: /* Process date */
        esp_gps->parent.date.day = convert_two_digit2number(esp_gps->item_str + 0);
        esp_gps->parent.date.month = convert_two_digit2number(esp_gps->item_str + 2);
        esp_gps->parent.date.year = convert_two_digit2number(esp_gps->item_str + 4);
        break;
    case 10: /* Process magnetic variation */
        esp_gps->parent.variation = strtof(esp_gps->item_str, NULL);
        break;
    default:
        break;
    }
}
#endif

#if CONFIG_NMEA_STATEMENT_GLL
/**
 * @brief Parse GLL statements
 *
 * @param esp_gps esp_gps_t type object
 */
static void parse_gll(esp_gps_t *esp_gps)
{
    /* Process GPGLL statement */
    switch (esp_gps->item_num) {
    case 1:/* Latitude */
        esp_gps->parent.latitude = parse_lat_long(esp_gps);
        break;
    case 2: /* Latitude north(1)/south(-1) information */
        if (esp_gps->item_str[0] == 'S' || esp_gps->item_str[0] == 's') {
            esp_gps->parent.latitude *= -1;
        }
        break;
    case 3: /* Longitude */
        esp_gps->parent.longitude = parse_lat_long(esp_gps);
        break;
    case 4: /* Longitude east(1)/west(-1) information */
        if (esp_gps->item_str[0] == 'W' || esp_gps->item_str[0] == 'w') {
            esp_gps->parent.longitude *= -1;
        }
        break;
    case 5:/* Process UTC time */
        parse_utc_time(esp_gps);
        break;
    case 6: /* Process valid status */
        esp_gps->parent.valid = (esp_gps->item_str[0] == 'A');
        break;
    default:
        break;
    }
}
#endif

#if CONFIG_NMEA_STATEMENT_VTG
/**
 * @brief Parse VTG statements
 *
 * @param esp_gps esp_gps_t type object
 */
static void parse_vtg(esp_gps_t *esp_gps)
{
    /* Process GPVGT statement */
    switch (esp_gps->item_num) {
    case 1: /* Process true course over ground */
        esp_gps->parent.cog = strtof(esp_gps->item_str, NULL);
        break;
    case 3:/* Process magnetic variation */
        esp_gps->parent.variation = strtof(esp_gps->item_str, NULL);
        break;
    case 5:/* Process ground speed in unit m/s */
        esp_gps->parent.speed = strtof(esp_gps->item_str, NULL) * 1.852;//knots to m/s
        break;
    case 7:/* Process ground speed in unit m/s */
        esp_gps->parent.speed = strtof(esp_gps->item_str, NULL) / 3.6;//km/h to m/s
        break;
    default:
        break;
    }
}
#endif

/**
 * @brief Parse received item
 *
 * @param esp_gps esp_gps_t type object
 * @return esp_err_t ESP_OK on success, ESP_FAIL on error
 */
static esp_err_t parse_item(esp_gps_t *esp_gps)
{
    esp_err_t err = ESP_OK;
    // TRACE_LOGD(TAG, "parse_item num=%d, str='%s'", esp_gps->item_num, esp_gps->item_str);

    /* start of a statement */
    if (esp_gps->item_num == 0 && esp_gps->item_str[0] == '$') {
        esp_gps->parent.receiver[0] = esp_gps->item_str[1];
        esp_gps->parent.receiver[1] = esp_gps->item_str[2];

        if (0) {
        }
#if CONFIG_NMEA_STATEMENT_GGA
        else if (strstr(esp_gps->item_str, "GGA")) {
            esp_gps->cur_statement = STATEMENT_GGA;
        }
#endif
#if CONFIG_NMEA_STATEMENT_GSA
        else if (strstr(esp_gps->item_str, "GSA")) {
            esp_gps->cur_statement = STATEMENT_GSA;
        }
#endif
#if CONFIG_NMEA_STATEMENT_RMC
        else if (strstr(esp_gps->item_str, "RMC")) {
            esp_gps->cur_statement = STATEMENT_RMC;
        }
#endif
#if CONFIG_NMEA_STATEMENT_GSV
        else if (strstr(esp_gps->item_str, "GSV")) {
            esp_gps->cur_statement = STATEMENT_GSV;
        }
#endif
#if CONFIG_NMEA_STATEMENT_GLL
        else if (strstr(esp_gps->item_str, "GLL")) {
            esp_gps->cur_statement = STATEMENT_GLL;
        }
#endif
#if CONFIG_NMEA_STATEMENT_VTG
        else if (strstr(esp_gps->item_str, "VTG")) {
            esp_gps->cur_statement = STATEMENT_VTG;
        }
#endif
        else {
            esp_gps->cur_statement = STATEMENT_UNKNOWN;
        }
        goto out;
    }
    /* Parse each item, depend on the type of the statement */
    if (esp_gps->cur_statement == STATEMENT_UNKNOWN) {
        goto out;
    }
#if CONFIG_NMEA_STATEMENT_GGA
    else if (esp_gps->cur_statement == STATEMENT_GGA) {
        parse_gga(esp_gps);
    }
#endif
#if CONFIG_NMEA_STATEMENT_GSA
    else if (esp_gps->cur_statement == STATEMENT_GSA) {
        parse_gsa(esp_gps);
    }
#endif
#if CONFIG_NMEA_STATEMENT_GSV
    else if (esp_gps->cur_statement == STATEMENT_GSV) {
        parse_gsv(esp_gps);
    }
#endif
#if CONFIG_NMEA_STATEMENT_RMC
    else if (esp_gps->cur_statement == STATEMENT_RMC) {
        parse_rmc(esp_gps);
    }
#endif
#if CONFIG_NMEA_STATEMENT_GLL
    else if (esp_gps->cur_statement == STATEMENT_GLL) {
        parse_gll(esp_gps);
    }
#endif
#if CONFIG_NMEA_STATEMENT_VTG
    else if (esp_gps->cur_statement == STATEMENT_VTG) {
        parse_vtg(esp_gps);
    }
#endif
    else {
        err =  ESP_FAIL;
    }
out:
    return err;
}

/**
 * Parse the given string, and tokenize an expression
 */
void parseNMEACommand(const char *buf, size_t len, NMEAStatement *statement, esp_gps_t *esp_gps)
{
	if (len == 0) return;
	if (buf == NULL) return;
    const char *d = buf;
    memset(statement, 0, sizeof(NMEAStatement));

	///////////////////////////////////////////////////////////////////////////
	while (*d) {
        /* Start of a statement */
        if (*d == '$') {
            /* Reset runtime information */
            esp_gps->asterisk = 0;
            esp_gps->item_num = 0;
            esp_gps->item_pos = 0;
            esp_gps->cur_statement = 0;
            esp_gps->crc = 0;
            esp_gps->sat_count = 0;
            esp_gps->sat_num = 0;
            /* Add character to item */
            esp_gps->item_str[esp_gps->item_pos++] = *d;
            esp_gps->item_str[esp_gps->item_pos] = '\0';
        }
        /* Detect item separator character */
        else if (*d == ',') {
            /* Parse current item */
            parse_item(esp_gps);
            /* Add character to CRC computation */
            esp_gps->crc ^= (uint8_t)(*d);
            /* Start with next item */
            esp_gps->item_pos = 0;
            esp_gps->item_str[0] = '\0';
            esp_gps->item_num++;
        }
        /* End of CRC computation */
        else if (*d == '*') {
            /* Parse current item */
            parse_item(esp_gps);
            /* Asterisk detected */
            esp_gps->asterisk = 1;
            /* Start with next item */
            esp_gps->item_pos = 0;
            esp_gps->item_str[0] = '\0';
            esp_gps->item_num++;
        }
        /* End of statement */
        else if (*d == '\r') {
            /* Convert received CRC from string (hex) to number */
            uint8_t crc = (uint8_t)strtol(esp_gps->item_str, NULL, 16);
            /* CRC passed */
            if (esp_gps->crc == crc) {
                switch (esp_gps->cur_statement) {
#if CONFIG_NMEA_STATEMENT_GGA
                case STATEMENT_GGA:
                    esp_gps->parsed_statement |= 1 << STATEMENT_GGA;
					statement->type = NMEA_STATEMENT_GGA;
                    break;
#endif
#if CONFIG_NMEA_STATEMENT_GSA
                case STATEMENT_GSA:
                    esp_gps->parsed_statement |= 1 << STATEMENT_GSA;
					statement->type = NMEA_STATEMENT_GSA;
                    break;
#endif
#if CONFIG_NMEA_STATEMENT_RMC
                case STATEMENT_RMC:
                    esp_gps->parsed_statement |= 1 << STATEMENT_RMC;
                    statement->type = NMEA_STATEMENT_RMC;
                    break;
#endif
#if CONFIG_NMEA_STATEMENT_GSV
                case STATEMENT_GSV:
                    statement->type = NMEA_STATEMENT_GSV;
                    if (esp_gps->sat_num == esp_gps->sat_count) {
                        esp_gps->parsed_statement |= 1 << STATEMENT_GSV;
                    }
                    break;
#endif
#if CONFIG_NMEA_STATEMENT_GLL
                case STATEMENT_GLL:
                    statement->type = NMEA_STATEMENT_GLL;
                    esp_gps->parsed_statement |= 1 << STATEMENT_GLL;
                    break;
#endif
#if CONFIG_NMEA_STATEMENT_VTG
                case STATEMENT_VTG:
                    statement->type = NMEA_STATEMENT_VTG;
                    esp_gps->parsed_statement |= 1 << STATEMENT_VTG;
                    break;
#endif
                default:
                    break;
                }
                /* Check if all statements have been parsed */
                if (((esp_gps->parsed_statement) & esp_gps->all_statements) == esp_gps->all_statements) {
                    esp_gps->parsed_statement = 0;
                    // /* Send signal to notify that GPS information has been updated */
                    // esp_event_post_to(esp_gps->event_loop_hdl, ESP_NMEA_EVENT, GPS_UPDATE,
                    //                   &(esp_gps->parent), sizeof(gps_t), 100 / portTICK_PERIOD_MS);
                }
            } else {
                TRACE_LOGW(TAG, "CRC Error on incoming NMEA message");
            }
            if (esp_gps->cur_statement == STATEMENT_UNKNOWN) {
                // /* Send signal to notify that one unknown statement has been met */
                // esp_event_post_to(esp_gps->event_loop_hdl, ESP_NMEA_EVENT, GPS_UNKNOWN,
                //                   esp_gps->buffer, len, 100 / portTICK_PERIOD_MS);
            }
        }
        /* Other non-space character */
        else {
            if (!(esp_gps->asterisk)) {
                /* Add to CRC */
                esp_gps->crc ^= (uint8_t)(*d);
            }
            /* Add character to item */
            esp_gps->item_str[esp_gps->item_pos++] = *d;
            esp_gps->item_str[esp_gps->item_pos] = '\0';
        }
        /* Process next character */
        d++;
   }
}

/**
 * Event handler for network events
 */
DEFINE_EVENT_HANDLER(_ModuleNMEAParser::uart_events)
(esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    esp_gps_t esp_gps;

	TRACE_LOGD(TAG, "event='%s', id='%d'", event_base, event_id);
	ModuleUARTRxEvent *rxEvent = (ModuleUARTRxEvent *)event_data;
	char buf[256];
	int len;

	switch (event_id)
	{
	case EVENT_UART_RX_PATTERN:
		TRACE_LOGD(TAG, "event_id=%d, task=%s, stack=%d", event_id,  pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL));
		len = rxEvent->consume((char *)buf, 255);
		buf[len] = '\0';
		TRACE_LOGD(TAG, "Got incoming NMEAcmd (len=%d): '%.*s'", len, len, buf);

		// Ignore empty lines
		if (len == 0)
			break;

		// Parse NMEA command
		{
			NMEAStatement statement;
            statement.type = NMEA_STATEMENT_UNKNOWN;
			parseNMEACommand(buf, len, &statement, &esp_gps);
			switch (statement.type)
			{
			case NMEA_STATEMENT_GGA:
				TRACE_LOGD(TAG, "Received NMEA_STATEMENT_GGA");
				eventPost(EVENT_NMEA_STATEMENT_GGA, &esp_gps.parent, sizeof(esp_gps.parent));
				break;
			case NMEA_STATEMENT_GSA:
				TRACE_LOGD(TAG, "Received NMEA_STATEMENT_GSA");
				eventPost(EVENT_NMEA_STATEMENT_GSA, &esp_gps.parent, sizeof(esp_gps.parent));
				break;
  			case NMEA_STATEMENT_RMC:
			  	TRACE_LOGD(TAG, "Received NMEA_STATEMENT_RMC");
				  eventPost(EVENT_NMEA_STATEMENT_RMC, &esp_gps.parent, sizeof(esp_gps.parent));
			  	break;
			case NMEA_STATEMENT_GSV:
				TRACE_LOGD(TAG, "Received NMEA_STATEMENT_GSV");
				eventPost(EVENT_NMEA_STATEMENT_GSV, &esp_gps.parent, sizeof(esp_gps.parent));
				break;
			case NMEA_STATEMENT_GLL:
				TRACE_LOGD(TAG, "Received NMEA_STATEMENT_GLL");
				eventPost(EVENT_NMEA_STATEMENT_GLL, &esp_gps.parent, sizeof(esp_gps.parent));
				break;
			case NMEA_STATEMENT_VTG:
				TRACE_LOGD(TAG, "Received NMEA_STATEMENT_VTG");
				eventPost(EVENT_NMEA_STATEMENT_VTG, &esp_gps.parent, sizeof(esp_gps.parent));
				break;
			default:
				TRACE_LOGW(TAG, "Received unknown NMEA STATEMENT: '%.*s'", len, buf);
				eventPost(EVENT_NMEA_UNDEFINED_STATEMENT, buf, len);
				break;
			}
		}
		break;

	case EVENT_UART_RX_DATA:
		len = rxEvent->consume((char *)buf, 255);
		buf[len] = '\0';
		TRACE_LOGD(TAG, "Got incoming DATA (len=%d): '%.*s'", len, len, buf);

		// Otherwise there are data on the buffer that we must consume
		eventPost(EVENT_NMEA_GENERIC_DATA_RECEIVED, buf, len);
		break;
	}
}

/**
 * Entry level stack initialization
 */
void _ModuleNMEAParser::setup()
{
	EVENT_HANDLER_REGISTER_ON(ModuleUART0, uart_events, ESP_EVENT_ANY_ID);
}

/**
 * Module activation and de-activation functions
 */
void _ModuleNMEAParser::activate()
{
	TRACE_LOGD(TAG, "activate");
	ackActivate();
}

void _ModuleNMEAParser::deactivate()
{
	TRACE_LOGD(TAG, "deactivate");
	ackDeactivate();
}

/**
 * Suspend or resume NMEA parsing
 */
void _ModuleNMEAParser::setSuspend(bool suspended)
{
	if (this->suspended == suspended)
		return;
	this->suspended = suspended;

	if (suspended)
	{
		ModuleUART0.patternDetectClear();
	}
	else
	{
		ModuleUART0.patternDetectSet('\n');
	}
}
