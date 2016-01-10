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

#include "kernel.h"
#include "errno.h"
#include "globals.h"

#include "vm/vmmap.h"
#include "vm/shadow.h"
#include "vm/anon.h"

#include "proc/proc.h"

#include "util/debug.h"
#include "util/list.h"
#include "util/string.h"
#include "util/printf.h"

#include "fs/vnode.h"
#include "fs/file.h"
#include "fs/fcntl.h"
#include "fs/vfs_syscall.h"

#include "mm/slab.h"
#include "mm/page.h"
#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/mmobj.h"

static slab_allocator_t *vmmap_allocator;
static slab_allocator_t *vmarea_allocator;

void
vmmap_init(void)
{
        vmmap_allocator = slab_allocator_create("vmmap", sizeof(vmmap_t));
        KASSERT(NULL != vmmap_allocator && "failed to create vmmap allocator!");
        vmarea_allocator = slab_allocator_create("vmarea", sizeof(vmarea_t));
        KASSERT(NULL != vmarea_allocator && "failed to create vmarea allocator!");
}

vmarea_t *
vmarea_alloc(void)
{
        vmarea_t *newvma = (vmarea_t *) slab_obj_alloc(vmarea_allocator);
        if (newvma) {
                newvma->vma_vmmap = NULL;
        }
        return newvma;
}

void
vmarea_free(vmarea_t *vma)
{
        KASSERT(NULL != vma);
        slab_obj_free(vmarea_allocator, vma);
}

/* a debugging routine: dumps the mappings of the given address space. */
size_t
vmmap_mapping_info(const void *vmmap, char *buf, size_t osize)
{
        KASSERT(0 < osize);
        KASSERT(NULL != buf);
        KASSERT(NULL != vmmap);

        vmmap_t *map = (vmmap_t *)vmmap;
        vmarea_t *vma;
        ssize_t size = (ssize_t)osize;

        int len = snprintf(buf, size, "%21s %5s %7s %8s %10s %12s\n",
                           "VADDR RANGE", "PROT", "FLAGS", "MMOBJ", "OFFSET",
                           "VFN RANGE");

        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) {
                size -= len;
                buf += len;
                if (0 >= size) {
                        goto end;
                }

                len = snprintf(buf, size,
                               "%#.8x-%#.8x  %c%c%c  %7s 0x%p %#.5x %#.5x-%#.5x\n",
                               vma->vma_start << PAGE_SHIFT,
                               vma->vma_end << PAGE_SHIFT,
                               (vma->vma_prot & PROT_READ ? 'r' : '-'),
                               (vma->vma_prot & PROT_WRITE ? 'w' : '-'),
                               (vma->vma_prot & PROT_EXEC ? 'x' : '-'),
                               (vma->vma_flags & MAP_SHARED ? " SHARED" : "PRIVATE"),
                               vma->vma_obj, vma->vma_off, vma->vma_start, vma->vma_end);
        } list_iterate_end();

end:
        if (size <= 0) {
                size = osize;
                buf[osize - 1] = '\0';
        }
        /*
        KASSERT(0 <= size);
        if (0 == size) {
                size++;
                buf--;
                buf[0] = '\0';
        }
        */
        return osize - size;
}

/* Create a new vmmap, which has no vmareas and does
 * not refer to a process. */
vmmap_t *
vmmap_create(void)
{
    vmmap_t *vmmap = (vmmap_t *)slab_obj_alloc(vmmap_allocator);
    vmmap->vmm_proc=NULL;
    list_init(&vmmap->vmm_list);
    dbg(DBG_PRINT,"(GRADING3B 1): Creating a new vmmap using vmmap_create function\n");
    return vmmap;

        /*NOT_YET_IMPLEMENTED("VM: vmmap_create");
        return NULL;*/
}

/* Removes all vmareas from the address space and frees the
 * vmmap struct. */
