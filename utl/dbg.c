/* 
**  (C) 2020 by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

// Globals
// =======

#include <stdlib.h>

void *((*malloc_std)(size_t))         = malloc;
void  ((*free_std)(void *))           = free;
void *((*realloc_std)(void *,size_t)) = realloc;
void *((*calloc_std)(size_t,size_t))  = calloc;

#ifdef DEBUG
#undef DEBUG
#endif

#define DEBUG DBG_TEST

#include "dbg.h"

// The global dbg_tst is to ensure dbgchk() can work outside a dbgtst() scope. It is not meant to be used in any way.
   dbg_tst_t dbg_tst  = {0, 0};
volatile int dbg      = 0;        // dbg will always be 0. Used to suppress warnings in some macros
         int dbg_lvl  = DBG_WARN; 
         int dbg_tmsp = DBG_NOTIME; 
       char *dbg_lvls = "NEWItT"; // NONE, ERROR, WARNING, INFO, TEST, TEST_BDD


// Debugging levels
// ================

char dbglvl_(char *lvl, char *tms) 
{  
  if (lvl) {
     switch(*lvl) {
       case 'T' : dbg_lvl = lvl[1] == '-'? DBG_TEST : DBG_BDD;  break;
       case 't' : dbg_lvl = DBG_TEST;  break;
       case 'I' : dbg_lvl = DBG_INFO;  break;
       case 'W' : dbg_lvl = DBG_WARN;  break;
       case 'E' : dbg_lvl = DBG_ERROR; break;
       case 'N' : dbg_lvl = DBG_NONE;  break;
     }
   }
   if (tms) {
     switch(*tms) {
       case 'T' : dbg_tmsp = DBG_TIME;  break;
       case 'N' : dbg_tmsp = DBG_NOTIME;  break;
     }
   }
   return dbg_lvls[(dbg_lvl+1)%6];
}

// Writing messages
// ================
                 // 0     1      2      3      4      5      6      7      8      9      10
char *dbg_msgtag = "XXXX" "FAIL" "PASS" "WARN" "INFO" "TRCE" "TST[" "TST]"
                   "CLK[" "CLK]" "TRK[" "TRK]" "GIVN" "WHEN" "THEN" "X";

int dbg_prttime(void)
{
  struct timespec clock;
  struct tm *tm;

  clock_gettime(CLOCK_REALTIME,&clock);
  tm = localtime(&(clock.tv_sec));
  fprintf(stderr,"%4d-%02d-%02d %02d:%02d:%02d.%06ld ",1900+tm->tm_year,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min, tm->tm_sec,clock.tv_nsec/1000);
  
  return 0;
}

int dbg_dmp(char *s, char *file, int line)
{
  if (dbg_lvl >= DBG_TEST) {
    if (s) dbgprt("DMP[: \x9%s:%d",file,line);
    if (s && s != dbg_lvls) fprintf(stderr,"%s\n",s);
    if (s != dbg_lvls) dbgprt("DMP]: \x9%s:%d",file,line);
  }
  return 0;
}

/* Trace memory allocation 
** =======================
**
**    +--------------+  <-- Actually allocated memory
**    |  0xCA5ABA5E  |   
**    +--------------+
**    |size (4 bytes)|
**    +--------------+  <-- Returned pointer
**    |              |   
**   //              //
**    |              |
**    +--------------+
**    |  0x10CCADD1  |   <-- Marker to identify overflow
**    +--------------+
*/

#define dbg_BEGCHK 0xCA5ABA5E
#define dbg_ENDCHK 0x10CCADD1
#define dbg_CLRCHK 0xB5B0CC1A
  
#define dbg_memptr(p) ((dbgmem_t *)(((uint8_t *)p) - offsetof(dbgmem_t,mem)))

static char *dbg_memerr[] = { "Valid","Freed","Invalid","Overflown" };

int dbg_memcheck(int inv,void *m, char *file, int line, dbg_tst_t *tst)
{
  dbgmem_t *p = &((dbgmem_t){0});
  int err = 0;
  int pass;

  if (dbg_lvl >= DBG_TEST) dbgprt("TRCE: MEM CHECK(%p) START\x9%s:%d",m, file, line);
  if (m != NULL) {
    p = dbg_memptr(m);

         if (memcmp(p->head, &((uint32_t){dbg_CLRCHK}), 4) == 0) err = 1;
    else if (memcmp(p->head, &((uint32_t){dbg_BEGCHK}), 4))      err = 2;
    else if (memcmp(p->mem+p->size,&((uint32_t){dbg_ENDCHK}),4)) err = 3;
  }

  // We pass the test if we have no error and we were checking for a valid pointer
  // or if we have an error and we were checking for an invalid pointer
  pass = ((!!err) == inv);

  dbgprt("%s: MEM CHECK %p[%d] (%s)\x9%s:%d", (pass?"PASS":"FAIL"), m, (err?0:p->size), dbg_memerr[err], file, line);

  tst->fail += !pass;
  tst->count++;
  errno = err;
  return err;
}
  
