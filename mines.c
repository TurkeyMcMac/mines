#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct tile {
	unsigned mine : 1;
	unsigned revealed : 1;
	unsigned flagged : 1;
};

#define MIN_WIDTH 1
#define MAX_WIDTH 26
#define MIN_HEIGHT 1
#define MAX_HEIGHT 30
#define MIN_MINES 0
#define MAX_MINES 780
#define CMD_MAX 7

const char alphabet[MAX_WIDTH] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

int g_width = 20;
int g_height = 20;
int g_n_mines = 40;
int g_n_flags = 0;
int g_n_found = 0;
struct tile g_board[MAX_WIDTH][MAX_HEIGHT];

void print_usage(char *progname, FILE *to)
{
	static char usage_str[] = "Usage: %s [options]\n";
	fprintf(to, usage_str, progname);
}

void print_help(char *progname, FILE *to)
{
	static char help_str[] = "%s help placeholder\n";
	print_usage(progname, to);
	fprintf(to, help_str, progname);
}

void print_version(char *progname, FILE *to)
{
	static char version_str[] = "%s 0.0.2\n";
	fprintf(to, version_str, progname);
}

void print_help_hint(char *progname, FILE *to)
{
	static char help_hint_str[] = "Run `%s -help` for more help.\n";
	fprintf(to, help_hint_str, progname);
}

int number_arg(char *argv[], int *i, int min, int max)
{
	char *progname = argv[0];
	char *opt = argv[*i];
	char *arg = argv[++*i];
	if (arg) {
		int num = atoi(arg);
		if (num < min || num > max) {
			fprintf(stderr, "%s: %s must be between %d and %d\n",
				progname, opt + 1, min, max);
			exit(EXIT_FAILURE);
		}
		return num;
	} else {
		fprintf(stderr, "%s: Usage: %s <number>\n", progname, opt);
	}
	exit(EXIT_FAILURE);
	return -1;
}

void parse_options(int argc, char *argv[])
{
	char *progname = argv[0];
	int i;
	for (i = 1; i < argc; ++i) {
		char *opt = argv[i];
		if (!strcmp(opt, "-h")
		 || !strcmp(opt, "-?")
		 || !strcmp(opt, "-help")) {
			print_help(progname, stdout);
			exit(EXIT_SUCCESS);
		} else if (!strcmp(opt, "-v") || !strcmp(opt, "-version")) {
			print_version(progname, stdout);
			exit(EXIT_SUCCESS);
		} else if (!strcmp(opt, "-width")) {
			g_width = number_arg(argv, &i, MIN_WIDTH, MAX_WIDTH);
		} else if (!strcmp(opt, "-height")) {
			g_height = number_arg(argv, &i, MIN_HEIGHT, MAX_HEIGHT);
		} else if (!strcmp(opt, "-mines")) {
			g_n_mines = number_arg(argv, &i, MIN_MINES, MAX_MINES);
		} else if (*opt == '-') {
			fprintf(stderr, "%s: Unrecognized option: %s\n",
				progname, opt);
			print_help_hint(progname, stderr);
			exit(EXIT_FAILURE);
		} else {
			fprintf(stderr, "%s: Unexpected argument: %s\n",
				progname, opt);
			print_usage(progname, stderr);
			print_help_hint(progname, stderr);
			exit(EXIT_FAILURE);
		}
	}
}

void init_board(void)
{
	int i, x, y;
	clock_t seed = clock();
	for (i = x = y = 0; i < g_n_mines; ++i) {
		g_board[x][y].mine = 1;
		if (++x >= g_width) {
			x = 0;
			++y;
		}
		seed = (seed + i) ^ (clock() - i * x + y);
	}
	srand(seed);
	for (i = x = y = 0; i < g_n_mines; ++i) {
		struct tile temp, *there;
		temp = g_board[x][y];
		there = &g_board[rand() % g_width][rand() % g_height];
		g_board[x][y] = *there;
		*there = temp;
		if (++x >= g_width) {
			x = 0;
			++y;
		}
	}
}

void reveal_all(void)
{
	int x, y;
	for (x = 0; x < g_width; ++x) {
		for (y = 0; y < g_height; ++y) {
			g_board[x][y].revealed = 1;
		}
	}
}

#define EACH_AROUND(x, y, ax, ay, run) do { \
	int right = x < g_width - 1; \
	int up = y > 0; \
	int left = x > 0; \
	int down = y < g_height - 1; \
	if (right) { ax = x + 1; ay = y; run; } \
	if (right && up) { ax = x + 1; ay = y - 1; run; } \
	if (up) { ax = x; ay = y - 1; run; } \
	if (left && up) { ax = x - 1; ay = y - 1; run; } \
	if (left) { ax = x - 1; ay = y; run; } \
	if (left && down) { ax = x - 1; ay = y + 1; run; } \
	if (down) { ax = x; ay = y + 1; run; } \
	if (right && down) { ax = x + 1; ay = y + 1; run; } \
} while (0)

int count_around(int x, int y)
{
	int count = 0;
	int ax, ay;
	EACH_AROUND(x, y, ax, ay, count += g_board[ax][ay].mine);
	return count;
}

