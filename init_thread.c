/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_thread.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daniviei <daniviei@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 17:57:12 by daniviei          #+#    #+#             */
/*   Updated: 2026/07/23 17:57:16 by daniviei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_dongles(t_simu *simu)
{
	int	i;

	simu->dongles = malloc(sizeof(t_dongle) * simu->nbr_of_coders);
	if (!simu->dongles)
		return (0);
	memset(simu->dongles, 0, sizeof(t_dongle) * simu->nbr_of_coders);
	i = -1;
	while (++i < simu->nbr_of_coders)
	{
		simu->dongles[i].id = i;
		simu->dongles[i].available_at = 0;
		pthread_mutex_init(&simu->dongles[i].occuped, NULL);
		pthread_cond_init(&simu->dongles[i].free, NULL);
		if (!pq_init(&simu->dongles[i].waiters,
				simu->nbr_of_coders,
				simu->scheduler))
			return (0);
	}
	return (1);
}

static int	init_coders(t_simu *simu)
{
	int	i;

	simu->coders = malloc(sizeof(t_coder) * simu->nbr_of_coders);
	if (!simu->coders)
		return (0);
	memset(simu->coders, 0, sizeof(t_coder) * simu->nbr_of_coders);
	i = -1;
	while (++i < simu->nbr_of_coders)
	{
		simu->coders[i].id = i + 1;
		simu->coders[i].left = i;
		simu->coders[i].right = (i + 1) % simu->nbr_of_coders;
		simu->coders[i].last_compile_start = simu->start_time;
		simu->coders[i].simu = simu;
		pthread_mutex_init(&simu->coders[i].lock, NULL);
	}
	return (1);
}

/* Iniciando a o mutex da simulaçao*/
int	init_simu(t_simu *simu)
{
	pthread_mutex_init(&simu->log_lock, NULL);
	pthread_mutex_init(&simu->state_lock, NULL);
	pthread_mutex_init(&simu->seq_lock, NULL);
	simu->stopped = 0;
	simu->seq_counter = 0;
	simu->start_time = now_milisec();
	if (!init_dongles(simu))
		return (0);
	if (!init_coders(simu))
		return (0);
	return (1);
}

int	run_simu(t_simu *simu)
{
	int	i;

	i = -1;
	while (++i < simu->nbr_of_coders)
	{
		if (pthread_create(&simu->coders[i].thread_id,
				NULL, coder_routine,
				&simu->coders[i])
		)
			return (0);
	}
	if (pthread_create(&simu->monitor,
			NULL,
			monitor_routine,
			simu)
	)
		return (0);
	i = -1;
	while (++i < simu->nbr_of_coders)
		pthread_join(simu->coders[i].thread_id, NULL);
	pthread_join(simu->monitor, NULL);
	return (1);
}

void	destroy_simu(t_simu *simu)
{
	int	i;

	i = -1;
	if (simu->dongles)
	{
		while (++i < simu->nbr_of_coders)
		{
			pthread_mutex_destroy(&simu->dongles[i].occuped);
			pthread_cond_destroy(&simu->dongles[i].free);
			pq_free(&simu->dongles[i].waiters);
		}
	}
	i = -1;
	if (simu->coders)
	{
		while (++i < simu->nbr_of_coders)
			pthread_mutex_destroy(&simu->coders[i].lock);
		pthread_mutex_destroy(&simu->log_lock);
		pthread_mutex_destroy(&simu->state_lock);
		pthread_mutex_destroy(&simu->seq_lock);
		free(simu->dongles);
		free(simu->coders);
	}
}
