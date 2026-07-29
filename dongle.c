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

int	take_dongles(t_coder *coder, t_dongle *dongles)
{
	t_request	req;

	make_request(coder, &req);
	pthread_mutex_lock(&dongles->occuped);
	pq_push(&dongles->waiters, req);
}