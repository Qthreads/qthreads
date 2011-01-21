#ifndef QT_SHEPHERD_INNARDS_H
#define QT_SHEPHERD_INNARDS_H

/* System Pre-requisites */
#include <pthread.h>

/* Internal Pre-requisites */
#include "qt_mpool.h"
#include "qt_atomics.h"

/* pre-declarations */
typedef struct qt_threadqueue_s qt_threadqueue_t;

#ifndef QTHREAD_SHEPHERD_TYPEDEF
typedef struct qthread_shepherd_s qthread_shepherd_t;
#endif

#ifdef QTHREAD_MULTITHREADED_SHEPHERDS
#define MAX_WORKERS_PER_SHEPHERD 1024

struct qthread_worker_s
{
    pthread_t worker;
    qthread_worker_id_t worker_id;
    qthread_worker_id_t packed_worker_id;
    qthread_shepherd_t *shepherd;
    qthread_t *current;
    volatile size_t active;
};
typedef struct qthread_worker_s qthread_worker_t;
#endif

/* The Shepherd Struct */
struct qthread_shepherd_s
{
    pthread_t shepherd;
    qthread_shepherd_id_t shepherd_id;	/* whoami */
#ifdef QTHREAD_MULTITHREADED_SHEPHERDS
    qthread_worker_t workers[MAX_WORKERS_PER_SHEPHERD]; // XXX: this is too big; should be allocated at runtime based on actual number of workers
    uint8_t stealing;  /* True when a worker is in the steal (attempt) process */
#endif
    qthread_t *current;
    qt_threadqueue_t *ready;
    /* memory pools */
    qt_mpool qthread_pool;
    qt_mpool queue_pool;
    qt_mpool threadqueue_pool;
    qt_mpool threadqueue_node_pool;
    qt_mpool lock_pool;
    qt_mpool addrres_pool;
    qt_mpool addrstat_pool;
    qt_mpool stack_pool;
    /* round robin scheduler - can probably be smarter */
    aligned_t sched_shepherd;
    volatile uintptr_t QTHREAD_CASLOCK(active);
    /* affinity information */
    unsigned int node;		/* whereami */
#ifdef QTHREAD_HAVE_LGRP
    unsigned int lgrp;
#endif
    unsigned int *shep_dists;
    qthread_shepherd_id_t *sorted_sheplist;
#ifdef STEAL_PROFILE // should give mechanism to make steal profiling optional
    size_t steal_called;
    size_t steal_attempted;
    size_t steal_amount_stolen;
    size_t steal_failed;
    size_t steal_successful;
#endif
#ifdef QTHREAD_SHEPHERD_PROFILING
    qtimer_t total_time;	/* how much time the shepherd spent running */
    double idle_maxtime;	/* max time the shepherd spent waiting for new threads */
    double idle_time;		/* how much time the shepherd spent waiting for new threads */
    size_t idle_count;		/* how many times the shepherd did a blocking dequeue */
    size_t num_threads;		/* number of threads handled */
#endif
#ifdef QTHREAD_LOCK_PROFILING
# ifdef QTHREAD_MUTEX_INCREMENT
    qt_hash uniqueincraddrs;    /* the unique addresses that are incremented */
    double incr_maxtime;        /* maximum time spent in a single increment */
    double incr_time;           /* total time spent incrementing */
    size_t incr_count;          /* number of increments */
# endif

    qt_hash uniquelockaddrs;	/* the unique addresses that are locked */
    double aquirelock_maxtime;	/* max time spent aquiring locks */
    double aquirelock_time;	/* total time spent aquiring locks */
    size_t aquirelock_count;	/* num locks aquired */
    double lockwait_maxtime;	/* max time spent blocked on a lock */
    double lockwait_time;	/* total time spent blocked on a lock */
    size_t lockwait_count;	/* num times blocked on a lock */
    double hold_maxtime;	/* max time spent holding locks */
    double hold_time;		/* total time spent holding locks (use aquirelock_count) */

    qt_hash uniquefebaddrs;	/* unique addresses that are associated with febs */
    double febblock_maxtime;	/* max time spent aquiring FEB words */
    double febblock_time;	/* total time spent aquiring FEB words */
    size_t febblock_count;	/* num FEB words aquired */
    double febwait_maxtime;	/* max time spent blocking on FEBs */
    double febwait_time;	/* total time spent blocking on FEBs */
    size_t febwait_count;	/* num FEB blocking waits required */
    double empty_maxtime;	/* max time addresses spent empty */
    double empty_time;		/* total time addresses spent empty */
    size_t empty_count;		/* num times addresses were empty */
#endif
};

extern pthread_key_t shepherd_structs;
static QINLINE qthread_shepherd_t *qthread_internal_getshep(void)
{
#ifdef QTHREAD_MULTITHREADED_SHEPHERDS
    return ((qthread_worker_t*)pthread_getspecific(shepherd_structs))->shepherd;
#else
    return (qthread_shepherd_t*)pthread_getspecific(shepherd_structs);
#endif
}
void qthread_back_to_master(qthread_t * t);

#endif
