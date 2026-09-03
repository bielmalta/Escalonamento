scheduler: scheduler.o
	gcc scheduler.o -o scheduler

scheduler.o: scheduler.c
	gcc -Wall -Wextra -std=c11 -c scheduler.c

clean:
	rm -f scheduler.o scheduler