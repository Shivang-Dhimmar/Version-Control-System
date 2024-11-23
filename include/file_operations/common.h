#ifndef COMMON_H
#define COMMON_H

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1024
#define MAX_CHANGES 1000
#define MAX_BUFFER_SIZE 1000000

typedef struct changes{
    char operation;
    int line_no;
    char * content;
}changes;

int read_lines(char  ** lines,FILE * source);
int read_changes(changes ** lines,FILE * source);

#endif