void
vmmap_destroy(vmmap_t *map)
{
    KASSERT(NULL != map);
    dbg(DBG_PRINT,"(GRADING3A 3.a): Removing the valid map in vmmap_destroy function\n");
    dbg(DBG_PRINT,"(GRADING3B 1): Removing the valid map in vmmap_destroy function\n");
    vmarea_t *vma;

    list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) 
    {
        dbg(DBG_PRINT,"(GRADING3B 1): Removing the area (vmmap_destroy)\n");
        vmmap_remove(map,vma->vma_start,vma->vma_end-vma->vma_start);  
    } list_iterate_end();
    
    slab_obj_free(vmmap_allocator, map);
    dbg(DBG_PRINT,"(GRADING3B 1): Done with vmmap_destroy\n");

       /* NOT_YET_IMPLEMENTED("VM: vmmap_destroy");*/
}

/* Add a vmarea to an address space. Assumes (i.e. asserts to some extent)
 * the vmarea is valid.  This involves finding where to put it in the list
 * of VM areas, and adding it. Don't forget to set the vma_vmmap for the
 * area. */
void
vmmap_insert(vmmap_t *map, vmarea_t *newvma)
{

    KASSERT(NULL != map && NULL != newvma);
    dbg(DBG_PRINT,"(GRADING3A 3.b): (vmmap_insert) New area is not NULL and map is not NULL\n");
    KASSERT(NULL == newvma->vma_vmmap);
    dbg(DBG_PRINT,"(GRADING3A 3.b): (vmmap_insert) vmarea is NULL\n");
    KASSERT(newvma->vma_start < newvma->vma_end);
    dbg(DBG_PRINT,"(GRADING3A 3.b): (vmmap_insert) vmarea not a valid start address\n");
    KASSERT(ADDR_TO_PN(USER_MEM_LOW) <= newvma->vma_start && ADDR_TO_PN(USER_MEM_HIGH) >= newvma->vma_end);
    dbg(DBG_PRINT,"(GRADING3A 3.b): (vmmap_insert) vmarea not having valid address range\n");
    dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_insert) vmarea not having valid address range\n");
    vmarea_t *vma;

    newvma->vma_vmmap=map;

    if (list_empty(&map->vmm_list))
        {
             dbg(DBG_VM,"grade54\n");
            dbg(DBG_VM,"grade 1\n");
            dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_insert) vmmap is empty, so inserting the vmarea straight away\n");
            list_insert_tail(&map->vmm_list, &newvma->vma_plink);
            return;
        }
    else
    {
         dbg(DBG_VM,"grade55\n");
        dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_insert) vmmap non-empty, so iterating the vmarea to insert the new one\n");
        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) 
        {
            if ((vma->vma_start > newvma->vma_start))
                {
                    dbg(DBG_PRINT,"(GRADING3D 1):(vmmap_insert) Inserting vmarea at an approprate position\n");
                    list_insert_before(&vma->vma_plink,&newvma->vma_plink);
                    return;
                }

        } list_iterate_end();
 dbg(DBG_VM,"grade56\n");
    dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_insert) Inserting vmarea at last in the list\n");
    list_insert_tail(&map->vmm_list, &newvma->vma_plink);
    }
    return;
 
        
}

/* Find a contiguous range of free virtual pages of length npages in
 * the given address space. Returns starting vfn for the range,
 * without altering the map. Returns -1 if no such range exists.
 *
 * Your algorithm should be first fit. If dir is VMMAP_DIR_HILO, you
 * should find a gap as high in the address space as possible; if dir
 * is VMMAP_DIR_LOHI, the gap should be as low as possible. */
