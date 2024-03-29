# PLANIFICATOR DE THREADURI

322CA - Bianca Ștefania Dumitru
Sisteme de operare

Decembrie 2022
----------------------------------------------------------------------------------------------------
## Introducere

* Planificator de threaduri
  *  programul implementeaza un planificator de threaduri preemptiv in Linux
  *  planificatorul este bazat pe un algoritm de tip Round Robin cu prioritati
  *  sistemul simulat este unul de tip uniprocesor, asadar un singur thread
        poate rula la un anumit moment
  *  ca mecanisme de sincronizare sunt folosite semafoare, fiecarui thread
        fiindu-i asignat cate unul  

## Continut

Proiectul este constituit din cateva fisiere, care compun logica principala
a planificatorului:

* 'queue.c' - Implementeaza o structura de date de tip coada de prioritati,
              dispunand de operatii precum push, peek, pop.
* 'queue.h' - Headerul corespunzator cozii.
* 'so_scheduler.c' - Implementeaza planificatorul si logica principala a
                     bibliotecii dinamice create. Acesta dispune de anumite
                     metode ce pot fi apelate pentru gestionarea unui program
                     ce lucreaza cu threaduri: 
    * 'so_init' - Initializeaza planificatorul.
    * 'so_fork' - Creeaza un nou thread si il pune in functiune.
    * 'so_wait' - Aduce threadul curent in starea de asteptare dupa un dispozitiv I/O.
    * 'so_signal' - Readuce toate threadurile blocate de un anumit dispozitiv I/O
                    in starea de a fi gata sa ruleze.
    * 'so_end' - Asteapta terminarea threadurilor si elibereaza resursele.
* 'so_scheduler.h' - Headerul corespunzator planificatorului.

## Cum functioneaza?

Programul implementeaza planificatorul ca pe o biblioteca dinamica.

Primul pas logic ar fi apelarea functiei so_init, ce initializeaza
planificatorul si elementele care ii corespund acestuia. Planificatorul
are asociata o cuanta de timp, ce reprezinta practic numarul maxim de actiuni
pe care are voie un thread sa le execute pana este preemptat.

Mai apoi, pot interveni operatii de tipul so_fork, so_wait sau so_signal, 
in orice ordine.

La apelarea so_fork, trebuie creat un thread cu prioritatea data si pus
sa ruleze functia handler data. Noul thread este introdus si in cele 2 cozi
ale planificatorui pentru a putea reveni la el cand e nevoie.

La apelarea so_wait, threadul curent este pus in asteptare de catre un
dispozitiv I/O dat ca parametru.

La apelarea so_signal, toate threadurile ce au fost blocate de dispozitivul
I/O dat ca parametru sunt deblocate.

Dupa executarea operatiilor specifice fiecarei functii, este apelata si
functia reschedule, care verifica daca threadul curent ar trebui inlocuit cu
urmatorul din coada de prioritati. Situatiile cand acesta ar trebui inlocuit sunt:
* nu exista un thread care ruleaza in acest moment
* threadul curent a fost pus in starea de asteptare de catre un dispozitiv I/O
* threadul curent si-a finalizat executia functiei handler
* a aparut in program un nou thread cu prioritatea mai mare decat cel curent
* timpul asociat threadului curent a fost terminat

La finalul executiei tuturor apelurilor so_fork, so_wait sau so_signal, este asteptat
apelul so_end, care permite tuturor threadurilor create sa isi termine executia,
eliberand mai apoi memoria tuturor resurselor folosite(semafoare, threaduri, cozi,
planificator).

## Cum se ruleaza programul?
Pentru a construi biblioteca dinamica, trebuie rulata comanda 'make' in Linux.

Pentru a testa biblioteca folosind testele prezente in fisierul '_test', trebuie
rulat 'make -f Makefile.checker'. Daca dorim sa rulam doar testul cu numarul
k, trebuie rulat 'LD_LIBRARY_PATH=. ./_test/run_test k'.

## Resurse
* https://ocw.cs.pub.ro/courses/so/teme/tema-4#round_robin_cu_prioritati
* https://ocw.cs.pub.ro/courses/so/cursuri/curs-04
* https://ocw.cs.pub.ro/courses/so/cursuri/curs-08
* https://ocw.cs.pub.ro/courses/so/cursuri/curs-09
* https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-08

* https://www.geeksforgeeks.org/use-posix-semaphores-c/
* https://www.geeksforgeeks.org/multithreading-c-2/

* https://linux.die.net/man/3/sem_init
* https://linux.die.net/man/3/sem_wait
* https://linux.die.net/man/3/sem_post
* https://man7.org/linux/man-pages/man3/pthread_create.3.html
* https://man7.org/linux/man-pages/man3/pthread_join.3.html
