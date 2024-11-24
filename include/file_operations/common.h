#ifndef COMMON_H
#define COMMON_H

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1024
#define MAX_CHANGES 1000

// In-memory structure to handle the changes operation efficiently
typedef struct changes{
    char operation;
    int line_no;
    char * content;
}changes;

int read_lines_file(char  ** lines,FILE * source);
int read_lines_string(char  ** lines,char * source);
int read_changes(changes ** lines,char * source);
#endif