PROGRAM:=main
OBJS:=

CFLAGS:=-O3 -Wextra -Wall -Werror -fno-strict-aliasing -std=gnu11\
	-I./sylvan/src
LDLIBS:=-lm -lpthread
LDFLAGS:=./sylvan/src/libsylvan.a

$(PROGRAM): $(PROGRAM).o $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) -v $(PROGRAM).o $(PROGRAM) $(OBJS)