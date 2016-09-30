#ifndef _LOG_MACROS_H
#define _LOG_MACROS_H

#include "log_functions.h"

typedef enum log_components
{
  COMPONENT_ALL = 0,               /* Used for changing logging for all components */
  COMPONENT_LOG,                   /* Keep this first, some code depends on it being the first component */
  COMPONENT_LOG_EMERG,             /* Component for logging emergency log messages - avoid infinite recursion */
  COMPONENT_MEMALLOC,
  COMPONENT_MEMLEAKS,
  COMPONENT_STDOUT,
  COMPONENT_COMM,
  COMPONENT_COUNT
} log_components_t;

int SetComponentLogFile(log_components_t component, char *name);
void SetComponentLogBuffer(log_components_t component, char *buffer);
void SetComponentLogLevel(log_components_t component, int level_to_set);
#define SetLogLevel(level_to_set) SetComponentLogLevel(COMPONENT_ALL, level_to_set)
int DisplayLogComponentLevel(log_components_t component, int level, char *format, ...)
__attribute__((format(printf, 3, 4))); /* 3=format 4=params */ ;
int DisplayErrorComponentLogLine(log_components_t component, int num_family, int num_error, int status, int ma_ligne);

enum log_type
{
  SYSLOG = 0,
  FILELOG,
  STDERRLOG,
  STDOUTLOG,
  TESTLOG,
  BUFFLOG
};

typedef struct log_component_info
{
  int   comp_value;
  char *comp_name;
  char *comp_str;
  int   comp_log_level;

  int   comp_log_type;
  char  comp_log_file[MAXPATHLEN];
  char *comp_buffer;
} log_component_info;

#define ReturnLevelComponent(component) CommLogComponents[component].comp_log_level 

log_component_info __attribute__ ((__unused__)) CommLogComponents[COMPONENT_COUNT];

#define LogAlways(component, format, args...) \
  do { \
    if (CommLogComponents[component].comp_log_type != TESTLOG || \
        CommLogComponents[component].comp_log_level <= NIV_FULL_DEBUG) \
      DisplayLogComponentLevel(component, NIV_NULL, "%s: " format, CommLogComponents[component].comp_str, ## args ); \
  } while (0)

#define LogTest(format, args...) \
  do { \
    DisplayLogComponentLevel(COMPONENT_ALL, NIV_NULL, format, ## args ); \
  } while (0)

#define LogMajor(component, format, args...) \
  do { \
    if (CommLogComponents[component].comp_log_level >= NIV_MAJOR) \
      DisplayLogComponentLevel(component, NIV_MAJ, "%s: MAJOR ERROR: " format, CommLogComponents[component].comp_str, ## args ); \
  } while (0)

#define LogCrit(component, format, args...) \
  do { \
    if (CommLogComponents[component].comp_log_level >= NIV_CRIT) \
      DisplayLogComponentLevel(component, NIV_CRIT, "%s: CRITICAL INFO: " format, CommLogComponents[component].comp_str, ## args ); \
   } while (0)

#define LogEvent(component, format, args...) \
  do { \
    if (CommLogComponents[component].comp_log_level >= NIV_EVENT) \
      DisplayLogComponentLevel(component, NIV_EVENT, "%s: EVENT: " format, CommLogComponents[component].comp_str, ## args ); \
  } while (0)

#define LogDebug(component, format, args...) \
  do { \
    if (CommLogComponents[component].comp_log_level >= NIV_DEBUG) \
      DisplayLogComponentLevel(component, NIV_DEBUG, "%s: DEBUG: " format, CommLogComponents[component].comp_str, ## args ); \
  } while (0)

#define LogFullDebug(component, format, args...) \
  do { \
    if (CommLogComponents[component].comp_log_level >= NIV_FULL_DEBUG) \
      DisplayLogComponentLevel(component, NIV_FULL_DEBUG, "%s: FULLDEBUG: " format, CommLogComponents[component].comp_str, ## args ); \
  } while (0)

#define LogError( component, a, b, c ) \
  do { \
    if (CommLogComponents[component].comp_log_level >= NIV_CRIT) \
      DisplayErrorComponentLogLine( component, a, b, c, __LINE__ ); \
  } while (0)

#define isFullDebug(component) \
  (CommLogComponents[component].comp_log_level >= NIV_FULL_DEBUG)

#endif
