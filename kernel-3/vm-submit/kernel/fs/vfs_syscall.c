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

/*
 *  FILE: vfs_syscall.c
 *  AUTH: mcc | jal
 *  DESC:
 *  DATE: Wed Apr  8 02:46:19 1998
 *  $Id: vfs_syscall.c,v 1.10 2014/12/22 16:15:17 william Exp $
 */

#include "kernel.h"
#include "errno.h"
#include "globals.h"
#include "fs/vfs.h"
#include "fs/file.h"
#include "fs/vnode.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/fcntl.h"
#include "fs/lseek.h"
#include "mm/kmalloc.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/stat.h"
#include "util/debug.h"

/* To read a file:
 *      o fget(fd)
 *      o call its virtual read fs_op
 *      o update f_pos
 *      o fput() it
 *      o return the number of bytes read, or an error
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for reading.
 *      o EISDIR
 *        fd refers to a directory.
 *
 * In all cases, be sure you do not leak file refcounts by returning before
 * you fput() a file that you fget()'ed.
 */
int
do_read(int fd, void *buf, size_t nbytes)
{
        
        int bytes_read =0;
        file_t *f;
        if((f=fget(fd)) == NULL || curproc->p_files[fd] == NULL)
          {
           dbg(DBG_PRINT,"(GRADING2B) given file descriptor is not valid\n");
           dbg(DBG_ERROR,"do_read(): the given %d fd is not a valid file descriptor\n",fd);
           return -EBADF; 
          }

        if((f->f_mode & FMODE_READ) != FMODE_READ)
          {
           fput(f);
           dbg(DBG_PRINT,"(GRADING2B) the given file is not open for reading\n");
           dbg(DBG_ERROR,"do_read(): FILE obtained is not open for reading\n");
           return -EBADF; 
          } 

        
        if(S_ISDIR(f->f_vnode->vn_mode))
        {
          dbg(DBG_PRINT,"(GRADING2B) The given file is actually a directory\n");
          dbg(DBG_ERROR, "do_read(): obtained file is a directory\n");
          fput(f);
          return -EISDIR;
        }
        
        bytes_read = f->f_vnode->vn_ops->read(f->f_vnode,f->f_pos,buf,nbytes);
         if(bytes_read>=0)
        f->f_pos = bytes_read + f->f_pos;
        fput(f);

        return bytes_read;
        /*NOT_YET_IMPLEMENTED("VFS: do_read");*/


        
}

/* Very similar to do_read.  Check f_mode to be sure the file is writable.  If
 * f_mode & FMODE_APPEND, do_lseek() to the end of the file, call the write
 * fs_op, and fput the file.  As always, be mindful of refcount leaks.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for writing.
 */
int
do_write(int fd, const void *buf, size_t nbytes)
{
        file_t *f;
        int bytes_written = 0;
        f=fget(fd);
        if(f == NULL || curproc->p_files[fd] == NULL)
        {
          dbg(DBG_PRINT,"(GRADING2B) given file descriptor is not valid\n");
          dbg(DBG_ERROR,"do_write(): fd is not a valid file descriptor\n");
          return -EBADF;
        }

        if((f->f_mode & FMODE_WRITE) == FMODE_WRITE)
        {
                dbg(DBG_PRINT,"(GRADING2B) The given file has write permission\n");

            if( (f->f_mode & FMODE_APPEND) == FMODE_APPEND)
        {
                dbg(DBG_PRINT,"(GRADING2B) data can be appended to the file\n");
            int pos = f->f_pos;
        f->f_pos = do_lseek(fd,0,SEEK_END); 
        } 
        bytes_written=f->f_vnode->vn_ops->write(f->f_vnode,f->f_pos,buf,nbytes);      
        }

        else
        {
        fput(f);
        dbg(DBG_PRINT,"(GRADING2B) The given file is not open for writing\n");
        dbg(DBG_ERROR,"do_write(): fd is not open for writing.\n");
        return -EBADF;
        }

        if(bytes_written > 0)
        {
        KASSERT((S_ISCHR(f->f_vnode->vn_mode)) || (S_ISBLK(f->f_vnode->vn_mode)) ||((S_ISREG(f->f_vnode->vn_mode)) && (f->f_pos <= f->f_vnode->vn_len)));
        dbg(DBG_PRINT,"(GRADING2A 3.a) A character special CHR or block BLK or REG write is successful \n");
        f->f_pos = f->f_pos + bytes_written;
        }
        fput(f);
        return bytes_written;
        /*NOT_YET_IMPLEMENTED("VFS: do_write");*/
        
}

/*
 * Zero curproc->p_files[fd], and fput() the file. Return 0 on success
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't a valid open file descriptor.
 */
int
do_close(int fd)
{
        if(fd < 0 || fd >= NFILES || curproc->p_files[fd] == NULL) 
        {
        dbg(DBG_PRINT,"(GRADING2B) given file descriptor is not valid\n");
        dbg(DBG_ERROR,"do_close(): fd is not a valid file descriptor\n");
        return -EBADF;
        }
        file_t *file = fget(fd);
        fput(curproc->p_files[fd]);
        fput(file);
        curproc->p_files[fd] = NULL;
        return 0;
        /*NOT_YET_IMPLEMENTED("VFS: do_close");
        return -1;*/
}

/* To dup a file:
 *      o fget(fd) to up fd's refcount
 *      o get_empty_fd()
 *      o point the new fd to the same file_t* as the given fd
 *      o return the new file descriptor
 *
 * Don't fput() the fd unless something goes wrong.  Since we are creating
 * another reference to the file_t*, we want to up the refcount.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't an open file descriptor.
 *      o EMFILE
 *        The process already has the maximum number of file descriptors open
 *        and tried to open a new one.
 */
int
do_dup(int fd)
{

     if(fd < 0 || fd >= NFILES) 
        {
                dbg(DBG_PRINT,"(GRADING2B) given file descriptor is not valid\n");
                dbg(DBG_ERROR,"File Desciptor not valid\n");
                return -EBADF;
        }
        file_t *f;
        int new_fd;
        if((f=fget(fd)) == NULL || curproc->p_files[fd] == NULL)
        {
            dbg(DBG_PRINT,"(GRADING2B) given file descriptor is not valid\n");
            dbg(DBG_ERROR,"do_dup(): fd is not a valid file descriptor.\n");
            return -EBADF;
        }

        if((new_fd = get_empty_fd(curproc)) <0)
        {
          dbg(DBG_PRINT,"(GRADING2B) out of file descriptors\n");
          dbg(DBG_ERROR, "do_dup(): out of file descriptors: The process already has NFILES descriptors opened\n");
          fput(f);
          return new_fd;
        }
        curproc->p_files[new_fd]= f;   /* can change to file */
        return new_fd;
        /*NOT_YET_IMPLEMENTED("VFS: do_dup");
        return -1;*/
}

/* Same as do_dup, but insted of using get_empty_fd() to get the new fd,
 * they give it to us in 'nfd'.  If nfd is in use (and not the same as ofd)
 * do_close() it first.  Then return the new file descriptor.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        ofd isn't an open file descriptor, or nfd is out of the allowed
 *        range for file descriptors.
 */
int
do_dup2(int ofd, int nfd)
{
        
        if(nfd<0 || nfd >= NFILES)
        {
        dbg(DBG_PRINT,"(GRADING2B) given file descriptor is not valid\n");
        dbg(DBG_ERROR,"do_dup2(): nfd is not an open file descriptor\n");
        return -EBADF;
        }

         if(ofd < 0 || ofd >= NFILES) 
        {
        dbg(DBG_PRINT,"(GRADING2B) given file descriptor is not valid\n");
        dbg(DBG_ERROR,"do_dup2(): ofd is not an open file descriptor.\n");
                return -EBADF;
        }
        file_t *f;
        if((f=fget(ofd)) == NULL || curproc->p_files[ofd] == NULL)
        {
        dbg(DBG_PRINT,"(GRADING2B) given file descriptor is not valid\n");
        dbg(DBG_ERROR,"do_dup2(): ofd is not a valid file descriptor.\n");
        return -EBADF;
        }


        if(nfd == ofd)
           {
            dbg(DBG_PRINT,"(GRADING2B) nfd is equal to the given ofd\n");
            fput(f);
            return nfd;
           }

        if((curproc->p_files[nfd]) != NULL)
        {
                dbg(DBG_PRINT,"(GRADING2B) given nfd is in use\n");
               int do_ret = do_close(nfd);
                 if(do_ret < 0)
                 {
                     dbg(DBG_PRINT,"(GRADING2B) Failed to close the given file descriptor nfd\n");
                     fput(f);
                     return do_ret;
                 }
                 }
        curproc->p_files[nfd] = f;
        return nfd;
        
        /*NOT_YET_IMPLEMENTED("VFS: do_dup2");
        return -1;*/
}

