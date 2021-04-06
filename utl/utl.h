/*
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

/* [[[
# Variadic functions

Say you want to define a variadic function with the following prototype:

    myfunc(int a [, char b [, void *c]])

In other words, you want `b` and `c` to be optional.

Simply, define your function with another name (say `my_func()`) and specify
how it should be called when invoked with 1, 2 or 3 paramenters as shown 
in the example below.

Example:

    #include "utl.h"

    int my_func(int a, char b, void *c);
    
    #define myfunc(...)     vrg(myfunc, __VA_ARGS__)
    #define myfunc1(a)      my_func(a,'\0',NULL)
    #define myfunc2(a,b)    my_func(a,b,NULL)
    #define myfunc3(a,b,c)  my_func(a,b,c)

**
]]] */

#ifndef VRG_VERSION
#define VRG_VERSION 0x0001000C

#define vrg_cnt(vrg1,vrg2,vrg3,vrg4,vrg5,vrg6,vrg7,vrg8,vrgN, ...) vrgN
#define vrg_argn(...)  vrg_cnt(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define vrg_cat0(x,y)  x ## y
#define vrg_cat(x,y)   vrg_cat0(x,y)

#define vrg(vrg_f,...) vrg_cat(vrg_f, vrg_argn(__VA_ARGS__))(__VA_ARGS__)

#endif/* 
**  (C) 2020 by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
**
**  DEBUG, TESTING (and LOGGING) MACROS.
**  ====================================
**
**  Contents
==  --------
**    * Introduction
**    * Debugging levels
**    * Writing messages
**    * Unit tests (and BDD)
**    * Debug Blocks
**    * Timing
**    * Tracking and watching
**    * Trace memory allocation 
*/

/*
**  Debugging Groups
**  ================
**
**   DBGx(...)  // __VA_ARGS__ // Description of the group (disabled)
**
**   DBGx(...)     __VA_ARGS__ // Description of the group (enabled) 
**
**   DBG_(...) is an always enabled group
**   DBG0(...) is an always disabled group
** 
*/

#ifndef DBG_VERSION
#define DBG_VERSION     0x0103000B
#define DBG_VERSION_STR "dbg 1.3.0-beta"

#ifdef DEBUG
extern volatile int dbg;      // dbg will always be 0. Used to suppress warnings in some macros 
#endif 

#ifdef DBG_FLOCK
  #ifdef __MINGW32__ 
    #define FLOCKFILE   _lock_file
    #define FUNLOCKFILE _unlock_file
  #else
    #define FLOCKFILE      flockfile
    #define FUNLOCKFILE    funlockfile
  #endif
#else
  #define FLOCKFILE(x)    (dbg=0)
  #define FUNLOCKFILE(x)  (dbg=0)
#endif

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

/* Debugging levels
** ================
**
** The functions behaviours depend on the `DEBUG` macro
**
** Reference
** ---------
**
**   DEBUG           --> If undefined, or defined as DBG_NONE removes all
**                       the debugging functions.
**                       If defined sets the level of debugging and enables
**                       the debuggin functions:
**
**                              level        enabled functions
**                            ---------  --------------------------
**                            DBG_ERROR  dbgerr() dbgmsg() dbgprt() dbgmst()
**                            DBG_WARN   as above plus dbgwrn()
**                            DBG_INFO   as above plus dbginf()
**                            DBG_TEST   all the dbg functions.
**
**                       Note NDEBUG has higher priority than DEBUG, if
**                       NDEBUG is defined, DEBUG will be undefined.
**
**  char dbglvl(char *lvl [, char* tms])
**                      --> Sets the *running level* for debugging. The `lvl` argument can be
**                          one of "TEST", "test", "INFO", "WARN", "ERROR" or "NONE".
**                          Returns the firt characted of the currently set level ('T','t','I','W','N')
**                          The level "test" is the same as "TEST" but with BDD functions disabled.
**                          You can enable/disable the timestamp with the tms argument:
**                            dbglvl("TEST","NOTIMESTAMP")  <-- timestamp disabled (default)
**                            dbglvl("TEST","TIMESTAMP")    <-- timestamp enabled (default)
** 
**  You can disable timestamp permanently defining DBG_NOTIMESTAMP before including the dbg.h header.
**                         
*/