int
vmmap_find_range(vmmap_t *map, uint32_t npages, int dir)
{
    dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_find_range) checking if the given range is available \n");
    vmarea_t *vma;
    int ret;

    if (dir==VMMAP_DIR_HILO)
    { dbg(DBG_VM,"grade57\n");
        dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_find_range) given dir is VMMAP_DIR_HILO \n");
        list_iterate_reverse(&map->vmm_list, vma, vmarea_t, vma_plink) 
        {

            if (vma-> vma_plink.l_next == &(map->vmm_list) && vma->vma_end+npages<= ADDR_TO_PN(USER_MEM_HIGH))
            {
                 dbg(DBG_VM,"grade58\n");
                dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_find_range) checking if the space after last area is empty for the range selected\n");
                return ADDR_TO_PN(USER_MEM_HIGH) - npages;
            }

            if (vma->vma_start - npages >= ADDR_TO_PN(USER_MEM_LOW)) 
            {
                 dbg(DBG_VM,"grade59\n");
                dbg(DBG_PRINT,"(GRADING3D 1): (vmmap_find_range) Checking if selected range is empty\n");
                int ret = vmmap_is_range_empty(map,vma->vma_start-npages,npages);
                if (ret == 1) 
                {
                     dbg(DBG_VM,"grade60\n");
                    dbg(DBG_PRINT,"(GRADING3D 1): (vmmap_find_range) range is available, hence return its starting vfn\n");
                    return vma->vma_start-npages;
                }
            }

        } list_iterate_end();
    }
         dbg(DBG_VM,"grade61\n");
        dbg(DBG_PRINT,"(GRADING3D 2): (vmmap_find_range) range of npages not available in the map\n");
        return -1;
}




/* Find the vm_area that vfn lies in. Simply scan the address space
 * looking for a vma whose range covers vfn. If the page is unmapped,
 * return NULL. */
vmarea_t *
vmmap_lookup(vmmap_t *map, uint32_t vfn)
{
        KASSERT(NULL != map);
        dbg(DBG_PRINT,"(GRADING3A 3.c): (vmmap_lookup) given vmmap is valid \n");
        dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_lookup) given vmmap is valid\n");

        vmarea_t *vma;
        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) 
        {
            if (vma->vma_end > vfn && vfn >=vma->vma_start) /*included =*/
            {
                 dbg(DBG_VM,"grade62\n");
                dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_lookup) vmarea for that vfn is found\n");
                return vma;
            }
        } list_iterate_end();
 dbg(DBG_VM,"grade63\n");
        dbg(DBG_PRINT,"(GRADING3D 1): (vmmap_lookup) vmarea for that vfn is not found\n");
        return NULL;

}

/* Allocates a new vmmap containing a new vmarea for each area in the
 * given map. The areas should have no mmobjs set yet. Returns pointer
 * to the new vmmap on success, NULL on failure. This function is
 * called when implementing fork(2). */
vmmap_t *
vmmap_clone(vmmap_t *map)
{
    dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_clone) Allocates a new vmmap for the given map\n");
    vmmap_t *new_vmmap= vmmap_create();
    vmarea_t *vma;
    list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) 
    {
        dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_clone) setting up a new area for original area of map\n");
            vmarea_t * new_vma= vmarea_alloc();
           /* if (new_vma== NULL)
            {
                 dbg(DBG_VFS,"grade64\n");
                dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_clone) Error in allocation of new vma\n");
                return NULL;
            }*/
            new_vma->vma_start=vma->vma_start;
            new_vma->vma_end=vma->vma_end;
            new_vma->vma_off=vma->vma_off;
            new_vma->vma_prot=vma->vma_prot;
            new_vma->vma_flags=vma->vma_flags;
            /*new_vma->vma_vmmap= new_vmmap;*/
            new_vma->vma_obj = vma->vma_obj;
            new_vma->vma_obj->mmo_ops->ref(new_vma->vma_obj);             
            list_link_init(&new_vma->vma_plink);
            list_link_init(&new_vma->vma_olink);
            vmmap_insert(new_vmmap,new_vma);

    } list_iterate_end();
 dbg(DBG_VM,"grade65\n");
    dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_clone) Successfully cloned the given vmmap\n");
    return new_vmmap;

/*       NOT_YET_IMPLEMENTED("VM: vmmap_clone");
        return NULL;*/
}

