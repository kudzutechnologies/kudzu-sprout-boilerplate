#ifndef KUDZUKERNEL_MODULENMEAPARSER_H
#define KUDZUKERNEL_MODULENMEAPARSER_H
class _ModuleNMEAParser;

#include <Module.hpp>
#include "Utilities/WaitGroupEvents.hpp"
#include "Utilities/WaitGroupPool.hpp"
#include <string.h>
#include <string>
#include <vector>
#include "esp_wifi.h"

/**
 * Default value when posting events
 */
#define MODULE_NMEAPARSER_MESSAGE_TIMEOUT     15000 / portTICK_PERIOD_MS


/**
 * Maximum number of bytes in the Message buffer
 */
#define MODULE_NMEAPARSER_MAX_MESSAGE_SIZE  256
#define NMEA_MAX_STATEMENT_ITEM_LENGTH (16)



#define GPS_MAX_SATELLITES_IN_USE (12)
#define GPS_MAX_SATELLITES_IN_VIEW (16)

/**
 * @brief GPS fix type
 *
 */
typedef enum {
    GPS_FIX_INVALID, /*!< Not fixed */
    GPS_FIX_GPS,     /*!< GPS */
    GPS_FIX_DGPS,    /*!< Differential GPS */
} gps_fix_t;

/**
 * @brief GPS fix mode
 *
 */
typedef enum {
    GPS_MODE_INVALID = 1, /*!< Not fixed */
    GPS_MODE_2D,          /*!< 2D GPS */
    GPS_MODE_3D           /*!< 3D GPS */
} gps_fix_mode_t;

/**
 * @brief GPS satellite information
 *
 */
typedef struct {
    uint8_t num;       /*!< Satellite number */
    uint8_t elevation; /*!< Satellite elevation */
    uint16_t azimuth;  /*!< Satellite azimuth */
    uint8_t snr;       /*!< Satellite signal noise ratio */
} gps_satellite_t;

/**
 * @brief GPS time
 *
 */
typedef struct {
    uint8_t hour;      /*!< Hour */
    uint8_t minute;    /*!< Minute */
    uint8_t second;    /*!< Second */
    uint16_t thousand; /*!< Thousand */
} gps_time_t;

/**
 * @brief GPS date
 *
 */
typedef struct {
    uint8_t day;   /*!< Day (start from 1) */
    uint8_t month; /*!< Month (start from 1) */
    uint16_t year; /*!< Year (start from 2000) */
} gps_date_t;

/**
 * @brief NMEA Statement
 *
 */
typedef enum {
    STATEMENT_UNKNOWN = 0, /*!< Unknown statement */
    STATEMENT_GGA,         /*!< GGA */
    STATEMENT_GSA,         /*!< GSA */
    STATEMENT_RMC,         /*!< RMC */
    STATEMENT_GSV,         /*!< GSV */
    STATEMENT_GLL,         /*!< GLL */
    STATEMENT_VTG          /*!< VTG */
} nmea_statement_t;

/**
 * @brief GPS object
 *
 */
typedef struct {
    float latitude;                                                /*!< Latitude (degrees) */
    float longitude;                                               /*!< Longitude (degrees) */
    float altitude;                                                /*!< Altitude (meters) */
    gps_fix_t fix;                                                 /*!< Fix status */
    uint8_t sats_in_use;                                           /*!< Number of satellites in use */
    gps_time_t tim;                                                /*!< time in UTC */
    gps_fix_mode_t fix_mode;                                       /*!< Fix mode */
    uint8_t sats_id_in_use[GPS_MAX_SATELLITES_IN_USE];             /*!< ID list of satellite in use */
    float dop_h;                                                   /*!< Horizontal dilution of precision */
    float dop_p;                                                   /*!< Position dilution of precision  */
    float dop_v;                                                   /*!< Vertical dilution of precision  */
    uint8_t sats_in_view;                                          /*!< Number of satellites in view */
    gps_satellite_t sats_desc_in_view[GPS_MAX_SATELLITES_IN_VIEW]; /*!< Information of satellites in view */
    gps_date_t date;                                               /*!< Fix date */
    bool valid;                                                    /*!< GPS validity */
    float speed;                                                   /*!< Ground speed, unit: m/s */
    float cog;                                                     /*!< Course over ground */
    float variation;                                               /*!< Magnetic variation */
    char receiver[2];                                              /*!< Receiver name ('GP' for gps) */
} gps_t;

/**
 * @brief NMEA Parser Event ID
 *
 */
typedef enum {
    GPS_UPDATE, /*!< GPS information has been updated */
    GPS_UNKNOWN /*!< Unknown statements detected */
} nmea_event_id_t;

enum NMEAType {
  NMEA_STATEMENT_GGA=0,
  NMEA_STATEMENT_GSA,
  NMEA_STATEMENT_RMC,
  NMEA_STATEMENT_GSV,
  NMEA_STATEMENT_GLL,
  NMEA_STATEMENT_VTG,
  NMEA_STATEMENT_UNKNOWN,
};

struct NMEAStatement {
  const char *  data;
  NMEAType      type;
};

enum ModuleNMEAParserEvents {
  EVENT_NMEA_TIMEOUT,
  EVENT_NMEA_STATEMENT_GGA,
  EVENT_NMEA_STATEMENT_GSA,
  EVENT_NMEA_STATEMENT_RMC,
  EVENT_NMEA_STATEMENT_GSV,
  EVENT_NMEA_STATEMENT_GLL,
  EVENT_NMEA_STATEMENT_VTG,
  EVENT_NMEA_UNDEFINED_STATEMENT,
  EVENT_NMEA_GENERIC_DATA_RECEIVED,
};

struct NMEADataPointer {
  char *    data;
  size_t    data_len;
};

/**
 * Statically allocated buffer for AT messages, that allows
 * dynamic split between a command and a value.
 */
class NMEAMessageBuffer {
private:
  friend class _ModuleNMEAParser;
  char      buffer[MODULE_NMEAPARSER_MAX_MESSAGE_SIZE];
  uint8_t   used;

public:
  union {
    uint8_t   status;
    uint8_t   type;
  };
};

/**
 * WiFi module is responsible for activating or de-activating the WiFi on the device
 */
class _ModuleNMEAParser: public Module {
public:

  _ModuleNMEAParser();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

  /**
   * Suspend or resume operation
   */
  void setSuspend(bool suspended);

protected:

  /**
   * Initialize setup
   */
  virtual void setup();

  /**
   * Declare a few event handlers
   */
  DECLARE_EVENT_HANDLER(uart_events);

  /**
   * Module activation and de-activation functions
   */
  virtual void activate();
  virtual void deactivate();

private:

  ModuleTimer_t                 nmea_timer;
  bool                          suspended;

};

extern _ModuleNMEAParser ModuleNMEAParser;

#endif
