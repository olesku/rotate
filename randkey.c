#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void genRandKey(char** dest) {
	int i;
	char c, *p;
	struct timeval tv;

	*dest = malloc(sizeof(char) * 96);
	if (*dest == NULL) {
		perror("genRandKey: Could not allocate memory");
		exit(1);
	}

	p = *dest;

	for (i = 32; i < 127; i++) {
		for(;;) {
			gettimeofday(&tv, NULL);
			srand((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
			c = (char) (rand() % (127-32)+32);

			if (strchr(*dest, c) == NULL) {
				*p++ = c;
				break;
			}
		}

	}

	*p = '\0';
}

int main(int argc, char* argv[]) {
	char *dest;
	genRandKey(&dest);

	printf("%s\n.", dest);
	free(dest);
	return 0;
}