#define DBG_NONE  -1
#define DBG_ERROR  0
#define DBG_WARN   1
#define DBG_INFO   2
#define DBG_TEST   3
#define DBG_BDD    4

// Defining NDEBUG or DBG_NONE disables everything
#if defined(DEBUG) && (defined(NDEBUG) || (DEBUG == DBG_NONE))
  #undef DEBUG
#endif

// The highest level of debugging is actually DBG_BDD
// To eliminate BDD messages from logs, set the level
// to "test" (lowercase!) with dbglvl("test");
#if defined(DEBUG) && (DEBUG == DBG_TEST)
  #undef DEBUG
  #define DEBUG DBG_BDD
#endif

// Macros to help defining variable arguments functions
#define dbg_exp(...)   __VA_ARGS__
#define dbg_0(x,...)     (x)
#define dbg_1(y,x,...)   (x)

// Used to keep track of the number of testcases within a dbgtst() scope
typedef struct dbg_tst_s {
  uint16_t count;  // Number of tests
  uint16_t fail;   // Number of failed tests
} dbg_tst_t;

#ifdef DEBUG
    extern    dbg_tst_t dbg_tst;  // The global dbg_tst is to ensure dbgchk() can work outside of
                                  // a dbgtst() scope. It is not meant to be used in any way.
    extern volatile int dbg;      // dbg will always be 0. Used to suppress warnings in some macros 
    extern          int dbg_lvl;  // Initialize to DBG_INFO
    extern          int dbg_tmsp; // Initialize to 1 - Print timestamp
    extern        char *dbg_lvls; // "NEWItT" NONE, ERROR, WARNING, INFO, TEST, TEST_BDD
#endif

#ifdef DEBUG
  char dbglvl_(char *lvl,char *tms);
  #define dbglvl(...) dbglvl_(dbg_exp(dbg_0(__VA_ARGS__,NULL)),dbg_exp(dbg_1(__VA_ARGS__,NULL,NULL)))
#else
  #define dbglvl(...) (DBG_NONE)
#endif


/* Timing & Timestamps
** ===================
**
** This takes a very crude measurement of the execution time of a block of code.
** You can use for profiling purpose or to check that the code has the expected
** performance even after a change.  
**
** If DEBUG is undefined or lower than DBG_TEST, code in the dbgclk scope is
** executed but time measurement is not taken.
**
** The messages are formatted as explained in the "Writing messages" section.
** The first one is of type `LPS[:` and the last one, of type `LPS]:`, will
** report the elapsed time.
**
** You can pair the two messages using the filename:linenumber at the end
** of the line.
**
**   2020-09-19 13:44:30.174397 LPS[:   myfile.c:17                    <--,
**    .... other messages produced by the code in the block ...           | pair
**   2020-09-19 13:44:30.321648 LPS]: 00s 147.250800ms   myfile.c:17   <--'
**
** Reference
** ---------
**
**   dbgclk {...} --> Measure the time needed to execute the block.
**   dbgnow()     --> Print a timestamp in the log
**
**  _dbgclk {...} --> Execute the block but don't measure time.
**  _dbgnow()     --> Do nothing.
**
*/

#define _dbgclk
#define _dbgnow

#define DBG_TIME   1
#define DBG_NOTIME 0

