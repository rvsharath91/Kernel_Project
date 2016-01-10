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
#include "globals.h"
#include "types.h"
#include "errno.h"

#include "util/string.h"
#include "util/printf.h"
#include "util/debug.h"

#include "fs/dirent.h"
#include "fs/fcntl.h"
#include "fs/stat.h"
#include "fs/vfs.h"
#include "fs/vnode.h"

/* This takes a base 'dir', a 'name', its 'len', and a result vnode.
 * Most of the work should be done by the vnode's implementation
 * specific lookup() function, but you may want to special case
 * "." and/or ".." here depnding on your implementation.
 *
 * If dir has no lookup(), return -ENOTDIR.
 *
 * Note: returns with the vnode refcount on *result incremented.
 */
int
lookup(vnode_t *dir, const char *name, size_t len, vnode_t **result)
{
     
     KASSERT(NULL != dir);
     dbg(DBG_PRINT,"(GRADING2A 2.a) dir is not null \n");

     KASSERT(NULL != name);
     dbg(DBG_PRINT,"(GRADING2A 2.a) name is not null \n");

     KASSERT(NULL != result);
     dbg(DBG_PRINT,"(GRADING2A 2.a) result is not null \n");

   if(len <= 0)
    {
          *result=vget(dir->vn_fs, dir->vn_vno);

          dbg(DBG_PRINT,"(GRADING2B) length of name is 0 \n");
          dbg(DBG_ERROR, "length of name is 0 \n");
         
          return 0;
    }

  if(len > NAME_LEN)
    {  
        dbg(DBG_PRINT,"(GRADING2B) length of the name is very long \n");
        dbg(DBG_ERROR, "length of the name is very long");
       
        return -ENAMETOOLONG;
    }

    if (!S_ISDIR(dir->vn_mode) || (dir->vn_ops->lookup == NULL))
    {   
        dbg(DBG_PRINT,"(GRADING2B) The dir passed is not actually a directory \n");
        dbg(DBG_ERROR,"The dir passed is not actually a directory  \n");
        
        return -ENOTDIR;
    }
     
     dbg(DBG_PRINT,"(GRADING2B) Searching in the directory \n");
     return dir->vn_ops->lookup(dir,name,len,result);
}


/* When successful this function returns data in the following "out"-arguments:
 *  o res_vnode: the vnode of the parent directory of "name"
 *  o name: the `basename' (the element of the pathname)
 *  o namelen: the length of the basename
 *
 * For example: dir_namev("/s5fs/bin/ls", &namelen, &name, NULL,
 * &res_vnode) would put 2 in namelen, "ls" in name, and a pointer to the
 * vnode corresponding to "/s5fs/bin" in res_vnode.
 *
 * The "base" argument defines where we start resolving the path from:
 * A base value of NULL means to use the process's current working directory,
 * curproc->p_cwd.  If pathname[0] == '/', ignore base and start with
 * vfs_root_vn.  dir_namev() should call lookup() to take care of resolving each
 * piece of the pathname.
 *
 * Note: A successful call to this causes vnode refcount on *res_vnode to
 * be incremented.
 */
int
dir_namev(const char *pathname, size_t *namelen, const char **name,
          vnode_t *base, vnode_t **res_vnode)
{
        KASSERT(NULL != pathname);
        dbg(DBG_PRINT,"(GRADING2A 2.b) Pathname is null \n");
        
        KASSERT(NULL != namelen);
        dbg(DBG_PRINT,"(GRADING2A 2.b) Name length not null \n");
        
        KASSERT(NULL != name);
        dbg(DBG_PRINT,"(GRADING2A 2.b) name is not null \n");
        
        KASSERT(NULL != res_vnode);
        dbg(DBG_PRINT,"(GRADING2A 2.b) res_vnode is not NULL \n");

        if(strlen(pathname) > MAXPATHLEN)
            {
                dbg(DBG_ERROR,"path is too long \n");
                return -ENAMETOOLONG;
            }

        if(strlen(pathname) <= 0)
        {
           dbg(DBG_ERROR, "length of pathname is 0 or less \n");
           return -EINVAL;
        }

        int ret_val = 0;
        int temp = 0;
        int pointer = 0;
        if ( base == NULL )
        { 
          dbg(DBG_PRINT,"(GRADING2B) base is null setting base to curproc->p_cwd \n");
          base = curproc->p_cwd;
        }

        if (pathname[0] == '/')
        {
          pointer =1;
          dbg(DBG_PRINT,"(GRADING2B) path starts with / so seeting base to vfs_root_vn \n");
          base = vfs_root_vn;
        }

         for(;;)
         {
              for ( temp = 0; pathname[pointer+temp] != '/'; temp++)
                      {
                   if (pathname[pointer+temp] == '\0')
                   {
                      if ( temp > NAME_LEN)
                      {
                          dbg(DBG_ERROR, "length of filename is too long");
                          dbg(DBG_PRINT,"(GRADING2B) length of filename is too long \n");

                          return -ENAMETOOLONG;
                      }

                      *res_vnode = vget(base->vn_fs,base->vn_vno);
                      KASSERT(NULL != *res_vnode);
                      dbg(DBG_PRINT,"\n(GRADING2A 2.b) pointer to corresponding vnode is not NULL\n");

                      if (!S_ISDIR((*res_vnode)->vn_mode))
                      {
                          vput(*res_vnode);
                          dbg(DBG_ERROR,"given dir is not a directory\n");
                          dbg(DBG_PRINT,"(GRADING2B) given dir is not a directory \n");

                          return -ENOTDIR;
                      }

                          *name = &pathname[pointer];
                          *namelen = temp; 
                          return 0;
                      }
                    } 
                                ret_val = lookup(base,&pathname[pointer],temp,res_vnode);
                                temp++;
                if (ret_val < 0)
                    {
                      dbg(DBG_ERROR,"lookup failed to search \n");
                      dbg(DBG_PRINT,"(GRADING2B) lookup failed to search \n");

                      return ret_val;
                    }
              
                vput(*res_vnode);
                base = *res_vnode;
                pointer = pointer+temp;
              }

                  return 0;
}