static void dbg_memmark(dbgmem_t *p, uint32_t beg, uint32_t end)
{
  memcpy(p->head, (void *)(&beg), 4);
  memcpy(p->mem + p->size, (void *)(&end), 4) ;
}
  
void *dbg_malloc(int sz, char *file,int line, dbg_tst_t *dbg_tst)
{
  dbgmem_t *p = NULL;
  void *ret = NULL;
    
  if (sz > 0) p = malloc_std(sizeof(dbgmem_t)+4+sz);
    
  if (p == NULL) {
    dbg_tst->fail++;
    dbg_tst->count++;
  }
  else {
    p->size = sz;
    dbg_memmark(p,dbg_BEGCHK, dbg_ENDCHK);
    ret = p->mem;
  }
  if (dbg_lvl >= DBG_TEST) 
    dbgprt("%s: MEM malloc(%d) -> %p \x9%s:%d", p?"TRCE":"FAIL", sz, ret, file, line);

  return ret;
}
  
void *dbg_calloc(int nitems, int size,char *file, int line, dbg_tst_t *dbg_tst)
{
  dbgmem_t *p = NULL;
  int sz;
  void *ret = NULL;
  
  sz = nitems*size ;
  
  if (sz > 0) p = calloc_std(sizeof(dbgmem_t)+4+sz, 1);

  if (p == NULL) {
    dbg_tst->fail++;
    dbg_tst->count++;
  }
  else {
    p->size = sz;
    dbg_memmark(p, dbg_BEGCHK, dbg_ENDCHK);
    ret = p->mem;
  }
  if (dbg_lvl >= DBG_TEST)
    dbgprt("%s: MEM calloc(%d,%d) -> %p\x9%s:%d",p?"TRCE":"FAIL",nitems,size,ret,file,line);
  return ret;
}
  
  void *dbg_realloc(void *m, int sz, char *file,int line, dbg_tst_t *tst)
  {
    dbgmem_t *p=NULL;
    void *ret = NULL;

    if (dbg_memcheck(0,m,file,line,tst) == 0) {
      if (m) p = dbg_memptr(m);
      if (sz) {
        p = realloc_std(p,sizeof(dbgmem_t)+4+sz);
        if (p) {
          p->size = sz;
          dbg_memmark(p, dbg_BEGCHK, dbg_ENDCHK);
          ret = p->mem;
        }
      }
      else {
        if (p) {
          dbg_memmark(p, dbg_CLRCHK, 0);
          ret = realloc_std(p,0);
          dbg_memcheck(1,((dbgmem_t *)p)->mem,file,line,tst);
        }
      }
    }  
 
    if (dbg_lvl >= DBG_TEST) dbgprt("TRCE: MEM realloc(%p,%d) -> %p\x9%s:%d",m,sz,ret,file,line);

    return ret;
  }
  
  void dbg_free(void *m, char *file,int line, dbg_tst_t *tst)
  {
    void *p = NULL;
    if ((dbg_memcheck(0,m,file,line,tst) == 0) && m) {
      p =dbg_memptr(m);
      dbg_memmark(p, dbg_CLRCHK, 0);
      ((dbgmem_t *)p)->size = 0;
    }
    if (dbg_lvl >= DBG_TEST) dbgprt("TRCE: MEM free(%p)\x9%s:%d",m,file,line);
    if (p) {
      free_std(p);
      //dbg_memcheck(1,m,file,line,tst);
    }
  }
  
  char *dbg_strdup(char *s, char *file, int line, dbg_tst_t *tst)
  {
    char *p=NULL;
    if (dbg_lvl >= DBG_TEST) dbgprt("TRCE: MEM strdup(%p)\x9%s:%d",s,file,line);
    if (s) {
      p = dbg_malloc(strlen(s)+1, file, line, tst);
      if (p) strcpy(p,s);
    }
    return p;
  }