#if defined(DEBUG) && (DEBUG >= DBG_TEST)

  typedef struct {
    struct timespec clk_start;
    struct timespec clk_end;
    long int        elapsed; 
    long int        nelapsed; 
  } dbgclk_t;


  #define dbg_prtclk(s)   (FLOCKFILE(stderr), dbg_time(), fputs("\xE" s,stderr),dbg = (dbg_tmsp?0:dbg_prttime()),     \
                          fprintf(stderr,"\x9%s:%d\xF\n",__FILE__,__LINE__), \
                          fflush(stderr), FUNLOCKFILE(stderr), dbg)
 
  #define dbgnow() dbg_prtclk("NOW=: ")

  #define dbgclk \
      for ( dbgclk_t dbg_ = {.elapsed = -1} \
            ; \
            (dbg_.elapsed < 0) \
         && ((dbg_lvl >= DBG_TEST) ? (dbg_prtclk("CLK[: "),clock_gettime(CLOCK_REALTIME,&dbg_.clk_start),1) \
                                   : 1) \
            ; \
            clock_gettime(CLOCK_REALTIME,&dbg_.clk_end), \
            dbg_.elapsed = ((dbg_lvl >= DBG_TEST) \
                             ? ( dbg_.elapsed = (dbg_.clk_end.tv_sec - dbg_.clk_start.tv_sec), \
                                 dbg_.nelapsed = (dbg_.clk_end.tv_nsec - dbg_.clk_start.tv_nsec),  \
  	                             (dbg_.nelapsed < 0)? (dbg_.elapsed--, dbg_.nelapsed += 1000000000) : 0, \
                                  dbgmsg("CLK]: %02lds %010.6fms", dbg_.elapsed,(double)dbg_.nelapsed/1000000.0))\
                             : 0 )\
      )
#else
  #define dbgclk   _dbgclk
  #define dbgnow() 
#endif


/* Writing messages
** ================
**
** To ease the extraction of information (e.g. via grep), the following 
** functions will print a single line with the following structure.
** 
**  2020-09-19 12:32:43.229469 \xEINFO: Informative text\x9myfile.c:120\xF\n
**  \_________________________/   \___/ \______..._____/   \__..._/ \_/\___/
**   \_ timestamp (optional)      /                        /        /   \
**                 message type__/             filename __/  line _/     \_ EOL
**                                                          number
**  
**  The TAB character (0x09) makes easier to identify the file name; just
**  start from the end of the line and move backward.
**
**  Note that End of line is '0x0F 0x0A' (or 0x0F 0x0D 0X0A). This is done to
**  avoid confusion if your text contains any LF.
**
**  Logs can also contain messages produced by dbgchk(). Check it down
**
**  Reference
**  ---------
**  
**  Note that the first argument **must** be a literal string (i.e: "xxx").
**
**   dbgmsg(char *, ...)  --> Prints a message on stderr (works as printf(...)).
**                            If DEBUG is not defined, do nothing.
**                            RETURNS 0 
** 
**   dbgprt(char *, ...)  --> Prints a message on stderr (works as printf(...)) omitting
**                            filename and line. If DEBUG is not defined, do nothing.
** 
**   dbgerr(char *, ...)  --> Prints an "FAIL:" message (if level >= DBG_ERROR)..
**   dbgwrn(char *, ...)  --> Prints a  "WARN:" message (if level >= DBG_WARN).
**   dbginf(char *, ...)  --> Prints an "INFO:" message (if level >= DBG_INFO).
**   dbgtrc(char *, ...)  --> Prints an "TRCE:" message (if level >= DBG_TEST).
**
**  _dbgmsg(char *, ...)  --> Do nothing. Used to disable the debug message.
**  _dbgprt(char *, ...)  --> Do nothing. Used to disable the debug message.
**  _dbgtrc(char *, ...)  --> Do nothing. Used to disable the debug message.
**  _dbginf(char *, ...)  --> Do nothing. Used to disable the debug message.
**  _dbgwrn(char *, ...)  --> Do nothing. Used to disable the debug message.
**  _dbgerr(char *, ...)  --> Do nothing. Used to disable the debug message.
**
*/

#define _dbgmsg(...)
#define _dbgprt(...)
#define _dbgtrc(...)
#define _dbginf(...)
#define _dbgwrn(...)
#define _dbgerr(...)
#define _dbgdmpstart()
#define _dbgdmpstop()

