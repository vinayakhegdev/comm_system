#include <stdlib.h>             /* for malloc */
#include <ctype.h>              /* for isdigit */
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <libgen.h>

#include "log_macros.h"

#define STR_LEN_TXT      2048
#define PATH_LEN         1024
#define MAX_STR_LEN      1024
#define MAX_NUM_FAMILY  50
#define UNUSED_SLOT -1

/* constants */
static int masque_log = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

/* Array of error families */

static family_t tab_family[MAX_NUM_FAMILY];

/* Global variables */

static char nom_programme[1024];
static char nom_host[256];
static int syslog_opened = 0 ;

/*
 * Variables specifiques aux threads.
 */

typedef struct ThreadLogContext_t
{

  char nom_fonction[STR_LEN];

} ThreadLogContext_t;

/* threads keys */
static pthread_key_t thread_key;
static pthread_once_t once_key = PTHREAD_ONCE_INIT;

#define LogChanges(format, args...) \
  do { \
    if (CommLogComponents[COMPONENT_LOG].comp_log_type != TESTLOG || \
        CommLogComponents[COMPONENT_LOG].comp_log_level == NIV_FULL_DEBUG) \
      DisplayLogComponentLevel(COMPONENT_LOG, NIV_NULL, "LOG: " format, ## args ); \
  } while (0)

#ifdef _DONT_HAVE_LOCALTIME_R

/* Localtime is not reentrant...
 * So we are obliged to have a mutex for calling it.
 * pffff....
 */
static pthread_mutex_t mutex_localtime = PTHREAD_MUTEX_INITIALIZER;

/* thread-safe and PORTABLE version of localtime */

static struct tm *Localtime_r(const time_t * p_time, struct tm *p_tm)
{
  struct tm *p_tmp_tm;

  if(!p_tm)
    {
      errno = EFAULT;
      return NULL;
    }

  pthread_mutex_lock(&mutex_localtime);

  p_tmp_tm = localtime(p_time);

  /* copy the result */
  (*p_tm) = (*p_tmp_tm);

  pthread_mutex_unlock(&mutex_localtime);

  return p_tm;
}
#else
# define Localtime_r localtime_r
#endif

void cleanup_keys(void *arg)  {
    ThreadLogContext_t *p_current_thread_vars = NULL;
    p_current_thread_vars = 
    (ThreadLogContext_t *) pthread_getspecific(thread_key);
    if(p_current_thread_vars) {
        free(p_current_thread_vars);
    }
}



/* Init of pthread_keys */
static void init_keys(void)
{
  if(pthread_key_create(&thread_key, cleanup_keys) == -1)
    LogCrit(COMPONENT_LOG, "init_keys - pthread_key_create returned %d", errno);
}                               /* init_keys */


const char *emergency = "* log emergency *";

/**
 * GetThreadContext :
 * manages pthread_keys.
 */
static ThreadLogContext_t *Log_GetThreadContext(int ok_errors)
{
  ThreadLogContext_t *p_current_thread_vars;

  /* first, we init the keys if this is the first time */
  if(pthread_once(&once_key, init_keys) != 0)
    {
      if (ok_errors)
        LogCrit(COMPONENT_LOG_EMERG, "Log_GetThreadFunction - pthread_once returned %d", errno);
      return NULL;
    }

  p_current_thread_vars = (ThreadLogContext_t *) pthread_getspecific(thread_key);

  /* we allocate the thread key if this is the first time */
  if(p_current_thread_vars == NULL)
    {
      /* allocates thread structure */
      p_current_thread_vars = (ThreadLogContext_t *) malloc(sizeof(ThreadLogContext_t));

      if(p_current_thread_vars == NULL)
        {
          if (ok_errors)
            LogCrit(COMPONENT_LOG_EMERG, "Log_GetThreadFunction - malloc returned %d", errno);
          return NULL;
        }

      /* inits thread structures */
      p_current_thread_vars->nom_fonction[0] = '\0';

      /* set the specific value */
      pthread_setspecific(thread_key, (void *)p_current_thread_vars);

      if (ok_errors)
        LogFullDebug(COMPONENT_LOG_EMERG, "malloc => %p", p_current_thread_vars);
    }

  return p_current_thread_vars;

}                               /* Log_GetThreadFunction */

static const char *Log_GetThreadFunction(int ok_errors)
{
  ThreadLogContext_t *context = Log_GetThreadContext(ok_errors);

  if (context == NULL)
    return emergency;
  else
    return context->nom_fonction;
}

void GetNameFunction(char *name, int len)
{
  const char *s = Log_GetThreadFunction(0);

  if (s != emergency && s != NULL)
    strncpy(name, s, len);
  else
    snprintf(name, len, "Thread %p", (caddr_t)pthread_self());
}

int ReturnLevelAscii(const char *LevelEnAscii)
{
  int i = 0;

  for(i = 0; i < NB_LOG_LEVEL; i++)
    if(!strcmp(tabLogLevel[i].str, LevelEnAscii))
      return tabLogLevel[i].value;

  return -1;
}                               /* ReturnLevelAscii */

char *ReturnLevelInt(int level)
{
  int i = 0;

  for(i = 0; i < NB_LOG_LEVEL; i++)
    if(tabLogLevel[i].value == level)
      return tabLogLevel[i].str;

  return NULL;
}                               /* ReturnLevelInt */

/*
 * Set le nom du programme en cours.
 */
void SetNamePgm(char *nom)
{

  strcpy(nom_programme, nom);
}                               /* SetNamePgm */

void SetNameHost(char *nom)
{
  strcpy(nom_host, nom);
}                               /* SetNameHost */

void SetNameFunction(char *nom)
{
  ThreadLogContext_t *context = Log_GetThreadContext(0);

  if (context != NULL)
    strcpy(context->nom_fonction, nom);
}                               /* SetNameFunction */

static void ArmeSignal(int signal, void (*action) ())
{
  struct sigaction act;     
  act.sa_flags = 0;
  act.sa_handler = action;
  sigemptyset(&act.sa_mask);

  if(sigaction(signal, &act, NULL) == -1)
    {
      LogError(COMPONENT_LOG, ERR_SYS, ERR_SIGACTION, errno);
      LogCrit(COMPONENT_LOG, "Impossible to arm signal %d", signal);
    }
}


void SetComponentLogLevel(log_components_t component, int level_to_set)
{
  if (component == COMPONENT_ALL)
    {
      SetLevelDebug(level_to_set);
      return;
    }

  if(level_to_set < NIV_NULL)
    level_to_set = NIV_NULL;

  if(level_to_set >= NB_LOG_LEVEL)
    level_to_set = NB_LOG_LEVEL - 1;

  if (CommLogComponents[component].comp_log_level != level_to_set)
    {
      LogChanges("Changing log level of %s from %s to %s",
                 CommLogComponents[component].comp_name,
                 ReturnLevelInt(CommLogComponents[component].comp_log_level),
                 ReturnLevelInt(level_to_set));
      CommLogComponents[component].comp_log_level = level_to_set;
    }
}

inline int ReturnLevelDebug()
{
  return CommLogComponents[COMPONENT_ALL].comp_log_level;
}                               /* ReturnLevelDebug */

void _SetLevelDebug(int level_to_set)
{
  int i;

  if(level_to_set < NIV_NULL)
    level_to_set = NIV_NULL;

  if(level_to_set >= NB_LOG_LEVEL)
    level_to_set = NB_LOG_LEVEL - 1;

  for (i = COMPONENT_ALL; i < COMPONENT_COUNT; i++)
      CommLogComponents[i].comp_log_level = level_to_set;
}                               /* SetLevelDebug */

void SetLevelDebug(int level_to_set)
{
  _SetLevelDebug(level_to_set);

  LogChanges("Setting log level for all components to %s",
             ReturnLevelInt(CommLogComponents[COMPONENT_ALL].comp_log_level));
}

static void IncrementeLevelDebug()
{
  _SetLevelDebug(ReturnLevelDebug() + 1);

  LogChanges("SIGUSR1 Increasing log level for all components to %s",
             ReturnLevelInt(CommLogComponents[COMPONENT_ALL].comp_log_level));
}                               /* IncrementeLevelDebug */

static void DecrementeLevelDebug()
{
  _SetLevelDebug(ReturnLevelDebug() - 1);

  LogChanges("SIGUSR2 Decreasing log level for all components to %s",
             ReturnLevelInt(CommLogComponents[COMPONENT_ALL].comp_log_level));
}                               /* DecrementeLevelDebug */

void InitLogging()
{
  int i;
  char *env_value;
  int newlevel, component, oldlevel;

  /* Initialisation du tableau des familys */
  tab_family[0].num_family = 0;
  tab_family[0].tab_err = (family_error_t *) tab_systeme_err;
  strcpy(tab_family[0].name_family, "Errors Systeme UNIX");

  for(i = 1; i < MAX_NUM_FAMILY; i++)
    tab_family[i].num_family = UNUSED_SLOT;

  ArmeSignal(SIGUSR1, IncrementeLevelDebug);
  ArmeSignal(SIGUSR2, DecrementeLevelDebug);

  for(component = COMPONENT_ALL; component < COMPONENT_COUNT; component++)
    {
      env_value = getenv(CommLogComponents[component].comp_name);
      if (env_value == NULL)
        continue;
      newlevel = ReturnLevelAscii(env_value);
      if (newlevel == -1) {
        LogMajor(COMPONENT_LOG, "Environment variable %s exists, but the value %s is not a valid log level.",
                 CommLogComponents[component].comp_name, env_value);
        continue;
      }
      oldlevel = CommLogComponents[component].comp_log_level;
      CommLogComponents[component].comp_log_level = newlevel;
      LogChanges("Using environment variable to switch log level for %s from %s to %s",
                 CommLogComponents[component].comp_name, ReturnLevelInt(oldlevel),
                 ReturnLevelInt(newlevel));
    }

}                               /* InitLevelDebug */


static void DisplayLogString_valist(char *buff_dest, log_components_t component, char *format, va_list arguments)
{
  char texte[STR_LEN_TXT];
  struct tm the_date;
  time_t tm;
  const char *function = Log_GetThreadFunction(component != COMPONENT_LOG_EMERG);

  tm = time(NULL);
  Localtime_r(&tm, &the_date);

  log_vsnprintf(texte, STR_LEN_TXT, format, arguments);

  snprintf(buff_dest, STR_LEN_TXT, "%.2d/%.2d/%.4d %.2d:%.2d:%.2d epoch=%ld : %s : %s-%d[%s] :%s\n",
           the_date.tm_mday, the_date.tm_mon + 1, 1900 + the_date.tm_year,
           the_date.tm_hour, the_date.tm_min, the_date.tm_sec, tm, nom_host,
           nom_programme, getpid(), function,
           texte);
}                               /* DisplayLogString_valist */

static int DisplayLogSyslog_valist(log_components_t component, int level, char * format, va_list arguments)
{
  char texte[STR_LEN_TXT];
  const char *function = Log_GetThreadFunction(component != COMPONENT_LOG_EMERG);

  if( !syslog_opened )
   {
     openlog("nfs-ganesha", LOG_PID, LOG_USER);
     syslog_opened = 1;
   }

  log_vsnprintf(texte, STR_LEN_TXT, format, arguments);

  syslog(tabLogLevel[level].syslog_level, "[%s] :%s", function, texte);
  return 1 ;
} /* DisplayLogSyslog_valist */

static int DisplayLogFd_valist(int fd, log_components_t component, char *format, va_list arguments)
{
  char tampon[STR_LEN_TXT];

  DisplayLogString_valist(tampon, component, format, arguments);
  return write(fd, tampon, strlen(tampon));
}                               /* DisplayLogFd_valist */

static int DisplayLogFlux_valist(FILE * flux, log_components_t component, char *format, va_list arguments)
{
  char tampon[STR_LEN_TXT];

  DisplayLogString_valist(tampon, component, format, arguments);

  fprintf(flux, "%s", tampon);
  return fflush(flux);
}                               /* DisplayLogFlux_valist */

static int DisplayTest_valist(log_components_t component, char *format, va_list arguments)
{
  char text[STR_LEN_TXT];

  log_vsnprintf(text, STR_LEN_TXT, format, arguments);

  fprintf(stdout, "%s\n", text);
  return fflush(stdout);
}

static int DisplayBuffer_valist(char *buffer, log_components_t component, char *format, va_list arguments)
{
  log_vsnprintf(buffer, STR_LEN_TXT, format, arguments);
}

static int DisplayLogPath_valist(char *path, log_components_t component, char *format, va_list arguments)
{
  char tampon[STR_LEN_TXT];
  int fd, my_status;

  DisplayLogString_valist(tampon, component, format, arguments);

  if(path[0] != '\0')
    {
#ifdef _LOCK_LOG
      if((fd = open(path, O_WRONLY | O_SYNC | O_APPEND | O_CREAT, masque_log)) != -1)
        {
          struct flock lock_file;

          lock_file.l_type = F_WRLCK;
          lock_file.l_whence = SEEK_SET;
          lock_file.l_start = 0;
          lock_file.l_len = 0;

          if(fcntl(fd, F_SETLKW, (char *)&lock_file) != -1)
            {
              write(fd, tampon, strlen(tampon));

              lock_file.l_type = F_UNLCK;

              fcntl(fd, F_SETLKW, (char *)&lock_file);

              close(fd);
              return SUCCES;
            }                   /* if fcntl */
          else
            {
              my_status = errno;
              close(fd);
            }
        }

#else
      if((fd = open(path, O_WRONLY | O_NONBLOCK | O_APPEND | O_CREAT, masque_log)) != -1)
        {
          write(fd, tampon, strlen(tampon));
          close(fd);
          return SUCCES;
        }
#endif
      else
        {
          my_status = errno;
        }
      fprintf(stderr, "Error %s : %s : status %d on file %s message was:\n%s\n",
              tab_systeme_err[ERR_FICHIER_LOG].label,
              tab_systeme_err[ERR_FICHIER_LOG].msg, my_status, path, tampon);

      return ERR_FICHIER_LOG;
    }
  return SUCCES;
}                               /* DisplayLogPath_valist */

int AddFamilyError(int num_family, char *name_family, family_error_t * tab_err)
{
  int i = 0;

  if((num_family < -1) || (num_family >= MAX_NUM_FAMILY))
    return -1;

  if(num_family == 0)
    return -1;

  for(i = 0; i < MAX_NUM_FAMILY; i++)
    if(tab_family[i].num_family == UNUSED_SLOT)
      break;

  if(i == MAX_NUM_FAMILY)
    return -1;

  tab_family[i].num_family = (num_family != -1) ? num_family : i;
  tab_family[i].tab_err = tab_err;
  strcpy(tab_family[i].name_family, name_family);

  return tab_family[i].num_family;
} 

char *ReturnNameFamilyError(int num_family)
{
  int i = 0;

  for(i = 0; i < MAX_NUM_FAMILY; i++)
    if(tab_family[i].num_family == num_family)
      {
        return tab_family[i].name_family;
      }

  return NULL;
}                               /* ReturnFamilyError */

static family_error_t *TrouveTabErr(int num_family)
{
  int i = 0;

  for(i = 0; i < MAX_NUM_FAMILY; i++)
    {
      if(tab_family[i].num_family == num_family)
        {
          return tab_family[i].tab_err;
        }
    }

  return NULL;
}  

static family_error_t TrouveErr(family_error_t * tab_err, int num)
{
  int i = 0;
  family_error_t returned_err;

  do
    {

      if(tab_err[i].numero == num || tab_err[i].numero == ERR_NULL)
        {
          returned_err = tab_err[i];
          break;
        }

      i += 1;
    }
  while(1);

  return returned_err;
}   

int MakeLogError(char *buffer, int num_family, int num_error, int status,
                  int ma_ligne)
{
  family_error_t *tab_err = NULL;
  family_error_t the_error;

  /* Find the family */
  if((tab_err = TrouveTabErr(num_family)) == NULL)
    return -1;

  /* find the error */
  the_error = TrouveErr(tab_err, num_error);

  if(status == 0)
    {
      return sprintf(buffer, "Error %s : %s : status %d : Line %d",
                     the_error.label, the_error.msg, status, ma_ligne);
    }
  else
    {
      char tempstr[1024];
      char *errstr;
      errstr = strerror_r(status, tempstr, 1024);

      return sprintf(buffer, "Error %s : %s : status %d : %s : Line %d",
                     the_error.label, the_error.msg, status, errstr, ma_ligne);
    }
}                               /* MakeLogError */

#define ONE_STEP  do { iterformat +=1 ; len += 1; } while(0)

#define NO_TYPE       0
#define INT_TYPE      1
#define LONG_TYPE     2
#define CHAR_TYPE     3
#define STRING_TYPE   4
#define FLOAT_TYPE    5
#define DOUBLE_TYPE   6
#define POINTEUR_TYPE 7

#define EXTENDED_TYPE 8

#define STATUS_SHORT       1
#define STATUS_LONG        2
#define CONTEXTE_SHORT     3
#define CONTEXTE_LONG      4
#define ERREUR_SHORT       5
#define ERREUR_LONG        6
#define ERRNUM_SHORT       7
#define ERRNUM_LONG        8
#define ERRCTX_SHORT       9
#define ERRCTX_LONG        10
#define CHANGE_ERR_FAMILY  11
#define CHANGE_CTX_FAMILY  12
#define ERRNO_SHORT        13
#define ERRNO_LONG         14

#define NO_LONG 0
#define SHORT_LG 1
#define LONG_LG 2
#define LONG_LONG_LG 3

#define MAX_STR_TOK LOG_MAX_STRLEN

int log_vsnprintf(char *out, size_t taille, char *format, va_list arguments)
{
  char *iterformat = NULL;
  char subformat[MAX_STR_TOK];
  char tmpout[MAX_STR_TOK];
  int type = NO_TYPE;
  int typelg = NO_LONG;
  int type_ext = 0;
  char *ptrsub = NULL;
  char *endofstr = NULL;
  int len = 0;
  int precision_in_valist = 0;
  int retval = 0;

  int err_family = ERR_POSIX;
  int ctx_family = ERR_SYS;
  int numero;
  char *label;
  char *msg;

  int tmpnumero;
  char *tmplabel;
  char *tmpmsg;
  family_error_t *tab_err;
  family_error_t the_error;

  out[0] = '\0';
  iterformat = format;
  ptrsub = subformat;

  do
    {
      type = NO_TYPE;
      typelg = NO_LONG;

      ptrsub = iterformat;
      endofstr = iterformat;
      len = 0;

      while(*iterformat != '\0' && *iterformat != '%')
        {
          ONE_STEP;
        }

      if(*iterformat == '\0')
        break;
      else
        endofstr = iterformat;

      ONE_STEP;

      while (*iterformat == '#' || *iterformat == '-' || *iterformat == '0' ||
             *iterformat == ' ' || *iterformat == '+' || *iterformat == 'I' ||
             *iterformat == '\'')
        {
          ONE_STEP;
        }

      while(isdigit(*iterformat) || *iterformat == '$')
        {
          ONE_STEP;
        }

      if(*iterformat == '*')
        {
          ONE_STEP;

          precision_in_valist += 1;

          while(isdigit(*iterformat) || *iterformat == '$')
            {
              ONE_STEP;
            }
        }

      if(*iterformat == '.')
        {
          ONE_STEP;

          if(*iterformat == '*')
            {
              ONE_STEP;

              precision_in_valist += 1;

              while(isdigit(*iterformat) || *iterformat == '$')
                {
                  ONE_STEP;
                }
            }
          else
            {
              if(isdigit(*iterformat))
                {

                  while(isdigit(*iterformat) || *iterformat == '$')
                    {
                      ONE_STEP;
                    }
                }               /*  if( isdigit( *iterformat ) */
              else
                {
                  ONE_STEP;
                }

            }                   /* if( *iterformat == '.' )  */
        }

      switch (*iterformat)
        {
        case 'h':
          if (iterformat[1] == 'h' || iterformat[1] == 'd' ||
              iterformat[1] == 'i' || iterformat[1] == 'u' ||
              iterformat[1] == 'x' || iterformat[1] == 'X' ||
              iterformat[1] == 'o')
            {
              typelg = SHORT_LG;
              ONE_STEP;
              if(*iterformat == 'h')
                {
                  ONE_STEP;
                  type = CHAR_TYPE;
                  typelg = NO_LONG;
                }
            }
          break;
        case 'L':
          typelg = LONG_LONG_LG;
          ONE_STEP;
          break;
        case 'q':
          /* long long */
          typelg = LONG_LONG_LG;
          ONE_STEP;
          break;
        case 'z':
        case 'Z':
        case 't':
        case 'j':
#          if __WORDSIZE == 64
          typelg = LONG_LG;
#          else
          typelg = LONG_LONG_LG;
#          endif
          ONE_STEP;
          break;
        case 'l':
          ONE_STEP;
          typelg = LONG_LG;
          if(*iterformat == 'l')
            {
              ONE_STEP;
              typelg = LONG_LONG_LG;
            }
          break;
        default:
          break;
        }

      type = NO_TYPE;

      switch (*iterformat)
        {
        case 'i':
        case 'd':
        case 'u':
        case 'o':
        case 'x':
        case 'X':

          type = INT_TYPE;
          ONE_STEP;
          break;
        case 'e':
        case 'E':
        case 'f':
        case 'F':
        case 'g':
        case 'G':
        case 'a':
        case 'A':
          type = DOUBLE_TYPE;
          ONE_STEP;
          break;
        case 'c':
        case 'C':
          type = CHAR_TYPE;
          ONE_STEP;
          break;
        case 's':
        case 'S':
          type = STRING_TYPE;
          ONE_STEP;
          break;
        case 'p':
          type = POINTEUR_TYPE;
          ONE_STEP;
          break;
        case '%':
          type = NO_TYPE;
          ONE_STEP;
          break;
        case 'b':
          type = EXTENDED_TYPE;
          type_ext = STATUS_SHORT;
          ONE_STEP;
          break;
        case 'B':
          type = EXTENDED_TYPE;
          type_ext = STATUS_LONG;
          ONE_STEP;
          break;
        case 'h':
          type = EXTENDED_TYPE;
          type_ext = CONTEXTE_SHORT;
          ONE_STEP;
          break;
        case 'H':
          type = EXTENDED_TYPE;
          type_ext = CONTEXTE_LONG;
          ONE_STEP;
          break;
        case 'y':
          type = EXTENDED_TYPE;
          type_ext = ERREUR_SHORT;
          ONE_STEP;
          break;
        case 'Y':
          type = EXTENDED_TYPE;
          type_ext = ERREUR_LONG;
          ONE_STEP;
          break;
        case 'r':
          type = EXTENDED_TYPE;
          type_ext = ERRNUM_SHORT;
          ONE_STEP;
          break;
        case 'R':
          type = EXTENDED_TYPE;
          type_ext = ERRNUM_LONG;
          ONE_STEP;
          break;
        case 'v':
          type = EXTENDED_TYPE;
          type_ext = ERRCTX_SHORT;
          ONE_STEP;
          break;
        case 'V':
          type = EXTENDED_TYPE;
          type_ext = ERRCTX_LONG;
          ONE_STEP;
          break;
        case 'J':
          type = EXTENDED_TYPE;
          type_ext = CHANGE_ERR_FAMILY;
          ONE_STEP;
          break;
        case 'K':
          type = EXTENDED_TYPE;
          type_ext = CHANGE_CTX_FAMILY;
          ONE_STEP;
          break;
        case 'w':
          type = EXTENDED_TYPE;
          type_ext = ERRNO_SHORT;
          ONE_STEP;
          break;
        case 'W':
          type = EXTENDED_TYPE;
          type_ext = ERRNO_LONG;
          ONE_STEP;
          break;
        case 'm':
          type = NO_TYPE;
          ONE_STEP;
          break;
        default:
          type = NO_TYPE;
          ONE_STEP;
          break;
        }

      strncpy(subformat, ptrsub, len);
      subformat[len] = '\0';

      if(type != EXTENDED_TYPE)
        {
          va_list tmp_args;

          /* vsnprintf may modify arguments */
          va_copy(tmp_args, arguments);

          retval += vsnprintf(tmpout, taille, subformat, tmp_args);

          /* free this temporary copy */
          va_end(tmp_args);

          while(precision_in_valist != 0)
            {
              va_arg(arguments, int);
              precision_in_valist -= 1;
            }

          switch (typelg)
            {
            case SHORT_LG:
              va_arg(arguments, int);
              break;

            case NO_LONG:
              switch (type)
                {
                case INT_TYPE:
                  va_arg(arguments, int);
                  break;

                case LONG_TYPE:
                  va_arg(arguments, long);
                  break;

                case FLOAT_TYPE:
                  /* float is promoted to double when passed through "..." */
                  va_arg(arguments, double);
                  break;

                case DOUBLE_TYPE:
                  va_arg(arguments, double);
                  break;

                case CHAR_TYPE:
                  /* char is promoted to int when passed through "..." */
                  va_arg(arguments, int);
                  break;

                case POINTEUR_TYPE:
                  va_arg(arguments, void *);
                  break;

                case STRING_TYPE:
                  va_arg(arguments, char *);
                  break;
                }

              break;

            case LONG_LG:
              switch (type)
                {
                case INT_TYPE:
                  va_arg(arguments, long int);
                  break;

                case LONG_TYPE:
                  va_arg(arguments, long long int);
                  break;

                case FLOAT_TYPE:
                  va_arg(arguments, double);
                  break;

                case DOUBLE_TYPE:
                  va_arg(arguments, long double);
                  break;
                }
              break;

            case LONG_LONG_LG:
              switch (type)
                {
                case INT_TYPE:
                case LONG_TYPE:
                  va_arg(arguments, long long int);
                  break;

                case FLOAT_TYPE:
                case DOUBLE_TYPE:
                  va_arg(arguments, long double);
                  break;
                }

              break;
            }
        }                       /* if( type != EXTENDED_TYPE ) */
      else
        {
#define VA_ARG_ERREUR_T numero = va_arg( arguments, long ) ; label  = va_arg( arguments, char * ) ; msg    = va_arg( arguments, char *)

          if(strlen(subformat) > 2)
            strncat(out, subformat, strlen(subformat) - 2);

          switch (type_ext)
            {
            case STATUS_SHORT:
            case CONTEXTE_SHORT:
              VA_ARG_ERREUR_T;
              snprintf(tmpout, MAX_STR_TOK, "%s(%d)", label, numero);
              break;

            case STATUS_LONG:
            case CONTEXTE_LONG:
              VA_ARG_ERREUR_T;
              snprintf(tmpout, MAX_STR_TOK, "%s(%d) : '%s'", label, numero, msg);
              break;

            case ERREUR_SHORT:
              VA_ARG_ERREUR_T;
              tmpnumero = numero;
              tmplabel = label;
              tmpmsg = msg;
              VA_ARG_ERREUR_T;
              snprintf(tmpout, MAX_STR_TOK, "%s %s(%d)", tmplabel, label, numero);
              break;

            case ERREUR_LONG:
              VA_ARG_ERREUR_T;
              tmpnumero = numero;
              tmplabel = label;
              tmpmsg = msg;
              VA_ARG_ERREUR_T;
              snprintf(tmpout, MAX_STR_TOK, "%s(%d) : '%s' -> %s(%d) : '%s'",
                       tmplabel, tmpnumero, tmpmsg, label, numero, msg);
              break;

            case CHANGE_ERR_FAMILY:
              err_family = va_arg(arguments, int);
              tmpout[0] = '\0'; 
              break;

            case CHANGE_CTX_FAMILY:
              ctx_family = va_arg(arguments, int);
              tmpout[0] = '\0';
              break;

            case ERRNUM_SHORT:
              if((tab_err = TrouveTabErr(err_family)) == NULL)
                {
                  snprintf(tmpout, MAX_STR_TOK, "?");
                  break;
                }
              numero = va_arg(arguments, int);
              the_error = TrouveErr(tab_err, numero);
              snprintf(tmpout, MAX_STR_TOK, "%s(%d)", the_error.label, the_error.numero);
              break;

            case ERRNUM_LONG:
              if((tab_err = TrouveTabErr(err_family)) == NULL)
                {
                  snprintf(tmpout, MAX_STR_TOK, "?");
                  break;
                }
              numero = va_arg(arguments, int);
              the_error = TrouveErr(tab_err, numero);
              snprintf(tmpout, MAX_STR_TOK, "%s(%d) : '%s'", the_error.label,
                       the_error.numero, the_error.msg);
              break;

            case ERRNO_SHORT:
              if((tab_err = TrouveTabErr(ERR_POSIX)) == NULL)
                {
                  snprintf(tmpout, MAX_STR_TOK, "?");
                  break;
                }
              numero = va_arg(arguments, int);
              the_error = TrouveErr(tab_err, numero);
              snprintf(tmpout, MAX_STR_TOK, "%s(%d)", the_error.label, the_error.numero);
              break;

            case ERRNO_LONG:
              if((tab_err = TrouveTabErr(ERR_POSIX)) == NULL)
                {
                  snprintf(tmpout, MAX_STR_TOK, "?");
                  break;
                }
              {
                char tempstr[1024];
                char *errstr;

                numero = va_arg(arguments, int);
                errstr = strerror_r(numero, tempstr, 1024);
                the_error = TrouveErr(tab_err, numero);
                snprintf(tmpout, MAX_STR_TOK, "%s(%d) : '%s'", the_error.label,
                         the_error.numero, errstr);
              }
              break;

            case ERRCTX_SHORT:
              if((tab_err = TrouveTabErr(ctx_family)) == NULL)
                {
                  snprintf(tmpout, MAX_STR_TOK, "?");
                  break;
                }
              numero = va_arg(arguments, int);
              the_error = TrouveErr(tab_err, numero);
              snprintf(tmpout, MAX_STR_TOK, "%s(%d)", the_error.label, the_error.numero);
              break;

            case ERRCTX_LONG:
              if((tab_err = TrouveTabErr(ctx_family)) == NULL)
                {
                  snprintf(tmpout, MAX_STR_TOK, "?");
                  break;
                }
              numero = va_arg(arguments, int);
              the_error = TrouveErr(tab_err, numero);
              snprintf(tmpout, MAX_STR_TOK, "%s(%d) : '%s'", the_error.label, numero,
                       the_error.msg);
              break;

            }                   /* switch */

        }                       /* else */

      strncat(out, tmpout, taille);
    }
  while(*iterformat != '\0');

  if(*endofstr == '%')
    ptrsub = iterformat;      
  strncat(out, ptrsub, taille);

  return retval;
}                               /* mon_vsprintf */

int log_snprintf(char *out, size_t n, char *format, ...)
{
  va_list arguments;
  int rc;

  va_start(arguments, format);
  rc = log_vsnprintf(out, n, format, arguments);
  va_end(arguments);

  return rc;
}

int log_fprintf(FILE * file, char *format, ...)
{
  va_list arguments;
  char tmpstr[LOG_MAX_STRLEN];
  int rc;

  va_start(arguments, format);
  memset(tmpstr, 0, LOG_MAX_STRLEN);
  rc = log_vsnprintf(tmpstr, LOG_MAX_STRLEN, format, arguments);
  va_end(arguments);
  fputs(tmpstr, file);
  return rc;
}

log_component_info __attribute__ ((__unused__)) CommLogComponents[COMPONENT_COUNT] =
{
  { COMPONENT_ALL,               "COMPONENT_ALL", "",
    NIV_EVENT,
  },
  { COMPONENT_LOG,               "COMPONENT_LOG", "LOG",
    NIV_EVENT,
    SYSLOG,
    "SYSLOG"
  },
  { COMPONENT_LOG_EMERG,         "COMPONENT_LOG_EMERG", "LOG",
    NIV_EVENT,
    SYSLOG,
    "SYSLOG"
  },
  { COMPONENT_MEMALLOC,          "COMPONENT_MEMALLOC", "MEM ALLOC",
    NIV_NULL,
    SYSLOG,
    "SYSLOG"
  },
  { COMPONENT_MEMLEAKS,          "COMPONENT_MEMLEAKS", "MEM LEAKS",
    NIV_NULL,
    SYSLOG,
    "SYSLOG"
  },
  { COMPONENT_STDOUT,            "COMPONENT_STDOUT", "STDOUT",
    NIV_EVENT,
    SYSLOG,
    "SYSLOG"
  },
  { COMPONENT_COMM,             "COMPONENT_COMM", "IPC_COMM",
      NIV_EVENT,
      SYSLOG,
      "SYSLOG"
  },
};

int DisplayLogComponentLevel(log_components_t component, int level, char *format, ...)
{
  va_list arguments;
  int rc;

  va_start(arguments, format);

  switch(CommLogComponents[component].comp_log_type)
    {
    case SYSLOG:
      rc = DisplayLogSyslog_valist(component, level, format, arguments);
      break;
    case FILELOG:
      rc = DisplayLogPath_valist(CommLogComponents[component].comp_log_file, component, format, arguments);
      break;
    case STDERRLOG:
      rc = DisplayLogFlux_valist(stderr, component, format, arguments);
      break;
    case STDOUTLOG:
      rc = DisplayLogFlux_valist(stdout, component, format, arguments);
      break;
    case TESTLOG:
      rc = DisplayTest_valist(component, format, arguments);
      break;
    case BUFFLOG:
      rc = DisplayBuffer_valist(CommLogComponents[component].comp_buffer, component, format, arguments);
    default:
      rc = ERR_FAILURE;
    }

  va_end(arguments);
  return rc;
}

int DisplayErrorComponentLogLine(log_components_t component, int num_family, int num_error, int status, int ma_ligne)
{
  char buffer[STR_LEN_TXT];

  if(MakeLogError(buffer, num_family, num_error, status, ma_ligne) == -1)
    return -1;
  return DisplayLogComponentLevel(component, NIV_CRIT, "%s: %s", CommLogComponents[component].comp_str, buffer);
}                               /* DisplayErrorLogLine */

static int isValidLogPath(char *pathname)
{
  char tempname[MAXPATHLEN];

  char *directory_name;
  struct stat *buf;
  int rc;

  strncpy(tempname, pathname, MAXPATHLEN);

  directory_name = dirname(tempname);
  if (directory_name == NULL)
      return 0;

  rc = access(directory_name, W_OK);
  switch(rc)
    {
    case 0:
      break; /* success !! */
    case EACCES:
      LogCrit(COMPONENT_LOG, "Either access is denied to the file or denied to one of the directories in %s",
              directory_name);
      break;
    case ELOOP:
      LogCrit(COMPONENT_LOG, "Too many symbolic links were encountered in resolving %s", directory_name);
      break;
    case ENAMETOOLONG:
      LogCrit(COMPONENT_LOG, "%s is too long of a pathname.", directory_name);
      break;
    case ENOENT:
      LogCrit(COMPONENT_LOG, "A component of %s does not exist.", directory_name);
      break;
    case ENOTDIR:
      LogCrit(COMPONENT_LOG, "%s is not a directory.", directory_name);
      break;
    case EROFS:
      LogCrit(COMPONENT_LOG, "Write permission was requested for a file on a read-only file system.");
      break;
    case EFAULT:
      LogCrit(COMPONENT_LOG, "%s points outside your accessible address space.", directory_name);
      break;
    }

  return 1;
}

/*
 * Sets the default logging method (whether to a specific filepath or syslog.
 * During initialization this is used and separate layer logging defaults to
 * this destination.
 */
void SetDefaultLogging(char *name)
{
  int component;

  SetComponentLogFile(COMPONENT_LOG, name);

  LogChanges("Setting log destination for ALL components to %s", name);
  for(component = COMPONENT_ALL; component < COMPONENT_COUNT; component++)
    {
      if (component == COMPONENT_STDOUT)
        continue;
      SetComponentLogFile(component, name);
    }
}                               /* SetDefaultLogging */

int SetComponentLogFile(log_components_t component, char *name)
{
  int newtype, changed;

  if (strcmp(name, "SYSLOG") == 0)
    newtype = SYSLOG;
  else if (strcmp(name, "STDERR") == 0)
    newtype = STDERRLOG;
  else if (strcmp(name, "STDOUT") == 0)
    newtype = STDOUTLOG;
  else if (strcmp(name, "TEST") == 0)
    newtype = TESTLOG;
  else
    newtype = FILELOG;

  if (newtype == FILELOG)
    {
      if (!isValidLogPath(name))
        {
          LogMajor(COMPONENT_LOG, "Could not set default logging to %s", name);
          errno = EINVAL;
          return -1;
        }
      }

  changed = newtype != CommLogComponents[component].comp_log_type ||
            newtype == FILELOG && strcmp(name, CommLogComponents[component].comp_log_file) != 0;

  if (component != COMPONENT_LOG && changed)
    LogChanges("Changing log destination for %s from %s to %s",
               CommLogComponents[component].comp_name,
               CommLogComponents[component].comp_log_file,
               name);

  CommLogComponents[component].comp_log_type = newtype;
  strncpy(CommLogComponents[component].comp_log_file, name, MAXPATHLEN);

  if (component == COMPONENT_LOG && changed)
    LogChanges("Changing log destination for %s from %s to %s",
               CommLogComponents[component].comp_name,
               CommLogComponents[component].comp_log_file,
               name);

  return 0;
}                               /* SetComponentLogFile */

void SetComponentLogBuffer(log_components_t component, char *buffer)
{
  CommLogComponents[component].comp_log_type = BUFFLOG;
  CommLogComponents[component].comp_buffer   = buffer;
}


