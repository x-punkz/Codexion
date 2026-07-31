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

	gettimeofday(&tv, NULL);
	return ((long)tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/*Milisegundos decorridos desde o inicip da simulaçao. */
long	elapsed_milisec(t_simu *simu)
{
	return (now_milisec() - simu->start_time);
}

/*Escreve 'n' (>=0) em decimal dentro do buf; devolve o numero de chars*/
static int	put_ulong(char *buf, long n)
{
	char	tmp[24];
	int		len;
	int		i;

	len = 0;
	if (n == 0)
		tmp[len++] = '0';
	while (n > 0)
	{
		tmp[len++] = '0' + (n % 10);
		n /= 10;
	}
	i = 0;
	while (i < len)
	{
		buf[i] = tmp[len - 1 - i];
		i++;
	}
	return (len);
}

/* Emite "timestamp id msg" numa unica sequencia de write() (saida imediata
e atomica). NAO trava nada: quem chama garante a serializacao.*/
void	write_line(t_coder *coder, const char *msg)
{
	char	buf[64];
	int		len;

	len = put_ulong(buf, elapsed_milisec(coder->simu));
	buf[len++] = ' ';
	len += put_ulong(buf + len, coder->id);
	buf[len++] = ' ';
	write(1, buf, len);
	write(1, msg, strlen(msg));
	write(1, "\n", 1);
}

/*
** Log de mudanca de estado de um coder. Serializado por log_lock e suprimido
** apos a parada, para que nenhuma mensagem apareca depois do "burned out".
*/
void	log_state(t_coder *coder, const char *msg)
{
	pthread_mutex_lock(&coder->simu->log_lock);
	if (!is_stopped(coder->simu))
		write_line(coder, msg);
	pthread_mutex_unlock(&coder->simu->log_lock);
}
