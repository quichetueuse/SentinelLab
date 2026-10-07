# Mesures obtenues lors des TPs

## TP3

```bash
eliot@DESKTOP-TOI6LP5:/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2$ ./build/noeud --duree 5
[noeud] pid 16218, période 500 ms. kill -USR1 16218 = bouton, -USR2 = stats
t= 500 ms T=22.45 °C LED=off PWM [###-------] 37 %
t= 1000 ms T=22.77 °C LED=off PWM [####------] 40 %
t= 1500 ms T=23.01 °C LED=off PWM [####------] 42 %
t= 2000 ms T=23.18 °C LED=off PWM [####------] 43 %
t= 2500 ms T=23.38 °C LED=off PWM [####------] 45 %
t= 3000 ms T=23.79 °C LED=off PWM [####------] 48 %
t= 3500 ms T=24.24 °C LED=off PWM [#####-----] 52 %
t= 4000 ms T=24.60 °C LED=off PWM [#####-----] 55 %
t= 4500 ms T=24.57 °C LED=off PWM [#####-----] 55 %
t= 5000 ms T=24.91 °C LED=off PWM [#####-----] 58 %
=== statistiques ===
ticks : 10
allocations : init 2, régime 0
historique : 10/64 mesures, dernière 24.91 °C
bouton : 0 appui(s), LED off (ODR=0x00000000)
rapport cyclique : 10.0 % (période 500 ms, émission 50 ms)
courant moyen : 10.002 mA, autonomie estimée 200 h (8.3 jours)
eliot@DESKTOP-TOI6LP5:/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2$ size build/noeud
text data bss dec hex filename
33194 1168 680 35042 88e2 build/noeud
```

```bash
eliot@DESKTOP-TOI6LP5:/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2$ ./build/noeud --duree 5 --fuite
[noeud] pid 16244, période 500 ms. kill -USR1 16244 = bouton, -USR2 = stats
Le programme est initialise, impossible d allouer plus de memoire.
Aborted ./build/noeud --duree 5 --fuite
```

```bash
eliot@DESKTOP-TOI6LP5:/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2$ g++ -fsanitize=address,undefined src/demo_bugs.cpp -o demo_bugs_asan
eliot@DESKTOP-TOI6LP5:/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2$ ./demo_bugs_asan tas
=================================================================
==16270==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x728f583e0060 at pc 0x5884b717a4f1 bp 0x7ffc4fe6f940 sp 0x7ffc4fe6f930
WRITE of size 4 at 0x728f583e0060 thread T0
#0 0x5884b717a4f0 in debordementTas() (/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2/demo_bugs_asan+0x24f0) (BuildId: c220ff6024c1aa0f41969cd5b081c23f7d59cf4a)
#1 0x5884b717aba0 in main (/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2/demo_bugs_asan+0x2ba0) (BuildId: c220ff6024c1aa0f41969cd5b081c23f7d59cf4a)
#2 0x765f5902a600 in **libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:59
#3 0x765f5902a717 in **libc_start_main_impl ../csu/libc-start.c:360
#4 0x5884b717a364 in \_start (/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2/demo_bugs_asan+0x2364) (BuildId: c220ff6024c1aa0f41969cd5b081c23f7d59cf4a)

0x728f583e0060 is located 0 bytes after 32-byte region [0x728f583e0040,0x728f583e0060)
allocated by thread T0 here:
#0 0x765f5a12c8cf in operator new[](unsigned long) ../../../../src/libsanitizer/asan/asan_new_delete.cpp:111
#1 0x5884b717a43f in debordementTas() (/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2/demo_bugs_asan+0x243f) (BuildId: c220ff6024c1aa0f41969cd5b081c23f7d59cf4a)
#2 0x5884b717aba0 in main (/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2/demo_bugs_asan+0x2ba0) (BuildId: c220ff6024c1aa0f41969cd5b081c23f7d59cf4a)
#3 0x765f5902a600 in **libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:59
#4 0x765f5902a717 in **libc_start_main_impl ../csu/libc-start.c:360
#5 0x5884b717a364 in \_start (/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2/demo_bugs_asan+0x2364) (BuildId: c220ff6024c1aa0f41969cd5b081c23f7d59cf4a)

SUMMARY: AddressSanitizer: heap-buffer-overflow (/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2/demo_bugs_asan+0x24f0) (BuildId: c220ff6024c1aa0f41969cd5b081c23f7d59cf4a) in debordementTas()
Shadow bytes around the buggy address:
0x728f583dfd80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
0x728f583dfe00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
0x728f583dfe80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
0x728f583dff00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
0x728f583dff80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
=>0x728f583e0000: fa fa 00 00 00 fa fa fa 00 00 00 00[fa]fa fa fa
0x728f583e0080: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
0x728f583e0100: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
0x728f583e0180: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
0x728f583e0200: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
0x728f583e0280: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
Addressable: 00
Partially addressable: 01 02 03 04 05 06 07
Heap left redzone: fa
Freed heap region: fd
Stack left redzone: f1
Stack mid redzone: f2
Stack right redzone: f3
Stack after return: f5
Stack use after scope: f8
Global redzone: f9
Global init order: f6
Poisoned by user: f7
Container overflow: fc
Array cookie: ac
Intra object redzone: bb
ASan internal: fe
Left alloca redzone: ca
Right alloca redzone: cb
==16270==ABORTING
```

```bash
eliot@DESKTOP-TOI6LP5:/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2$ valgrind --leak-check=full ./build/noeud --duree 5
==16283== Memcheck, a memory error detector
==16283== Copyright (C) 2002-2024, and GNU GPL'd, by Julian Seward et al.
==16283== Using Valgrind-3.26.0 and LibVEX; rerun with -h for copyright info
==16283== Command: ./build/noeud --duree 5
==16283==
[noeud] pid 16283, période 500 ms. kill -USR1 16283 = bouton, -USR2 = stats
t= 505 ms T=22.46 °C LED=off PWM [###-------] 37 %
t= 1000 ms T=22.77 °C LED=off PWM [####------] 40 %
t= 1500 ms T=23.01 °C LED=off PWM [####------] 42 %
t= 2000 ms T=23.18 °C LED=off PWM [####------] 43 %
t= 2500 ms T=23.38 °C LED=off PWM [####------] 45 %
t= 3000 ms T=23.79 °C LED=off PWM [####------] 48 %
t= 3500 ms T=24.24 °C LED=off PWM [#####-----] 52 %
t= 4000 ms T=24.60 °C LED=off PWM [#####-----] 55 %
t= 4500 ms T=24.57 °C LED=off PWM [#####-----] 55 %
t= 5000 ms T=24.91 °C LED=off PWM [#####-----] 58 %
=== statistiques ===
ticks : 10
allocations : init 0, régime 0
historique : 10/64 mesures, dernière 24.91 °C
bouton : 0 appui(s), LED off (ODR=0x00000000)
rapport cyclique : 10.5 % (période 500 ms, émission 50 ms)
courant moyen : 10.508 mA, autonomie estimée 190 h (7.9 jours)
==16283==
==16283== HEAP SUMMARY:
==16283== in use at exit: 0 bytes in 0 blocks
==16283== total heap usage: 5 allocs, 5 frees, 75,104 bytes allocated
==16283==
==16283== All heap blocks were freed -- no leaks are possible
==16283==
==16283== For lists of detected and suppressed errors, rerun with: -s
==16283== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

## TP5
