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

int anon_count = 0; /* for debugging/verification purposes */

static slab_allocator_t *anon_allocator;

static void anon_ref(mmobj_t *o);
static void anon_put(mmobj_t *o);
static int  anon_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf);
static int  anon_fillpage(mmobj_t *o, pframe_t *pf);
static int  anon_dirtypage(mmobj_t *o, pframe_t *pf);
static int  anon_cleanpage(mmobj_t *o, pframe_t *pf);

static mmobj_ops_t anon_mmobj_ops = {
        .ref = anon_ref,
        .put = anon_put,
        .lookuppage = anon_lookuppage,
        .fillpage  = anon_fillpage,
        .dirtypage = anon_dirtypage,
        .cleanpage = anon_cleanpage
};

/*
 * This function is called at boot time to initialize the
 * anonymous page sub system. Currently it only initializes the
 * anon_allocator object.
 */
void
anon_init()
{
        /*NOT_YET_IMPLEMENTED("VM: anon_init");*/
     
      anon_allocator = slab_allocator_create("anon",sizeof(mmobj_t));
      KASSERT(anon_allocator); 
      dbg(DBG_PRINT,"(GRADING3A 4.a): Initializing an anon page\n");

}

/*
 * You'll want to use the anon_allocator to allocate the mmobj to
 * return, then then initialize it. Take a look in mm/mmobj.h for
 * macros which can be of use here. Make sure your initial
 * reference count is correct.
 */
mmobj_t *
anon_create()
{
      dbg(DBG_PRINT,"(GRADING3B 1): Allocating a mmobj\n");
      mmobj_t *newmmobj = slab_obj_alloc(anon_allocator);

      dbg(DBG_PRINT,"(GRADING3B 1): Initializing the mmobj created\n");
      mmobj_init(newmmobj, &anon_mmobj_ops);
      newmmobj->mmo_refcount = 1;

      return newmmobj;
}

/* Implementation of mmobj entry points: */

/*
 * Increment the reference count on the object.
 */
static void
anon_ref(mmobj_t *o)
{
       /* NOT_YET_IMPLEMENTED("VM: anon_ref");*/
      KASSERT(o && (0 < o->mmo_refcount) && (&anon_mmobj_ops == o->mmo_ops));
      dbg(DBG_PRINT,"(GRADING3A 4.b): reference count greater than 0 and Memory object operations present is equal to that of created obj \n");
      o->mmo_refcount += 1;
}

/*
 * Decrement the reference count on the object. If, however, the
 * reference count on the object reaches the number of resident
 * pages of the object, we can conclude that the object is no
 * longer in use and, since it is an anonymous object, it will
 * never be used again. You should unpin and uncache all of the
 * object's pages and then free the object itself.
 */
static void
anon_put(mmobj_t *o)
{

      KASSERT(o && (0 < o->mmo_refcount) && (&anon_mmobj_ops == o->mmo_ops));
      dbg(DBG_PRINT,"(GRADING3A 4.b): reference count greater than 0 and Memory object operations present is equal to that of created obj \n");
        int count=0 , actual=0;
        pframe_t *pf;
        
        if(o->mmo_refcount == o->mmo_nrespages+1)
        {
           dbg(DBG_VM,"grade44\n");
              dbg(DBG_PRINT,"(GRADING3B 1): Decremented reference count of mmobj is equal to Resident pages\n");
              list_iterate_begin(&o->mmo_respages,pf,pframe_t,pf_olink)
              {       
                        count++;
                        /*if(pframe_is_free(pf))
                        {
                           dbg(DBG_VM,"grade45\n");
                          dbg(DBG_PRINT,"(GRADING3B 1): The frame is free, incrementing the actual count\n");
                          actual++;
                        }*/
                        
                       /* else if(pframe_is_busy(pf) ) 
                        { 
                           dbg(DBG_VM,"grade46\n");
                            dbg(DBG_PRINT,"(GRADING3B 1):The frame is busy\n");
                            while(pframe_is_busy(pf))
                            {
                               dbg(DBG_VM,"grade47\n");
                              dbg(DBG_PRINT,"(GRADING3B 1): Sleeping on the wait queue till the frame is not busy\n");
                              sched_sleep_on(&pf->pf_waitq);
                            }
                            if(pframe_is_pinned(pf))
                            {
                               dbg(DBG_VM,"grade48\n");
                              dbg(DBG_PRINT,"(GRADING3B 1): The frame is pinned, so unpinning \n");
                              pframe_unpin(pf);
                            }
                             dbg(DBG_VM,"grade49\n");
                            dbg(DBG_PRINT,"(GRADING3B 1):Freeing the frame after the frame is out of busy state\n");
                            pframe_free(pf);
                            actual++;

                        }*/

                         
                        
                           dbg(DBG_VM,"grade50\n");
                            dbg(DBG_PRINT,"(GRADING3B 1):The frame is not busy\n");
                            if(pframe_is_pinned(pf))
                            {
                               dbg(DBG_VM,"grade51\n");
                                dbg(DBG_PRINT,"(GRADING3B 1):The frame is pinned, so unpinning\n");
                                pframe_unpin(pf);
                            }
                             dbg(DBG_VM,"grade52\n");
                            dbg(DBG_PRINT,"(GRADING3B 1):Freeing the frame\n");            
                            pframe_free(pf);
                            actual++;

                        
                }list_iterate_end();

                if(count == actual)
                {
                   dbg(DBG_VM,"grade53\n");
                  dbg(DBG_PRINT,"(GRADING3B 1):The reference count on the object reaches the number of resident pages of the object, so freeing the object\n");                   
                  slab_obj_free(anon_allocator,(void*)o);
                }
        }
        else 
        {
           dbg(DBG_VM,"grade54\n");
          dbg(DBG_PRINT,"(GRADING3B 1): Decrementing the reference count\n");
          o->mmo_refcount--;
        }


}

/* Get the corresponding page from the mmobj. No special handling is
 * required. */
static int
anon_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf)
{
        /*NOT_YET_IMPLEMENTED("VM: anon_lookuppage");*/
      dbg(DBG_PRINT,"(GRADING3B 1), calling corresponding page of the given mmobj and returning that page\n");
      int ret_val = pframe_get(o,pagenum,pf);
      return ret_val;
}

/* The following three functions should not be difficult. */

static int
anon_fillpage(mmobj_t *o, pframe_t *pf)
{
       /* NOT_YET_IMPLEMENTED("VM: anon_fillpage");*/
       /*return o->mmo_ops->fillpage(o,pf);*/
      KASSERT(pframe_is_busy(pf));
      dbg(DBG_PRINT,"(GRADING3A 4.d): The given page frame is busy\n");
      KASSERT(!pframe_is_pinned(pf));
      dbg(DBG_PRINT,"(GRADING3A 4.d): The given page frame is not pinned\n");
      pframe_pin(pf);
      dbg(DBG_PRINT,"(GRADING3B 1): Doing memset on the frame\n");
      memset(pf->pf_addr,0,PAGE_SIZE);

      return 0;
        
}

static int
anon_dirtypage(mmobj_t *o, pframe_t *pf)
{
      
      return -1;
}

static int
anon_cleanpage(mmobj_t *o, pframe_t *pf)
{

    return -1;

}
