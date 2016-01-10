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

#include "mm/mm.h"
#include "mm/page.h"
#include "mm/mman.h"

#include "vm/mmap.h"
#include "vm/vmmap.h"

#include "proc/proc.h"

/*
 * This function implements the brk(2) system call.
 *
 * This routine manages the calling process's "break" -- the ending address
 * of the process's "dynamic" region (often also referred to as the "heap").
 * The current value of a process's break is maintained in the 'p_brk' member
 * of the proc_t structure that represents the process in question.
 *
 * The 'p_brk' and 'p_start_brk' members of a proc_t struct are initialized
 * by the loader. 'p_start_brk' is subsequently never modified; it always
 * holds the initial value of the break. Note that the starting break is
 * not necessarily page aligned!
 *
 * 'p_start_brk' is the lower limit of 'p_brk' (that is, setting the break
 * to any value less than 'p_start_brk' should be disallowed).
 *
 * The upper limit of 'p_brk' is defined by the minimum of (1) the
 * starting address of the next occuring mapping or (2) USER_MEM_HIGH.
 * That is, growth of the process break is limited only in that it cannot
 * overlap with/expand into an existing mapping or beyond the region of
 * the address space allocated for use by userland. (note the presence of
 * the 'vmmap_is_range_empty' function).
 *
 * The dynamic region should always be represented by at most ONE vmarea.
 * Note that vmareas only have page granularity, you will need to take this
 * into account when deciding how to set the mappings if p_brk or p_start_brk
 * is not page aligned.
 *
 * You are guaranteed that the process data/bss region is non-empty.
 * That is, if the starting brk is not page-aligned, its page has
 * read/write permissions.
 *
 * If addr is NULL, you should NOT fail as the man page says. Instead,
 * "return" the current break. We use this to implement sbrk(0) without writing
 * a separate syscall. Look in user/libc/syscall.c if you're curious.
 *
 * Also, despite the statement on the manpage, you MUST support combined use
 * of brk and mmap in the same process.
 *
 * Note that this function "returns" the new break through the "ret" argument.
 * Return 0 on success, -errno on failure.
 */
int
do_brk(void *addr, void **ret)
{
KASSERT(ret != NULL);
if (addr == NULL || addr == curproc->p_brk){
dbg(DBG_PRINT, "(GRADING3D 2), Address is null or address is equal to curproc->p_brk\n");
*ret = curproc->p_brk;
return 0;
}
if (addr < curproc->p_start_brk || (uint32_t) addr > USER_MEM_HIGH){
dbg(DBG_PRINT, "(GRADING3D 2), Address passed is invalid\n");
return -ENOMEM;
}
vmarea_t *vma;
uint32_t newbrk = ADDR_TO_PN(PAGE_ALIGN_UP(addr));
uint32_t oldbrk = ADDR_TO_PN(PAGE_ALIGN_UP(curproc->p_brk));
uint32_t  brkvfn = ADDR_TO_PN(curproc->p_start_brk);
 vma = vmmap_lookup(curproc->p_vmmap,brkvfn);
       
        if(addr < curproc->p_start_brk || newbrk > MIN((vma->vma_end)+1,USER_MEM_HIGH))
                 {
                  dbg(DBG_PRINT, "(GRADING3D 2), Invalid address\n"); 
            return -ENOMEM;
                 }
         
if (ADDR_TO_PN(PAGE_ALIGN_UP(addr)) != ADDR_TO_PN(PAGE_ALIGN_UP(curproc->p_brk))){
    dbg(DBG_PRINT, "(GRADING3D 2), Given address is not equal to the current process p_brk\n");
 if (addr <= curproc->p_brk){
     dbg(DBG_PRINT, "(GRADING3D 2), Given address is less than or equal to current process p_brk\n");

vmmap_remove(curproc->p_vmmap, newbrk, oldbrk - newbrk);

} else {
KASSERT(addr > curproc->p_brk);
dbg(DBG_PRINT, "(GRADING3D 2), Address is greater than current process p_brk\n");

if (!vmmap_is_range_empty(curproc->p_vmmap, oldbrk, newbrk - oldbrk)){
  dbg(DBG_PRINT, "(GRADING3D 2), The given map doesnt have any empty memory\n");
return -ENOMEM;
}
if (vma == NULL){
 dbg(DBG_PRINT, "(GRADING3D 2), Given vmarea is NULL\n");
vmmap_map(curproc->p_vmmap, NULL, ADDR_TO_PN(PAGE_ALIGN_UP(curproc->p_start_brk)),
newbrk - ADDR_TO_PN(PAGE_ALIGN_UP(curproc->p_start_brk)),
PROT_READ | PROT_WRITE,
MAP_PRIVATE, 0, VMMAP_DIR_LOHI, &vma);
} else {
  dbg(DBG_PRINT, "(GRADING3D 2), Vmarea is not NULL\n");

vma->vma_end = newbrk;
}
}
}
curproc->p_brk = addr;
*ret = addr;
return 0;
}



