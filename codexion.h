#ifndef CODEXION_H
# define CODEXION_H

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <ctype.h>

typedef enum	e_schedule
{
	POLICY_FIFO,
	POLICY_EDF
}	t_sched;

typedef struct s_request
{
	int		coder_id;
	long	seq;
	long	deadline;
}	t_request;

typedef struct s_queue
{
	t_request	*data;
	int			size;
	int			capacity;
	t_sched		sched;
}	t_queue;

typedef struct	s_dongle
{
	int				id;
	pthread_mutex_t	occuped;
	pthread_cond_t	free;
	long			available_at;
	int				held;
	t_queue			waiters;
}	t_dongle;

typedef struct	s_simu	t_simu;

typedef struct	s_coder
{
	int				id;
	int				nbr_of_copiles_done;
	int				left;
	int				right;
	long			last_compile_start;
	int				compiles;
	t_simu			*simu;
	pthread_t		thread_id;
	pthread_mutex_t	lock;
}	t_coder;

struct	s_simu
{
	int     		nbr_of_coders;
	long    		time_to_burnout;
	long    		time_to_compile;
	long    		time_to_debug;
	long    		time_to_refactor;
	int     		nbr_of_compiles_required;
	long    		cooldown;
	t_sched    		scheduler;

	long			start_time;
	long			seq_counter;
	int				stopped;

	t_coder			*coders;
	t_dongle		*dongles;
	pthread_t		monitor;

	pthread_mutex_t	log_lock;
	pthread_mutex_t	state_lock;
	pthread_mutex_t	seq_lock;
};

/********** Parser **********/
int		parser(t_simu *simu, char **argv);

/******** time / log ********/
long	now_milisec(void);
long	elapsed_milisec(t_simu *simu);
void	write_line(t_coder *coder, const char *msg);
void	log_state(t_coder *coder, const char *msg);

/*** init / run / destroy ***/
int		init_simu(t_simu *simu);
int		run_simu(t_simu *simu);
void	destroy_simu(t_simu *simu);

/******** threads ***********/
void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);

/******** dongles ***********/
int		take_dongle(t_coder *coder, t_dongle *dongle);
void	release_dongle(t_coder *c, t_dongle *d);
void	make_request(t_coder *c, t_request *req);
int		can_take(t_dongle *d, t_coder *c);
void	wait_turn(t_dongle *d, t_coder *c);

/* ---- fila de prioridade ---- */
int		pq_init(t_queue *pq, int capacity, t_sched sched);
int		pq_push(t_queue *pq, t_request req);
int		pq_pop(t_queue *pq, t_request *out);
int		pq_peek(t_queue *pq, t_request *out);
void	pq_free(t_queue *pq);

/* internos do heap (pqueue_utils.c) */
int		req_before(t_request a, t_request b, t_sched sched);
void	shift_up(t_queue *pq, int i);
void	shift_down(t_queue *pq, int i);

/* ---- controle de parada ---- */
int		is_stopped(t_simu *simu);
void	set_stopped(t_simu *simu);

#endif
