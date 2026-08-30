#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#define MIN_NUMBER 1
#define MAX_NUMBER 100
#define MAX_ATTEMPTS 7

#define INPUT_RETRY_USEC 10000

/*
 * Simple userspace PRNG.
 *
 * This is NOT cryptographically secure.
 * It is perfectly adequate for a small game.
 */
static uint32_t random_state;

static void random_seed(uint32_t seed)
{
	if (seed == 0)
		seed = 0x12345678;

	random_state = seed;
}

static uint32_t random_u32(void)
{
	/*
	 * xorshift32
	 */
	uint32_t x = random_state;

	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;

	random_state = x;

	return x;
}

static int random_range(int min, int max)
{
	return min + (int)(random_u32() % (uint32_t)(max - min + 1));
}


/*
 * Read an entire line from stdin.
 *
 * The function does NOT return until the user presses Enter.
 * Characters are collected one at a time.
 *
 * If no input is currently available, getchar() returns EOF.
 * In that case, wait briefly and try again.
 */
static int read_line(char *buffer, size_t size)
{
	size_t pos = 0;

	for (;;) {
		int c = getchar();

		if (c == EOF) {
			usleep(INPUT_RETRY_USEC);
			continue;
		}

		/*
		 * Enter pressed.
		 */
		if (c == '\n' || c == '\r') {
			buffer[pos] = '\0';
			return 1;
		}

		/*
		 * Store the character if there is room.
		 */
		if (pos + 1 < size) {
			buffer[pos++] = (char)c;
		}
	}
}


static int play_round(void)
{
	int secret = random_range(MIN_NUMBER, MAX_NUMBER);

	printf("\n");
	printf("NEW ROUND\n");
	printf("I'm thinking of a number from %d to %d.\n",
	       MIN_NUMBER,
	       MAX_NUMBER);
	printf("You have %d attempts.\n\n", MAX_ATTEMPTS);

	for (int attempt = 1; attempt <= MAX_ATTEMPTS; ++attempt) {
		char buffer[64];

		printf("Attempt %d/%d - Your guess: \n",
		       attempt,
		       MAX_ATTEMPTS);

		/*
		 * Wait until the user presses Enter.
		 */
		if (!read_line(buffer, sizeof(buffer))) {
			printf("\nInput ended.\n");
			return 0;
		}

		char *end;
		long value = strtol(buffer, &end, 10);

		/*
		 * Skip trailing whitespace.
		 */
		while (*end == ' ' ||
		       *end == '\t' ||
		       *end == '\n' ||
		       *end == '\r')
		{
			++end;
		}

		/*
		 * If anything other than whitespace follows
		 * the number, the input is invalid.
		 */
		if (*end != '\0') {
			printf("That's not a valid number.\n");
			--attempt;
			continue;
		}

		if (value < MIN_NUMBER || value > MAX_NUMBER) {
			printf("Please enter a number between %d and %d.\n",
			       MIN_NUMBER,
			       MAX_NUMBER);
			
			printf("You entered %d.\n", value);

			--attempt;
			continue;
		}

		int guess = (int)value;

		if (guess == secret) {
			printf("\n");
			printf("YOU GOT IT!\n");
			printf("The number was %d.\n", secret);
			printf("You needed %d attempt", attempt);

			if (attempt != 1)
				putchar('s');

			printf(".\n");

			return 1;
		}

		if (guess < secret) {
			printf("Too low!\n");
		} else {
			printf("Too high!\n");
		}

		int remaining = MAX_ATTEMPTS - attempt;

		if (remaining > 0) {
			printf("%d attempt", remaining);

			if (remaining != 1)
				putchar('s');

			printf(" remaining.\n\n");
		}
	}

	printf("\n");
	printf("GAME OVER!\n");
	printf("The number was %d.\n", secret);

	return 1;
}


int main(void)
{
	/*
	 * Use the kernel clock to seed the userspace PRNG.
	 */
	random_seed(time_ms());

	int score = 0;
	int rounds = 0;

	printf("NANO GUESSING GAME\n");
	printf("\n");
	printf("Welcome!\n");
	printf("Guess the number I'm thinking of.\n");

	for (;;) {
		int result = play_round();

		if (result == 0)
			break;

		++rounds;

		if (result == 1)
			++score;

		printf("\n");
		printf("Score: %d/%d\n", score, rounds);
		printf("\n");

		char answer[16];

		printf("Play again? [y/n]: ");

		if (!read_line(answer, sizeof(answer)))
			break;

		if (answer[0] != 'y' && answer[0] != 'Y')
			break;
	}

	printf("\n");
	printf("THANKS FOR PLAYING\n");
	printf("Final score: %d/%d\n", score, rounds);

	return 0;
}