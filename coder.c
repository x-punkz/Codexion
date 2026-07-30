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

/* Dorme `ms` em fatias curtas, acordando cedo se a simulacao parar. */
static void	precise_sleep(long milisec, t_simu *simu)
{
	long	end;

	end = now_milisec() + milisec;
	while (now_milisec() < end)
	{
		if (is_stopped(simu))
			return ;
		usleep(300);
	}
}

/* Caso especial n==1: um dongle so; pega uma vez e nunca junta o par. */
static void	lone_coder(t_coder *coder)
{
	take_dongle(coder, &coder->simu->dongles[0]);
	take_dongle(coder, &coder->simu->dongles[0]);
}

/* Reinicia o relogio de burnout, conta a compilacao e
cumpre time_to_compile. */
static void	do_compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->lock);
	coder->last_compile_start = now_milisec();
	coder->compiles++;
	pthread_mutex_unlock(&coder->lock);
	log_state(coder, "\033[32mis compiling\033[0m");
	precise_sleep(coder->simu->time_to_compile, coder->simu);
}

/* Um ciclo completo. Retorna 0 se a simulacao parou no meio. */
static int	compile_cycle(t_coder *coder, int lower_id, int high_id)
{
	if (!take_dongle(coder, &coder->simu->dongles[lower_id]))
		return (0);
	if (!take_dongle(coder, &coder->simu->dongles[high_id]))
	{
		release_dongle(coder, &coder->simu->dongles[lower_id]);
		return (0);
	}
	do_compile(coder);
	release_dongle(coder, &coder->simu->dongles[lower_id]);
	release_dongle(coder, &coder->simu->dongles[high_id]);
	log_state(coder, "\033[33mis debugging\033[0m");
	precise_sleep(coder->simu->time_to_debug, coder->simu);
	log_state(coder, "\033[1;33mis refactoring\033[0m");
	precise_sleep(coder->simu->time_to_refactor, coder->simu);
	return (1);
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
			break ;
	}
	return (NULL);
}
