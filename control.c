/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   control.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daniviei <daniviei@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 15:09:30 by daniviei          #+#    #+#             */
/*   Updated: 2026/07/29 15:09:44 by daniviei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Le a flag de parada de forma thread-safe. */
int	is_stopped(t_simu *simu)
{
	int	safe_return;

	pthread_mutex_lock(&simu->state_lock);
	safe_return = simu->stopped;
	pthread_mutex_unlock(&simu->state_lock);
	return (safe_return);
}

/*Sinaliza que a simulacao acabou (burnout ou todos compilaram o bastante)*/
void	set_stopped(t_simu *simu)
{
	pthread_mutex_lock(&simu->state_lock);
	simu->stopped = 1;
	pthread_mutex_unlock(&simu->state_lock);
}