/* Insert a mapping into the map starting at lopage for npages pages.
 * If lopage is zero, we will find a range of virtual addresses in the
 * process that is big enough, by using vmmap_find_range with the same
 * dir argument.  If lopage is non-zero and the specified region
 * contains another mapping that mapping should be unmapped.
 *
 * If file is NULL an anon mmobj will be used to create a mapping
 * of 0's.  If file is non-null that vnode's file will be mapped in
 * for the given range.  Use the vnode's mmap operation to get the
 * mmobj for the file; do not assume it is file->vn_obj. Make sure all
 * of the area's fields except for vma_obj have been set before
 * calling mmap.
 *
 * If MAP_PRIVATE is specified set up a shadow object for the mmobj.
 *
 * All of the input to this function should be valid (KASSERT!).
 * See mmap(2) for for description of legal input.
 * Note that off should be page aligned.
 *
 * Be very careful about the order operations are performed in here. Some
 * operation are impossible to undo and should be saved until there
 * is no chance of failure.
 *
 * If 'new' is non-NULL a pointer to the new vmarea_t should be stored in it.
 */
int
vmmap_map(vmmap_t *map, vnode_t *file, uint32_t lopage, uint32_t npages,
          int prot, int flags, off_t off, int dir, vmarea_t **new)
{
    KASSERT(NULL != map);
    dbg(DBG_PRINT,"(GRADING3A 3.d): (vmmap_map) given vmmap is not NULL\n");
    KASSERT(0 < npages);
    dbg(DBG_PRINT,"(GRADING3A 3.d): (vmmap_map) Valid given number of pages \n");
    KASSERT((MAP_SHARED & flags) || (MAP_PRIVATE & flags));
    dbg(DBG_PRINT,"(GRADING3A 3.d): (vmmap_map) Valid flags \n");
    KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_LOW) <= lopage)); 
    dbg(DBG_PRINT,"(GRADING3A 3.d): (vmmap_map) Valid start page number\n");
    KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_HIGH) >= (lopage + npages)));
    dbg(DBG_PRINT,"(GRADING3A 3.d): (vmmap_map) Valid end page number\n");
    KASSERT(PAGE_ALIGNED(off));
    dbg(DBG_PRINT,"(GRADING3A 3.d): (vmmap_map) offset page number is page_aligned address\n");
    int ret=0;
    if (lopage==0)
    {
         dbg(DBG_VM,"grade66\n");
        dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_map) Lopage is NULL\n");
        ret=vmmap_find_range(map,npages, dir);
        if (ret<0)
        {
             dbg(DBG_VM,"grade67\n");
            dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_map) The given range is invalid\n");
            return -ENOMEM; /*check return value later*/
        }
    }
    else
    {
         dbg(DBG_VM,"grade68\n");
        dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_map) given lopage not NULL\n");
        int ret1= vmmap_is_range_empty(map, lopage, npages);
        if (ret1==0)
        {
             dbg(DBG_VM,"grade69\n");
            dbg(DBG_PRINT,"(GRADING3D 1): (vmmap_map) provided range already mapped; removing the map\n");
            vmmap_remove(map, lopage, npages);
            
        }
        ret=lopage;
    }


    if(ret)
    {
        dbg(DBG_PRINT,"(GRADING3D 1): lopage is not zero\n");
        vmarea_t * new_vma= vmarea_alloc();
        new_vma->vma_start=ret;
        new_vma->vma_end=ret+npages;
        new_vma->vma_off=ADDR_TO_PN(off);
        new_vma->vma_prot=prot;
        new_vma->vma_flags=flags;
        /*new_vma->vma_vmmap=map;*/
        list_link_init(&new_vma->vma_olink);
        /*list_init(&new_vma->vma_plink);*/
    
        
        if (file==NULL || flags & MAP_ANON)
        {
            dbg(DBG_VM,"grade70\n");
            dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_map) Either file is equal to NULL or flags is set as MAP_ANON \n");
            mmobj_t *mmobj= anon_create();
            new_vma->vma_obj=mmobj;
        }
        else
        {
            dbg(DBG_VM,"grade71\n");
            dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_map) mapping anon object to given vmarea\n");
            mmobj_t *file_obj;  
            file->vn_ops->mmap(file,new_vma,&file_obj);
            new_vma->vma_obj=file_obj;

        }

        if ( flags & MAP_PRIVATE) 
        {
            dbg(DBG_VM,"grade72\n");
            dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_map) MAP_PRIVATE is set in the flags\n");
            dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_map) mapping shadow object to the given vmarea\n");
            mmobj_t *mmobj_s= shadow_create();
            mmobj_s->mmo_shadowed = new_vma->vma_obj;        
            mmobj_s->mmo_un.mmo_bottom_obj = mmobj_bottom_obj(new_vma->vma_obj);
            list_insert_head(&new_vma->vma_obj->mmo_un.mmo_vmas, &new_vma->vma_olink);
            new_vma->vma_obj=mmobj_s;
        }


        if (new!= NULL)   /*changed the below lines */
        {
            dbg(DBG_VM,"grade73\n");
            dbg(DBG_PRINT,"(GRADING3D 1): (vmmap_map) returning the vmarea after mapping \n");       
            *new=new_vma;
            vmmap_insert(map,new_vma);
            
        }
        else
        {  
        dbg(DBG_VM,"grade74\n");  
        dbg(DBG_PRINT,"(GRADING3B 1): (vmmap_map) Mapping in vmmap_map is done\n");
        vmmap_insert(map,new_vma);   
        }
        
    }
    return 0;

}

