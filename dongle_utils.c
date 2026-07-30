/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daniviei <daniviei@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 20:38:27 by daniviei          #+#    #+#             */
/*   Updated: 2026/07/28 20:38:30 by daniviei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Monta o pedido do coder: deadline (para EDF) e seq global (chegada, para
** FIFO e desempate do EDF). seq vem de um contador protegido por seq_lock.
*/
void	make_request(t_coder *coder, t_request *req)
{
	pthread_mutex_lock(&coder->lock);
	req->deadline = coder->last_compile_start + coder->simu->time_to_burnout;
	pthread_mutex_unlock(&coder->lock);
	req->coder_id = coder->id;
	pthread_mutex_lock(&coder->simu->seq_lock);
	req->seq = coder->simu->seq_counter;
	coder->simu->seq_counter++;
	pthread_mutex_unlock(&coder->simu->seq_lock);
}

/* 1 se o coder pode pegar o dongle agora: livre, fora do cooldown e no topo. */
int	can_take(t_dongle *dongle, t_coder *coder)
{
	t_request	top;

	if (dongle->held || now_milisec() < dongle->available_at)
		return (0);
	if (!pq_peek(&dongle->waiters, &top))
		return (0);
	return (top.coder_id == coder->id);
}

/** Espera a vez. Se ja sou o topo e so falta o cooldown, durmo com timeout ate
** available_at; caso contrario espero um broadcast (release/parada).*/
void	wait_turn(t_dongle *dongle, t_coder *coder)
{
	struct timespec	time_spec;
	t_request		top;

	if (!dongle->held
		&& pq_peek(&dongle->waiters, &top)
		&& top.coder_id == coder->id)
	{
		time_spec.tv_sec = dongle->available_at / 1000;
		time_spec.tv_nsec = (dongle->available_at % 1000) * 1000000;
		pthread_cond_timedwait(&dongle->free, &dongle->occuped, &time_spec);
	}
	else
		pthread_cond_wait(&dongle->free, &dongle->occuped);
}
