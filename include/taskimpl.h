/* Copyright (c) 2005-2006 Russ Cox, MIT; see COPYRIGHT */

#define USE_UCONTEXT 1

#if defined(__sun__)
#   define __EXTENSIONS__ 1 /* SunOS */
#   if defined(__SunOS5_6__) || defined(__SunOS5_7__) || defined(__SunOS5_8__)
/* NOT USING #define __MAKECONTEXT_V2_SOURCE 1 / * SunOS */
#   else
#       define __MAKECONTEXT_V2_SOURCE 1
#   endif
#endif

#if defined(__OpenBSD__) || defined(__mips__)
#   undef USE_UCONTEXT
#   define USE_UCONTEXT 0
#endif

#if defined(__APPLE__)
#   include <AvailabilityMacros.h>
#   if defined(MAC_OS_X_VERSION_10_5)
#       undef USE_UCONTEXT
#       define USE_UCONTEXT 0
#   endif
#endif

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <signal.h>

#if USE_UCONTEXT

#include <ucontext.h>

#endif

#include <sys/utsname.h>
#include <inttypes.h>
#include "task.h"

#define nil ((void*)0)
#define nelem(x) (sizeof(x)/sizeof((x)[0]))

#define ulong task_ulong
#define uint task_uint
#define uchar task_uchar
#define ushort task_ushort
#define uvlong task_uvlong
#define vlong task_vlong

typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long long uvlong;
typedef long long vlong;

#if defined(__FreeBSD__) && __FreeBSD__ < 5
extern  int     getmcontext(mcontext_t*);
extern  void    setmcontext(const mcontext_t*);
#define setcontext(u)   setmcontext(&(u)->uc_mcontext)
#define getcontext(u)   getmcontext(&(u)->uc_mcontext)
extern  int     swapcontext(ucontext_t*, const ucontext_t*);
extern  void    makecontext(ucontext_t*, void(*)(), int, ...);
#endif

#if defined(__APPLE__)
#   define mcontext libthread_mcontext
#   define mcontext_t libthread_mcontext_t
#   define ucontext libthread_ucontext
#   define ucontext_t libthread_ucontext_t
#   if defined(__i386__)
#       include "386-ucontext.h"
#   elif defined(__x86_64__)
#       include "amd64-ucontext.h"
#   else
#       include "power-ucontext.h"
#   endif
#endif

#if defined(__OpenBSD__)
#   define mcontext libthread_mcontext
#   define mcontext_t libthread_mcontext_t
#   define ucontext libthread_ucontext
#   define ucontext_t libthread_ucontext_t
#   if defined __i386__
#       include "386-ucontext.h"
#   else
#       include "power-ucontext.h"
#   endif
extern pid_t rfork_thread(int, void*, int(*)(void*), void*);
#endif

#if defined(__arm__)
int getmcontext(mcontext_t*);
void setmcontext(const mcontext_t*);
#define setcontext(u)   setmcontext(&(u)->uc_mcontext)
#define getcontext(u)   getmcontext(&(u)->uc_mcontext)
#endif

#if defined(__mips__)
#include "mips-ucontext.h"
int getmcontext(mcontext_t*);
void setmcontext(const mcontext_t*);
#define setcontext(u)   setmcontext(&(u)->uc_mcontext)
#define getcontext(u)   getmcontext(&(u)->uc_mcontext)
#endif

typedef struct Context Context;

enum
{
    STACK = 8192
};

struct Context
{
    ucontext_t uc;
};

struct Task
{
    char name[256];     // offset known to acid
    char state[256];
    Task *next;         // 协程的双向链表
    Task *prev;
    Task *allnext;
    Task *allprev;
    Context context;    // 实际上就是ucontext_t的包装, 后者就是系统里面的ucontext结构
    uvlong alarmtime;   // 如果是延时协程, 这个字段指定延时时间
    uint id;            // 协程id
    uchar *stk;         // 堆栈起始位置, 使用的时候是按向下使用的方式的
    uint stksize;       // 堆栈长度
    int exiting;        // 是否已经退出
    int alltaskslot;    // alltask[]的当前占用最高槽位数, 总数能到alltaskslot + 64 - alltaskslot%64
    int system;         // 是否为系统协程, 在计算是否还有用户协程时, 会忽略 system = 1 的协程
    int ready;          // 为1代表在运行状态
    void (*startfn)(void *);    // 协程对应函数
    void *startarg;
    void *udata;
};

void taskready(Task *);

void taskswitch(void);

void addtask(Tasklist *, Task *);

void deltask(Tasklist *, Task *);

extern Task *taskrunning;

extern int taskcount;