/*
 * We have no guarantee that the region of the address space being
 * unmapped will play nicely with our list of vmareas.
 *
 * You must iterate over each vmarea that is partially or wholly covered
 * by the address range [addr ... addr+len). The vm-area will fall into one
 * of four cases, as illustrated below:
 *
 * key:
 *          [             ]   Existing VM Area
 *        *******             Region to be unmapped
 *
 * Case 1:  [   ******    ]
 * The region to be unmapped lies completely inside the vmarea. We need to
 * split the old vmarea into two vmareas. be sure to increment the
 * reference count to the file associated with the vmarea.
 *
 * Case 2:  [      *******]**
 * The region overlaps the end of the vmarea. Just shorten the length of
 * the mapping.
 *
 * Case 3: *[*****        ]
 * The region overlaps the beginning of the vmarea. Move the beginning of
 * the mapping (remember to update vma_off), and shorten its length.
 *
 * Case 4: *[*************]**
 * The region completely contains the vmarea. Remove the vmarea from the
 * list.
 */
int
vmmap_remove(vmmap_t *map, uint32_t lopage, uint32_t npages)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_remove");
        return -1;*/

        vmarea_t *vma;

        if (vmmap_is_range_empty(map, lopage, npages)) 
        {
             dbg(DBG_VM,"grade75\n");
            dbg(DBG_PRINT,"(GRADING3D 2): (vmmap_remove) No mapping in the given range\n");
            return 0;
        }
        dbg(DBG_PRINT,"(GRADING3D 2): (vmmap_remove) mapping is present in the given range\n");
        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink)
        {
             dbg(DBG_VM,"grade76\n");
            dbg(DBG_PRINT,"(GRADING3D 2): (vmmap_remove) Finding the mapping in each vmarea\n");
            if(vma->vma_start < lopage && vma->vma_end <= lopage+npages && vma->vma_end> lopage) /*case 2*/
            {
                 dbg(DBG_VM,"grade77\n");
                dbg(DBG_PRINT,"(GRADING3D 2): (vmmap_remove) vmmap mapping as sepecified in case 2\n");
                vma->vma_end=lopage;
            }

            else if(vma->vma_start >= lopage && vma->vma_end > lopage+npages && vma->vma_start<lopage+npages) /*case 3*/
            {
                 dbg(DBG_VM,"grade78\n");
                dbg(DBG_PRINT,"(GRADING3D 2): (vmmap_remove) vmmap mapping as sepecified in case 3\n");
                vma->vma_off= vma->vma_off + lopage +npages - vma->vma_start; /*added vms_start*/
                vma->vma_start = lopage+npages;
            }

            else if (vma->vma_start < lopage && vma->vma_end > lopage+npages) /*case 1*/
            {
                 dbg(DBG_VM,"grade79\n");
                dbg(DBG_PRINT,"(GRADING3D 2): (vmmap_remove) vmmap mapping as sepecified in case 1\n");
                vmarea_t *vma_clone = vmarea_alloc();
                vma_clone->vma_start=vma->vma_start;
                vma_clone->vma_end= lopage;
                vma_clone->vma_prot=vma->vma_prot;
                vma_clone->vma_flags=vma->vma_flags;
                vma_clone->vma_off=vma->vma_off;
                vma_clone->vma_obj=vma->vma_obj;
                vma_clone->vma_vmmap=vma->vma_vmmap;
                vma_clone->vma_obj->mmo_ops->ref(vma_clone->vma_obj);
                mmobj_t *bottomobj= mmobj_bottom_obj(vma->vma_obj);

                list_link_init(&vma_clone->vma_plink);
                list_link_init(&vma_clone->vma_olink);
                list_insert_before(&vma->vma_plink,&vma_clone->vma_plink);

                if (bottomobj != vma->vma_obj) 
                {
                     dbg(DBG_VM,"grade80\n");
                    dbg(DBG_PRINT,"(GRADING3D 2): (vmmap_remove)Inserting into bottom object list\n");
                    list_insert_head(&bottomobj->mmo_un.mmo_vmas, &vma_clone->vma_olink);
                }
                
                vma->vma_off=vma->vma_off+ lopage+npages- vma->vma_start;
                vma->vma_start = lopage+npages;
                /*vmmap_insert(map,vma_clone);*/
                return 0;


            }

            else if (vma->vma_start>= lopage && vma->vma_end <= lopage+npages) /*case 4*/
            {
                 dbg(DBG_VM,"grade81\n");
                dbg(DBG_PRINT,"(GRADING3D 2): (vmmap_remove) vmmap mapping as sepecified in case 4\n");
                vma->vma_obj->mmo_ops->put(vma->vma_obj);
                list_remove(&vma->vma_plink);
                if (list_link_is_linked(&vma->vma_olink)) 
                {
                     dbg(DBG_VM,"grade82\n");
                     dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_remove) Removing the olink before removing the entire area \n");
                    list_remove(&vma->vma_olink);
                }
                
                vmarea_free(vma);
            }

        }list_iterate_end();
         dbg(DBG_VM,"grade83\n");
    dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_remove) Successfully removed the mapping for the specified region\n");
    return 0;
}

