// This was made by claude code using my code as source
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>

#define START "<!--MODIFIED_AT_START-->"
#define END   "<!--MODIFIED_AT_END-->"

/* macOS/BSD use st_atimespec/st_mtimespec; Linux/glibc use st_atim/st_mtim */
#if defined(__APPLE__)
#define ST_ATIM st_atimespec
#define ST_MTIM st_mtimespec
#else
#define ST_ATIM st_atim
#define ST_MTIM st_mtim
#endif

int main(int argc, char *argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "Error: add the name of the file\n"); 
		return 1;
	}

	struct stat st;
	if (stat(argv[1], &st) != 0) {
		perror("stat"); 
		return 1;
	}

	/* ctime() returns "Www Mmm dd hh:mm:ss yyyy\n\0" (24 visible chars).
	Copy only the 24 visible chars and drop the trailing newline. */
	char text[32];
	char *ct = ctime(&st.st_mtime);
	if (!ct) { 
		fprintf(stderr, "ctime failed\n"); 
		return 1;
	}
	strncpy(text, ct, 24);
	text[24] = '\0';
	/* Belt-and-suspenders: strip any newline wherever it ends up. */
	text[strcspn(text, "\n")] = '\0';

	FILE *fp = fopen(argv[1], "r");
	if (!fp) { 
		perror("fopen"); 
		return 1; 
	}
	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	if (len < 0) { 
		perror("ftell"); 
		fclose(fp); 
		return 1; 
	}
	rewind(fp);

	char *content = malloc((size_t)len + 1);
	if (!content) { 
		fprintf(stderr, "out of memory\n"); 
		fclose(fp); 
		return 1;
	}

	size_t nread = fread(content, 1, (size_t)len, fp);
	fclose(fp);
	if (nread != (size_t)len) {
		fprintf(stderr, "Error: short read (%zu of %ld bytes)\n", nread, len);
		free(content);
		return 1;
	}
	content[len] = '\0';

	char *s = strstr(content, START);
	char *e = s ? strstr(s + strlen(START), END) : NULL;
	if (!s || !e) {
		fprintf(stderr, "tags not found\n"); 
		free(content); 
		return 1;
	}
	s += strlen(START);

	/* build new content: [before s][text][from e...] */
	char *out = malloc((size_t)(s - content) + strlen(text) + strlen(e) + 1);
	if (!out) { 
		fprintf(stderr, "out of memory\n"); 
		free(content); 
		return 1;
	}
	sprintf(out, "%.*s%s%s", (int)(s - content), content, text, e);

	fp = fopen(argv[1], "w");
	if (!fp) { 
		perror("fopen"); 
		free(content); 
		free(out); 
		return 1; 
	}

	size_t outlen = strlen(out);
	size_t nwritten = fwrite(out, 1, outlen, fp);
	fclose(fp);
	if (nwritten != outlen) {
		fprintf(stderr, "Error: short write (%zu of %zu bytes)\n", nwritten, outlen);
		free(content);
		free(out);
		return 1;
	}

	struct timespec times[2] = { st.ST_ATIM, st.ST_MTIM };
	if (utimensat(AT_FDCWD, argv[1], times, 0) != 0) {
		perror("utimensat");
		/* not fatal to the file update itself, but worth reporting */
	}

	free(content);
	free(out);
	return 0;
}