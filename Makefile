CFLAGS = -Wall -O2
CC     = gcc

sudoku: sudoku.c
	$(CC) $(CFLAGS) -o sudoku sudoku.c

clean:
	rm -f sudoku
