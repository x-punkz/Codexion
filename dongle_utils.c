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
