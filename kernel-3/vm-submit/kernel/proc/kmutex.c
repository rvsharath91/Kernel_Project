/******************************************************************************/
/* Important Fall 2015 CSCI 402 usage information:                            */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

#include "globals.h"
#include "errno.h"

#include "util/debug.h"

#include "proc/kthread.h"
#include "proc/kmutex.h"

/*
 * IMPORTANT: Mutexes can _NEVER_ be locked or unlocked from an
 * interrupt context. Mutexes are _ONLY_ lock or unlocked from a
 * thread context.
 */

void
kmutex_init(kmutex_t *mtx)
{
    /*NOT_YET_IMPLEMENTED("PROCS: kmutex_init");*/
	mtx->km_holder = NULL;
	sched_queue_init(&mtx->km_waitq);
}

/*
 * This should block the current thread (by sleeping on the mutex's
 * wait queue) if the mutex is already taken.
 *
 * No thread should ever try to lock a mutex it already has locked.
 */
void
kmutex_lock(kmutex_t *mtx)
{
	KASSERT(curthr && (curthr != mtx->km_holder));
	dbg(DBG_PRINT,"(GRADING1A 5.a) curthr %s: %d does not hold mutex\n", curthr->kt_proc->p_comm, curthr->kt_proc->p_pid);

	if(mtx->km_holder == NULL)
	{
		dbg(DBG_PRINT,"(GRADING1C 7)Thread '%s: %d' acquired the mutex\n", curthr->kt_proc->p_comm, curthr->kt_proc->p_pid);

		mtx->km_holder = curthr;
        /*NOT_YET_IMPLEMENTED("PROCS: kmutex_lock");*/
	}
	else
	{
		dbg(DBG_PRINT,"(GRADING1C 7)Thread '%s' blocked on the mutex\n", curthr->kt_proc->p_comm);

		sched_sleep_on(&mtx->km_waitq);
	}
}

/*
 * This should do the same as kmutex_lock, but use a cancellable sleep
 * instead.
 */
int
kmutex_lock_cancellable(kmutex_t *mtx)
{
        /*NOT_YET_IMPLEMENTED("PROCS: kmutex_lock_cancellable");*/
	KASSERT(curthr && (curthr != mtx->km_holder));
	dbg(DBG_PRINT,"(GRADING1A 5.b) curthr %s: %d does not hold mutex\n", curthr->kt_proc->p_comm, curthr->kt_proc->p_pid);
	dbg(DBG_PRINT,"(GRADING1A 5.b) curthr %s: %d does not hold mutex\n", curthr->kt_proc->p_comm, curthr->kt_proc->p_pid);

	if(mtx->km_holder == NULL)
	{
		dbg(DBG_PRINT,"(GRADING1C 7)kmutex_lock_cancellable(): Thread '%s: %d' acquired the mutex\n", curthr->kt_proc->p_comm, curthr->kt_proc->p_pid);

		mtx->km_holder = curthr;
        /*NOT_YET_IMPLEMENTED("PROCS: kmutex_lock");*/
	}
	else
	{	
		dbg(DBG_PRINT,"(GRADING1C 7)kmutex_lock_cancellable(): Thread '%s: %d' blocked on the mutex\n", curthr->kt_proc->p_comm, curthr->kt_proc->p_pid);

		return sched_cancellable_sleep_on(&mtx->km_waitq);
	}
        return 0;
}

/*
 * If there are any threads waiting to take a lock on the mutex, one
 * should be woken up and given the lock.
 *
 * Note: This should _NOT_ be a blocking operation!
 *
 * Note: Don't forget to add the new owner of the mutex back to the
 * run queue.
 *
 * Note: Make sure that the thread on the head of the mutex's wait
 * queue becomes the new owner of the mutex.
 *
 * @param mtx the mutex to unlock
 */
void
kmutex_unlock(kmutex_t *mtx)
{
      /*  NOT_YET_IMPLEMENTED("PROCS: kmutex_unlock");*/
	KASSERT(curthr && (curthr == mtx->km_holder));
	dbg(DBG_PRINT,"(GRADING1A 5.c) curthr %s: %d holds the mutex\n", curthr->kt_proc->p_comm, curthr->kt_proc->p_pid);

	mtx->km_holder = NULL;

	if(mtx->km_waitq.tq_size!=0)
	{
		mtx->km_holder = sched_wakeup_on(&mtx->km_waitq);

		
		dbg(DBG_PRINT,"(GRADING1C 7)kmutex_unlock(): Thread '%s: %d' released the mutex\n", curthr->kt_proc->p_comm, curthr->kt_proc->p_pid);

	}
	
	    KASSERT(curthr != mtx->km_holder);
		dbg(DBG_PRINT,"(GRADING1A.5.c) curthr %s: %d does not hold the mutex\n", curthr->kt_proc->p_comm, curthr->kt_proc->p_pid);
	   
}
