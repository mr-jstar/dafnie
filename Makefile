CFLAGS = -ggdb -Wall -DSHOW_POP_PROFILE -DDEEP_LOG

PROGRAM = dafnie
OBJECTS = main.o dafnie.o params.o myrandom.o

dafnie: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $(PROGRAM) -lm

skew: skew.o myrandom.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

test: dafnie config
	./dafnie config stat.csv log

dafnie.o: dafnie.c dafnie.h myrandom.h
params.o: params.c params.h
main.o: main.c dafnie.h params.h ver.h
myrandom.o: myrandom.c myrandom.h
skew.o: skew.c skew.h myrandom.h

clean:
	rm *.o dafnie