/*
 * Returns 1 if the given address space has no mappings for the
 * given range, 0 otherwise.
 */
int
vmmap_is_range_empty(vmmap_t *map, uint32_t startvfn, uint32_t npages)
{
    KASSERT((startvfn < startvfn+npages) && (ADDR_TO_PN(USER_MEM_LOW) <= startvfn) && (ADDR_TO_PN(USER_MEM_HIGH) >= startvfn+npages));
    dbg(DBG_PRINT,"(GRADING3A 3.e):(vmmap_is_range_empty) given address range is valid\n");
    vmarea_t* vma;

    if (list_empty(&(map->vmm_list)))
    {
         dbg(DBG_VM,"grade84\n");
        dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_is_range_empty)No vmarea in the given vmmap\n");
        return 1;
    }
    dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_is_range_empty) checking for empty range \n");
    list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) 
    {
        if (!(vma->vma_start >= startvfn+npages) && !(vma->vma_end <= startvfn))
        {
             dbg(DBG_VM,"grade85\n");
            dbg(DBG_PRINT,"(GRADING3B):(vmmap_is_range_empty) Range is not empty, mapping found\n");
            return 0;
        }
    } list_iterate_end();
     dbg(DBG_VM,"grade86\n");
    dbg(DBG_PRINT,"(GRADING3B 1), The given range is empty as there are no mappings\n");
    return 1;


}

/* Read into 'buf' from the virtual address space of 'map' starting at
 * 'vaddr' for size 'count'. To do so, you will want to find the vmareas
 * to read from, then find the pframes within those vmareas corresponding
 * to the virtual addresses you want to read, and then read from the
 * physical memory that pframe points to. You should not check permissions
 * of the areas. Assume (KASSERT) that all the areas you are accessing exist.
 * Returns 0 on success, -errno on error.
 */
