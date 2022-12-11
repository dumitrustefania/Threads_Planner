#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "queue.h"
#include "structs.h"
#include "so_scheduler.h"

Scheduler *sched = NULL;

/**
 * Extragem primul thread din coada de prioritati si il facem sa ruleze.
 * Dupa ce i-am dat pop, ii dam si push inapoi (o sa fie plasat imediat 
 * dupa toate threadurile cu prioritatea egala cu a sa).
 * Facem acest lucru doarece implementarea mea presupune ca in coada
 * de prioritati trebuie sa existe in orice moment toate threadurile
 * create pana atunci, indiferent de starea lor.
 */
void run_next()
{
	Thread *next = pop(sched->pq);
	push(sched->pq, next);
	next->status = RUNNING;
	next->time = 0;
	sched->running = next;
	sem_post(&next->sem);
}

/**
 * Functie care verifica daca threadul care ruleaza in prezent trebuie inlocuit.
 * Daca da, aceste e inlocuit corespunzator cu urmatorul in coada de prioritati.
 */
void reschedule()
{
	Thread *t = sched->running;
	Thread *next = peek(sched->pq);

	/**
	 * Daca nu mai exista niciun thread READY, disponibil sa ruleze,
	 * inseamna ca am ajuns la sfarsitul programului, asa ca ne intoarcem.
     */
	if (!next)
	{	
		sem_post(&t->sem);
		return;
	}
	
	/**
	 * Daca nu ruleaza niciun thread in prezent sau threadul care a rulat pana
	 * acum a fost pus in starea de WAITING sau TERMINATED, cautam urmatorul
	 * thread si il punem sa ruleze.
     */
	if (t == NULL || t->status == WAITING || t->status == TERMINATED)
	{	
		run_next();
		return;
	}

	/**
	 * Daca a fost introdus in coada de prioritati un nou thread ce are
	 * prioritatea mai mare decat threadul care rula in prezent,
	 * il punem pe cel cu prioritatea mai mare sa ruleze
	 * Threadul curent isi schimba starea din RUNNING in READY.
     */
	if (t->priority < next->priority)
	{
		t->status = READY;
		run_next();
		return;
	}

	/**
	 * Daca threadului curent i-a expirat cuanta de timp,
	 * verificam daca exista vreun thread cu o prioritate cel putin egala
	 * sau mai mare care sa ii poata lua locul.
	 * Daca exista, il inlocuim cu acesta, iar daca nu, ii resetam cuanta de timp
	 * si il lasam tot pe el sa ruleze in continuare.
     */
	if (t->time >= sched->time)
	{
		if (t->priority <= next->priority)
		{
			run_next();
			t->status = READY;
			return;
		}
		else
			t->time = 0;
	}

	sem_post(&t->sem);
}

/**
 * Functia apelata odata cu crearea fiecarui nou thread.
 */
static void *start_thread(void *args)
{
	Thread *t = (Thread *)args;

	sem_wait(&t->sem);

	/**
	 * Este rulat handlerul dat ca parametru threadului la creare.
     */
	t->handler(t->priority);

	/**
	 * Threadul e marcat ca avand executia terminata.
     */
	t->status = TERMINATED;

	/**
	 * Schedulerul cauta urmatorul thread care sa ruleze.
     */
	reschedule();
	pthread_exit(NULL);
}

/**
 * Functie care creeaza un nou thread. Aloc memorie pentru acesta
 * si ii initializez valorile, inclusiv semaforul.
 */
Thread *new_thread(so_handler *func, unsigned int priority)
{
	Thread *t = malloc(sizeof(Thread));
	if (!t)
        exit(-1);

	t->handler = func;
	t->status = NEW;
	t->priority = priority;
	t->handler = func;
	t->time = 0;
	t->io = -1;

	int ret = sem_init(&t->sem, 0, 0);
	if (ret)
		exit(-1);

	return t;
}

/**
 * Initializez scheduler-ul de threaduri. Aloc memorie pentru acesta
 * si ii initializez valorile, creand si cele 2 cozi (cea de prioritati
 * si cea normala), in care vor fi stocate threadurile create.
 */
int so_init(unsigned int time, unsigned int io)
{
	if (sched || time == 0 || io > SO_MAX_NUM_EVENTS)
		return -1;

	sched = malloc(sizeof(*sched));
	if (!sched)
        exit(-1);

	sched->time = time;
	sched->io = io;
	sched->running = NULL;
	sched->pq = create_queue();
	sched->threads = create_queue();

	return 0;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	if (func == NULL || priority < 0 || priority > SO_MAX_PRIO)
		return INVALID_TID;

	/**
	 * Creez un nou thread folosind parametrii primiti. Functia pthread_create
	 * porneste threadul, invocand rutina start_thread cu argumentul new_t.
     */
	Thread *new_t = new_thread(func, priority);
	int ret = pthread_create(&new_t->tid, NULL, &start_thread, (void *)new_t);
	if (ret)
		exit(-1);	

	/**
	 * Dupa pornirea threadului, e marcat ca fiind READY si introdus in cele 2 cozi.
     */
	new_t->status = READY;
	push(sched->pq, new_t);
	basic_push(sched->threads, new_t);

	/**
	 * Verifcam daca crearea noului thread influenteaza threadul care ruleaza in prezent,
	 * prin apelarea functiei reschedule(). Daca exista un thread care ruleaza in prezent,
	 * ii crestem si cuanta de timp cu o unitate.
     */
	Thread *t = sched->running;
	if (t)
	{
		t->time++;
		reschedule();
		sem_wait(&t->sem);
	}
	else
		reschedule();

	return new_t->tid;
}

int so_signal(unsigned int io)
{
	int count= 0;

	if (io >= sched->io)
		return -1;

	/**
	 * Tuturor threadurilor din coada de prioritati care sunt blocate in prezent
	 * de I/O dat ca parametru le este schimbata starea in READY, iar I/O-ul care 
	 * le blocheaza in prezent este setat pe -1 (deoarece nu mai sunt blocate).
     */
	for (int i = 0; i < sched->pq->size; i++)
	{	Thread *t = sched->pq->container[i]; 
		if (t->io == io)
		{	
			t->status = READY;
			t->io = -1;
			count++;
		}
	}

	/**
	 * Incrementez cuanta de timp a threadului care ruleaza in prezent cu o unitate
	 * si apelez reschedule, ca sa schimb threadul curent in caz ca este nevoie.
     */
	Thread *t = sched->running;

	t->time++;
	reschedule();
	sem_wait(&t->sem);

	return count;
}

int so_wait(unsigned int io)
{
	if (io >= sched->io)
		return -1;

	Thread *t = sched->running;
	/**
	 * Threadul curent este pus in starea de WAITING si marcat ca blocat de I/O dat.
     */
	t->io = io;
	t->status = WAITING;

	/**
	 * Incrementez cuanta de timp a threadului care ruleaza in prezent cu o unitate
	 * si apelez reschedule, pentru ca threadul a fost blocat, asa ca e obligatoriu
	 * sa ruleze urmatorul la rand.
     */
	t->time++;
	reschedule();
	sem_wait(&t->sem);

	return 0;
}

void so_exec(void)
{
	Thread *t = sched->running;

	/**
	 * Incrementez cuanta de timp a threadului care ruleaza in prezent cu o unitate
	 * si apelez reschedule, ca sa schimb threadul curent in caz ca i s-a terminat
	 * timpul alocat.
     */
	t->time++;
	reschedule();
	sem_wait(&t->sem);
}

void so_end(void)
{
	if (!sched)
		return;

	/**
	 * Las toate threadurile salvate in coada simpla sa se termine.
	 */ 
	for (int i = 0; i < sched->threads->size; i++)
		pthread_join(sched->threads->container[i]->tid, NULL);

	/**
	 * Distrug toate semafoarele threadurilor create si eliberez memoria
	 * threadurilor.
     */
	for (int i = 0; i < sched->pq->size; i++)
	{
		sem_destroy(&sched->pq->container[i]->sem);
		free(sched->pq->container[i]);
	}

	/**
	 * Eliberez memoria pentru cele 2 cozi si scheduler.
     */
	destroy_queue(sched->threads);
	destroy_queue(sched->pq);
	free(sched);
	sched = NULL;
}
