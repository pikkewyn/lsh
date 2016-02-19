# lsh
very lazy shell

Zaimplementuj prostą wersję powłoki o nazwie lsh. Jak prawdziwa powłoka, lsh odczytuje linię ze standardowego wejścia i
przeszukuje ścieżki ze zmiennej PATH (inaczej mówiąc zamiast execve wykonuje execvp) i wykonuje podany program.
Proszę pamiętać o ustawieniu argumentów wykonywanej komendy. Jeśli linia kończy się znakiem (&), wtedy lsh powinien nie
czekać aż komenda zostanie skończona i od razu wrócić. W innym przypadku lsh powinien zaczekać, aż program wykona się.
lsh powinien skończyć swoje działanie naciskając klawisze Control+D lub pisząc exit.

Zaimplementuj w programie lsh z poprzedniego zadania potoki | (ang. pipe). Wskazówka: Zobacz program lssort.c

Kompilacja:
gcc -Wall --pedantic --std=gnu99 -o lsh lsh.c


Przyklad dzialania:
--------------------
./lsh
lsh> ls
akiso2013w22.png  alp-ch03-processes.pdf  execvp.c  lsh.c.bak   README.txt
akiso2013w23.png  a.out           lsh.c     lssort.png  test.c
lsh> echo foo
foo
lsh> sleep 20
^Clsh>
lsh> sleep 20&
lsh> sleep 20&
lsh>
lsh> pwd
/home/janek/development/lsh
lsh>
lsh>
lsh> sleep 20&
25336 exited normally
25338 exited normally
lsh>
lsh> exit

TODO:
-----------------------
+   obsluga procesu w tle przy &
-   zmienic wait, wypisuje jak ma kolejny background, lepiej jak by wypisywalo przy kolejnej komendzie
+   ctrl+c dla komendy
+   obsluga \n na koncu
+/- obsluga ctrl+d ----> non empty line
+   obsluga exit
-   obsluga kodu wyjscia
-   c99 standard compatibility
-   dodac obsluge pipes
-   przetestowac pod windowsem? ( moze byc problem ze znakiem nowej linii itd )
