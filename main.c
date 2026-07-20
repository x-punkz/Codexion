#include "codexion.h"

int	main(int argc, char **argv)
{
	t_simu	simu;

	if (argc == 9)
	{
		printf("retorno do parser: %d\n", parser(&simu, argv));
	}
	else
		write(2, "Pass the 8 arguments\n", 22);
	printf("\n");
}