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

#include "types.h"
#include "globals.h"
#include "errno.h"

#include "util/debug.h"
#include "util/string.h"

#include "proc/proc.h"
#include "proc/kthread.h"

#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/page.h"
#include "mm/pframe.h"
#include "mm/mmobj.h"
#include "mm/pagetable.h"
#include "mm/tlb.h"

#include "fs/file.h"
#include "fs/vnode.h"

#include "vm/shadow.h"
#include "vm/vmmap.h"

#include "api/exec.h"

#include "main/interrupt.h"

/* Pushes the appropriate things onto the kernel stack of a newly forked thread
 * so that it can begin execution in userland_entry.
 * regs: registers the new thread should have on execution
 * kstack: location of the new thread's kernel stack
 * Returns the new stack pointer on success. */
static uint32_t
fork_setup_stack(const regs_t *regs, void *kstack)
{
        /* Pointer argument and dummy return address, and userland dummy return
         * address */
        uint32_t esp = ((uint32_t) kstack) + DEFAULT_STACK_SIZE - (sizeof(regs_t) + 12);
        *(void **)(esp + 4) = (void *)(esp + 8); /* Set the argument to point to location of struct on stack */
        memcpy((void *)(esp + 8), regs, sizeof(regs_t)); /* Copy over struct */
        return esp;
}


/*
 * The implementation of fork(2). Once this works,
 * you're practically home free. This is what the
 * entirety of Weenix has been leading up to.
 * Go forth and conquer.
 */
int
do_fork(struct regs *regs)
{
     KASSERT(regs != NULL);
     dbg(DBG_PRINT, "(GRADING3A 7.a) registers are not NULL\n");
    KASSERT(curproc != NULL);
     dbg(DBG_PRINT, "(GRADING3A 7.a) current process is not NULL\n");
     KASSERT(curproc->p_state == PROC_RUNNING);
     dbg(DBG_PRINT, "(GRADING3A 7.a) current processes state is running\n");

    proc_t *newproc = proc_create("fork");
    KASSERT(newproc->p_state == PROC_RUNNING);
       dbg(DBG_PRINT, "(GRADING3A 7.a) new processes state is running\n");

    /*vmmap_destroy(newproc->p_vmmap);*/
    vmmap_t *newvmap = vmmap_clone(curproc->p_vmmap);
    vmarea_t *vma;

    list_iterate_begin(&curproc->p_vmmap->vmm_list,vma,vmarea_t,vma_plink)
    {
    dbg(DBG_PRINT,"(GRADING3B 6) iterating through the vmm list of current processes vmmap \n");   
dbg(DBG_VM,"grade25\n");
        if(!(vma->vma_flags & MAP_SHARED)) 
        {
            dbg(DBG_VM,"grade26\n");
                  dbg(DBG_PRINT,"(GRADING3B 6) flag is set to PRIVATE\n");

            mmobj_t *child = shadow_create();
            mmobj_t *parent =  shadow_create();
            vmarea_t * pvma = vmmap_lookup(newvmap,vma->vma_start);

            mmobj_t* tempmmobj = vma->vma_obj;
            mmobj_t* bottomObj = mmobj_bottom_obj(tempmmobj);
            child->mmo_un.mmo_bottom_obj = bottomObj; 
            list_insert_head(&(bottomObj->mmo_un.mmo_vmas),&(pvma->vma_olink));
            
            parent->mmo_shadowed = tempmmobj;
            child->mmo_shadowed = tempmmobj;

            parent->mmo_un.mmo_bottom_obj = bottomObj;

            pvma->vma_obj = child;
            vma->vma_obj = parent;

      
        }
        else
        {
            dbg(DBG_VM,"grade27\n");
                  dbg(DBG_PRINT,"(GRADING3B 6) flag is set to SHARED\n");

      
            continue;
        }


    }list_iterate_end();
 newproc->p_vmmap = newvmap;
        newvmap->vmm_proc = newproc;
  

    int i;
        for(i = 0 ;i<NFILES ;i++)
        {
            if(curproc->p_files[i]!= NULL)
            {
                dbg(DBG_VM,"grade28\n");
                            dbg(DBG_PRINT,"(GRADING3B 6) curproc has open files associated with it\n");

                newproc->p_files[i] = curproc->p_files[i];
                fref(newproc->p_files[i]);
            }
            else
            {            dbg(DBG_PRINT,"(GRADING3B 6) curproc has no open file\n");
                dbg(DBG_VM,"grade29\n");
                newproc->p_files[i] = NULL;
            }
        }
        dbg(DBG_VM,"grade30\n");
          dbg(DBG_PRINT,"(GRADING3B 6) cloning the current thread\n");

    kthread_t *newthr = kthread_clone(curthr);
    newthr->kt_proc = newproc;
  list_insert_tail(&newproc->p_threads, &newthr->kt_plink);
  pt_unmap_range(curproc->p_pagedir,USER_MEM_LOW,USER_MEM_HIGH);
    tlb_flush_all();
    /*context_setup(&(thr->kt_ctx),userland_entry,1,(void *)regs,thr->kt_kstack,DEFAULT_STACK_SIZE,newproc->p_pagedir);*/

    KASSERT(newproc->p_pagedir != NULL);
   dbg(DBG_PRINT, "(GRADING3A 7.a) new processes page table is not NULL\n");
KASSERT(newthr->kt_kstack != NULL);
   dbg(DBG_PRINT, "(GRADING3A 7.a) new threads kstack is not NULL\n");
        newthr->kt_ctx.c_pdptr = newproc->p_pagedir;
                newthr->kt_ctx.c_eip = (uintptr_t)userland_entry;
        newthr->kt_ctx.c_ebp = curthr->kt_ctx.c_ebp;
        regs->r_eax=0;

        newthr->kt_ctx.c_esp = fork_setup_stack(regs,  newthr->kt_kstack);
        
newproc->p_brk = curproc->p_brk;
        newproc->p_start_brk = curproc->p_start_brk;
          dbg(DBG_PRINT,"(GRADING3B 6) making the new thread runnable\n");

        sched_make_runnable(newthr);

        return newproc->p_pid;
        /*NOT_YET_IMPLEMENTED("VM: do_fork");
        return 0;*/
     
}
