LIBS				:=

SRC_DIR			:= src
SRCS				:= $(shell find $(SRC_DIR) -name '*.c')

BUILD_DIR   := .build
OBJS        := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS        := $(OBJS:.o=.d)

CC          := clang
CFLAGS      := -Wall -Wextra -Werror
CPPFLAGS    := -MMD -MP -I include
LDLIBS      := $(addprefix -l,$(LIBS))

RM          := rm -rf
MAKEFLAGS   += --no-print-directory
DIR_DUP     = mkdir -p $(@D)

all: $(OBJS) server client

server: src/server.c
	$(CC) .build/server.o $(LDLIBS) -o server

client: src/client.c
	$(CC) .build/client.o $(LDLIBS) -o client

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(DIR_DUP)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
	$(info CREATED $@)

-include $(DEPS)

clean:
	$(RM) $(OBJS) $(DEPS)
	$(info CLEANED)

fclean: clean
	$(RM) $(NAME)

re:
	$(MAKE) fclean
	$(MAKE) all

.PHONY: clean fclean re all

.SILENT:
