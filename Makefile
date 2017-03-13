CFLAGS = -ggdb -Wall -DSHOW_POP_PROFILE -DDEEP_LOG

OBJECTS = main.o dafnie.o params.o myrandom.o

eldevol: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $(@) -lm

test: eldevol config324.txt
	./dafnie config324.txt stat.csv log

dafnie.o: dafnie.c dafnie.h myrandom.h
params.o: params.c params.h
main.o: main.c dafnie.h params.h ver.h
myrandom.o: myrandom.c myrandom.h

clean:
	rm *.o eldevol
