#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef short board_t;

#define BOARD_SIZE (81 * sizeof(board_t))

int set_val(int num)
{
	return 1 << (num - 1);
}

void unset_val(board_t *b, int k, int num)
{
	b[k] &= ~(set_val(num));
}

int count_bits(short n)
{
	int i = 0;
	while(n)
		i++, n &= n - 1;
	return i;
}

int bit_set(short n, int i)
{
	return n & (1 << (i - 1));
}

int one_set(short s)
{
	return count_bits(s) == 1;
}

int only_this_set(short s, int n)
{
	return s == (1 << (n - 1));
}

int get_first(short s)
{
	int i;
	for(i = 1; i <= 9; i++)
		if(bit_set(s, i))
			return i;
	return 0;
}

int get_only(short s)
{
	if(!one_set(s))
		return 0;
	return get_first(s);
}

board_t *new_board()
{
	board_t *b = malloc(BOARD_SIZE);
	if(!b)
		return NULL;
	int i;
	for(i = 0; i < 81; i++)
		b[i] = 0x1ff;
	return b;
}

board_t *read_board(const char* str)
{
	board_t *b = new_board();
	const char* it = str;
	int i = 0;
	while(*it != '\0') {
		if(*it >= '1' && *it <= '9')
			b[i++] = set_val(*it - '0');
		else if(*it == '0' || *it == '.')
			i++;
		it++;
		if(i == 81)
			break;
	}
	if(i < 81)
		free(b);
	return b;
}

int get_row(int i)
{
	return i / 9;
}

int get_col(int i)
{
	return i % 9;
}

int get_box(int i)
{
	return get_row(i) / 3 << 4 | get_col(i) / 3;
}

int is_peer_of(int i, int k)
{
	if(i == k)
		return 0;
	if(get_row(i) == get_row(k))
		return 1;
	if(get_col(i) == get_col(k))
		return 1;
	if(get_box(i) == get_box(k))
		return 1;
	return 0;
}

int assign(board_t *b, int i, int val);

int propagate_units(board_t *b, int i)
{
	int k, v;
	int r = get_row(i);
	int c = get_col(i);
	for(v = 1; v <= 9; v++) {
		if(!bit_set(b[i], v))
			continue;

		/* this row */
		for(k = 0; k < 9; k++) {
			if(k == c)
				continue;
			if(bit_set(b[r * 9 + k], v))
				break;
		}
		if(k == 9) {
			int ret = assign(b, i, v);
			if(!ret)
				return 0;
		}

		/* this column */
		for(k = 0; k < 9; k++) {
			if(k == r)
				continue;
			if(bit_set(b[k * 9 + c], v))
				break;
		}
		if(k == 9) {
			int ret = assign(b, i, v);
			if(!ret)
				return 0;
		}

		/* this box */
		for(k = 0; k < 9; k++) {
			if(r == (r / 3) * 27 / 9 + k % 3 && c == ((c / 3) * 3 + k / 3) % 9)
				continue;
			if(bit_set(b[((r / 3) * 27 / 9 + k % 3) * 9 + ((c / 3) * 3 + k / 3) % 9], v))
				break;
		}
		if(k == 9) {
			int ret = assign(b, i, v);
			if(!ret)
				return 0;
		}
	}
	return 1;
}

int assign(board_t *b, int i, int val)
{
	b[i] = set_val(val);
	int k;
	for(k = 0; k < 81; k++) {
		if(is_peer_of(i, k)) {
			if(only_this_set(b[k], val))
				return 0;
			else if(bit_set(b[k], val)) {
				unset_val(b, k, val);
				int v = get_only(b[k]);
				if(v != 0) {
					int ret = assign(b, k, v);
					if(!ret)
						return 0;
				}
				int ret = propagate_units(b, k);
				if(!ret)
					return 0;
			}
		}
	}
	return 1;
}

int init_propagate(board_t *b)
{
	int i;
	for(i = 0; i < 81; i++) {
		if(one_set(b[i])) {
			int ret = assign(b, i, get_first(b[i]));
			if(!ret)
				return 0;
		}
	}
	return 1;
}

int is_solved(board_t *b)
{
	int i;
	for(i = 0; i < 81; i++)
		if(count_bits(b[i]) != 1)
			return 0;
	return 1;
}

board_t *search(board_t *b)
{
	int i;
	int min_ind = 0;
	int min_num = 10;
	if(is_solved(b))
		return b;
	for(i = 0; i < 81; i++) {
		int this_bits = count_bits(b[i]);
		if(this_bits > 1 && this_bits < min_num)
			min_ind = i, min_num = this_bits;
	}
	for(i = 1; i <= 9; i++) {
		if(bit_set(b[min_ind], i)) {
			board_t *nb = malloc(BOARD_SIZE);
			memcpy(nb, b, BOARD_SIZE);
			int ret = assign(nb, min_ind, i);
			if(ret) {
				board_t *ret2 = search(nb);
				if(ret2)
					return ret2;
			}
		}
	}
	return NULL;
}

board_t *read_board_from_file(const char* filepath)
{
	FILE *fp = fopen(filepath, "r");
	if(!fp)
		return NULL;
	char buf[1024];
	memset(buf, 0x00, sizeof(buf));
	int ret = fread(buf, 1, 1024, fp);
	if(ret <= 0)
		return NULL;
	board_t *b = read_board(buf);
	fclose(fp);
	return b;
}

int max(int a, int b)
{
	if(a > b)
		return a;
	return b;
}

int bits_to_str(short n, char* b)
{
	int i, j;
	for(i = 1, j = 0; i <= 9; i++)
		if(bit_set(n, i))
			*b++ = i + '0', j++;
	return j;
}

void print_board(board_t* b, FILE *fp)
{
	int i;
	int maxlen = 0;
	for(i = 0; i < 81; i++) {
		maxlen = max(maxlen, count_bits(b[i]));
	}
	for(i = 0; i < 81; i++) {
		char buf[10];
		memset(buf, 0x00, sizeof(buf));
		int j = bits_to_str(b[i], buf);
		fputs(buf, fp);
		int k = maxlen - j;
		while(k-- > 0)
			fputc(' ', fp);
		fputc(' ', fp);
		if(i % 9 == 8) {
			fputc('\n', fp);
			if(i == 26 || i == 53) {
				int k;
				for(k = 0; k < maxlen * 3 + 16; k++)
					fputc('-', fp);
				fputc('\n', fp);
			}
		}
		else if(i % 3 == 2)
			fputc('|', fp);
	}
	fputc('\n', fp);
}

void usage(const char* a)
{
	fprintf(stderr, "Usage: %s <file path to board>\n", a);
}

int main(int argc, char** argv)
{
	if(argc < 2)
		usage(argv[0]), exit(1);
	board_t *b = read_board_from_file(argv[1]);
	if(!b)
		fprintf(stderr, "Could not parse\n"), exit(1);
	int ret = init_propagate(b);
	if(!ret)
		fprintf(stderr, "Initial propagation failed\n");
	else {
		board_t *sb = search(b);
		if(!sb) {
			free(b);
			fprintf(stderr, "Search failed\n");
		}
		else {
			print_board(sb, stdout);
			free(sb);
		}
	}
	return 0;
}

