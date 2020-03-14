%line 1+1 kernel.s
[bits 32]
%line 4+1 kernel.s

[extern put_str]

[section .data]
intr_str db "interrupt occur!",0xa,0
[global intr_entry_table]

intr_entry_table:

%line 29+1 kernel.s

[section .text]
%line 30+0 kernel.s
intr0x00entry:
 push 0x0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
[section .data]
 dd intr0x00entry
%line 31+1 kernel.s
[section .text]
%line 31+0 kernel.s
intr0x01entry:
 push 0x0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
[section .data]
 dd intr0x01entry
%line 32+1 kernel.s
[section .text]
%line 32+0 kernel.s
intr0x02entry:
 push 0x0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
[section .data]
 dd intr0x02entry
%line 33+1 kernel.s
[section .text]
%line 33+0 kernel.s
intr0x03entry:
 push 0x0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
[section .data]
 dd intr0x03entry
%line 34+1 kernel.s
[section .text]
%line 34+0 kernel.s
intr0x04entry:
 push 0x0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
[section .data]
 dd intr0x04entry
%line 35+1 kernel.s
[section .text]
%line 35+0 kernel.s
intr0x05entry:
 push 0x0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
[section .data]
 dd intr0x05entry
%line 36+1 kernel.s
[section .text]
%line 36+0 kernel.s
intr0x06entry:
 push 0x0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
[section .data]
 dd intr0x06entry
