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

int is_num_valid(char **arg)
{
    int i;
    int j;

    i = 1;
    j = 0;
    while(i <= 7)
    {
        j = 0;
        while(arg[i][j])
        {
            if (!isdigit(arg[i][j]))
            {
                write(2, "arg isn't a valid number\n", 25);
                return(0);
            }
            j++;
        }
        i++;
    }
    printf("ultimo i:%i\n", i);
    printf("argv atual: %s\n", arg[i]);
    return(1);
}

int    parser(t_simu *simu, char **argv)
{   
    if ((is_num_valid(argv)) && (!strcmp(argv[8], "fifo") || !strcmp(argv[8], "edf")))
    {
        printf("num e schedule ok\n");
        simu->nbr_of_coders = atoi(argv[1]);
        simu->time_to_burnout = atoi(argv[2]);
        simu->time_to_compile = atoi(argv[3]);
        simu->time_to_debug = atoi(argv[4]);
        simu->time_to_refactor = atoi(argv[5]);
        simu->nbr_of_compiles_required = atoi(argv[6]);
        simu->cooldown = atoi(argv[7]);
        
        if (strcmp(argv[8], "fifo") == 0)
            simu->scheduler = POLICY_FIFO;
        else if (strcmp(argv[8], "edf") == 0)
            simu->scheduler = POLICY_EDF;
        return(1);
    }
    write(2, "Error: Invalid schedule\n", 24);
    return(3);

}
