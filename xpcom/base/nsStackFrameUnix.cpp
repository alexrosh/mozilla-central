/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is nsStackFrameWin.h code, released
 * December 20, 2000.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 2003 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU Public License (the "GPL"), in which case the
 * provisions of the GPL are applicable instead of those above.
 * If you wish to allow use of your version of this file only
 * under the terms of the GPL and not to allow others to use your
 * version of this file under the MPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL.  If you do not delete
 * the provisions above, a recipient may use your version of this
 * file under either the MPL or the GPL.
 */

#include "nsStackFrameUnix.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "nscore.h"

// On glibc 2.1, the Dl_info api defined in <dlfcn.h> is only exposed
// if __USE_GNU is defined.  I suppose its some kind of standards
// adherence thing.
//
#if (__GLIBC_MINOR__ >= 1) && !defined(__USE_GNU)
#define __USE_GNU
#endif

#ifdef HAVE_LIBDL
#include <dlfcn.h>
#endif



// This thing is exported by libiberty.a (-liberty)
// Yes, this is a gcc only hack
#if defined(MOZ_DEMANGLE_SYMBOLS)
extern "C" char * cplus_demangle(const char *,int);
#include <stdlib.h> // for free()
#endif // MOZ_DEMANGLE_SYMBOLS

#if (defined(__linux__) || defined(__sun)) && defined(__GNUC__)
void DemangleSymbol(const char * aSymbol, 
                    char * aBuffer,
                    int aBufLen)
{
    aBuffer[0] = '\0';

#if defined(MOZ_DEMANGLE_SYMBOLS)
    /* See demangle.h in the gcc source for the voodoo */
    char * demangled = cplus_demangle(aSymbol,3);
    
    if (demangled)
    {
        strncpy(aBuffer,demangled,aBufLen);
        free(demangled);
    }
#endif // MOZ_DEMANGLE_SYMBOLS
}

#else
void DemangleSymbol(const char * aSymbol, 
                    char * aBuffer,
                    int aBufLen)
{
  // lose
  aBuffer[0] = '\0';
}
#endif



#if defined(linux) && defined(__GLIBC__) && (defined(__i386) || defined(PPC)) // i386 or PPC Linux stackwalking code

#include <setjmp.h>
//

void DumpStackToFile(FILE* aStream)
{
  jmp_buf jb;
  setjmp(jb);

  // Stack walking code courtesy Kipp's "leaky".

  // Get the frame pointer out of the jmp_buf
#if defined(__i386) 
  unsigned long* bp = (unsigned long*) (jb[0].__jmpbuf[JB_BP]);
#elif defined(PPC)
  unsigned long* bp = (unsigned long*) (jb[0].__jmpbuf[JB_GPR1]);
#endif

  int skip = 2;
  for (unsigned long *nextbp = (unsigned long*) *bp++, pc = *bp;
       pc >= 0x08000000 && pc < 0x7fffffff && nextbp > bp;
       bp = nextbp, nextbp = (unsigned long*) *bp++, pc = *bp) {
    if (--skip <= 0) {
      Dl_info info;
      int ok = dladdr((void*) pc, &info);
      if (!ok) {
        fprintf(aStream, "UNKNOWN %p\n", (void *)pc);
        continue;
      }

      PRUint32 foff = (char*)pc - (char*)info.dli_fbase;

      const char * symbol = info.dli_sname;
      int len;
      if (!symbol || !(len = strlen(symbol))) {
        fprintf(aStream, "UNKNOWN [%s +0x%08X]\n",
                info.dli_fname, foff);
        continue;
      }

      char demangled[4096] = "\0";

      DemangleSymbol(symbol, demangled, sizeof(demangled));

      if (strlen(demangled)) {
        symbol = demangled;
        len = strlen(symbol);
      }

      PRUint32 off = (char*)pc - (char*)info.dli_saddr;
      fprintf(aStream, "%s+0x%08X [%s +0x%08X]\n",
              symbol, off, info.dli_fname, foff);
    }
  }
}

#elif defined(__sun) && (defined(__sparc) || defined(sparc) || defined(__i386) || defined(i386))

/*
 * Stack walking code for Solaris courtesy of Bart Smaalder's "memtrak".
 */

#include <synch.h>
#include <ucontext.h>
#include <sys/frame.h>
#include <sys/regset.h>
#include <sys/stack.h>

static int    load_address ( void * pc, void * arg, FILE * aStream );
static int    write_address_file ( void * pc );
static struct bucket * newbucket ( void * pc );
static struct frame * cs_getmyframeptr ( void );
static void   cs_walk_stack ( void * (*read_func)(char * address),
                              struct frame * fp,
                              int (*operate_func)(void *, void *),
                              void * usrarg, FILE * aStream );
static void   cs_operate ( void (*operate_func)(void *, void *),
                           void * usrarg, FILE * aStream );

#ifndef STACK_BIAS
#define STACK_BIAS 0
#endif /*STACK_BIAS*/

