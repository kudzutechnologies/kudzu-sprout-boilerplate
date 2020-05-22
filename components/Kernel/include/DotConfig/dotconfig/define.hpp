#ifndef DOT_CONFIG_CONFIG_H
#define DOT_CONFIG_CONFIG_H
#include <vector>
#include <string>
#include <stdint.h>
#include "matrix.hpp"
#include "declarations.hpp"

/**
 * Macros to declare data binding
 */

#define DT_NONE               0
#define DT_FIXED_STRING       1
#define DT_STD_STRING         2
#define DT_INT                3
#define DT_FLOAT              4
#define DT_BOOL               5
#define DT_MATRIX             6
#define DT_STD_CSTR_VECTOR    7
#define DT_CALLBACK_BTN       8
#define DT_CALLBACK_GEN       9

#define DTA_NONE              0
#define DTA_RO                1
#define DTA_WO                2
#define DTA_RW                DTA_RO | DTA_WO

#define BIND_NONE()                   .bind = { .ptr = NULL,      .ud = NULL, .uid = {0,0,0,0,0,0,0,0}, .type = DT_NONE,              .access = DTA_RW,     .len = 0 }
#define BIND_FIXED_STRING(P,L)        .bind = { .ptr = (void*)P,  .ud = NULL, .uid = {0,0,0,0,0,0,0,0}, .type = DT_FIXED_STRING,      .access = DTA_RW,     .len = L }
#define BIND_STD_STRING(P)            .bind = { .ptr = (void*)&P, .ud = NULL, .uid = {0,0,0,0,0,0,0,0}, .type = DT_STD_STRING,        .access = DTA_RW,     .len = 0 }
#define BIND_INT(P)                   .bind = { .ptr = (void*)&P, .ud = NULL, .uid = {0,0,0,0,0,0,0,0}, .type = DT_INT,               .access = DTA_RW,     .len = 0 }
#define BIND_FLOAT(P)                 .bind = { .ptr = (void*)&P, .ud = NULL, .uid = {0,0,0,0,0,0,0,0}, .type = DT_FLOAT,             .access = DTA_RW,     .len = 0 }
#define BIND_BOOL(P)                  .bind = { .ptr = (void*)&P, .ud = NULL, .uid = {0,0,0,0,0,0,0,0}, .type = DT_BOOL,              .access = DTA_RW,     .len = 0 }
#define BIND_MATRIX(P)                .bind = { .ptr = (void*)((DotMatrixInterface*)&P), \
                                                                  .ud = NULL, .uid = {0,0,0,0,0,0,0,0}, .type = DT_MATRIX,            .access = DTA_RW,     .len = 0 }

#define BIND_RO_STD_CSTR_VECTOR(P,L)  .bind = { .ptr = (void*)&P, .ud = NULL, .uid = {0,0,0,0,0,0,0,0}, .type = DT_STD_CSTR_VECTOR,   .access = DTA_RO,     .len = 0 }
#define BIND_CALLBACK(CB,UD)          .bind = { .ptr = (void*)((buttonPressCallback)CB), \
                                                .ud = (void*)UD, \
                                                                              .uid = {0,0,0,0,0,0,0,0}, .type = DT_CALLBACK_BTN,      .access = DTA_NONE,   .len = 0 }
#define BIND_GENERATOR(CB,UD)         .bind = { .ptr = (void*)((generatorCallback)CB), \
                                                .ud = (void*)UD, \
                                                                              .uid = {0,0,0,0,0,0,0,0}, .type = DT_CALLBACK_GEN,      .access = DTA_RO,     .len = 0 }
#define BIND_WO_FIXED_STRING(P,L)     .bind = { .ptr = (void*)P,  .ud = NULL, .uid = {0,0,0,0,0,0,0,0}, .type = DT_FIXED_STRING,      .access = DTA_WO,     .len = L }
#define BIND_WO_STD_STRING(P)         .bind = { .ptr = (void*)&P, .ud = NULL, .uid = {0,0,0,0,0,0,0,0}, .type = DT_STD_STRING,        .access = DTA_WO,     .len = 0 }

/**
 * Macros to declare type widgets
 */

#define WT_HTML               0
#define WT_TEXT               1
#define WT_PASSWORD           2
#define WT_DROPDOWN           3
#define WT_DROPDOWN_REF       4
#define WT_LABEL              5
#define WT_MATRIX             6
#define WT_NUMBER             7
#define WT_SLIDER             8
#define WT_DIAL               9
#define WT_DROPDOWN_INDEX     10
#define WT_SWITCH             11
#define WT_LED                12
#define WT_BUTTON             13
#define WT_NONE               127


