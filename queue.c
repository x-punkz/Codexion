/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pqueue.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daniviei <daniviei@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 20:58:47 by daniviei          #+#    #+#             */
/*   Updated: 2026/07/28 21:06:34 by daniviei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*Aloca o heap. capacity = numero max de coders esperando. 1=ok, 0=erro*/
int	pq_init(t_queue *pq, int capacity, t_sched sched)
{
	if (capacity < 1)
		capacity = 1;
	pq->data = malloc(sizeof(t_request) * capacity);
	if (!pq->data)
		return (0);
	pq->size = 0;
	pq->capacity = capacity;
	pq->sched = sched;
	return (1);
}

/*pedido e reordena para cima. 0 = fila cheia.*/
int	pq_push(t_queue *pq, t_request req)
{
	if (pq->size >= pq->capacity)
		return (0);
	pq->data[pq->size] = req;
	pq->size++;
	shift_up(pq, pq->size - 1);
	return (1);
}
