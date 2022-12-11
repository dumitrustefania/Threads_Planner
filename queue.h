#ifndef QUEUE_H_
#define QUEUE_H_

#include "structs.h"

/**
 * Aloc memorie pentru coada si ii initializez valorile.
 * Coada este de fapt o structura ce contine campurile size,
 * capacity, cat si un vector de threaduri alocat dinamic.
 */
queue* create_queue();

/**
 * Eliberez memoria pentru coada.
 */
void destroy_queue(queue *);

/**
 * Realoc memoria containerului cozii, incrementand-o cu o unitate.
 */
void realloc_queue(queue *);

/**
 * Adaug un nou thread in coada, mentinand principiul unei cozi
 * de prioritati. Asadar, threadurile vor fi ordonate mereu in functie
 * de prioritatea lor. Cele cu prioritatea mai mare vor fi primele.
 */
void push(queue *, Thread *);

/**
 * Adaug un nou thread in coada pe prima pozitie disponibila.
 */
void basic_push(queue *, Thread *);

/**
 * Returnez primul element din coada cu starea READY.
 */
Thread* peek(queue *);

/**
 * Returnez si elimin primul element din coada cu starea READY.
 */
Thread * pop(queue *);

#endif      /* QUEUE_H_ */
