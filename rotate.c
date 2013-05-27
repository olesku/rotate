/* 
	Simple character rotation "encryption" tool.
	Written by Ole Fredrik Skudsvik <ole.skudsvik@gmail.com> 2012.

	Example of use:
	./rotate -n 20 -o file.enc file
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>

#define DEFAULT_ROUNDS 15
	
long totalChrCnt = 0;
long cryptedChrCnt = 0;

int verbose = 0;

enum ENCTASK {
	ENC_ENCRYPT = 0,
	ENC_DECRYPT = 1
};

int rot(int n, char* key, unsigned char** dest, long len, unsigned char* src, enum ENCTASK encTask) {
	int i;
	unsigned char *p_src, *p_dest;
	char *p_key;

	*dest = (unsigned char *) malloc((sizeof(unsigned char)) * len + 1);
	if (*dest == NULL) {
		perror("Could not allocate memory for dest buffer.\n");
		exit(1);
	}

	p_dest = *dest;

	for (p_src = src; p_src < (src+len); p_src++) {
		totalChrCnt++;

		/* Don't touch null characters and non-ascii data. */
		if (*p_src > CHAR_MAX || *p_src < CHAR_MIN || (int)*p_src == 0) {
			*p_dest++ = *p_src;
			continue;
		}

		p_key = strchr(key, *p_src);
		if (p_key == NULL) {
			*p_dest++ = *p_src;
			continue;
		}

		for(i = 0; i < n; i++)  {
			if (encTask == ENC_ENCRYPT) {
				p_key++;
				if (*p_key == '\0') p_key = key;
			} else if (encTask == ENC_DECRYPT) {
				p_key--;
				if (p_key < key) p_key = (key + strlen(key)) - 1;
			}
		}

		*p_dest++ = *p_key; 
		cryptedChrCnt++;
	}

	*p_dest = '\0';

	return 0;

} 

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

void doEncrypt(char* inFile, char* outFile, int n, char* key, enum ENCTASK encTask) {
	FILE *IN_FP 	= stdin, *OUT_FP	=	stdout;
	long nRead, nReadTot = 0, szinFile = 0;
	unsigned char buf[8192], *cryptBuf;
	struct stat st;

	if (inFile != NULL) {	
		IN_FP = fopen(inFile, "r+b");
		if (IN_FP == NULL) {
			perror("Could not open input file");
			exit(1);	
		}

		stat(inFile, &st);
		szinFile = (long)st.st_size;
	}

	if (outFile != NULL) {
		OUT_FP = fopen(outFile, "w+");
		if (OUT_FP == NULL) {
			perror("Could not open output file");
			exit(1);	
		}
	}

	while ( (nRead = fread(&buf, 1, 8192, IN_FP)) ) {
		nReadTot += nRead;
		buf[nRead] = '\0';
		rot(n, key, &cryptBuf, nRead, buf, encTask);
		fwrite(cryptBuf, nRead, 1, OUT_FP);
		
		if (IN_FP != stdin && verbose) {
			fprintf(stderr, "Progress: %ld%% (%ld/%ld)\r", 
							((nReadTot * 100) / szinFile), nReadTot, szinFile);
			fflush(stderr);
		}

		free(cryptBuf);
	}

	if (nRead == -1) {
		perror("read()\n");
		exit(1);
	}

	if (inFile != NULL) { 
		fclose(IN_FP); 
		if (verbose) fprintf(stderr, "\n"); 
	}

	if (outFile != NULL) fclose(OUT_FP);

}

void showUsage(char* appname) {
	printf("\nUsage:\n"
			"%s <arguments> <optional: infile>\n\n"
			"Valid arguments:\n" 
			" -e            - Encrypt input (default).\n"
			" -d            - Decrypt input.\n"
			" -n <n>        - Rotate n rounds (default: %i).\n"
			" -t <n>        - Use character table <n> (default: 0).\n"
			" -o <filename> - Write output to filename (default: STDOUT).\n"
			" -v            - Be verbose.\n",
			appname, DEFAULT_ROUNDS);

	exit(0);
}

int main(int argc, char* argv[]) {
	int rounds = DEFAULT_ROUNDS, chrTblIndex = 0, c;
	char *outFile = NULL;
	char *inFile  = NULL;
	char *encKey	= NULL;
	enum ENCTASK encTask = ENC_ENCRYPT;

	while ( ( c = getopt(argc, argv, "n:o:t:ved") ) != -1 ) {
		switch(c) {
			case 'e':
				encTask = ENC_ENCRYPT;
			break;

			case 'd':
				encTask = ENC_DECRYPT;
			break;

			case 'n':
				rounds = atoi(optarg);
				if (rounds < 1) {
					fprintf(stderr, "Rounds need to be a positive number.\n");
					exit(1);
				}
			break;

			case 't':
				chrTblIndex = atoi(optarg);
				if (chrTblIndex < 0) {
					fprintf(stderr, "Character table index cannot be a negative number.\n");
					exit(1);
				}
			break;

			case 'o':
				outFile = (char*) malloc(sizeof(char) * strlen(optarg) + 1);
				strncpy(outFile, optarg, strlen(optarg));
			break;

			case 'v':
				verbose = 1;
			break;

			case ':':
				fprintf(stderr, "The option %c requires an argument.\n", optopt);
				exit(1);
			break;


			case '?':
				showUsage(argv[0]);
			break;
		}
	}

	if (argv[optind] != NULL) {
		inFile = (char*) malloc(sizeof(char) * strlen(argv[optind]) + 1);
		strncpy(inFile, argv[optind], strlen(argv[optind]));
	}

	genRandKey(&encKey);
	doEncrypt(inFile, outFile, rounds, encKey, encTask);

	if (verbose) {
		fprintf(stderr, "%s %ld characters out of %ld characters (%ld%%).\n", 
				(encTask == ENC_ENCRYPT) ? "Encrypted" : "Decrypted", cryptedChrCnt, 
				totalChrCnt, (cryptedChrCnt * 100) / totalChrCnt);
	}

	if (outFile != NULL) free(outFile);
	if (inFile != NULL) free(inFile);
	if (encKey != NULL) free(encKey);

	return 0;
}