int
vmmap_read(vmmap_t *map, const void *vaddr, void *buf, size_t count)
{
    dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_read)Reading from the given virtual address space of vmmap\n");    
    uint32_t phypageno;
   /* size_t act_count;*/
    pframe_t *ppage;
    vmarea_t *vma;
    uint32_t vpageno;
    uintptr_t offset;
    /*int res;
    while (count >0)
    {*/
        dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_read) Reading the content into buf from the mapped physical page\n");
        vpageno=ADDR_TO_PN((uint32_t)vaddr);
        offset = PAGE_OFFSET((uint32_t)vaddr);
        vma=vmmap_lookup(map,vpageno);
        phypageno=vma->vma_off +(vpageno-vma->vma_start);   
        pframe_lookup(vma->vma_obj,phypageno,0,&ppage); /*looks for the physical for that page number */
       /* if (res< 0)
        {
             dbg(DBG_VFS,"grade87\n");
            dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_read) Physical page not found\n");
            return res;
        }*/
        dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_read) Physical page found\n");
      /*  if (count < (PAGE_SIZE-offset))
        {
             dbg(DBG_VM,"grade88\n");
            dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_read) count is less than (PAGE_SIZE-offset)\n");
            act_count=count;
        }*/

       /* else
        {
             dbg(DBG_VM,"grade89\n");
            dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_read) count is more than (PAGE_SIZE-offset)\n");
            act_count= PAGE_SIZE-offset;
        }*/

        memcpy((void *)buf,(void *)((uintptr_t)ppage->pf_addr+offset),count); /*type caste done */
       /* count=count-act_count;
        buf=buf+act_count;
        vaddr=vaddr+act_count;
    }*/
         dbg(DBG_VM,"grade90\n");
    dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_read) Successfully read content\n");
    return 0;

        /*NOT_YET_IMPLEMENTED("VM: vmmap_read");
        return 0;*/
}

/* Write from 'buf' into the virtual address space of 'map' starting at
 * 'vaddr' for size 'count'. To do this, you will need to find the correct
 * vmareas to write into, then find the correct pframes within those vmareas,
 * and finally write into the physical addresses that those pframes correspond
 * to. You should not check permissions of the areas you use. Assume (KASSERT)
 * that all the areas you are accessing exist. Remember to dirty pages!
 * Returns 0 on success, -errno on error.
 */
int
vmmap_write(vmmap_t *map, void *vaddr, const void *buf, size_t count)
{
        /*NOT_YET_IMPLEMENTED("VM: vmmap_write");
        return 0;*/
        dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_write)Wrinting into given virtual address space of vmmap\n");
        uint32_t phypageno;
        /*size_t act_count;*/
        pframe_t *ppage;
        vmarea_t *vma;
        uint32_t vpageno;
        uint32_t offset;
        /*int res;*/
    
        dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_write) Writing the content from buf into mapped physical page\n");
        vpageno=ADDR_TO_PN((uint32_t)vaddr);
        offset = PAGE_OFFSET((uint32_t)vaddr);
        vma=vmmap_lookup(map,vpageno);
        phypageno= vma->vma_off+ (vpageno-vma->vma_start);   /*added */
        pframe_lookup(vma->vma_obj,phypageno,1,&ppage);
       /* if (res< 0)
        {
             dbg(DBG_VFS,"grade91\n");
            dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_write) Physical page not found\n");
            
            return res;
        }*/
      /*  if ((PAGE_SIZE-offset) > count)
        {
             dbg(DBG_VM,"grade92\n");
            dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_write) count is less than (PAGE_SIZE-offset)\n");
            act_count=count;
        }    
        else
        {
             dbg(DBG_VM,"grade93\n");
            dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_write) count is more than (PAGE_SIZE-offset)\n");
            act_count= PAGE_SIZE-offset;
        }*/
        memcpy((void *)((uintptr_t)ppage->pf_addr+offset),(void *)buf,count); /*type casting might require */
        pframe_dirty(ppage);
        /*count=count-act_count;
        buf=buf+act_count;
        vaddr=vaddr+act_count;*/
 dbg(DBG_VM,"grade94\n");
    
    dbg(DBG_PRINT,"(GRADING3B 1):(vmmap_write) Successfully WROTE the content\n");
    return 0;
}


