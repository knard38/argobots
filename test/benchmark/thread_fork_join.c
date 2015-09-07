/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 * See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef USE_PAPI
#include <papi.h>
#endif
#include "abt.h"
#include "abttest.h"

#define START_NULTS 1024     /* Initial test config in terms of #ULTs*/

/* FIXME: We might want to define cycle counters as ABT_timers
 * and be globally accessible */

#if defined(__x86_64__)
static inline unsigned long long get_cycles()
{
      unsigned hi, lo;
      __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
      unsigned long long cycle = ((unsigned long long)lo)|( ((unsigned long long)hi)<<32) ;

      return cycle;
}
#else
#error "Cycle information not supported on this platform"
#endif

#ifdef USE_PAPI
#define ABTX_papi_assert(expr)                   \
do {                                             \
    if(expr != PAPI_OK) {                        \
        fprintf(stderr, "Error at " #expr "\n"); \
        exit(-1);                                \
    }                                            \
} while(0)
#endif

#ifndef USE_PAPI
#define ABTX_start_prof(start_time, evset)       \
do {                                             \
    start_time = get_cycles();                   \
} while(0)
#else
#define ABTX_start_prof(start_time, evset)       \
do {                                             \
    start_time = get_cycles();                   \
    ABTX_papi_assert(PAPI_start(evset));         \
} while(0)
#endif

#ifndef USE_PAPI
#define ABTX_stop_prof(start_time, nults, time_sum, time_sqrsum, evset, vals, \
                       llcm_sum, llcm_sqrsum, totm_sum, tlbm_sqrsum)          \
do {                                                                          \
      float elaps_time = (float)(get_cycles() - start_time)/nults;            \
      time_sum += elaps_time;                                                 \
      time_sqrsum += elaps_time*elaps_time;                                   \
} while(0)
#else
#define ABTX_stop_prof(start_time, nults, time_sum, time_sqrsum, evset, vals, \
                       llcm_sum, llcm_sqrsum, totm_sum, tlbm_sqrsum)          \
do {                                                                          \
      ABTX_papi_assert(PAPI_stop(evset, vals));                               \
      float llcm = (float)vals[0]/nults, tlbm = (float)vals[1]/nults;         \
      llcm_sum += llcm; llcm_sqrsum += llcm*llcm;                             \
      totm_sum += tlbm; tlbm_sqrsum += tlbm*tlbm;                             \
      float elaps_time = (float)(get_cycles() - start_time)/nults;            \
      time_sum += elaps_time;                                                 \
      time_sqrsum += elaps_time*elaps_time;                                   \
} while(0)
#endif

#ifndef USE_PAPI
#define ABTX_prof_summary(my_es, nults, iter,                                                       \
                          crea_time, crea_timestd, crea_llcm, crea_llcmstd, crea_tlbm, crea_tlbmstd,\
                          join_time, join_timestd, join_llcm, join_llcmstd, join_tlbm, join_tlbmstd,\
                          free_time, free_timestd, free_llcm, free_llcmstd, free_tlbm, free_tlbmstd)\
do {                                                                                                \
        crea_time /= iter; crea_timestd = sqrt(crea_timestd/iter - crea_time*crea_time);            \
        join_time /= iter; join_timestd = sqrt(join_timestd/iter - join_time*join_time);            \
        free_time /= iter; free_timestd = sqrt(free_timestd/iter - free_time*free_time);            \
        printf("%d \t %d \t %d \t %.2f [%.2f] \t %.2f [%.2f] \t %.2f [%.2f]\n", my_es, nults, iter,   \
                         crea_time, crea_timestd, join_time, join_timestd, free_time, free_timestd);\
       fflush(stdout);                                                                              \
} while(0)
#else
#define ABTX_prof_summary(my_es, nults, iter,                                                       \
                          crea_time, crea_timestd, crea_llcm, crea_llcmstd, crea_tlbm, crea_tlbmstd,\
                          join_time, join_timestd, join_llcm, join_llcmstd, join_tlbm, join_tlbmstd,\
                          free_time, free_timestd, free_llcm, free_llcmstd, free_tlbm, free_tlbmstd)\
