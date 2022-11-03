CC = gcc
CFLAGS = -c -Wall -Wextra -Werror

# Target library
lib := libuthread.a

# List of all objects and files for easier cleanup
# files = queue.c queue.h
files = queue.c uthread.c context.c preempt.c sem.c
objects = queue.o uthread.o context.o preempt.o sem.o

# .PHONY is used in order to specify it is a recipe, for avoiding conflicts with other files
.PHONY: all
all: $(lib)

# Compile necessary object files then create the rachive
$(lib): $(files)
	$(CC) $(CFLAGS) $(files)
	ar rcs $@ $(objects)

.PHONY: clean
clean:
	-rm -rf $(objects) $(lib)