/*
 * This routine creates a special file of the type specified by 'mode' at
 * the location specified by 'path'. 'mode' should be one of S_IFCHR or
 * S_IFBLK (you might note that mknod(2) normally allows one to create
 * regular files as well-- for simplicity this is not the case in Weenix).
 * 'devid', as you might expect, is the device identifier of the device
 * that the new special file should represent.
 *
 * You might use a combination of dir_namev, lookup, and the fs-specific
 * mknod (that is, the containing directory's 'mknod' vnode operation).
 * Return the result of the fs-specific mknod, or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        mode requested creation of something other than a device special
 *        file.
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_mknod(const char *path, int mode, unsigned devid)
{


        if (mode !=S_IFBLK && mode !=S_IFCHR)
        {
                dbg(DBG_PRINT,"(GRADING2B) The special file is neither Block device nor character device\n");
                dbg(DBG_ERROR, "Mode specified is wrong\n");
                return -EINVAL;
        }
        if (strlen(path) > MAXPATHLEN)
        {
                dbg(DBG_PRINT,"(GRADING2B) Path length exceeds maximum path length that is allowed\n");
                dbg(DBG_ERROR, "Path length exceeding maximum path length\n");
                return -ENAMETOOLONG;
        }


        size_t namelen =0;
        const char *name = NULL;
        vnode_t *res_vnode = NULL;

        int dir_value = dir_namev(path, &namelen, &name, NULL, &res_vnode);
        if (dir_value<0)
        {
                 dbg(DBG_PRINT,"(GRADING2B) Error in acquiring the vnode, name and name length of the path specified\n");
                 dbg(DBG_ERROR, "Error in calling dir_namev function\n");
                 return dir_value; 
        }
        vput(res_vnode);
        int result=lookup(res_vnode,name, namelen, &res_vnode);

        KASSERT(NULL != res_vnode->vn_ops->mknod);
        dbg(DBG_PRINT,"(GRADING2A 3.b) res_vnode->vn_ops is not NULL\n");

        if (result==0)
        {
                dbg(DBG_PRINT,"(GRADING2B) The given path already exists\n");
                dbg(DBG_ERROR, "Path already exists\n");
                vput(res_vnode);
                return -EEXIST;
        }
        if (result == -ENOENT)
        {
                dbg(DBG_PRINT,"(GRADING2B) There is no entry for the particular file\n");
                dbg(DBG_PRINT, "No entry for that particular file\n");
                /*vput(res_vnode);*/
                int res = res_vnode->vn_ops->mknod(res_vnode,name,namelen,mode,devid);
                return res;
        }
        return 0;
        /*NOT_YET_IMPLEMENTED("VFS: do_mknod");
        return -1;*/
}

/* Use dir_namev() to find the vnode of the dir we want to make the new
 * directory in.  Then use lookup() to make sure it doesn't already exist.
 * Finally call the dir's mkdir vn_ops. Return what it returns.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      on ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_mkdir(const char *path)
{
        if (strlen(path) > NAME_LEN)
        {
                dbg(DBG_PRINT,"(GRADING2B) path length exceeds the maximum path length limit\n");
                dbg(DBG_ERROR, "Path length exceeding maximum path length\n");
                return -ENAMETOOLONG;
        }

        size_t namelen;
        const char *name;
        vnode_t *res_vnode;

        int dir_value = dir_namev(path, &namelen, &name, NULL, &res_vnode);
        
        if (dir_value<0 )
        {
                dbg(DBG_PRINT,"(GRADING2B) Error in calling dir_namev function\n");
                dbg(DBG_ERROR, "Error in calling dir_namev function\n");
                return dir_value; 
        }
        vput(res_vnode);
        int result=lookup(res_vnode,name, namelen, &res_vnode);

        dbg(DBG_PRINT,"(GRADING2A 3.c) res_vnode->vn_ops is not NULL\n");

        if (result==-ENOTDIR)
        {
                dbg(DBG_PRINT,"(GRADING2B) A component used as a directory in path is not, in fact, a directory\n");
                dbg(DBG_ERROR, "A component used as a directory in path is not, in fact, a directory\n");
                return -ENOTDIR;
        }

        if (result==0)
        {
                dbg(DBG_PRINT,"(GRADING2B) Specified path already exists\n");
                dbg(DBG_ERROR, "Path already exists\n");
                vput(res_vnode);
                return -EEXIST;
        }
        if (result == -ENOENT)
        {
            KASSERT(NULL != res_vnode->vn_ops->mkdir);
                dbg(DBG_PRINT,"(GRADING2B) No entry for this file\n");
                dbg(DBG_PRINT,"No entry for that particular file\n");
                /*vput(res_vnode);*/
                return res_vnode->vn_ops->mkdir(res_vnode,name,namelen);
        }
        return 0;

        /*NOT_YET_IMPLEMENTED("VFS: do_mkdir");
        return -1;*/
}


/* Use dir_namev() to find the vnode of the directory containing the dir to be
 * removed. Then call the containing dir's rmdir v_op.  The rmdir v_op will
 * return an error if the dir to be removed does not exist or is not empty, so
 * you don't need to worry about that here. Return the value of the v_op,
 * or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        path has "." as its final component.
 *      o ENOTEMPTY
 *        path has ".." as its final component.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_rmdir(const char *path)
{
        if (strlen(path) > MAXPATHLEN)
        {
                dbg(DBG_PRINT,"(GRADING2B) Path length exceeding maximum path length\n");
                dbg(DBG_ERROR, "Path length exceeding maximum path length\n");
                return -ENAMETOOLONG;
        }

        size_t namelen;
        const char *name;
        vnode_t *res_vnode;


        int dir_value = dir_namev(path, &namelen, &name, NULL, &res_vnode);
        if (dir_value<0)
        {
                dbg(DBG_PRINT,"(GRADING2B) Error in calling dir_namev function\n");
                dbg(DBG_ERROR, "Error in calling dir_namev function\n");
                return dir_value; 
        }

        /*int result=lookup(res_vnode,name, namelen, &res_vnode);*/

        KASSERT(NULL != res_vnode->vn_ops->rmdir);
        dbg(DBG_PRINT,"\n(GRADING2A 3.d) res_vnode->vn_ops is not NULL");

      /*  if (result==-ENOTDIR)
        {
                dbg(DBG_ERROR, "\n A component used as a directory in path is not, in fact, a directory.");
                return -ENOTDIR;

        }*/

        if(strcmp(name,".")==0)
        {

                vput(res_vnode);
                dbg(DBG_PRINT,"(GRADING2B) do_rmdir(): path has '.' as final component\n");
                dbg(DBG_ERROR, "do_rmdir(): path has '.' as final component\n");
                return -EINVAL;
        }
        if(strcmp(name,"..")==0)
        {
                vput(res_vnode);
                dbg(DBG_PRINT,"(GRADING2B) do_rmdir(): path has '..' as final component\n");
                dbg(DBG_ERROR, "do_rmdir(): path has '..' as final component\n");
                return -ENOTEMPTY;
        }
        
        int res_val = res_vnode->vn_ops->rmdir(res_vnode,name,namelen);
        vput(res_vnode);
        return res_val;



        /*NOT_YET_IMPLEMENTED("VFS: do_rmdir");
        return -1;*/
}