#ifdef DEBUG
  int dbg_tms(int);  
  int dbg_prttime(void);

  #ifdef DBG_NOTIMESTAMP
    #define dbg_time() (dbg=0)
  #else
    #define dbg_time() (dbg = (dbg_tmsp && dbg_prttime()))
  #endif

  #define dbgtms(x) (dbg_tmsp=(x))

  #define dbgprt(...)   (FLOCKFILE(stderr), dbg_time(), fprintf(stderr,"\xE" __VA_ARGS__),     \
                          fputs("\xF\n",stderr), \
                          fflush(stderr), FUNLOCKFILE(stderr), dbg)
  
  #define dbgmsg(...)   (FLOCKFILE(stderr), dbg_time(), fprintf(stderr,"\xE" __VA_ARGS__),     \
                          fprintf(stderr,"\x9%s:%d\xF\n",__FILE__,__LINE__), \
                          fflush(stderr), FUNLOCKFILE(stderr), dbg)
  
  #define dbgerr(...)    ((dbg_lvl >= DBG_ERROR) ? (dbg_tst.count++, dbg_tst.fail++, dbgmsg("FAIL: " __VA_ARGS__)):0)
  
  #if DEBUG < DBG_WARN
    #define dbgwrn      _dbgwrn
  #else
    #define dbgwrn(...)  ((dbg_lvl >= DBG_WARN) ? dbgmsg("WARN: " __VA_ARGS__):0)
  #endif
  
  #if DEBUG < DBG_INFO
    #define dbginf      _dbginf
  #else
    #define dbginf(...)  ((dbg_lvl >= DBG_INFO) ? dbgmsg("INFO: " __VA_ARGS__):0)
  #endif

  int dbg_dmp(char *s, char *file, int line);

  #if DEBUG < DBG_TEST
    #define dbgtrc      _dbgtrc
    #define dbgdmp      _dbgdmp
    #define dbgdmpstart()
    #define dbgdmpstop()
  #else
    #define dbgtrc(...)   ((dbg_lvl >= DBG_TEST) ? dbgmsg("TRCE: " __VA_ARGS__):0)
    #define dbgdmp(s)     dbg_dmp(s,__FILE__,__LINE__)
    #define dbgdmpstart() dbgdmp(dbg_lvls)
    #define dbgdmpstop()  dbgdmp(NULL)
  #endif

#else
  #define dbgmsg _dbgmsg 
  #define dbgprt _dbgprt
  #define dbginf _dbginf
  #define dbgwrn _dbgwrn
  #define dbgerr _dbgerr
  #define dbgtrc _dbgtrc
  #define dbgdmpstart() _dbgdmpstart()
  #define dbgdmpstop()  _dbgdmpstop()

  #define dbgtms(x) 
#endif

/* Unit tests (and BDD)
** ====================
**
** These functions are used to write unit tests. Check the tst directory to 
** see many examples on how to use them.
**
** If DEBUG is undefined or lower than DBG_TEST, each function dbgxxx() behaves
** as its counterpart _dbgxxx().
**
** Note that the formatting string **must** be a literal (i.e.: "xxx").
**
** Reference
** ---------
**
**   dbgtst(char *)            --> Starts a test scenario.
**
**   dbgchk(test, char *, ...) --> Perform the test and set errno (0: OK, 1: KO). If test fails
**                                 prints a message on stderr (works as printf(...)).
**   
**   dbggvn(char *) {...}      --> Print the GIVEN clause (BDD)
**   dbgwhn(char *) {...}      --> Print the WHEN  clause (BDD)
**   dbgthn(char *) {...}      --> Print the THEN  clause (BDD)
**
**   dbgmst(test, char *, ...) --> Works as assert() but prints a "FAIL:" message on stderr.
**
**  _dbgtst(char *)            --> Do nothing. Used to disable the debug message.
**  _dbgchk(test, char *, ...) --> Do nothing. Used to disable the debug message.
**  _dbggvn(char *) {...}      --> Do not print the GIVEN clause (BDD)
**  _dbgwhn(char *) {...}      --> Do not print the WHEN  clause (BDD)
**  _dbgthn(char *) {...}      --> Do not print the THEN  clause (BDD)
**  _dbgmst(e,...)             --> Equivalent to assert()
**
*/

