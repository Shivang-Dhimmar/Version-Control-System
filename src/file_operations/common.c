#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"common.h"

/*  This function is for reading files content.
    Read the File line by line & store it in lines c-string array & return the number of lines readed*/
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
/*  This function is for reading formatted string content.
    Read the formatted string line by line & store it in lines c-string array & return the number of lines readed */
int read_lines_string(char  ** lines,char * source){
    // create temporary buffer to got one line at a time from formatted string 
    char * buffer=(char *)malloc(sizeof(char)*MAX_LINE_LENGTH);
    if(!buffer){
        printf("Error in memory allocation for buffer.\n");
        exit(EXIT_FAILURE);
    }
    int no_lines=0,capacity=MAX_LINES,temp=0;
    // return 0 if source string is empty.
    if(source[0]=='\0'){
        return 0;
    }

    // parse the string until the end of string
    while (source[temp] != '\0') {
        // If there is nonempty line then read it from string and store it in c-string array
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
        // If there is empty line then add "" in c-string array.
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
        // if string is completely parsed then free the dynamically allocated buffer & return lines readed
        if (source[temp] == '\0') {
            free(buffer);
            return no_lines;
        }     
    }
    return no_lines;
}

/*  Read the File line by line & store it in 'changes' structure array & return the number of changes available in file
    struct changes is used to manipulate the changes efficiently in memory */
int read_changes(changes ** lines,char * source){
    // create temporary buffer to got one change line at a time from formatted string 
    char * content=(char *)malloc(sizeof(char)*MAX_LINE_LENGTH);
    if(!content){
        printf("Error in memory allocation for buffer.\n");
        exit(EXIT_FAILURE);
    }
    int no_lines=0,capacity=MAX_CHANGES,temp=0;
    char operation,*token;
    int line_number;
    token = strtok(source, ",");

    // read the changes formatted string until the string is completely readed
    while (token != NULL) {
        // get operation
        // To handle empty content in difference field of changes 
        if(temp>2 && source[temp-2]=='\0'){
            operation=token[1];
        }
        else{
            operation = token[0]; 
        }
        temp+=2;

        // get line number
        token = strtok(NULL, ",");
        if (token != NULL) {
            line_number = atoi(token);
        }
        temp+=strlen(token)+1;

        //get content of difference
        // To handle empty content in differnece filed of changes
        if(source[temp]=='\n'){
            // token = strtok(NULL, "\n");
            strcpy(content, "");
            temp++;
        }
        else{
            token = strtok(NULL, "\n");
            strcpy(content, token); 
            temp+=strlen(token)+1;
        }

        // insert the changes content into the in-memory 'changes' structure array
        if(no_lines==capacity){
            capacity*=2;
            changes * temp = realloc((*lines), sizeof(changes*) * capacity); 
            if (!temp) {
                printf("Error in memory allocation for storing lines pointer.");
                exit(EXIT_FAILURE);
            }
            (*lines) = temp; 
        }
        (*lines)[no_lines].content=(char *)malloc(strlen(content)+1);
        if(!(*lines)[no_lines].content){
            printf("Error in memory allocation for storing lines.\n");
            exit(EXIT_FAILURE);
        }
        strcpy((*lines)[no_lines].content,content);
        (*lines)[no_lines].operation=operation;
        (*lines)[no_lines].line_no=line_number;
        token = strtok(NULL, ",");
        no_lines++;
    }
    free(content);
    return no_lines;
}