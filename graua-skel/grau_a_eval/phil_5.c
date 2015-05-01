/*
 * =====================================================================================
 *
 *       Filename:  phil_1.c
 *
 *    Description:  Com ordenação
 *
 *        Version:  1.0
 *        Created:  22-03-2015 19:31:24
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <assert.h>
#include <libthread/thread.h>
#include "cond.h"

#define PHIL_MAX_ITER  10000
#define PHIL_CHATTINESS 1000


enum phil_state { THINKING, HUNGRY, EATING };

struct phil_struct {
    enum phil_state *states;
    int *numbers;
    long long qn;
    sema_t state_mtx;
    cond_t *cv;
};

struct phil_thr_params {
    int phil_i;
    int n_phils;
    struct phil_struct * philosophers;
    struct timeval waiting_time;
};


int can_i_eat(struct phil_struct * phils, int n_phils, int me) {
    // 0 -> can't eat
    // 1 -> can eat
    int left = (me + n_phils-1) % n_phils;
    int right = (me + 1) % n_phils;

    if (phils->states[left] == EATING ||
            phils->states[right] == EATING) 
        return 0;
    else if (phils->states[left] == HUNGRY &&
            phils->numbers[left] < phils->numbers[me])
        return 0;
    else if (phils->states[right] == HUNGRY &&
            phils->numbers[right] < phils->numbers[me])
        return 0;

    return 1;
}

void phil_thr(void *p_) {
    int iter = PHIL_MAX_ITER;
    struct phil_thr_params *p = (struct phil_thr_params *) p_;

    printf("Philosopher #%d -- ready!\n", p->phil_i);

    while (iter--) {
        struct timeval start, end, res;
        int left, right;

        left = (p->phil_i + (p->n_phils - 1)) % p->n_phils ;
        right = (p->phil_i + 1) % p->n_phils ;

        if (!(iter % PHIL_CHATTINESS)) {
            printf("Philosopher #%d gonna eat for the %dth time\n", p->phil_i, PHIL_MAX_ITER - iter);
        }

        gettimeofday(&start, NULL);

        sema_wait(&p->philosophers->state_mtx);
        p->philosophers->states[p->phil_i] = HUNGRY;
        p->philosophers->numbers[p->phil_i] = p->philosophers->qn++;

        thread_yield();


        while (!can_i_eat(p->philosophers, p->n_phils, p->phil_i))
        {
            cond_wait(&p->philosophers->cv[p->phil_i], &p->philosophers->state_mtx);
        }

        thread_yield();

        p->philosophers->states[p->phil_i] = EATING;

        sema_signal(&p->philosophers->state_mtx);


        gettimeofday(&end, NULL);

        timersub(&end, &start, &res);

        timeradd(&p->waiting_time, &res, &p->waiting_time);


#ifndef NOSLEEP
        usleep(rand() % 1000);
#endif
        //printf("Philosopher #%d done eating\n", p->phil_i);

        sema_wait(&p->philosophers->state_mtx);
        thread_yield();
        p->philosophers->states[p->phil_i] = THINKING;


        if (p->philosophers->states[left] == HUNGRY)
            cond_signal(&p->philosophers->cv[left]);

        if (p->philosophers->states[right] == HUNGRY)
            cond_signal(&p->philosophers->cv[right]);

        sema_signal(&p->philosophers->state_mtx);

        if (!(iter % PHIL_CHATTINESS)) {
            printf("Philosopher #%d gonna think for the %dth time\n", p->phil_i, PHIL_MAX_ITER - iter);
        }

#ifndef NOSLEEP
        usleep(rand() % 100);
#endif
    }

    printf("Philosopher #%d -- done!\n", p->phil_i);

    thread_exit(NULL);
}

int main(int argc, char *argv[])
{
    struct phil_thr_params      *params;
    struct phil_struct          phils;
    thread_t                    **phil_ids;
    int                         i, n_phils;

    n_phils = atoi(argv[1]); // or segfault

    assert (n_phils > 1);

    params = malloc(n_phils * sizeof (struct phil_thr_params));
    phil_ids = malloc(n_phils * sizeof(thread_t*));
    phils.cv = malloc(n_phils * sizeof(pthread_cond_t));
    phils.states = malloc(n_phils * sizeof(enum phil_state));
    phils.numbers = malloc(n_phils * sizeof(long long));
    phils.qn = 0;

    assert(phils.cv);
    assert(phils.numbers);
    assert(params);
    assert(phil_ids);

    threading_init();

    for (i = 0; i < n_phils; i++) {
        cond_init(&phils.cv[i]);
        phils.states[i] = THINKING;
        phils.numbers[i] = 0;
    }


    sema_init(&phils.state_mtx, 1);

    for (i = 0; i < n_phils; i++) {
        params[i].philosophers = &phils;
        params[i].n_phils = n_phils;
        params[i].phil_i = i;
        params[i].waiting_time = (struct timeval){ .tv_sec = 0, .tv_usec = 0 };
    }

    printf("Starting philosophers\n");

    for (i = 0; i < n_phils; i++) {
        phil_ids[i] = thread_create(phil_thr, &params[i]);
    }


    printf("Waiting philosophers\n");

    for (i = 0; i < n_phils; i++) {
        thread_join(phil_ids[i]);
    }


    for (i = 0; i < n_phils; i++) {
        printf("Philosopher #%d waited %ldsecs + %ldusecs\n", 
                i, (long int)params[i].waiting_time.tv_sec,
                (long int)params[i].waiting_time.tv_usec);
    }

    threading_exit();


    return 0;
}