#define _dbgchk(...)
#define _dbgmst(e,...) assert(e)
#define _dbgtst(...)   if (1) ; else

#define _dbggvn(...)
#define _dbgwhn(...)
#define _dbgthn(...)

#if defined(DEBUG) && (DEBUG >= DBG_TEST)
  #define dbgtst(desc_) \
      for ( dbg_tst_t dbg_tst = {0,0} \
            ; \
            (dbg_tst.fail <= dbg_tst.count) \
         && (dbg_lvl >= DBG_TEST) && !dbgmsg("TST[: %s",desc_) \
            ; \
            dbgmsg("TST]: FAILED %d/%d - %s", dbg_tst.fail,dbg_tst.count, desc_), \
            dbg_tst.fail = dbg_tst.count + 1 \
          )
                             
  #define dbgchk(e,...) \
      do { if (dbg_lvl >= DBG_TEST) { \
        int dbg_err=!(e); dbg_tst.count++; dbg_tst.fail+=dbg_err; \
        FLOCKFILE(stderr); dbg_time(); \
        fprintf(stderr,"\xE%s: (%s)\x9%s:%d\xF\n",(dbg_err?"FAIL":"PASS"),#e,__FILE__,__LINE__); \
        if (dbg_err && *(dbg_exp(dbg_0(__VA_ARGS__)))) \
          { fprintf(stderr,"\xE" __VA_ARGS__); fputs("\xF\n",stderr); } \
        fflush(stderr); FUNLOCKFILE(stderr); \
        errno = dbg_err; dbg=0; \
      }} while(0)

  #define dbgmst(e,...) do { dbgchk(e, __VA_ARGS__); if (errno) abort();} while(0)

  #define dbggvn(...) if ((dbg_lvl >= DBG_BDD) && dbgmsg("GIVN: " __VA_ARGS__)) ; else
  #define dbgwhn(...) if ((dbg_lvl >= DBG_BDD) && dbgmsg("WHEN: " __VA_ARGS__)) ; else
  #define dbgthn(...) if ((dbg_lvl >= DBG_BDD) && dbgmsg("THEN: " __VA_ARGS__)) ; else

  #define dbgchkfail()  do { if (dbg_lvl >= DBG_TEST) {\
                          int err = errno; \
                          dbg_tst.count++;  \
                          dbgmsg("%s",err ? (dbg_tst.fail--, "PASS: Previous test failed as expected") \
                                          : (dbg_tst.fail+=2,"FAIL: Previous test was expected to fail")); \
                          errno = !err; \
                       }} while (0)

#else
  #define dbgchk _dbgchk
  #define dbgmst _dbgmst 
  #define dbgtst _dbgtst

  #define dbggvn _dbggvn
  #define dbgwhn _dbgwhn
  #define dbgthn _dbgthn
#endif

/* Debug Blocks
** ============
** 
** If you want to execute a block of code exclusively for debugging purpose (and
** easily exclude it from the production code), you may use the dbgblk macro.
**
** Reference
** ---------
**
**   dbgblk {...} --> Execute the block if DEBUG is defined as DBG_TEST.
**  _dbgblk {...} --> Do not execute the code block.
*/
#define _dbgblk if (1) ; else

#ifdef DEBUG
  #define dbgblk if (dbg_lvl < DBG_TEST) ; else
#else
  #define dbgblk _dbgblk
#endif

