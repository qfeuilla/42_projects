pistes :

gdb level3:
```
(gdb) b v
Breakpoint 1 at 0x80484ad
(gdb) r
Starting program: /home/user/level3/level3 

Breakpoint 1, 0x080484ad in v ()
(gdb) set {int}0x804988c=0x40
(gdb) print {int}0x804988c
$1 = 64
(gdb) step
Single stepping until exit from function v,
which has no line number information.
q
q
Wait what?!
$ cd ../level4
/bin/sh: 1: cd: can't cd to ../level4
$ cat ../level4/.pass
cat: ../level4/.pass: Permission denied
$ pwd
/home/user/level3
$ ls
level3
$ exit
0x08048525 in main ()
(gdb) quit
```