int reveal(int x, int y)
{
	int around;
	int ax, ay;
	if (g_board[x][y].mine) return 0;
	if (g_board[x][y].revealed) return 1;
	g_board[x][y].revealed = 1;
	around = count_around(x, y);
	if (around == 0) {
		EACH_AROUND(x, y, ax, ay, reveal(ax, ay));
	}
	return 1;
}

int tile_char(int x, int y)
{
	struct tile t = g_board[x][y];
	if (t.revealed) {
		if (t.mine) {
			return '*';
		} else {
			int around = count_around(x, y);
			if (around > 0) {
				return '0' + around;
			} else {
				return ' ';
			}
		}
	} else if (t.flagged) {
		return 'F';
	} else {
		return '@';
	}
}

void print_column_names(void)
{
	int x;
	printf("    ");
	for (x = 0; x < g_width; ++x) {
		printf(" %c", alphabet[x]);
	}
	putchar('\n');
}

void print_horiz_border(void)
{
	int x;
	printf("    -");
	for (x = 0; x < g_width; ++x) {
		printf("--");
	}
	putchar('\n');
}

void print_board(void)
{
	int y;
	puts("\n\n\n");
	print_column_names();
	print_horiz_border();
	for (y = 0; y < g_height; ++y) {
		int x;
		int row = y + 1;
		printf("%2d |", row);
		for (x = 0; x < g_width; ++x) {
			printf("`%c", tile_char(x, y));
		}
		printf("`| %-2d\n", row);
	}
	print_horiz_border();
	print_column_names();
	printf("Flags: %d/%d\n", g_n_flags, g_n_mines);
}

struct command {
	enum {
		CMD_NONE = '\n',
		CMD_REVEAL = 'r',
		CMD_FLAG = 'f',
		CMD_HELP = '?',
		CMD_QUIT = 'q'
	} kind;
	int x, y;
};

int read_input(char *buf, int max)
{
	int i = 0;
	int ch;
	do {
		ch = getchar();
		if (ch == '\n' || ch == EOF) goto finished;
	} while (isspace(ch));
	for (i = 0; i < max; ++i) {
		buf[i] = ch;
		if (ch == '\n' || ch == EOF) goto finished;
		ch = getchar();
	}
	while (ch != '\n' && ch != EOF) {
		ch = getchar();
		++i;
	}
finished:
	if (i == 0 && feof(stdin)) return -1;
	return i;
}

int parse_location(const char *input, int *x, int *y)
{
	char *letter = memchr(alphabet, toupper(input[0]), sizeof(alphabet));
	if (!letter) return -1;
	*x = (int)(letter - alphabet);
	if (*x < 0 || *x >= g_width) return -1;
	*y = atoi(input + 1) - 1;
	if (*y < 0 || *y >= g_height) return -1;
	return 0;
}

int run_command(const char *input)
{
	int x, y;
	char yn[1];
	switch (*input) {
	case '\0':
		print_board();
		return 1;
	case 'f':
		if (parse_location(input + 1, &x, &y)) break;
		if (!g_board[x][y].revealed) {
			if (g_board[x][y].flagged) {
				g_board[x][y].flagged = 0;
				--g_n_flags;
				g_n_found -= g_board[x][y].mine;
			} else {
				g_board[x][y].flagged = 1;
				++g_n_flags;
				g_n_found += g_board[x][y].mine;
			}
			if (g_n_found == g_n_mines && g_n_flags == g_n_found) {
				reveal_all();
				print_board();
				puts("All mines found! You win!");
				return 0;
			}
		}
		print_board();
		return 1;
	case 'h':
	case '?':
		puts("Help placeholder.");
		return 1;
	case 'q':
		printf("Are you sure you want to quit? [yN] ");
		yn[0] = 'n';
		if (read_input(yn, sizeof(yn)) < 0 || tolower(yn[0]) == 'y') {
			reveal_all();
			print_board();
			puts("Game quitted.");
			return 0;
		}
		return 1;
	case 'r':
		++input;
		/* FALLTHROUGH */
	default:
		if (parse_location(input, &x, &y)) break;
		if (!reveal(x, y)) {
			reveal_all();
			print_board();
			puts("You hit a mine! Game over.");
			return 0;
		}
		print_board();
		return 1;
	}
	puts("Invalid command. Use command '?' for help.");
	return 1;
}

int calc_score(void)
{
	return g_n_found * 1000 / g_width / g_height;
}

int main(int argc, char *argv[])
{
	char cmd[CMD_MAX + 1];
	int len;
	parse_options(argc, argv);
	init_board();
	print_board();
	puts("Type a command. For help, type '?' then ENTER.");
	cmd[CMD_MAX] = '\0';
	while ((len = read_input(cmd, CMD_MAX)) >= 0) {
		if (len <= CMD_MAX) {
			cmd[len] = '\0';
		} else {
			printf("Error: Command too long; "
				"characters after '%c' ignored.\n",
				cmd[CMD_MAX - 1]);
			continue;
		}
		if (!run_command(cmd)) {
			printf("Score: %d\n", calc_score());
			break;
		}
	}
	return 0;
}