/* Tracking and watching
** =====================
**
** Check the `dbgtrk.c` source file for further details on these functions.
** If DEBUG is undefined or lower than DBG_TEST, dbgtrk() behaves as _dbgtrk()
**
** Reference
** ---------
**
**   dbgtrk(s) {...} --> Specify the patterns to be tracked within log generated by 
**                       the instructions in the scope of the code block.
**                       Patterns are separated by a '\1' character. The first
**                       character specify what should be checked about the pattern:
**                          -  the pattern does not appear
**                          =  the pattern appears exactly once
**                          +  the paterrn appears one or more times
**
**  _dbgtrk(s) {...} --> Execute the block but don't mark string tracking.
**
*/

#define _dbgtrk(...) 

#if defined(DEBUG) && (DEBUG >= DBG_TEST)
  #define dbgtrk(patterns) for (int dbg_trk = (dbg_lvl >= DBG_TEST)? !dbgmsg("TRK[: %s",patterns):1; \
                                    dbg_trk; \
                                    dbg_trk = (dbg_lvl >= DBG_TEST)?  dbgmsg("TRK]: "):0)
#else
  #define dbgtrk _dbgtrk
#endif

// Trace memory allocation 
// =======================

  typedef struct dbgmem_s {
    int  size;
    char head[4];
    char mem[];
  } dbgmem_t;
  
  #if defined(DEBUG) && defined(DBG_MEM)

  char *dbg_strdup(char *s, char *file, int line, dbg_tst_t *tst);
  void  dbg_free(void *p, char *file,int line, dbg_tst_t *tst);
  void *dbg_realloc(void *m, int sz, char *file,int line, dbg_tst_t *tst);
  void *dbg_calloc(int nitems, int size,char *file, int line,dbg_tst_t *tst);
  void *dbg_malloc(int sz, char *file,int line, dbg_tst_t *tst);
  int   dbg_memcheck(int inv,void *m, char *file, int line, dbg_tst_t *tst);

  extern void *((*malloc_std)(size_t))        ;
  extern void  ((*free_std)(void *))          ;
  extern void *((*realloc_std)(void *,size_t));
  extern void *((*calloc_std)(size_t,size_t)) ;

  #define malloc(n)       dbg_malloc(n,__FILE__,__LINE__,&dbg_tst)
  #define calloc(n,s)     dbg_calloc(n,s,__FILE__,__LINE__,&dbg_tst)
  #define free(p)         dbg_free(p,__FILE__,__LINE__,&dbg_tst)
  #define realloc(p,s)    dbg_realloc(p,s,__FILE__,__LINE__,&dbg_tst)
  #define dbgchkmem(p)    dbg_memcheck(0,p,__FILE__,__LINE__,&dbg_tst)
  #define dbgchkmeminv(p) dbg_memcheck(1,p,__FILE__,__LINE__,&dbg_tst)
  
  #ifdef strdup
    #undef strdup
  #endif 
  
  #define strdup(s) dbg_strdup(s,__FILE__,__LINE__,&dbg_tst)
  
#else // DBG_MEM

  #define free_std         free
  #define malloc_std       malloc
  #define realloc_std      realloc
  #define calloc_std       calloc
  #define dbgchkmem(p)     (errno = 0)
  #define dbgchkmeminv(p)  (errno = 0)

#endif // DBG_MEM

#endif // DBG_VERSION
#ifndef BUF_H___
#define BUF_H___

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

typedef struct buf_s {
    char *buffer;
    int32_t size;
    int32_t count;
    int32_t pos;
} *buf_t;


buf_t   buf_new();
buf_t   buf_free(buf_t buf);
int32_t buf_size(buf_t buf);
int32_t buf_pos(buf_t buf);
int32_t buf_count(buf_t buf);
char   *buf_str(buf_t buf, int32_t pos);
int32_t buf_makeroom(buf_t buf, int32_t size);
int     buf_putc(buf_t buf,int c);
int32_t buf_printf(buf_t b, const char *fmt, ...);
int32_t buf_putc(buf_t buf, int c);
int32_t buf_puts(buf_t buf, char *src);
int32_t buf_write(buf_t buf, char *src, int32_t len);

#endif