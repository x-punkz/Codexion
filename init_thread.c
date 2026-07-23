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

int	init_dongles(t_simu *simu)
{
	int	i;

	if (!(simu->dongles = malloc(sizeof(t_dongle) * simu->nbr_of_coders)));
		return (0);
	memset(simu->dongles, 0, sizeof(t_dongle) * simu->nbr_of_coders);
	i = -1;
	while (++i < simu->nbr_of_coders)
	{
		simu->dongles[i].id = i;
		simu->dongles[i].avaiable_at = 0;
		pthread_mutex_init(&simu->dongles[i].occuped, NULL);
		pthread_cond_init(&simu->dongles[i].free, NULL);
		if (!pq_init(&simu->dongles[i].waiters,
				simu->nbr_of_coders,
				simu->scheduler))
			return (0);
	}
	return (1);
}

int	init_coders()

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
