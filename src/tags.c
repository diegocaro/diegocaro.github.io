// this is inplace 
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <sys/stat.h> // stat()
#include <time.h> // ctime()
#include <fcntl.h> // utimensat()

#define MODIFIED_AT_START "<!--MODIFIED_AT_START-->"
#define MODIFIED_AT_END "<!--MODIFIED_AT_END-->"

size_t read_file(char **output, FILE *fp) {
	/*
	Parameters:
	-	**output: pointer used to return the content of the fp, it must be freed in the caller
	- *fp: file to read
	
	
	Returns: 
	number of bytes read, or -1 if error occurred when reading.
	if an error occur, output will not be modified.
	*/
	char *ptr;
	int length;
	
	fseek(fp, SEEK_SET, SEEK_END);
	length = ftell(fp);
	
	if (length == -1) {
		//perror("Error reading the file");
		return -1;
	}
	fseek(fp, SEEK_SET, 0);
	
	ptr = malloc(sizeof(char)*(length+1));
	if (ptr == NULL) {
		perror("not memory available");
		return -1;
	}
	fread(ptr, sizeof(char), length, fp); 
	ptr[length] = '\0';
	*output = ptr;

	return length;
}

long get_interval(char **start, char **end, char *content) {
	/*
	Parameters:
	- *start_found_at: the pointer where the start tag ends
	- *end_found_at: the point where the end tag begins
	
	Example:
	<!--MODIFIED_AT_START--> this text will be replaced later <!--MODIFIED_AT_END-->
	^																	^
	start_found_at										end_found_at
	
	
	Returns: the distance between start_found_at and end_found at
	and -1 if any error occurred
	*/
	char *start_modified;
	char *end_modified;
	 
	start_modified = strstr(content, MODIFIED_AT_START);
	end_modified = strstr(content, MODIFIED_AT_END);
	
	if (start_modified==NULL || end_modified == NULL) {
		fprintf(stderr, "start/end tag not found\n");
		return -1;
	}

	start_modified += strlen(MODIFIED_AT_START);
	
	if (start_modified > end_modified) {
		fprintf(stderr, "tags malformed\n");
		return -1;
	}
	
	*start = start_modified;
	*end = end_modified;
	
	return end_modified - start_modified;
}

int main(int argc, char *argv[]) {
	FILE *fp;
	char *content;
	size_t length;
	char *filename;
	char * start_modified;
	char * end_modified;
	long distance;
	char text[26];
	size_t len_text;
	struct stat file_stat;
	struct timespec new_times[2]; // stores access time and modified time
	size_t len_tail, len_head;
	char *mtime_str;
	
	if (argc <= 1) {
		fprintf(stderr, "Error: add the name of the file");
		exit(-1);
	}
	filename = argv[1];
		
	if (stat(filename, &file_stat) != 0) {
		fprintf(stderr, "error when retreiving modified time of file '%s'\n", filename);
		perror("Error getting file stats");
		exit(-1);
	}
	// 26 ctime always return 26 chars
	mtime_str = ctime(&file_stat.st_mtime);
	if (mtime_str == NULL) {
		perror("Error getting modified time");
		exit(-1);
	}
	strncpy(text, mtime_str, 26);
	text[24] = '\0'; // drop the \n in ctime
	printf("modified time: %s\n", text);
	// exit(-1);
	len_text = strlen(text);

	fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Error openning the file '%s'\n", filename); 
		exit(-1);
	}
	length = read_file(&content, fp);
	// printf("length: %zu\n", length);
	// printf("content: %s\n", content);
	fclose(fp);
		
	distance = get_interval(&start_modified, &end_modified, content);
	// printf("distance: %ld\n", distance);
	// printf("start_modified: %ld\n", start_modified);
	// printf("end_modified: %ld\n", end_modified);

		
	if (distance >= 0) {
		len_head =  content - start_modified;
		len_tail =  strnlen(end_modified, length - len_text - len_head);
		memmove(start_modified + len_text, start_modified + distance, len_tail);
		memcpy(start_modified, text, len_text);
		*(start_modified+len_text+len_tail) = '\0';
		// printf("%s\n", content);
			
		// write content
		fp = fopen(filename, "w");
		fwrite(content, sizeof(char), strlen(content), fp);
		fclose(fp);
			
		// copy original values when the file was read
#ifdef __APPLE__
		new_times[0] = file_stat.st_atimespec;
		new_times[1] = file_stat.st_mtimespec;
#else
			// for linux
		new_times[0] = file_stat.st_atim;
		new_times[1] = file_stat.st_mtim;
#endif
			
		// update the access/modified time
		if (utimensat(AT_FDCWD, filename, new_times, 0) < 0) {
			perror("Error updating file modification time");
			fprintf(stderr, "File '%s' was updated anyway", filename);
		}
	}
	else {
		fprintf(stderr, "file not modified\n");
	}
		

	free(content);

	return 0;
}
