#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "queue.h"

queue *create_queue()
{
    queue *q = malloc(sizeof(queue));
    if (!q)
        exit(-1);
    
    q->size = 0;
    q->capacity = 0;
    q->container = NULL;
    return q;
}

void destroy_queue(queue *q)
{
    free(q->container);
    free(q);
}

Thread *peek(queue *q)
{
    for (int i = 0; i < q->size; i++)
        if (q->container[i]->status == READY)
            return q->container[i];

    return NULL;
}

Thread *pop(queue *q)
{
    Thread *top = NULL;
    int top_idx = 0;

    /**
     * Caut primul element din containerul cozii care are starea READY.
     * Ii retin pozitia cu ajutorul variabilei top_idx.
     */
    for (int i = 0; i < q->size; i++)
        if (q->container[i]->status == READY)
        {
            top = q->container[i];
            top_idx = i;
            break;
        }

    /**
     * Shiftez la stanga toata coada pentru a elimina threadul de pe pozitia top_idx.
     */
    for (int i = top_idx; i < q->size - 1; i++)
        q->container[i] = q->container[i + 1];

    q->size--;

    return top;
}

void realloc_queue(queue *q)
{   
    if (q->size > q->capacity)
    {
        Thread **new_ptr = (Thread **)realloc(q->container, q->size * sizeof(Thread *));
        if (!new_ptr)
            exit(-1);
        
        q->container = new_ptr;

        q->capacity++;
    }
}

void push(queue *q, Thread *t)
{
    q->size++;
    realloc_queue(q);

    int added = 0;
    for (int i = 0; i < q->size - 1; i++)
        if (q->container[i]->priority < t->priority)
        {   /**
            * Shiftez la dreapta coada ca sa ii fac loc noului thread.
            */
            for (int j = q->size - 1; j >= i && j >= 1; j--)
                q->container[j] = q->container[j - 1];

            q->container[i] = t;
            added = 1;
            break;
        }

    /**
     * Daca nu exista niciun element cu prioritatea mai mica decat threadul nou.
     */
    if (!added)
        q->container[q->size - 1] = t;
}

void basic_push(queue *q, Thread *t)
{
    q->size++;
    realloc_queue(q);

    q->container[q->size - 1] = t;
}