do {                                                                                                \
        crea_time /= iter; crea_timestd = sqrt(crea_timestd/iter - crea_time*crea_time);            \
        crea_llcm /= iter; crea_llcmstd = sqrt(crea_llcmstd/iter - crea_llcm*crea_llcm);            \
        crea_tlbm /= iter; crea_tlbmstd = sqrt(crea_tlbmstd/iter - crea_tlbm*crea_tlbm);            \
        join_time /= iter; join_timestd = sqrt(join_timestd/iter - join_time*join_time);            \
        join_llcm /= iter; join_llcmstd = sqrt(join_llcmstd/iter - join_llcm*join_llcm);            \
        join_tlbm /= iter; join_tlbmstd = sqrt(join_tlbmstd/iter - join_tlbm*join_tlbm);            \
        free_time /= iter; free_timestd = sqrt(free_timestd/iter - free_time*free_time);            \
        free_llcm /= iter; free_llcmstd = sqrt(free_llcmstd/iter - free_llcm*free_llcm);            \
        free_tlbm /= iter; free_tlbmstd = sqrt(free_tlbmstd/iter - free_tlbm*free_tlbm);            \
        printf("%d \t %d \t %d \t %.2f [%.2f] \t %.2f [%.2f] \t %.2f [%.2f]"                        \
                              "\t %.2f [%.2f] \t %.2f [%.2f] \t %.2f [%.2f]"                        \
                              "\t %.2f [%.2f] \t %.2f [%.2f] \t %.2f [%.2f]\n", my_es, nults, iter, \
                         crea_time, crea_timestd, crea_llcm, crea_llcmstd, crea_tlbm, crea_tlbmstd, \
                         join_time, join_timestd, join_llcm, join_llcmstd, join_tlbm, join_tlbmstd, \
                         free_time, free_timestd, free_llcm, free_llcmstd, free_tlbm, free_tlbmstd);\
       fflush(stdout);                                                                              \
} while(0)
#endif


static ABT_xstream* xstreams;
static ABT_pool*    pools;
static int niter, max_ults, ness;

/* The goal of the following sequence generator is to
 * output a data points spread uniformly on a logarithmic
 * scale. For example, if one wants to generate powers
 * two to be used as x-axis labels, but the number of ticks
 * is too small, this generator can output more terms to be
 * used as labels while having a uniform distribution on a 
 * log_2 scale. */

/* The following data-structure holds the state of the
 * sequence generator */
typedef struct seq_state_t {
    int base;
    int prev_term;     /* previously generated term */
    int cur_stride;    /* current stride */
    int last_pow_term; /* last term which is power of base */
/* maximum number of terms non-power of base between two
 * successive powers of base */
    int max_nonpow_terms;
}seq_state_t;

static inline void seq_init(seq_state_t* state, const int base,
                                  const int prev_term,
                                  const int last_pow_term,
                                  const int max_nonpow_terms) {
    state->base = base;
    state->prev_term = prev_term;
    state->cur_stride = prev_term;
    state->last_pow_term = last_pow_term;
    state->max_nonpow_terms = max_nonpow_terms;
}
/* Core of the sequence generator */
static inline int seq_get_next_term(seq_state_t* state) {
   int cur_term; /* term to return */
   cur_term = state->prev_term + state->cur_stride;
   if (cur_term == state->last_pow_term*state->base) {
       while(cur_term/state->cur_stride - 1 > state->max_nonpow_terms)
           state->cur_stride *= state->base;
       state->last_pow_term = cur_term;
   }
   state->prev_term = cur_term;
   return cur_term;
}

static void thread_func(void *arg)
{
    ABT_TEST_UNUSED(arg);
}

