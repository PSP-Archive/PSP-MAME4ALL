#ifndef _MAME4ALL_PROFILER_H_
#define _MAME4ALL_PROFILER_H_


#ifndef PROFILER_MAME4ALL

#define mame4all_prof_start(A)
#define mame4all_prof_end(A)

#else

#ifdef PROFILER_SDL
#include <SDL.h>
#define PROFILER__REDUCTION__ 0
#define PROFILER__ADJUST__ 0
#define get_time_in_micros() (unsigned long long)SDL_GetTicks()

#else

#ifndef DREAMCAST
#include <sys/time.h>
#define PROFILER__REDUCTION__ 0
#define PROFILER__ADJUST__ 0
#ifdef WIN32
int gettimeofday(struct timeval *tp, struct timezone *tzp);
#endif
static __inline__ unsigned long long get_time_in_micros(void)
{
	unsigned long long ret;
	struct timeval tm;
	gettimeofday(&tm,NULL);
	ret=tm.tv_sec;
	ret*=1000000LL;
	ret+=tm.tv_usec;
	return ret;
}

#else
#include <kos.h>
#define PROFILER__REDUCTION__ 2
#define PROFILER__ADJUST__ 1
#define get_time_in_micros() timer_us_gettime64()
#endif
#endif

#define MAME4ALL_PROFILER_MAX 64

extern unsigned long long mame4all_prof_initial[MAME4ALL_PROFILER_MAX];
extern unsigned long long mame4all_prof_sum[MAME4ALL_PROFILER_MAX];
extern unsigned long long mame4all_prof_executed[MAME4ALL_PROFILER_MAX];
extern int mame4all_prof_started[MAME4ALL_PROFILER_MAX];

static __inline__ void mame4all_prof_start(unsigned a)
{
	if (mame4all_prof_started[a])
		return;
	mame4all_prof_executed[a]++;
	mame4all_prof_initial[a]=get_time_in_micros();
	mame4all_prof_started[a]=1;
}


static __inline__ void mame4all_prof_end(unsigned a)
{
	if (!mame4all_prof_started[a])
		return;
	extern unsigned mame4all_prof_total;
	unsigned i;
	for(i=0;i<mame4all_prof_total;i++)
		mame4all_prof_initial[i]+=PROFILER__REDUCTION__;
	mame4all_prof_sum[a]+=get_time_in_micros()-mame4all_prof_initial[a]+PROFILER__ADJUST__;
	mame4all_prof_started[a]=0;
}
#undef PROFILER__REDUCTION__
#undef PROFILER__ADJUST__

void mame4all_prof_init(void);
void mame4all_prof_add(char *msg);
void mame4all_prof_show(void);
void mame4all_prof_setmsg(int n, char *msg);


#endif





#endif
