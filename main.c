/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daniviei <daniviei@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 17:08:57 by daniviei          #+#    #+#             */
/*   Updated: 2026/07/23 17:09:02 by daniviei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_simu	simu;

	if (argc == 9)
	{
		if (!parser(&simu, argv))
			return (1);
		if (!init_simu(&simu))
		{
			destroy_simu(&simu);
			return (1);
		}
		run_simu(&simu);
		destroy_simu(&simu);
	}
	else
		write(2, "Pass the 8 arguments\n", 22);
	printf("\n");
}
