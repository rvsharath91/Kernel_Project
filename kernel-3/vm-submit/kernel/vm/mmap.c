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
#include "types.h"

#include "mm/mm.h"
#include "mm/tlb.h"
#include "mm/mman.h"
#include "mm/page.h"

#include "proc/proc.h"

#include "util/string.h"
#include "util/debug.h"

#include "fs/vnode.h"
#include "fs/vfs.h"
#include "fs/file.h"
#include "fs/stat.h"

#include "vm/vmmap.h"
#include "vm/mmap.h"

/*
 * This function implements the mmap(2) syscall, but only
 * supports the MAP_SHARED, MAP_PRIVATE, MAP_FIXED, and
 * MAP_ANON flags.
 *
 * Add a mapping to the current process's address space.
 * You need to do some error checking; see the ERRORS section
 * of the manpage for the problems you should anticipate.
 * After error checking most of the work of this function is
 * done by vmmap_map(), but remember to clear the TLB.
 */

int
do_mmap(void *addr, size_t len, int prot, int flags,
        int fd, off_t off, void **ret)
{
 

   vnode_t *vn;
  /* checking each field if it is valid or not*/
    if(len <= 0)
    {
        dbg(DBG_PRINT, "(GRADING3D 2), Invalid length\n");
        return -EINVAL;
    }
    if(len > USER_MEM_HIGH)
    {
        dbg(DBG_PRINT, "(GRADING3D 2), Invalid length\n");
        return -EINVAL;
    }
    if((len + (uint32_t)addr) > USER_MEM_HIGH)
    {
        dbg(DBG_PRINT, "(GRADING3D 2), Range of address to be mapped exceeds maximum memory\n");
        return -EINVAL;
    }
    /*if(!PAGE_ALIGNED(len))
    {
        dbg(DBG_PRINT, "(GRADING3D 2), given len is not page aligned\n");
        return -EINVAL;
    }*/
    if(!PAGE_ALIGNED(off))
    {
        dbg(DBG_PRINT, "(GRADING3D 2), offset is not page aligned\n");
        return -EINVAL;
    }
     if (!(flags & MAP_ANON) && (flags & MAP_FIXED) && !PAGE_ALIGNED(addr))
    {
       dbg(DBG_PRINT, "(GRADING3D 2), Invalid flag and address not page aligned\n");
       return -EINVAL; 
    }
    if(((flags & MAP_SHARED)!= MAP_SHARED) && ((flags & MAP_PRIVATE) != MAP_PRIVATE))
    {
        dbg(DBG_PRINT, "(GRADING3D 2), flag is neither map shared nor map private\n");
        return -EINVAL;
    }

    /*if(((flags & MAP_TYPE)!= MAP_SHARED) || ((flags & MAP_TYPE) != MAP_PRIVATE))
        return -EINVAL;*/

    /* According to man page, if addr is null then the flag cant be MAP_FIXED*/

    if(addr == NULL && (flags & MAP_FIXED))
    {
        dbg(DBG_PRINT, "(GRADING3D 2), address cannot be NULL and flag cannot be map fixed\n");
        return -EINVAL;
    }

    if(addr != NULL && (uint32_t) addr < USER_MEM_LOW)
    {
       dbg(DBG_PRINT, "(GRADING3D 2), address cannot be lower than user memory low\n");
       return - EINVAL;
    }


    if (!(flags & MAP_ANON))
    {
      dbg(DBG_PRINT, "(GRADING3D 2), flag passed is not map annon\n");
     if(curproc->p_files[fd] == NULL)
     {
        dbg(DBG_PRINT, "(GRADING3D 2), Invalid file descriptor\n");
        return -EBADF;
     }
     if(fd < 0 || fd > NFILES)
     {
        dbg(DBG_PRINT, "(GRADING3D 2), Invalid file descriptor\n");
        return -EBADF;
     }
    

    file_t *f = curproc->p_files[fd];
    vn = f->f_vnode;
    if ((flags & MAP_PRIVATE) && !(f->f_mode & FMODE_READ))
     {
       dbg(DBG_PRINT, "(GRADING3D 2), The given file doesnt have read permission\n");
       return -EACCES;
     }
if ((flags & MAP_SHARED) && (prot & PROT_WRITE) &&
!((f->f_mode & FMODE_READ) && (f->f_mode & FMODE_WRITE)))
{
  dbg(DBG_PRINT, "(GRADING3D 2), Invalid file descriptor\n");
  return -EACCES;
}

    /* get the vnode with the help of given fd*/

      
     }
    else
     {
       dbg(DBG_PRINT, "(GRADING3D 2), given flag is MAP_ANON\n");
       vn = NULL;
     }
       /* call vmmap_map to get the vma corresponding to the given addr*/

        vmarea_t *map_vma;
        int vmret = vmmap_map(curproc->p_vmmap, vn,ADDR_TO_PN(addr),(uint32_t)PAGE_ALIGN_UP(len)/PAGE_SIZE, prot,flags,off,VMMAP_DIR_HILO,&map_vma);

        if(ret != NULL && vmret >=0)
        {
         dbg(DBG_PRINT, "(GRADING3D 2), successfully mapped using vmmap_map\n");
        /*insert the start address of the vma into given ret field*/
        *ret = PN_TO_ADDR(map_vma->vma_start);
        
        pt_unmap_range(curproc->p_pagedir, (uintptr_t) PN_TO_ADDR(map_vma->vma_start),(uintptr_t) PN_TO_ADDR(map_vma->vma_start) + (uintptr_t) PAGE_ALIGN_UP(len));

        /*Flush the TLB*/
        tlb_flush_range((uintptr_t) PN_TO_ADDR(map_vma->vma_start),(uint32_t)PAGE_ALIGN_UP(len)/PAGE_SIZE );
        }
        KASSERT(NULL != curproc->p_pagedir);
        dbg(DBG_PRINT,"(GRADING3A 3.a) Pagedir of the curproc is not null\n");
        return vmret;
        /*NOT_YET_IMPLEMENTED("VM: do_mmap");
        return -1;*/
       
}



