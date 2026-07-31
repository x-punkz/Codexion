/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daniviei <daniviei@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 17:09:09 by daniviei          #+#    #+#             */
/*   Updated: 2026/07/23 17:09:11 by daniviei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_num_valid(char **arg)
{
	int		i;
	int		j;

	i = 1;
	j = 0;
	while (i <= 7)
	{
		j = 0;
		if (!arg[i][0])
		{
			write(2, "Error: Invalid argument\n", 25);
			return (0);
		}
		while (arg[i][j])
		{
			if (arg[i][j] < '0' || arg[i][j] > '9')
			{
				write(2, "Error: arg isn't a valid number\n", 32);
				return (0);
			}
			j++;
		}
		i++;
	}
	return (1);
}

int	parser(t_simu *simu, char **argv)
{
	if ((is_num_valid(argv))
		&& (!strcmp(argv[8], "fifo")
			|| !strcmp(argv[8], "edf")))
	{
		simu->nbr_of_coders = (long)atoi(argv[1]);
		if (simu->nbr_of_coders < 1)
		{
			write(2, "Error: number_of_coders must be >= 1\n", 38);
			return (0);
		}
		simu->time_to_burnout = (long)atoi(argv[2]);
		simu->time_to_compile = (long)atoi(argv[3]);
		simu->time_to_debug = (long)atoi(argv[4]);
		simu->time_to_refactor = (long)atoi(argv[5]);
		simu->nbr_of_compiles_required = atoi(argv[6]);
		simu->cooldown = (long)atoi(argv[7]);
		if (strcmp(argv[8], "fifo") == 0)
			simu->scheduler = POLICY_FIFO;
		else if (strcmp(argv[8], "edf") == 0)
			simu->scheduler = POLICY_EDF;
		return (1);
	}
	return (0);
}
