# ProjektTIN [![Build Status](https://travis-ci.org/fl0rek/ProjektTIN.svg?branch=master)](https://travis-ci.org/fl0rek/ProjektTIN)
Projekt na przedmiot TIN realizowany w 4 osobowym zespole.

Treść zadania:

Gracze mogą znajdować się w przestrzeni IPv4 i IPv6. Można przyjąć, że zaproszenie do zabawy rozsyłane jest e-mailem przez inicjatora, który ma adres publiczny. Uczestnikami sesji mogą być widzowie; widzowie mogą rozgłaszać pomiędzy sobą komentarze. Po rozgrywce wszyscy mogą rozgłaszać komentarze i możliwe jest wówczas prześledzenie przebiegu rozgrywki. Do komentowania można wybierać historyczne rozgrywki rejestrowane na komputerze inicjatora. Ponadto należy zaprojektować moduł do Wireshark umożliwiający wyświetlanie i analizę zdefiniowanych komunikatów.

## Dependencies:
 - cmake >= 3.1
 - gcc >= 5
 or
 - clang >= 3.9
 - libboost >= 1.55
 - qt5

## Building:

```bash
$ ./build.sh
```

or manually:

```bash
  # ClientAPP and ServerAPP
$ mdkir build
$ cmake ..
$ make

  # GameAPP
$ mkdir build/GameAPP
$ cd build/GameAPP
$ qmake --qt=qt5 ../../GameAPP
$ make

  # ChatAPP
$ mkdir build/ChatAPP
$ cd build/ChatAPP
$ qmake --qt=qt5 ../../ChatAPP
$ make
```
