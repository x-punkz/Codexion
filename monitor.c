/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daniviei <daniviei@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 19:38:54 by daniviei          #+#    #+#             */
/*   Updated: 2026/07/29 19:39:09 by daniviei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* 1 se o coder passou do prazo sem comecar a compilar*/
static int	check_burnout(t_coder *coder)
{
	long	last;

	pthread_mutex_lock(&coder->lock);
	last = coder->last_compile_start;
	pthread_mutex_unlock(&coder->lock);
	return (now_milisec() - last >= coder->simu->time_to_burnout);
}

/** Anuncia o burnout e para a simulacao de forma atomica: ambos sob log_lock,
 garantindo que nenhuma outra mensagem apareca depois do "burned out".*/
static void	declare_burnout(t_coder *coder)
{
	pthread_mutex_lock(&coder->simu->log_lock);
	if (!is_stopped(coder->simu))
	{
		write_line(coder, "burned out");
		set_stopped(coder->simu);
	}
	pthread_mutex_unlock(&coder->simu->log_lock);
}

/*Acorda todos os que esperam por dongles, para que possam sair na parada.*/
static void	wake_all_dongles(t_simu *simu)
{
	int	i;

	i = -1;
	while (++i < simu->nbr_of_coders)
	{
		pthread_mutex_lock(&simu->dongles[i].occuped);
		pthread_cond_broadcast(&simu->dongles[i].free);
		pthread_mutex_unlock(&simu->dongles[i].occuped);
	}
	
}

/* 1 se todos ja compilaram o numero exigido de vezes.*/
static int	all_done(t_simu *simu)
{
	int	i;
	int	done;

	i = -1;
	while (++i < simu->nbr_of_coders)
	{
		pthread_mutex_lock(&simu->coders[i].lock);
		done = (simu->coders[i].compiles >= simu->nbr_of_compiles_required);
		pthread_mutex_unlock(&simu->coders[i].lock);
		if (!done)
			return (0);
	}
	return (1);
}

/* Thread monitora: varre os coders detectando burnout (< 10ms) e checa a
 condicao de termino. Ao parar, acorda quem estiver esperando por dongles.*/
void	*monitor_routine(void *arg)
{
	t_simu	*simu;
	int		i;

	simu = (t_simu *)arg;
	while (!is_stopped(simu))
	{
		i = -1;
		while (++i < simu->nbr_of_coders)
		{
			if (check_burnout(&simu->coders[i]))
			{
				declare_burnout(&simu->coders[i]);
				wake_all_dongles(simu);
				return (NULL);
			}
		}
		if (all_done(simu))
			set_stopped(simu);
		usleep(300);
	}
	wake_all_dongles(simu);
	return (NULL);
}