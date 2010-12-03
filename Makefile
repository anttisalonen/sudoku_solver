CFLAGS = -Wall -O2
CC     = gcc

sudoku:
	$(CC) $(CFLAGS) -o sudoku sudoku.c

clean:
	rm -f sudoku
