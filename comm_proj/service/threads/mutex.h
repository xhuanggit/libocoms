/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2006 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart, 
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2007      Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2007      Los Alamos National Security, LLC.  All rights
 *                         reserved. 
 * Copyright (c) 2007      Voltaire. All rights reserved.
 * Copyright (c) 2010      Oracle and/or its affiliates.  All rights reserved.
 *
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 */

#ifndef  SERVICE_MUTEX_H
#define  SERVICE_MUTEX_H 1

#include "ccs_config.h"

#if CCS_ENABLE_MULTI_THREADS
#include "service/sys/atomic.h"
#endif  /* CCS_ENABLE_MULTI_THREADS */
#if CCS_ENABLE_DEBUG
#include "service/util/output.h"
#endif

BEGIN_C_DECLS

/**
 * @file:
 *
 * Mutual exclusion functions.
 *
 * Functions for locking of critical sections.
 */

/*
 * declaring this here so that CL does not complain
 */ 
CCS_DECLSPEC extern bool ccs_uses_threads;

#if CCS_ENABLE_DEBUG
CCS_DECLSPEC extern bool service_mutex_check_locks;
#endif

/**
 * Opaque mutex object
 */
typedef struct service_mutex_t service_mutex_t;


/**
 * Try to acquire a mutex.
 *
 * @param mutex         Address of the mutex.
 * @return              0 if the mutex was acquired, 1 otherwise.
 */
static inline int service_mutex_trylock(service_mutex_t *mutex);


/**
 * Acquire a mutex.
 *
 * @param mutex         Address of the mutex.
 */
static inline void service_mutex_lock(service_mutex_t *mutex);


/**
 * Release a mutex.
 *
 * @param mutex         Address of the mutex.
 */
static inline void service_mutex_unlock(service_mutex_t *mutex);


/**
 * Try to acquire a mutex using atomic operations.
 *
 * @param mutex         Address of the mutex.
 * @return              0 if the mutex was acquired, 1 otherwise.
 */
static inline int service_mutex_atomic_trylock(service_mutex_t *mutex);


/**
 * Acquire a mutex using atomic operations.
 *
 * @param mutex         Address of the mutex.
 */
static inline void service_mutex_atomic_lock(service_mutex_t *mutex);


/**
 * Release a mutex using atomic operations.
 *
 * @param mutex         Address of the mutex.
 */
static inline void service(service_mutex_t *mutex);

END_C_DECLS

#ifdef __WINDOWS__
#include "mutex_windows.h"
#else
#include "mutex_unix.h"
#endif

BEGIN_C_DECLS

/**
 * Check and see if the process is using multiple threads.
 *
 * @retval true If the process may have more than one thread.
 * @retval false If the process only has a single thread.
 *
 * The value that this function returns is influenced by:
 *
 * - how MPI_INIT or MPI_INIT_THREAD was invoked,
 * - what the final MPI thread level was determined to be,
 * - whether the OMPI or MPI libraries are multi-threaded (Jan 2003:
 *   they're not),
 * - whether configure determined if we have thread support or not
 *
 * MPI_INIT and MPI_INIT_THREAD (specifically, back-end OMPI startup
 * functions) invoke ccs_set_using_threads() to influence the value of
 * this function, depending on their situation. Some examples:
 *
 * - if configure determined that we do not have threads, then this
 * value will always be false.
 *
 * - if MPI_INIT is invoked, and the ompi libraries are [still]
 * single-threaded, this value will be false.
 *
 * - if MPI_INIT_THREAD is invoked with MPI_THREAD_MULTIPLE, we have
 * thread support, and the final thread level is determined to be
 * MPI_THREAD_MULTIPLE, this value will be true.
 *
 * - if the process is a single-threaded OMPI executable (e.g., mpicc),
 * this value will be false.
 *
 * Hence, this function will return false if there is guaranteed to
 * only be one thread in the process.  If there is even the
 * possibility that we may have multiple threads, true will be
 * returned.
 */
#define ccs_using_threads()  ccs_uses_threads

/**
 * Set whether the process is using multiple threads or not.
 *
 * @param have Boolean indicating whether the process is using
 * multiple threads or not.
 *
 * @retval ccs_using_threads The new return value from
 * ccs_using_threads().
 *
 * This function is used to influence the return value of
 * ccs_using_threads().  If configure detected that we have thread
 * support, the return value of future invocations of
 * ccs_using_threads() will be the parameter's value.  If configure
 * detected that we have no thread support, then the retuen from
 * ccs_using_threads() will always be false.
 */