/*
 * Same as do_rmdir, but for files.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EISDIR
 *        path refers to a directory.
 *      o ENOENT
 *        A component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_unlink(const char *path)
{

      if(strlen(path) > MAXPATHLEN)
     {
      dbg(DBG_PRINT,"(GRADING2B) do_unlink(): path name Provided is too long\n");
      dbg(DBG_ERROR, "do_unlink(): path name Provided is too long\n");
      return -ENAMETOOLONG;
     }

     if(strlen(path) <=0)
     {
      dbg(DBG_PRINT,"(GRADING2B) do_unlink(): given path name is not valid\n");
      dbg(DBG_ERROR, "do_unlink(): path name is not valid\n");
      return -EINVAL;
     }
        size_t namelen;
        const char *name;
        vnode_t *res_vnode;
        vnode_t *check_vnode;
        int ret_val =0;
        int val=0;
        int unlinking = 0;
        ret_val = dir_namev(path,&namelen,&name,NULL,&res_vnode);

        if(ret_val<0)
        {  
            dbg(DBG_PRINT,"(GRADING2B) Error while calling dir_namev function\n");   
            return ret_val;
        }
        

        val = lookup(res_vnode,name,namelen,&check_vnode);

        if(val<0)
        {   
            vput(res_vnode);
            dbg(DBG_PRINT,"(GRADING2B) Error while calling lookup function\n"); 
            return val;
        }

        if(S_ISDIR(check_vnode->vn_mode))
        {
          vput(res_vnode);
          vput(check_vnode);
          dbg(DBG_PRINT,"(GRADING2B) The given path is a directory\n");
          dbg(DBG_ERROR, "do_unlink(): path is directory\n");
          return -EISDIR;
        }

        KASSERT(NULL != res_vnode->vn_ops->unlink);
        dbg(DBG_PRINT, "(GRADING2A 3.e) do_unlink(): corresponding vn_ops no NULL\n");
        unlinking = res_vnode->vn_ops->unlink(res_vnode, name, namelen);
        vput(res_vnode);
        vput(check_vnode);
        return unlinking;
        /*NOT_YET_IMPLEMENTED("VFS: do_unlink");
        return -1;*/
}

/* To link:
 *      o open_namev(from)
 *      o dir_namev(to)
 *      o call the destination dir's (to) link vn_ops.
 *      o return the result of link, or an error
 *
 * Remember to vput the vnodes returned from open_namev and dir_namev.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        to already exists.
 *      o ENOENT
 *        A directory component in from or to does not exist.
 *      o ENOTDIR
 *        A component used as a directory in from or to is not, in fact, a
 *        directory.
 *      o ENAMETOOLONG
 *        A component of from or to was too long.
 *      o EISDIR
 *        from is a directory.
 */