/* This returns in res_vnode the vnode requested by the other parameters.
 * It makes use of dir_namev and lookup to find the specified vnode (if it
 * exists).  flag is right out of the parameters to open(2); see
 * <weenix/fcntl.h>.  If the O_CREAT flag is specified, and the file does
 * not exist call create() in the parent directory vnode.
 *
 * Note: Increments vnode refcount on *res_vnode.
 */
int
open_namev(const char *pathname, int flag, vnode_t **res_vnode, vnode_t *base)
{
       /* NOT_YET_IMPLEMENTED("VFS: open_namev"); */
                
        const char *named = NULL;
        size_t len = 0;
        vnode_t *res_vnode1;
        int ret_val = dir_namev(pathname, &len,&named,base, &res_vnode1);
        
        if (ret_val != 0) 
        {
            dbg(DBG_ERROR, "dir not found\n");
                    dbg(DBG_PRINT,"(GRADING2B) dir not found \n");
            return ret_val;
        }
      
            int ret_val1 = lookup(res_vnode1,named,len,res_vnode);
            if (ret_val1 < 0) {
                              if (flag & O_CREAT) {

                               dbg(DBG_ERROR,"file not found but O_CREAT flag is on \n");
                    dbg(DBG_PRINT,"(GRADING2B) file not found but O_CREAT flag is on \n");
                            KASSERT(NULL != res_vnode1->vn_ops->create);
                            dbg(DBG_PRINT,"(GRADING2A 2.c) new file can be created\n");
                         
                            int ret_val2 = res_vnode1->vn_ops->create(res_vnode1,named,len,res_vnode);
                               if (ret_val2 < 0) {
                                    dbg(DBG_PRINT, "(GRADING2C) Cannot create the file\n");
                                    vput(res_vnode1);
                                    return ret_val2;
                                   }
                                    } else {
                                              dbg(DBG_PRINT, "(GRADING2B) Could not find the file  \n");
                                        vput(res_vnode1);
                                        return ret_val1;
                                    }    
                                }
                                  dbg(DBG_PRINT, "(GRADING2B) File with the given name alredy exists\n");
                                  vput(res_vnode1);  
                                  
                            return 0;  
}

#ifdef __GETCWD__
/* Finds the name of 'entry' in the directory 'dir'. The name is writen
 * to the given buffer. On success 0 is returned. If 'dir' does not
 * contain 'entry' then -ENOENT is returned. If the given buffer cannot
 * hold the result then it is filled with as many characters as possible
 * and a null terminator, -ERANGE is returned.
 *
 * Files can be uniquely identified within a file system by their
 * inode numbers. */
int
lookup_name(vnode_t *dir, vnode_t *entry, char *buf, size_t size)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_name");
        return -ENOENT;
}


/* Used to find the absolute path of the directory 'dir'. Since
 * directories cannot have more than one link there is always
 * a unique solution. The path is writen to the given buffer.
 * On success 0 is returned. On error this function returns a
 * negative error code. See the man page for getcwd(3) for
 * possible errors. Even if an error code is returned the buffer
 * will be filled with a valid string which has some partial
 * information about the wanted path. */
ssize_t
lookup_dirpath(vnode_t *dir, char *buf, size_t osize)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_dirpath");

        return -ENOENT;
}
#endif /* __GETCWD__ */