/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daniviei <daniviei@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 19:14:00 by daniviei          #+#    #+#             */
/*   Updated: 2026/07/28 19:14:03 by daniviei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	lone_coder(t_coder *coder)
{
	take_dongle(coder, &coder->simu->dongles[0]);
	take_dongle(coder, &coder->simu->dongles[0]);
}
/*
** Thread do coder. Ordena os dois dongles por id (menor primeiro) para
** quebrar a espera circular de Coffman => sem deadlock.
*/
void	*coder_routine(void *arg)
{
	t_coder	*coder;
	int		lower_id;
	int		high_id;

	coder = (t_coder *)arg;
	lower_id = coder->left;
	high_id = coder->right;
	if (lower_id > high_id)
	{
		lower_id = coder->right;
		high_id = coder->left;
	}
	if (coder->simu->nbr_of_coders == 1)
	{
		lone_coder(coder);
		return (NULL);
	}
	while (!is_stopped(coder->simu))
	{
		if (!compile_cycle(coder, lower_id, high_id))
			break;
	}
	return (NULL);
}