static inline bool ccs_set_using_threads(bool have)
{
#if CCS_ENABLE_MULTI_THREADS
    ccs_uses_threads = have;
#else
    have = have;               /* just shut up the compiler */
    ccs_uses_threads = false;
#endif
    return ccs_uses_threads;
}


/**
 * Lock a mutex if ccs_using_threads() says that multiple threads may
 * be active in the process.
 *
 * @param mutex Pointer to a service_mutex_t to lock.
 *
 * If there is a possibility that multiple threads are running in the
 * process (as determined by ccs_using_threads()), this function will
 * block waiting to lock the mutex.
 *
 * If there is no possibility that multiple threads are running in the
 * process, return immediately.
 */

#if CCS_ENABLE_MULTI_THREADS
#define CCS_THREAD_LOCK(mutex)                 \
    do {                                        \
        if (ccs_using_threads()) {             \
            service_mutex_lock(mutex);             \
        }                                       \
    } while (0)
#elif CCS_ENABLE_DEBUG
#define CCS_THREAD_LOCK(mutex)                                         \
    do {                                                                \
        (mutex)->m_lock_debug++;                                        \
        if (service_mutex_check_locks && 1 != (mutex)->m_lock_debug) {     \
            service_output(0, "Warning -- mutex already locked at %s:%d,"  \
                        " now at %s:%d",                                \
                        (mutex)->m_lock_file,                           \
                        (mutex)->m_lock_line,                           \
                        __FILE__, __LINE__);                            \
        }                                                               \
        (mutex)->m_lock_file = __FILE__;                                \
        (mutex)->m_lock_line = __LINE__;                                \
    } while (0)
#else
#define CCS_THREAD_LOCK(mutex)
#endif


/**
 * Try to lock a mutex if ccs_using_threads() says that multiple
 * threads may be active in the process.
 *
 * @param mutex Pointer to a service_mutex_t to trylock
 *
 * If there is a possibility that multiple threads are running in the
 * process (as determined by ccs_using_threads()), this function will
 * trylock the mutex.
 *
 * If there is no possibility that multiple threads are running in the
 * process, return immediately without modifying the mutex.
 *
 * Returns 0 if mutex was locked, non-zero otherwise.
 */
#if CCS_ENABLE_MULTI_THREADS
#define CCS_THREAD_TRYLOCK(mutex) (ccs_using_threads() ? service_mutex_trylock(mutex) : 0)
#elif CCS_ENABLE_DEBUG
static inline int
service_thread_debug_trylock(service_mutex_t *mutex, const char *file, int line)
{
    int ret = -1;

    if (0 == (mutex)->m_lock_debug) {
        (mutex)->m_lock_debug++;
        (mutex)->m_lock_file = file;
        (mutex)->m_lock_line = line;
        ret = 0;
    } else {
        if (service_mutex_check_locks) {
            service_output(0, "Warning -- during trylock, mutex already locked at %s:%d "
                        "now at %s:%d",  
                        file, line,
                        (mutex)->m_lock_file,
                        (mutex)->m_lock_line);
        }
    }

    return ret;
}
#define CCS_THREAD_TRYLOCK(mutex) service_thread_debug_trylock(mutex, __FILE__, __LINE__)
#else
#define CCS_THREAD_TRYLOCK(mutex) 0
#endif


/**
 * Unlock a mutex if ccs_using_threads() says that multiple threads
 * may be active in the process.
 *
 * @param mutex Pointer to a service_mutex_t to unlock.
 *
 * If there is a possibility that multiple threads are running in the
 * process (as determined by ccs_using_threads()), this function will
 * unlock the mutex.
 *
 * If there is no possibility that multiple threads are running in the
 * process, return immediately without modifying the mutex.
 */
#if CCS_ENABLE_MULTI_THREADS
#define CCS_THREAD_UNLOCK(mutex)               \
    do {                                        \
        if (ccs_using_threads()) {             \
            service_mutex_unlock(mutex);           \
        }                                       \
    } while (0)
#elif CCS_ENABLE_DEBUG
#define CCS_THREAD_UNLOCK(mutex)                                       \
    do {                                                                \
        (mutex)->m_lock_debug--;                                        \
        if (service_mutex_check_locks && 0 > (mutex)->m_lock_debug) {      \
            service_output(0, "Warning -- mutex was double locked from %s:%d", \
                        __FILE__, __LINE__);                            \
        } else if (service_mutex_check_locks && 0 > (mutex)->m_lock_debug) { \
            service_output(0, "Warning -- mutex not locked from %s:%d",    \
                        __FILE__, __LINE__);                            \
        } else {                                                        \
            (mutex)->m_lock_file = NULL;                                \
            (mutex)->m_lock_line = 0;                                   \
        }                                                               \
    } while (0)
