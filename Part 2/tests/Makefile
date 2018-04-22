SRCS=test_kos_multi_threaded_all_getAll_fromFile.c test_kos_multi_threaded_time.c
OBJS=${SRCS:.c=}

INCLUDES =  -I ../include 
CC = gcc
LIBKOS =  ../kos/libkos.a
CFLAGS = -g -O0 -Wall -pthread $(INCLUDES) 

COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CFLAGS)

all: $(OBJS)

test_kos_multi_threaded_all_getAll_fromFile: test_kos_multi_threaded_all_getAll_fromFile.c $(LIBKOS)

test_kos_multi_threaded_time: test_kos_multi_threaded_time.c $(LIBKOS)


clean: 
	rm -f $(OBJS)


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
