Documentation for Kernel Assignment 1
=====================================

+-------------+
| BUILD & RUN |
+-------------+

Comments: make and then run ./weenix -n. Enter help to get the list all tests that can be run.

+-----------------+
| SKIP (Optional) |
+-----------------+

No.

+---------+
| GRADING |
+---------+

(A.1) In main/kmain.c:
    (a) In bootstrap(): 3 out of 3 pts
    (b) In initproc_create(): 3 out of 3 pts

(A.2) In proc/proc.c:
    (a) In proc_create(): 4 out of 4 pts
    (b) In proc_cleanup(): 5 out of 5 pts
    (c) In do_waitpid(): 8 out of 8 pts

(A.3) In proc/kthread.c:
    (a) In kthread_create(): 2 out of 2 pts
    (b) In kthread_cancel(): 1 out of 1 pt
    (c) In kthread_exit(): 3 out of 3 pts

(A.4) In proc/sched.c:
    (a) In sched_wakeup_on(): 1 out of 1 pt
    (b) In sched_make_runnable(): 1 out of 1 pt

(A.5) In proc/kmutex.c:
    (a) In kmutex_lock(): 1 out of 1 pt
    (b) In kmutex_lock_cancellable(): 1 out of 1 pt
    (c) In kmutex_unlock(): 2 out of 2 pts

(B) Kshell : 20 out of 20 pts
    Comments: 

(C.1) waitpid any test, etc. (4 out of 4 pts)
(C.2) Context switch test (1 out of 1 pt)
(C.3) wake me test, etc. (2 out of 2 pts)
(C.4) wake me uncancellable test, etc. (2 out of 2 pts)
(C.5) cancel me test, etc. (4 out of 4 pts)
(C.6) reparenting test, etc. (2 out of 2 pts)
(C.7) show race test, etc. (3 out of 3 pts)
(C.8) kill child procs test (2 out of 2 pts)
(C.9) proc kill all test (2 out of 2 pts)

(D.1) sunghan_test(): producer/consumer test (9 out of 9 pts)
(D.2) sunghan_deadlock_test(): deadlock test (4 out of 4 pts)

(E) Additional self-checks: Not done
    Comments: (please provide details, add subsections and/or items as needed)

Missing required section(s) in README file (procs-README.txt): All are filled
Submitted binary file : Not submitting binary file
Submitted extra (unmodified) file : No
Wrong file location in submission : (Comments?)
Altered or removed top comment block in a .c file : No
Use dbg_print(...) instead of dbg(DBG_PRINT, ...) : Have used the latter
Not properly indentify which dbg() printout is for which item in the grading guidelines : properly identified
Cannot compile : It can be compiled
Compiler warnings : No compiler warnings
"make clean" : Deletes all .o files
Useless KASSERT : Have used valid KASSERT
Insufficient/Confusing dbg : Nil
Kernel panic : No Kernel panic
Cannot halt kernel cleanly : Kernel halts cleanely

+------+
| BUGS |
+------+

Comments: Nil

+---------------------------+
| CONTRIBUTION FROM MEMBERS |
+---------------------------+

If not equal-share contribution, please list percentages.
-Equal Contribution

+------------------+
| OTHER (Optional) |
+------------------+

Special DBG setting in Config.mk for certain tests: No
Comments on deviation from spec (you will still lose points, but it's better to let the grader know): No
General comments on design decisions: No

