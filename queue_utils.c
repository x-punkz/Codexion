/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daniviei <daniviei@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 21:08:03 by daniviei          #+#    #+#             */
/*   Updated: 2026/07/28 21:08:16 by daniviei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Retorna 1 se `a` deve ser servido antes de `b`.
** FIFO: menor seq (ordem de chegada).
** EDF:  menor deadline; empate desfeito pelo menor seq (regra deterministica).
*/
int	req_before(t_request a, t_request b, t_sched sched)
{
	if (sched == POLICY_EDF)
	{
		if (a.deadline != b.deadline)
			return (a.deadline < b.deadline);
	}
	return (a.seq < b.seq);
}

/* Sobe o elemento em 'i' enquanto tiver prioridade maior que o pai. */
void	shift_up(t_queue *pq, int i)
{
	int			parent;
	t_request	tmp;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!req_before(pq->data[i], pq->data[parent], pq->sched))
			break ;
		tmp = pq->data[parent];
		pq->data[parent] = pq->data[i];
		pq->data[i] = tmp;
		i = parent;
	}
}

void	shift_down(t_queue *pq, int i)
{
	int			best;
	int			child;
	t_request	tmp;

	best = i;
	child = 2 * i + 1;
	while (child < pq->size)
	{
		if (req_before(pq->data[child], pq->data[best], pq->sched))
			best = child;
		if (child + 1 < pq->size
			&& req_before(pq->data[child + 1], pq->data[best], pq->sched))
			best = child + 1;
		if (best == i)
			break ;
		tmp = pq->data[i];
		pq->data[i] = pq->data[best];
		pq->data[best] = tmp;
		i = best;
		child = 2 * i + 1;
	}
}