#define LOGSIZE 4096

/* type of demangling function */
typedef int demf_t(const char *, char *, size_t);

static demf_t *demf;

static int initialized = 0;

#if defined(sparc) || defined(__sparc)
#define FRAME_PTR_REGISTER REG_SP
#endif

#if defined(i386) || defined(__i386)
#define FRAME_PTR_REGISTER EBP
#endif

struct bucket {
    void * pc;
    int index;
    struct bucket * next;
};

struct mybuf {
    char * buffer;
    int chars_left;
};


static void myinit();

#pragma init (myinit)

static void
myinit()
{

    if (! initialized) {
#ifndef __GNUC__
        void *handle;
        const char *libdem = "libdemangle.so.1";

        /* load libdemangle if we can and need to (only try this once) */
        if ((handle = dlopen(libdem, RTLD_LAZY)) != NULL) {
            demf = (demf_t *)dlsym(handle,
                           "cplus_demangle"); /*lint !e611 */
                /*
                 * lint override above is to prevent lint from
                 * complaining about "suspicious cast".
                 */
        }
#endif /*__GNUC__*/
    }    
    initialized = 1;
}


static int
write_address_file(void * pc, FILE* aStream)
{
    static struct bucket table[2048];
    static mutex_t lock;
    struct bucket * ptr;

    unsigned int val = NS_PTR_TO_INT32(pc);

    ptr = table + ((val >> 2)&2047);

    mutex_lock(&lock);
    while (ptr->next) {
        if (ptr->next->pc == pc)
            break;
        ptr = ptr->next;
    }

    if (ptr->next) {
        mutex_unlock(&lock);
        return (ptr->next->index);
    } else {
        char buffer[4096], dembuff[4096];
        Dl_info info;
        char *func, *lib;

        ptr->next = newbucket(pc);
        mutex_unlock(&lock);
 
        if (dladdr(pc, & info) == 0) {
            func = "??";
            lib  = "??";
        } else {
            lib =   (char *) info.dli_fname;
            func =  (char *) info.dli_sname;
        }
 
#ifdef __GNUC__
        nsTraceRefcnt::DemangleSymbol(func, dembuff, sizeof(dembuff));
        if (strlen(dembuff)) {
            func = dembuff;
        }
#else
        if (demf) {
            if (demf(func, dembuff, sizeof (dembuff)) == 0)
                func = dembuff;
		}
#endif /*__GNUC__*/
 
        fprintf(aStream, "%u %s:%s+0x%x\n",
                ptr->next->index,
                lib,
                func,
                (char *)pc - (char*)info.dli_saddr);
 
        return (ptr->next->index);
    }
}


static int
load_address(void * pc, void * arg, FILE * aStream)
{
    struct mybuf * buf = (struct mybuf *) arg;

    char name[80];
    int len;

    sprintf(name, " %u", write_address_file(pc, aStream));

    len = strlen(name);

    if (len >= buf->chars_left)
        return (1);

    strcat(buf->buffer, name);

    buf->chars_left -= len;

    return (0);
}


static struct bucket *
newbucket(void * pc)
{
    struct bucket * ptr = (struct bucket *) malloc(sizeof (*ptr));
    static int index; /* protected by lock in caller */
                     
    ptr->index = index++;
    ptr->next = NULL;
    ptr->pc = pc;    
    return (ptr);    
}


static struct frame *
csgetframeptr()
{
    ucontext_t u;
    struct frame *fp;

    (void) getcontext(&u);

    fp = (struct frame *)
        ((char *)u.uc_mcontext.gregs[FRAME_PTR_REGISTER] +
        STACK_BIAS);

    /* make sure to return parents frame pointer.... */

    return ((struct frame *)((ulong_t)fp->fr_savfp + STACK_BIAS));
}


static void
cswalkstack(struct frame *fp, int (*operate_func)(void *, void *, FILE *),
    void *usrarg, FILE * aStream)
{

    while (fp != 0 && fp->fr_savpc != 0) {

        if (operate_func((void *)fp->fr_savpc, usrarg, aStream) != 0)
            break;
        /*
         * watch out - libthread stacks look funny at the top
         * so they may not have their STACK_BIAS set
         */

        fp = (struct frame *)((ulong_t)fp->fr_savfp +
            (fp->fr_savfp?(ulong_t)STACK_BIAS:0));
    }
}


static void
cs_operate(int (*operate_func)(void *, void *, FILE *), void * usrarg, FILE *aStream)
{
    cswalkstack(csgetframeptr(), operate_func, usrarg, aStream);
}

void DumpStackToFile(FILE* aStream)
{
    char buffer[LOGSIZE];
    struct mybuf mybuf;

    if (!initialized)
        myinit();

    mybuf.chars_left = LOGSIZE - strlen(buffer)-1;
    mybuf.buffer = buffer;
    cs_operate(load_address, &mybuf, aStream);
}
#endif
