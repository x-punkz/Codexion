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
	long			avaiable_at;
	int				held;
	t_queue			waiters;
}	t_dongle;

typedef struct	s_simu	t_simu;

typedef struct	s_coder
{
	int				id;
	int				nbr_of_copiles_done;
	int 			last_copile_start;
	int				left;
	int				right;
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
long	milisec(void);

#endif
