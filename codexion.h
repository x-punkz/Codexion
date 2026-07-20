#ifndef CODEXION_H
# define CODEXION_H

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <ctype.h>

typedef struct	s_simu
{
    int     nbr_of_coders;
	long    time_to_burnout;
	long    time_to_compile;
	long    time_to_debug;
	long    time_to_refactor;
	int     nbr_of_compiles_required;
	long    cooldown;
	char    *scheduler;
}   t_simu;

typedef struct	s_coder
{
	int			id;
	int			nbr_of_copiles_done;
	int 		last_copile_start;
	int			left;
	int			right;
	t_simu		*simu;
	pthread_t	thread_id;
}	t_coder;

typedef struct	s_dongle
{
	pthread_mutex_t	occuped;
	pthread_cond_t	free;
	long			avaiable_at;
	t_queue			waiters;
	
}	t_dongle;

typedef struct s_request
{
	int		coder_id;
	long	seq;
	long	deadline;
}	t_request;

typedef enum	e_schedule
{
	POLICY_FIFO,
	POLICY_EDF
}	t_sched;

typedef struct s_queue
{
	t_request	*data;
	int			size;
	int			capacity;
	t_sched		sched;
}	t_queue;




/********** Parser **********/
int    parser(t_simu *simu, char **argv);
#endif