static void thread_xtream_master(void* arg) {
#ifdef USE_PAPI
    /* Create the Event Set */
    int event_set = PAPI_NULL;
    long_long values[2];
    ABTX_papi_assert(PAPI_create_eventset(&event_set));
    /* Add events to monitor */
    ABTX_papi_assert(PAPI_add_event(event_set, PAPI_L3_TCM));
    ABTX_papi_assert(PAPI_add_event(event_set, PAPI_TLB_DM));
#endif
    int my_es = (int)(size_t) arg;
    ABT_thread* my_ults = (ABT_thread*) malloc(max_ults*sizeof(ABT_thread));

    int nults;
    seq_state_t state;
    seq_init(&state, 2, START_NULTS/2, START_NULTS/2, 1);
    while((nults = seq_get_next_term(&state)) <= max_ults) {
        float crea_time = 0.0, crea_timestd = 0.0, crea_llcm = 0.0, crea_llcmstd = 0.0, crea_tlbm = 0.0, crea_tlbmstd = 0.0;
        float join_time = 0.0, join_timestd = 0.0, join_llcm = 0.0, join_llcmstd = 0.0, join_tlbm = 0.0, join_tlbmstd = 0.0;
        float free_time = 0.0, free_timestd = 0.0, free_llcm = 0.0, free_llcmstd = 0.0, free_tlbm = 0.0, free_tlbmstd = 0.0;
        int i;
        /* The following line tries to keep the total number of iterations constant */
        int iter = niter/(nults/START_NULTS);
        for(i=0; i<iter; i++) {
            int t; unsigned long long start_time;

            ABTX_start_prof(start_time, event_set);
            for(t=0; t<nults; t++)
                ABT_thread_create(pools[my_es], thread_func, NULL, ABT_THREAD_ATTR_NULL, &my_ults[t]);
            ABTX_stop_prof(start_time, nults, crea_time, crea_timestd, event_set, values, crea_llcm, crea_llcmstd, crea_tlbm, crea_tlbmstd);

            ABTX_start_prof(start_time, event_set);
            for(t=0; t<nults; t++)
                ABT_thread_join(my_ults[t]);
            ABTX_stop_prof(start_time, nults, join_time, join_timestd, event_set, values, join_llcm, join_llcmstd, join_tlbm, join_tlbmstd);

            ABTX_start_prof(start_time, event_set);
            for(t=0; t<nults; t++)
                ABT_thread_free(&my_ults[t]);
            ABTX_stop_prof(start_time, nults, free_time, free_timestd, event_set, values, free_llcm, free_llcmstd, free_tlbm, free_tlbmstd);
        }
        ABTX_prof_summary(my_es, nults, iter, crea_time, crea_timestd, crea_llcm, crea_llcmstd, crea_tlbm, crea_tlbmstd,
                                              join_time, join_timestd, join_llcm, join_llcmstd, join_tlbm, join_tlbmstd,
                                              free_time, free_timestd, free_llcm, free_llcmstd, free_tlbm, free_tlbmstd);
    }

    free(my_ults);
}

int main(int argc, char *argv[])
{

#ifdef USE_PAPI
    int ret = PAPI_library_init(PAPI_VER_CURRENT);
    if (ret != PAPI_VER_CURRENT) {
        fprintf(stderr, "PAPI library init error!\n");
        exit(1);
    }
#endif

    int i;
    ABT_test_read_args(argc, argv);
    niter = ABT_test_get_arg_val(ABT_TEST_ARG_N_ITER);
    ness  = ABT_test_get_arg_val(ABT_TEST_ARG_N_ES);
    max_ults = ABT_test_get_arg_val(ABT_TEST_ARG_N_ULT);

    xstreams = (ABT_xstream*) malloc (ness*sizeof(ABT_xstream));
    pools = (ABT_pool*) malloc (ness*sizeof(ABT_pool));

    ABT_init(argc, argv);

    /* output beginning */
    int line_size = 75, nults;
    ABT_test_print_line(stdout, '-', line_size);
#ifndef USE_PAPI
    printf("# ES \t #ULTS \t #Iter \t Create: time [std] \t  Join: time [std] \t Free: time [std]\n");
#else
    printf("# ES \t #ULTS \t #Iter \t Create: time [std] LLCm [std] TLBm [std] \t  Join: time [std]  LLCm [std] TLBm [std] \t Free: time [std] LLCm [std] TLBm [std]\n");
#endif

    ABT_test_print_line(stdout, '-', line_size);

    /* Create ESs*/
    ABT_xstream_self(&xstreams[0]);
    for(i=1; i<ness; i++) {
        ABT_xstream_create(ABT_SCHED_NULL, &xstreams[i]);
        ABT_xstream_get_main_pools(xstreams[i], 1, &pools[i]);
    }

    for(i=1; i<ness; i++)
        ABT_thread_create(pools[i], thread_xtream_master, (void*)(size_t)i, ABT_THREAD_ATTR_NULL, NULL);

    ABT_xstream_get_main_pools(xstreams[0], 1, &pools[0]);
    thread_xtream_master((void*)(size_t)0);

    for(i=1; i<ness; i++) {
        ABT_xstream_join(xstreams[i]);
        ABT_xstream_free(&xstreams[i]);
    }

    ABT_finalize();

    free(pools);
    free(xstreams);

    return EXIT_SUCCESS;
}

