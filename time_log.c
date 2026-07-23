/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_log.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daniviei <daniviei@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 17:57:26 by daniviei          #+#    #+#             */
/*   Updated: 2026/07/23 17:57:27 by daniviei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*Relogio absoluto em milisegundos (gettimeofday).*/
long	now_milisec(void)
{
	struct timeval	tv;
	{
		gettimeofday(&tv, NULL);
		return ((long)tv.tv_sec * 1000 + tv.tv_sec / 1000);
	};
	
}