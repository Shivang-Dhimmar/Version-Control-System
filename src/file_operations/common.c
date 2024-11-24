#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"common.h"

// Read the File line by line & store it in lines c-string array & return the number of lines readed
int read_lines_file(char  ** lines,FILE * source){
    // create temporary buffer to got one line at a time from file 
    char * buffer=(char *)malloc(sizeof(char)*MAX_LINE_LENGTH);
    if(!buffer){
        printf("Error in memory allocation for buffer.\n");
        exit(EXIT_FAILURE);
    }
    int no_lines=0,capacity=MAX_LINES;

    // reading file line by line and storing it's content in in-memory c-style array
    while(fgets(buffer,MAX_LINE_LENGTH,source)){
        // If we reach the initial capacity of array of c-strings then allocate double size array for storing lines content
        if(no_lines==capacity){
            capacity*=2;
            char **temp = realloc((*lines), sizeof(char *) * capacity); 
            if (!temp) {
                printf("Error in memory allocation for storing lines pointer.");
                exit(EXIT_FAILURE);
            }
            lines = temp; 
        }
        buffer[strcspn(buffer,"\n")]='\0';
        // allocate dynamic memory for storing line content of file and assign it to c-string array
        lines[no_lines]=(char *)malloc(strlen(buffer)+1);
        if(!lines[no_lines]){
            printf("Error in memory allocation for storing lines.\n");
            exit(EXIT_FAILURE);
        }
        //copy the string
        strcpy(lines[no_lines],buffer);
        no_lines++;
    }
    free(buffer);
    return no_lines;
}

// Read the File line by line & store it in lines c-string array & return the number of lines readed
int read_lines_string(char  ** lines,char * source){
    // create temporary buffer to got one line at a time from file 
    char * buffer=(char *)malloc(sizeof(char)*MAX_LINE_LENGTH);
    if(!buffer){
        printf("Error in memory allocation for buffer.\n");
        exit(EXIT_FAILURE);
    }
    int no_lines=0,capacity=MAX_LINES,temp=0;
    if(source[0]=='\0'){
        return 0;
    }

    while (source[temp] != '\0') {
        if (sscanf(source + temp, "%[^\n]", buffer) == 1) {
            if(no_lines==capacity){
                capacity*=2;
                char **temp = realloc((*lines), sizeof(char *) * capacity); 
                if (!temp) {
                    printf("Error in memory allocation for storing lines pointer.");
                    exit(EXIT_FAILURE);
                }
                lines = temp; 
            }
            // allocate dynamic memory for storing line content of file and assign it to c-string array
            lines[no_lines]=(char *)malloc(strlen(buffer)+1);
            if(!lines[no_lines]){
                printf("Error in memory allocation for storing lines.\n");
                exit(EXIT_FAILURE);
            }
            //copy the string
            strcpy(lines[no_lines],buffer);
            no_lines++;
            temp += strlen(buffer);
            if (source[temp] == '\n') {
                temp++;  
            }
        }
        if(source[temp]=='\n'){
            if(no_lines==capacity){
                capacity*=2;
                char **temp = realloc((*lines), sizeof(char *) * capacity); 
                if (!temp) {
                    printf("Error in memory allocation for storing lines pointer.");
                    exit(EXIT_FAILURE);
                }
                lines = temp; 
            }
            lines[no_lines]=(char *)malloc(1);
            if(!lines[no_lines]){
                printf("Error in memory allocation for storing lines.\n");
                exit(EXIT_FAILURE);
            }
            strcpy(lines[no_lines],"");
            no_lines++;
            temp++;
        }
        if (source[temp] == '\0') {
            free(buffer);
            return no_lines;
        }     
    }
    return no_lines;
}

// Read the File line by line & store it in 'changes' structure array & return the number of changes available in file
// struct changes is used to manipulate the changes efficiently in memory
int read_changes(changes ** lines,char * source){
    char * content=(char *)malloc(sizeof(char)*MAX_LINE_LENGTH);
    if(!content){
        printf("Error in memory allocation for buffer.\n");
        exit(EXIT_FAILURE);
    }
    int no_lines=0,capacity=MAX_CHANGES;
    char operation;
    int line_number;
    while(sscanf(source, "%c,%d,%[^\n]\n", &operation, &line_number, content) == 3) {
        if(no_lines==capacity){
            capacity*=2;
            changes * temp = realloc((*lines), sizeof(changes*) * capacity); 
            if (!temp) {
                printf("Error in memory allocation for storing lines pointer.");
                exit(EXIT_FAILURE);
            }
            (*lines) = temp; 
        }
        content[strcspn(content,"\n")]='\0';
        (*lines)[no_lines].content=(char *)malloc(strlen(content)+1);
        if(!(*lines)[no_lines].content){
            printf("Error in memory allocation for storing lines.\n");
            exit(EXIT_FAILURE);
        }
        strcpy((*lines)[no_lines].content,content);
        (*lines)[no_lines].operation=operation;
        (*lines)[no_lines].line_no=line_number;
        no_lines++;
    }
    free(content);
    return no_lines;
}