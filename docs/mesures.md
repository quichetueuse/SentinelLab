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

### Mesure au repos

```bash
eliot@DESKTOP-TOI6LP5:/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2$ ./build/noeud --periode 100 --duree 5
[noeud] pid 17873, période 100 ms. kill -USR1 17873 = bouton, -USR2 = stats
t=   100 ms  T=22.00 °C  LED=off  PWM [###-------]  33 %
t=   200 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=   300 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=   400 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=   500 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=   600 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=   700 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=   800 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=   900 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1000 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1100 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1200 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1300 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1400 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1500 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1600 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1700 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1800 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1900 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2000 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2100 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2200 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2300 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2400 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2500 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2600 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2700 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2800 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2900 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3000 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3100 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3200 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3300 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3400 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3500 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3600 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3700 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3800 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3900 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4000 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4100 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4200 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4300 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4400 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4500 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4600 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4700 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4800 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4900 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  5000 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
=== statistiques ===
ticks            : 50
allocations      : init 4, régime 0
historique       : 49/64 mesures, dernière 22.42 °C
bouton           : 0 appui(s), LED off (ODR=0x00000000)
rapport cyclique : 49.9 % (période 100 ms, émission 50 ms)
courant moyen    : 49.911 mA, autonomie estimée 40 h (1.7 jours)
acquisition : gigue min 88 µs  max 449 µs  moy 312 µs
```

### Mesure en pleine charge (avec stress test)

```bash
eliot@DESKTOP-TOI6LP5:/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2$ stress-ng --cpu 4 --timeout 10s &
[1] 18061
eliot@DESKTOP-TOI6LP5:/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2$ ./build/noeud --periode 100 --duree 10stress-ng: info:  [18061] setting to a 10 secs run per stressor
stress-ng: info:  [18061] dispatching hogs: 4 cpu

[noeud] pid 18075, période 100 ms. kill -USR1 18075 = bouton, -USR2 = stats
t=   100 ms  T=22.00 °C  LED=off  PWM [###-------]  33 %
t=   200 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=   300 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=   400 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=   500 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=   600 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=   700 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=   800 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=   900 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1000 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1100 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1200 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1300 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1400 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1500 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1600 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1700 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1800 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  1900 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2000 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2100 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2200 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2300 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2400 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2500 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2600 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2700 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2800 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  2900 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3000 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3100 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3200 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3300 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3400 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3500 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3600 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3700 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3800 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  3900 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4000 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4100 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4200 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4300 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4400 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4500 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4600 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4700 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4800 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  4900 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  5000 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  5100 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  5200 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  5300 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  5400 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  5500 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  5600 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  5700 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  5800 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  5900 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  6000 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  6100 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  6200 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  6300 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  6400 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  6500 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  6600 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  6700 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  6800 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  6900 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  7000 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  7100 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  7200 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  7300 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  7400 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  7500 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  7600 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  7700 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  7800 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  7900 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  8000 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  8100 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  8200 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  8300 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
stress-ng: info:  [18061] skipped: 0
stress-ng: info:  [18061] passed: 4: cpu (4)
stress-ng: info:  [18061] failed: 0
stress-ng: info:  [18061] metrics untrustworthy: 0
stress-ng: info:  [18061] successful run completed in 10.01 secs
t=  8400 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  8500 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  8600 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  8700 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  8800 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  8900 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  9000 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  9100 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  9200 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  9300 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  9400 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  9500 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  9600 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  9700 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  9800 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t=  9900 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
t= 10000 ms  T=22.42 °C  LED=off  PWM [###-------]  37 %
=== statistiques ===
ticks            : 100
allocations      : init 4, régime 0
historique       : 64/64 mesures, dernière 22.42 °C
bouton           : 0 appui(s), LED off (ODR=0x00000000)
rapport cyclique : 50.0 % (période 100 ms, émission 50 ms)
courant moyen    : 49.996 mA, autonomie estimée 40 h (1.7 jours)
acquisition : gigue min 85 µs  max 828 µs  moy 166 µs
```

### Passerelle Java en multi-threading

```bash
eliot@DESKTOP-TOI6LP5:/mnt/c/Users/Eliot/Documents/C++/SentinelLab/TP-JOUR-2$ ./build/noeud --duree 10 | java java/Passerelle.java mesures.csv
[noeud] pid 18204, période 500 ms. kill -USR1 18204 = bouton, -USR2 = stats
[passerelle] écriture dans mesures.csv, pid 18205
Résumé: 0 mesures enregistrées dans le csv
[passerelle] 0 mesures, moyenne glissante 22.42 °C, VmRSS 134800 kB
acquisition : gigue min 84 µs  max 486 µs  moy 317 µs
```