int
do_link(const char *from, const char *to)
{
dbg(DBG_PRINT, "Entering do_link\n"); 
       if(strlen(from)> NAME_LEN ||strlen(to)> NAME_LEN){
                dbg(DBG_PRINT,"(GRADING3D 1),path name is too long\n");
            return -ENAMETOOLONG;
        }
     if((strlen(from) > MAXPATHLEN) || (strlen(to) > MAXPATHLEN))
     {
      dbg(DBG_PRINT,"(GRADING2B) A component of from or to is too long\n");
      dbg(DBG_ERROR, "do_link(): A component of from or to is too long\n");
      return -ENAMETOOLONG;
     }

     if((strlen(from)) <= 0 || (strlen(to) <=0))
     {
      dbg(DBG_PRINT,"(GRADING2B) A component of from or to is not valid\n");
      dbg(DBG_ERROR, "do_link(): A component of from or to is not valid\n");
      return -EINVAL;
     }

        size_t namelen;
        int dir_val;
       /* int open_val;*/
        const char *name;
        int res=0;
        /*int res_val =0;*/
        int val=0;
        vnode_t *old_vnode;
        vnode_t *res_vnode;
        vnode_t * res_vnode1;   /*old_vnode (from), res_vnode(to)*/
        open_namev(from,0,&old_vnode,NULL);

       /* if(open_val < 0)
        {
          dbg(DBG_PRINT,"(GRADING2B) open_namev failed\n");
          return open_val;
        }*/
 if (S_ISDIR(old_vnode->vn_mode)) {
        dbg(DBG_PRINT, "(GRADING3D 1),The returned file is a directory\n");
            vput(old_vnode);
            return -EISDIR;
        }
         dir_namev(to,&namelen,&name,NULL,&res_vnode);
        /*if(res_val < 0)
        {
         dbg(DBG_PRINT,"(GRADING2B) Error calling dir_namev\n");
         dbg(DBG_ERROR, "do_link(): dir_namev failed\n");
         return res_val;
        }

        if(res_vnode->vn_ops->link == NULL)
        {
            vput(old_vnode);
          dbg(DBG_PRINT,"(GRADING2B) open_namev failed\n");
          return -ENOTDIR;
        }*/

        if(lookup(res_vnode,name,namelen, &res_vnode1) == 0)
        {
        dbg(DBG_PRINT,"(GRADING2B) given path already exists\n");
        vput(res_vnode);
                vput(old_vnode);
                vput(res_vnode1);
        return -EEXIST;
        }

       /* else
        {*/
         dbg(DBG_PRINT,"(GRADING2B) calling link function\n");
         res = res_vnode->vn_ops->link(old_vnode,res_vnode,name,strlen(name)); 
         vput(res_vnode);
                 vput(old_vnode);

         return res; 
         
        
               
        /*NOT_YET_IMPLEMENTED("VFS: do_link");*/
    

}

/*      o link newname to oldname
 *      o unlink oldname
 *      o return the value of unlink, or an error
 *
 * Note that this does not provide the same behavior as the
 * Linux system call (if unlink fails then two links to the
 * file could exist).
 */
int
do_rename(const char *oldname, const char *newname)
{
        if(strlen(oldname) <=0)
        {
                dbg(DBG_PRINT,"(GRADING2D) String length of oldname not valid\n");
                dbg(DBG_ERROR,"String length of oldname not valid\n");
                return -EINVAL;
        }
        if(strlen(newname) <=0)
        {
                dbg(DBG_PRINT,"(GRADING2D) String length of newname not valid\n");
                dbg(DBG_ERROR,"String length of newname not valid\n");
                return -EINVAL;
        }

         dbg(DBG_PRINT,"(GRADING2D) the name was changed \n");
        do_link(oldname,newname);
        return do_unlink(oldname);

        /*NOT_YET_IMPLEMENTED("VFS: do_rename");
        return -1;*/
}

/* Make the named directory the current process's cwd (current working
 * directory).  Don't forget to down the refcount to the old cwd (vput()) and
 * up the refcount to the new cwd (open_namev() or vget()). Return 0 on
 * success.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        path does not exist.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 *      o ENOTDIR
 *        A component of path is not a directory.
 */
int
do_chdir(const char *path)
{

        if(strlen(path) > MAXPATHLEN)
        {
          dbg(DBG_PRINT,"(GRADING2B) one of the component in the path field is too long\n");
          dbg(DBG_PRINT,"do_chdir(): one of the component in the path field is too long\n");
          return -ENAMETOOLONG;
        } 
        if(strlen(path) <= 0)
        {
          dbg(DBG_PRINT,"(GRADING2B) Invalid path name\n");
          dbg(DBG_PRINT,"do_chdir(): Invalid path name\n");
          return -EINVAL;
        }

        size_t namelen;
        char *name;
        vnode_t *res_vnode;
        int ret_val =0;
        ret_val = open_namev(path,0,&res_vnode,NULL);

         if(ret_val<0)
         {
            dbg(DBG_PRINT,"(GRADING2B) Error while accessing open_namev\n");
            return ret_val;
         }

        if(!S_ISDIR(res_vnode->vn_mode))
        {
         vput(res_vnode);
         dbg(DBG_PRINT,"(GRADING2B) Given path is not a directory\n");
         return -ENOTDIR;
        }
       
        vput(curproc->p_cwd);
        curproc->p_cwd = res_vnode;
        
        return 0;

        /*NOT_YET_IMPLEMENTED("VFS: do_chdir");
        return -1;*/
}

/* Call the readdir fs_op on the given fd, filling in the given dirent_t*.
 * If the readdir fs_op is successful, it will return a positive value which
 * is the number of bytes copied to the dirent_t.  You need to increment the
 * file_t's f_pos by this amount.  As always, be aware of refcounts, check
 * the return value of the fget and the virtual function, and be sure the
 * virtual function exists (is not null) before calling it.
 *
 * Return either 0 or sizeof(dirent_t), or -errno.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        Invalid file descriptor fd.
 *      o ENOTDIR
 *        File descriptor does not refer to a directory.
 */
int
do_getdent(int fd, struct dirent *dirp)
{

     if(fd < 0 || fd >= NFILES) 
        {
                dbg(DBG_PRINT,"(GRADING2B) File Desciptor not valid\n");
                dbg(DBG_ERROR,"File Desciptor not valid\n");
                return -EBADF;
        }


        if(curproc->p_files[fd] == NULL) 
        {
                dbg(DBG_PRINT,"(GRADING2B) File Desciptor not valid\n");
                dbg(DBG_ERROR,"File Desciptor not valid\n");
                return -EBADF;
        }

        file_t *f;
        f=fget(fd);
        if (f==NULL)
        {
                dbg(DBG_PRINT,"(GRADING2B) File Desciptor not valid\n");
                dbg(DBG_ERROR,"File Desciptor not valid\n");
                return -EBADF; 
        }

            if (!(S_ISDIR(f->f_vnode->vn_mode)))
                 {
                dbg(DBG_PRINT,"(GRADING2B) Given path is not a directory\n");
                fput(f);
                return -ENOTDIR;
                 }
         if(f->f_vnode->vn_ops->readdir)
         {    
         dbg(DBG_PRINT,"(GRADING2B) Calling readdir function\n"); 
        int res=f->f_vnode->vn_ops->readdir(f->f_vnode,f->f_pos,dirp);
        
        fput(f);
        if(res==0)
        {
                dbg(DBG_PRINT,"(GRADING2B) End of file reached\n");
                return res;
        }
        else
        {   
                dbg(DBG_PRINT,"(GRADING2B) successfully read %d\n", res);
                f->f_pos+=res;
                return sizeof(dirent_t);
        }
    }
   /*NOT_YET_IMPLEMENTED("VFS: do_getdent");*/
        return -ENOTDIR;
}
/*
 * Modify f_pos according to offset and whence.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not an open file descriptor.
 *      o EINVAL
 *        whence is not one of SEEK_SET, SEEK_CUR, SEEK_END; or the resulting
 *        file offset would be negative.
 */
int
do_lseek(int fd, int offset, int whence)
{
     if(fd < 0 || fd >= NFILES) 
        {
                dbg(DBG_PRINT, "(GRADING2B) File Desciptor not valid\n");
                dbg(DBG_ERROR,"File Desciptor not valid\n");
                return -EBADF;
        }
        file_t *f;
        if((f=fget(fd)) == NULL)
        {
         dbg(DBG_PRINT, "(GRADING2B) File Desciptor not valid\n");
         dbg(DBG_PRINT, "Invalid file descriptor\n");
         return -EBADF;
        }
        if(f->f_pos+offset <0)
        {   
            fput(f);
            dbg(DBG_PRINT, "(GRADING2B) Invalid offset\n");
            dbg(DBG_PRINT, "Invalid offset\n");
            return -EINVAL;
        }
        if((whence == SEEK_SET) && (offset>=0))
        {
           dbg(DBG_PRINT, "(GRADING2B) Setting the file position to the given offset\n");
           f->f_pos = offset;
        }  
        else if((whence == SEEK_CUR) && (f->f_pos+offset>=0))
        {
           dbg(DBG_PRINT, "(GRADING2B) Incrementing the file position by offset value \n");
           f->f_pos = f->f_pos+offset;
        }
        else if((whence == SEEK_END) && (f->f_vnode->vn_len + offset >=0))
        {
           dbg(DBG_PRINT, "(GRADING2B) Setting the file position beyond end of line\n");
           f->f_pos = (f->f_vnode->vn_len)+offset;  /* the term in bracket gives us the end of file.*/
        }
           
       
        else
        {
            fput(f);

                dbg(DBG_PRINT, "(GRADING2B) Invalid whence\n");
                return -EINVAL;
        }

        if(f->f_pos < 0)
        {
               dbg(DBG_PRINT, "(GRADING2B) Setting the file position to 0\n");
            f->f_pos = 0;
        }
        fput(f);
        return f->f_pos;
        /*NOT_YET_IMPLEMENTED("VFS: do_lseek");
        return -1;*/
}