#else
#define CCS_THREAD_UNLOCK(mutex)
#endif


/**
 * Lock a mutex if ccs_using_threads() says that multiple threads may
 * be active in the process for the duration of the specified action.
 *
 * @param mutex    Pointer to a service_mutex_t to lock.
 * @param action   A scope over which the lock is held.
 *
 * If there is a possibility that multiple threads are running in the
 * process (as determined by ccs_using_threads()), this function will
 * acquire the lock before invoking the specified action and release
 * it on return.
 *
 * If there is no possibility that multiple threads are running in the
 * process, invoke the action without acquiring the lock.
 */

#if CCS_ENABLE_MULTI_THREADS
#define CCS_THREAD_SCOPED_LOCK(mutex, action)  \
    do {                                        \
        if(ccs_using_threads()) {              \
            service_mutex_lock(mutex);             \
            (action);                           \
            service_mutex_unlock(mutex);           \
        } else {                                \
            (action);                           \
        }                                       \
    } while (0)
#elif CCS_ENABLE_DEBUG
#define CCS_THREAD_SCOPED_LOCK(mutex, action)                          \
    do {                                                                \
        if (0 != (mutex)->m_lock_debug) {                               \
            service_output(0, "scoped_lock: Warning -- mutex already "     \
                        "locked at %s:%d, now at %s:%d",                \
                        __FILE__, __LINE__,                             \
                        (mutex)->m_lock_file,                           \
                        (mutex)->m_lock_line);                          \
        }                                                               \
        (mutex)->m_lock_debug--;                                        \
        (action);                                                       \
        (mutex)->m_lock_debug++;                                        \
    } while (0)
#else
#define CCS_THREAD_SCOPED_LOCK(mutex, action) (action)
#endif

/**
 * Use an atomic operation for increment/decrement if ccs_using_threads()
 * indicates that threads are in use by the application or library.
 */

#if CCS_ENABLE_MULTI_THREADS
#define CCS_THREAD_ADD32(x,y) \
   (ccs_using_threads() ? service_atomic_add_32(x,y) : (*x += y))
#else
#define CCS_THREAD_ADD32(x,y) (*x += y)
#endif

#if CCS_ENABLE_MULTI_THREADS
#define CCS_THREAD_ADD64(x,y) \
    (ccs_using_threads() ? service_atomic_add_64(x,y) : (*x += y))
#else
#define CCS_THREAD_ADD64(x,y) (*x += y)
#endif

#if CCS_ENABLE_MULTI_THREADS
#define CCS_THREAD_ADD_SIZE_T(x,y) \
    (ccs_using_threads() ? service_atomic_add_size_t(x,y) : (*x += y))
#else
#define CCS_THREAD_ADD_SIZE_T(x,y) (*x += y)
#endif

#define CCS_CMPSET(x, y, z) ((*(x) == (y)) ? ((*(x) = (z)), 1) : 0)

#if CCS_ENABLE_MULTI_THREADS
# if CCS_HAVE_ATOMIC_CMPSET_32
#  define CCS_ATOMIC_CMPSET_32(x, y, z) \
    (ccs_using_threads() ? service_atomic_cmpset_32(x, y, z) : CCS_CMPSET(x, y, z))
# endif
# if CCS_HAVE_ATOMIC_CMPSET_64
#  define CCS_ATOMIC_CMPSET_64(x, y, z) \
    (ccs_using_threads() ? serivce_atomic_cmpset_64(x, y, z) : CCS_CMPSET(x, y, z))
# endif
# if CCS_HAVE_ATOMIC_CMPSET_32 || CCS_HAVE_ATOMIC_CMPSET_64
#  define CCS_ATOMIC_CMPSET(x, y, z) \
    (ccs_using_threads() ? service_atomic_cmpset(x, y, z) : CCS_CMPSET(x, y, z))
# endif
#else
# if CCS_HAVE_ATOMIC_CMPSET_32
#  define CCS_ATOMIC_CMPSET_32(x, y, z) CCS_CMPSET(x, y, z)
# endif
# if CCS_HAVE_ATOMIC_CMPSET_64
#  define CCS_ATOMIC_CMPSET_64(x, y, z) CCS_CMPSET(x, y, z)
# endif
# if CCS_HAVE_ATOMIC_CMPSET_32 || CCS_HAVE_ATOMIC_CMPSET_64
#  define CCS_ATOMIC_CMPSET(x, y, z) CCS_CMPSET(x, y, z)
# endif
#endif

END_C_DECLS

#endif                          /* CCS_MUTEX_H */
