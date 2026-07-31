# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: daniviei <daniviei@student.42.rio>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/30 21:25:20 by daniviei          #+#    #+#              #
#    Updated: 2026/07/30 21:25:26 by daniviei         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread

SRCS		= coder.c \
			  control.c \
			  dongle.c \
			  dongle_utils.c \
			  init_thread.c \
			  main.c \
			  monitor.c \
			  parser.c \
			  queue.c \
			  queue_utils.c \
			  time_log.c

OBJS		= $(SRCS:.c=.o)

HEADER		= codexion.h

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re