/*
 * This function implements the munmap(2) syscall.
 *
 * As with do_mmap() it should perform the required error checking,
 * before calling upon vmmap_remove() to do most of the work.
 * Remember to clear the TLB.
 */
int
do_munmap(void *addr, size_t len)
{

dbg(DBG_PRINT, "(GRADING3D 2), Entered do_munmap\n");
if(len == 0 || len == (size_t) - 1){
        dbg(DBG_PRINT, "(GRADING3D 2), Length Invalid\n");      
        return -EINVAL;
    }
  
if ((uintptr_t)addr < USER_MEM_LOW || (uintptr_t)addr > USER_MEM_HIGH) {
                dbg(DBG_PRINT, "(GRADING3D 2), invalid length and address\n");
                return -EINVAL;
    }

if (len+(uintptr_t)addr < USER_MEM_LOW || len+(uintptr_t)addr > USER_MEM_HIGH) {
                dbg(DBG_PRINT, "(GRADING3D 5), invalid length and address\n");
                return -EINVAL;
    }

    uint32_t num = 0;
    num = (uint32_t)(len/PAGE_SIZE);
    len = len%PAGE_SIZE;
    if(len)
        num++;

vmmap_remove(curproc->p_vmmap, ADDR_TO_PN(addr),(uint32_t) num);
     tlb_flush_range((uintptr_t)addr,num);
     
     if ((num+ (uintptr_t)PN_TO_ADDR(ADDR_TO_PN(addr)) <= USER_MEM_HIGH)) {
            dbg(DBG_PRINT, "(GRADING3D 2), Adress within the max memory\n");
            uintptr_t add = ADDR_TO_PN(addr) + num;
            pt_unmap_range(pt_get(),(uintptr_t)addr,(uintptr_t)PN_TO_ADDR(add));
        }
       
     return 0;
        /*NOT_YET_IMPLEMENTED("VM: do_munmap");
        return -1;*/

}

