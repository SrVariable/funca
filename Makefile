NAME = funca
CFLAGS = -Wall -Wextra -Werror

ifdef DEBUG
	CFLAGS += -ggdb
endif

all: $(NAME)

$(NAME): $(NAME).c
	$(CC) $(CFLAGS) -o $(NAME) $(NAME).c

debug:
	$(MAKE) -B DEBUG=1

clean:
	$(RM) $(NAME)

.PHONY: all clean debug 
