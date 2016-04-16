PROGRAM:=screen2000
OBJS:=
CC = g++

CPPFLAGS:=-g -O3 -Wall -Werror -fno-strict-aliasing -stdlib=libc++ -std=c++11\
	-I./sylvan/src -Wno-error=deprecated -Wno-error=unused-function\
	-Wc++11-extensions -Wno-error-unused-variable
LDLIBS:=-lm -lpthread
LDFLAGS:=./sylvan/src/libsylvan.a 

$(PROGRAM): $(PROGRAM).o $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

%.o: %.cpp %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) -v $(PROGRAM).o $(PROGRAM) $(OBJS)