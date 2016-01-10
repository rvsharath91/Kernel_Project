Documentation for Kernel Assignment 2
=====================================

+-------+
| BUILD |
+-------+

Comments:make and do ./weenix -n to run.
+-----------------+
| SKIP (Optional) |
+-----------------+

Skip nothing to skip
+---------+
| GRADING |
+---------+

(A) KASSERT :  (46 out of 46)

    (1) In fs/vnode.c:
        (a) In special_file_read(): (6 out of 6)
        (b) In special_file_write(): (6 out of 6)

    (2) In fs/namev.c:
        (a) In lookup(): (6 out of 6)
        (b) In dir_namev():(10 out of 10)
        (c) In open_namev(): (2 out of 2)

    (3) In fs/vfs_syscall.c:
        (a) In do_write(): (6 out of 6)
        (b) In do_mknod(): (2 out of 2)
        (c) In do_mkdir(): (2 out of 2)
        (d) In do_rmdir(): (2 out of 2)
        (e) In do_unlink():(2 out of 2)
        (f) In do_stat(): (2 out of 2)
 

(B) vfstest (should be invoked under kshell): (39 out of 39)
 
(C) Ted Faber's file system tests (these must be invoked under kshell) (5 out of 5)
    (1) faber_fs_thread_test() (3 out of 3)
    (2) faber_directory_test() (2 out of 2)

(D) SELF-checks : Rename test 
					(1) rename_test() (10 out of 10)
          			All other parts are covered by vfstest

+---------------+
| Minus points: |
+---------------+

Missing/incomplete required section(s) in README file (vfs-README.txt): No section missed
Submitted binary file : No
Submitted extra (unmodified) file : No
Wrong file location in submission : No
Altered or removed top comment block in a .c file : Not altered/removed
Use dbg_print(...) instead of dbg(DBG_PRINT, ...) : used proper notations
Not properly indentify which dbg() printout is for which item in the grading guidelines : Properly identified
Cannot compile      : its compiling
Compiler warnings   : No warnings
"make clean"        : deletes all .o files
Kernel panic : Not happening
Cannot halt kernel cleanly : Kernel halts cleanly

+------+
| BUGS |
+------+
Comments: No bugs detected as such.

+---------------------------+
| Contribution from members |
+---------------------------+

-Equal Contribution from all four

+-----------------+
| Other (Optional)|
+-----------------+

Special DGB setting in Config.mk for certain tests: No
Comments on deiation from spec : No
General comments on design decisions: No

