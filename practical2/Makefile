PROGRAM:=main
OBJS:=

CPPFLAGS:=-g -O3 -Wextra -Wall -Werror -fno-strict-aliasing -std=c++11\
	-I./sylvan/src -Wno-error=deprecated
LDLIBS:=-lm -lpthread
LDFLAGS:=./sylvan/src/libsylvan.a

$(PROGRAM): $(PROGRAM).o $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

%.o: %.cpp %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) -v $(PROGRAM).o $(PROGRAM) $(OBJS)