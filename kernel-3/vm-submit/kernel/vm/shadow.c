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

#include "util/string.h"
#include "util/debug.h"

#include "mm/mmobj.h"
#include "mm/pframe.h"
#include "mm/mm.h"
#include "mm/page.h"
#include "mm/slab.h"
#include "mm/tlb.h"

#include "vm/vmmap.h"
#include "vm/shadow.h"
#include "vm/shadowd.h"

#define SHADOW_SINGLETON_THRESHOLD 5

int shadow_count = 0; /* for debugging/verification purposes */
#ifdef __SHADOWD__
/*
 * number of shadow objects with a single parent, that is another shadow
 * object in the shadow objects tree(singletons)
 */
static int shadow_singleton_count = 0;
#endif

static slab_allocator_t *shadow_allocator;

static void shadow_ref(mmobj_t *o);
static void shadow_put(mmobj_t *o);
static int  shadow_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf);
static int  shadow_fillpage(mmobj_t *o, pframe_t *pf);
static int  shadow_dirtypage(mmobj_t *o, pframe_t *pf);
static int  shadow_cleanpage(mmobj_t *o, pframe_t *pf);

static mmobj_ops_t shadow_mmobj_ops = {
        .ref = shadow_ref,
        .put = shadow_put,
        .lookuppage = shadow_lookuppage,
        .fillpage  = shadow_fillpage,
        .dirtypage = shadow_dirtypage,
        .cleanpage = shadow_cleanpage
};

/*
 * This function is called at boot time to initialize the
 * shadow page sub system. Currently it only initializes the
 * shadow_allocator object.
 */
void
shadow_init()
{
      /*  NOT_YET_IMPLEMENTED("VM: shadow_init");*/

        shadow_allocator = slab_allocator_create("shadow",sizeof(mmobj_t));
           KASSERT(shadow_allocator);
        dbg(DBG_PRINT, "(GRADING3A 6.a) shadow_allocator is not NULL\n");
}

/*
 * You'll want to use the shadow_allocator to allocate the mmobj to
 * return, then then initialize it. Take a look in mm/mmobj.h for
 * macros which can be of use here. Make sure your initial
 * reference count is correct.
 */
mmobj_t *
shadow_create()
{
        /*NOT_YET_IMPLEMENTED("VM: shadow_create");
        return NULL;*/

       mmobj_t *newmmobj = slab_obj_alloc(shadow_allocator);
        /*memset(newmmobj,0,sizeof(mmobj_t));*/
        
        dbg(DBG_PRINT,"(GRADING3B 6), new mmobj is not NULL\n");
        mmobj_init(newmmobj, &shadow_mmobj_ops);
        newmmobj->mmo_refcount = 1;

        return newmmobj;
}

/* Implementation of mmobj entry points: */

/*
 * Increment the reference count on the object.
 */
static void
shadow_ref(mmobj_t *o)
{
        /*NOT_YET_IMPLEMENTED("VM: shadow_ref");*/
        KASSERT(o && (0 < o->mmo_refcount) && (&shadow_mmobj_ops == o->mmo_ops));
        dbg(DBG_PRINT,"(GRADING3A 6.b) mmo reference count greater than 0 and mmmo operations exists\n");

        o->mmo_refcount++;
}

/*
 * Decrement the reference count on the object. If, however, the
 * reference count on the object reaches the number of resident
 * pages of the object, we can conclude that the object is no
 * longer in use and, since it is a shadow object, it will never
 * be used again. You should unpin and uncache all of the object's
 * pages and then free the object itself.
 */
static void
shadow_put(mmobj_t *o)
{
       /* NOT_YET_IMPLEMENTED("VM: shadow_put");*/
     KASSERT(o && (0 < o->mmo_refcount) && (&shadow_mmobj_ops == o->mmo_ops));
    dbg(DBG_PRINT,"(GRADING3A 6.c) mmo reference count greater than 0 and mmmo operations exists\n");

        if (o->mmo_refcount == o->mmo_nrespages+1)
         {
             dbg(DBG_VM,"grade31\n");
            dbg(DBG_PRINT, "(GRADING3D 1) refcount is one greater than respages");
            pframe_t *pf;
            list_iterate_begin(&(o->mmo_respages), pf, pframe_t, pf_olink) 
            {
                while(pframe_is_pinned(pf)) 
                {
                     dbg(DBG_VM,"grade32\n");
                    dbg(DBG_PRINT, "(GRADING3D 1) pframe is pinned");
                     pframe_unpin(pf);
                }
                if(pframe_is_dirty(pf)) {
                     dbg(DBG_VM,"grade33\n");
                    dbg(DBG_PRINT, "(GRADING3D 1) pframe is dirty");
                     pframe_clean(pf);
                }
                     pframe_free(pf);
            }list_iterate_end();
         }
                o->mmo_refcount--;
                
                if (o->mmo_refcount==0) {
                     dbg(DBG_VM,"grade34\n");
            dbg(DBG_PRINT, "(GRADING3D 1) mmo refcount is equal to zero");
            o->mmo_shadowed->mmo_ops->put(o->mmo_shadowed);
            slab_obj_free(shadow_allocator,(mmobj_t*)o);
        }


}

/* This function looks up the given page in this shadow object. The
 * forwrite argument is true if the page is being looked up for
 * writing, false if it is being looked up for reading. This function
 * must handle all do-not-copy-on-not-write magic (i.e. when forwrite
 * is false find the first shadow object in the chain which has the
 * given page resident). copy-on-write magic (necessary when forwrite
 * is true) is handled in shadow_fillpage, not here. It is important to
 * use iteration rather than recursion here as a recursive implementation
 * can overflow the kernel stack when looking down a long shadow chain */
static int
shadow_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf)
{
        /*NOT_YET_IMPLEMENTED("VM: shadow_lookuppage");
        return 0;*/
        pframe_t* frame;
        mmobj_t* tempobj =o;
        int ret_val = 0;
        if (forwrite) 
        { dbg(DBG_VM,"grade35\n");
            dbg(DBG_PRINT, "(GRADING3D 1) forwrite is 1");
            ret_val = pframe_get(o,pagenum,pf);

            if (ret_val==0) 
            {
                 dbg(DBG_VM,"grade36\n");
                dbg(DBG_PRINT, "(GRADING3D 1) pframe received from pframe_get");
                KASSERT(NULL != (*pf));
                dbg(DBG_PRINT,"(GRADING3A 6.d) page frame not equal to NULL\n");
                KASSERT((pagenum == (*pf)->pf_pagenum) && (!pframe_is_busy(*pf)));
                dbg(DBG_PRINT,"(GRADING3A 6.d) page frame is not busy and page frame is valid\n");
                pframe_clear_busy(*pf);
                
            }
                    return ret_val;
        } 
        else
         {
             dbg(DBG_VM,"grade37\n");
            dbg(DBG_PRINT, "(GRADING3D 1) forwrite is 0");
            while (tempobj != mmobj_bottom_obj(o))
             {
                 dbg(DBG_VM,"grade38\n");
                dbg(DBG_PRINT, "(GRADING3D 1) temp_obj is not equal to bottom_obj");
                frame = pframe_get_resident(tempobj,pagenum);
                if (frame) {
                     dbg(DBG_VM,"grade39\n");
                   dbg(DBG_PRINT, "(GRADING3D 1) pframe returned successfully");
                    *pf = frame;
                   KASSERT(NULL != (*pf));
                dbg(DBG_PRINT,"(GRADING3A 6.d) page frame not equal to NULL\n");
                KASSERT((pagenum == (*pf)->pf_pagenum) && (!pframe_is_busy(*pf)));
                dbg(DBG_PRINT,"(GRADING3A 6.d) page frame is not busy and page frame is valid\n");
                    pframe_clear_busy(*pf);
                    
                    return 0;
                } 
                else 
                {
                     dbg(DBG_VM,"grade40\n");
                    dbg(DBG_PRINT, "(GRADING3D 1) shadow object being accessed");
                    tempobj =tempobj->mmo_shadowed;
                }
            }

            ret_val = pframe_lookup(tempobj,pagenum,0,pf);
            if (!ret_val) {
                KASSERT(NULL != (*pf));
                dbg(DBG_PRINT,"(GRADING3A 6.d) page frame not equal to NULL\n");
                KASSERT((pagenum == (*pf)->pf_pagenum) && (!pframe_is_busy(*pf)));
                dbg(DBG_PRINT,"(GRADING3A 6.d) page frame is not busy and page frame is valid\n");
                pframe_clear_busy(*pf);
                
            }
            
            return ret_val;
       }

}

/* As per the specification in mmobj.h, fill the page frame starting
 * at address pf->pf_addr with the contents of the page identified by
 * pf->pf_obj and pf->pf_pagenum. This function handles all
 * copy-on-write magic (i.e. if there is a shadow object which has
 * data for the pf->pf_pagenum-th page then we should take that data,
 * if no such shadow object exists we need to follow the chain of
 * shadow objects all the way to the bottom object and take the data
 * for the pf->pf_pagenum-th page from the last object in the chain).
 * It is important to use iteration rather than recursion here as a 
 * recursive implementation can overflow the kernel stack when 
 * looking down a long shadow chain */
static int
shadow_fillpage(mmobj_t *o, pframe_t *pf)
{
        /*NOT_YET_IMPLEMENTED("VM: shadow_fillpage");
        return 0;*/
       KASSERT(pframe_is_busy(pf));
        dbg(DBG_PRINT,"(GRADING3A 6.e) page frame is busy\n");
        KASSERT(!pframe_is_pinned(pf));
        dbg(DBG_PRINT,"(GRADING3A 6.e) page frame is not pinned\n");
 pframe_t *myframe;
        int ret_val;
        if (o->mmo_shadowed) 
        {
             dbg(DBG_VM,"grade41\n");
        dbg(DBG_PRINT, "(GRADING3D 1) shadow object being accessed");

            ret_val = shadow_lookuppage(o->mmo_shadowed,pf->pf_pagenum,0,&myframe);
           
                    if (ret_val==0) 
                    {
                         dbg(DBG_VM,"grade42\n");
                dbg(DBG_PRINT, "(GRADING3D 1) shadow lookup page successful");

                        if (myframe != NULL)
                         {
                             dbg(DBG_VM,"grade43\n");
                    dbg(DBG_PRINT, "(GRADING3D 1) page returned is not NULL");

                            memcpy(pf->pf_addr,myframe->pf_addr,PAGE_SIZE);
                            pframe_pin(pf);
                            return 0;
                        }
                    }
        }
        return ret_val;
        
}

/* These next two functions are not difficult. */

static int
shadow_dirtypage(mmobj_t *o, pframe_t *pf)
{
      /*  NOT_YET_IMPLEMENTED("VM: shadow_dirtypage");
        return -1;*/

       /*  return o->mmo_ops->dirtypage(o,pf);*/
    return 0;
}

static int
shadow_cleanpage(mmobj_t *o, pframe_t *pf)
{
        /*NOT_YET_IMPLEMENTED("VM: shadow_cleanpage");
        return -1;*/

         /*return o->mmo_ops->cleanpage(o,pf);*/
    return 0;
}