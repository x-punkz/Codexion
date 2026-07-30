/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daniviei <daniviei@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 20:36:31 by daniviei          #+#    #+#             */
/*   Updated: 2026/07/28 20:36:32 by daniviei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	take_dongle(t_coder *coder, t_dongle *dongle)
{
	t_request	req;

	make_request(coder, &req);
	pthread_mutex_lock(&dongle->occuped);
	pq_push(&dongle->waiters, req);
	while (!is_stopped(coder->simu) && !can_take(dongle, coder))
		wait_turn(dongle, coder);
	if (is_stopped(coder->simu))
	{
		pthread_mutex_unlock(&dongle->occuped);
		return (0);
	}
	pq_pop(&dongle->waiters, &req);
	dongle->held = 1;
	pthread_mutex_unlock(&dongle->occuped);
	log_state(coder, "has taken a dongle");
	return (1);
}

/*
** Libera o dongle e inicia o cooldown. O broadcast (feito com o lock em maos)
** acorda os que esperam sem risco de lost-wakeup.
*/
void	release_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->occuped);
	dongle->held = 0;
	dongle->available_at = now_milisec() + coder->simu->cooldown;
	pthread_cond_broadcast(&dongle->free);
	pthread_mutex_unlock(&dongle->occuped);
}