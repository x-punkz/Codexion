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