/*
 * Find the vnode associated with the path, and call the stat() vnode operation.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        A component of path does not exist.
 *      o ENOTDIR
 *        A component of the path prefix of path is not a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_stat(const char *path, struct stat *buf)
{

        if(strlen(path) > MAXPATHLEN)
        {
          dbg(DBG_PRINT,"(GRADING2B) A component in the path specified is too long\n");
          return -ENAMETOOLONG;
        }
        if(strlen(path) <= 0)
        {
          dbg(DBG_PRINT, "(GRADING2B) do_stat(): Path specified is not a valid path\n");
          return -EINVAL;
        }
        size_t namelen;
        const char *name;
        vnode_t *res_vnode;
        int ret_val =0;
        ret_val = dir_namev(path,&namelen,&name,NULL,&res_vnode);
        
        if(ret_val<0)
        {
            dbg(DBG_PRINT, "(GRADING2B) Error accessing dir_namev\n");
            return ret_val;
        }

        if(!S_ISDIR(res_vnode->vn_mode))
        {
         vput(res_vnode);
         dbg(DBG_PRINT, "(GRADING2B) given Path is not a directory\n");
         return -ENOTDIR;
        }
        vput(res_vnode);

        ret_val = lookup(res_vnode,name,namelen,&res_vnode);
        if(ret_val < 0)
        {
         dbg(DBG_PRINT, "(GRADING2B) Error in Lookup function\n");
         dbg(DBG_ERROR, "do_stat(): Error while returning from lookup()\n");
         return ret_val;
        }
        KASSERT(NULL != res_vnode->vn_ops->stat);
        dbg(DBG_PRINT, "(GRADING2A 3.f) do_stat(): function pointer to stat exists\n");
        vput(res_vnode);
        return res_vnode->vn_ops->stat(res_vnode,buf);
        
        /*NOT_YET_IMPLEMENTED("VFS: do_stat"); return -1;*/
}

#ifdef __MOUNTING__
/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutely sure your Weenix is perfect.
 *
 * This is the syscall entry point into vfs for mounting. You will need to
 * create the fs_t struct and populate its fs_dev and fs_type fields before
 * calling vfs's mountfunc(). mountfunc() will use the fields you populated
 * in order to determine which underlying filesystem's mount function should
 * be run, then it will finish setting up the fs_t struct. At this point you
 * have a fully functioning file system, however it is not mounted on the
 * virtual file system, you will need to call vfs_mount to do this.
 *
 * There are lots of things which can go wrong here. Make sure you have good
 * error handling. Remember the fs_dev and fs_type buffers have limited size
 * so you should not write arbitrary length strings to them.
 */
int
do_mount(const char *source, const char *target, const char *type)
{
        NOT_YET_IMPLEMENTED("MOUNTING: do_mount");
        return -EINVAL;
}

/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutley sure your Weenix is perfect.
 *
 * This function delegates all of the real work to vfs_umount. You should not worry
 * about freeing the fs_t struct here, that is done in vfs_umount. All this function
 * does is figure out which file system to pass to vfs_umount and do good error
 * checking.
 */
int
do_umount(const char *target)
{
        NOT_YET_IMPLEMENTED("MOUNTING: do_umount");
        return -EINVAL;
}
#endif
