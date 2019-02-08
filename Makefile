
all:
	gcc -std=c99 -Wall evaluation.c mpc.c -ledit -lm -o lispy