#define WLED_RED              "Cr"
#define WLED_GREEN            "Cg"
#define WLED_BLUE             "Cb"
#define WLED_YELLOW           "Cy"

#define WIDGET_NONE()                   .widget = { .opts = { },                .ref = NULL, .writable = 0, .type = WT_NONE }
#define WIDGET_HTML(H)                  .widget = { .opts = { H },              .ref = NULL, .writable = 0, .type = WT_HTML }
#define WIDGET_TEXT()                   .widget = { .opts = { },                .ref = NULL, .writable = 1, .type = WT_TEXT }
#define WIDGET_PASSWORD()               .widget = { .opts = { },                .ref = NULL, .writable = 1, .type = WT_PASSWORD }
#define WIDGET_DROPDOWN(...)            .widget = { .opts = { __VA_ARGS__ },    .ref = NULL, .writable = 1, .type = WT_DROPDOWN }
#define WIDGET_DROPDOWN_REF(X)          .widget = { .opts = {"", "Loading..."}, .ref = &X,   .writable = 1, .type = WT_DROPDOWN_REF }
#define WIDGET_DROPDOWN_INDEX(...)      .widget = { .opts = { __VA_ARGS__ },    .ref = NULL, .writable = 1, .type = WT_DROPDOWN_INDEX }
#define WIDGET_LABEL()                  .widget = { .opts = { },                .ref = NULL, .writable = 0, .type = WT_LABEL }
#define WIDGET_LABEL_UNITS(U)           .widget = { .opts = { U },              .ref = NULL, .writable = 0, .type = WT_LABEL }
#define WIDGET_NUMBER()                 .widget = { .opts = { },                .ref = NULL, .writable = 1, .type = WT_NUMBER }
#define WIDGET_SLIDER(MIN, MAX)         .widget = { .opts = { #MIN, #MAX },     .ref = NULL, .writable = 1, .type = WT_SLIDER }
#define WIDGET_SLIDER_UNITS(MIN,MAX,U)  .widget = { .opts = { #MIN, #MAX, U },  .ref = NULL, .writable = 1, .type = WT_SLIDER }
#define WIDGET_DIAL(MIN, MAX)           .widget = { .opts = { #MIN, #MAX },     .ref = NULL, .writable = 0, .type = WT_DIAL }
#define WIDGET_DIAL_UNITS(MIN, MAX,U)   .widget = { .opts = { #MIN, #MAX, U },  .ref = NULL, .writable = 0, .type = WT_DIAL }
#define WIDGET_SWITCH()                 .widget = { .opts = { },                .ref = NULL, .writable = 1, .type = WT_SWITCH }
#define WIDGET_LED(COLOR)               .widget = { .opts = { COLOR },          .ref = NULL, .writable = 0, .type = WT_LED }
#define WIDGET_BUTTON(T)                .widget = { .opts = { T },              .ref = NULL, .writable = 0, .type = WT_BUTTON }
#define WIDGET_BUTTON_CONFIRM(T,M)      .widget = { .opts = { T, M },           .ref = NULL, .writable = 0, .type = WT_BUTTON }

/**
 * The data definition provides the meta-information regarding the pointer binding
 */
struct DataDefinition {
  void * ptr;
  void * ud;
  char uid[8];
  uint8_t type : 4;
  uint8_t access : 2;
  uint32_t len : 26;
};

/**
 * The widget definition carries the information required to correctly
 * define a widget.
 */
struct WidgetDefinition {
  std::vector<const char*> opts;
  std::vector<const char*> *ref;
  char writable : 1;
  char type : 7;
};

/**
 * A configuration `value` is the leaf configurable parameter that
 * the user can read or define.
 */
struct ValueDefinition {
  const char *          name;
  DataDefinition        bind;
  WidgetDefinition      widget;
  const char *          help;

  /**
   * Reference to an external variable that will set to true if this value
   * was modified.
   */
  bool *                dirty;
};

/**
 * A configuration `section` has a title and one or more values
 */
struct SectionDefinition {
  const char * title;
  std::vector<ValueDefinition> values;
};

/**
 * A configuration `page` has a title and one or more sections
 */
struct PageDefinition {
  const char * title;
  std::vector<SectionDefinition> sections;
};

#endif
