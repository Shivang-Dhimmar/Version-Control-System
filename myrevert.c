#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include<string.h>
#include "mydiff.h"

int read_lines(char  ** lines,FILE * source){
    char * buffer=(char *)malloc(sizeof(char)*MAX_LINE_LENGTH);
    if(!buffer){
        printf("Error in memory allocation for buffer.\n");
        exit(EXIT_FAILURE);
    }
    int no_lines=0,capacity=MAX_LINES;
    while(fgets(buffer,MAX_LINE_LENGTH,source)){
        if(no_lines==capacity){
            capacity*=2;
            char ** temp = realloc((*lines), sizeof(char*) * capacity); 
            if (!temp) {
                printf("Error in memory allocation for storing lines pointer.");
                exit(EXIT_FAILURE);
            }
            lines = temp; 
        }
        buffer[strcspn(buffer,"\n")]='\0';
        lines[no_lines]=(char *)malloc(strlen(buffer)+1);
        if(!lines[no_lines]){
            printf("Error in memory allocation for storing lines.\n");
            exit(EXIT_FAILURE);
        }
        strcpy(lines[no_lines],buffer);
        no_lines++;
    }
    free(buffer);
    return no_lines;
}

int read_changes(changes ** lines,FILE * source){
    char * content=(char *)malloc(sizeof(char)*MAX_LINE_LENGTH);
    if(!content){
        printf("Error in memory allocation for buffer.\n");
        exit(EXIT_FAILURE);
    }
    int no_lines=0,capacity=MAX_CHANGES;
    char operation;
    int line_number;
    while(fscanf(source, "%c,%d,%[^\n]\n", &operation, &line_number, content) == 3) {
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

int main(int argc,char * argv[]){
    if(argc!=2){
        printf("Invalid Command. Use ./myrevert <new_file_name>\n");
        exit(EXIT_FAILURE);
    }
    FILE * changes_file=fopen("changes.txt","r");
    FILE * source_file=fopen(argv[1],"r");
    char ** source_lines=malloc(sizeof(char *)*MAX_LINES);
    changes * changes_lines= (changes *)malloc(sizeof(changes)*MAX_CHANGES);

    if(!changes_file || !source_file){
        printf("Error in opening changes file.\n");
        exit(EXIT_FAILURE);
    }
    if(!source_lines || !changes_lines){
        printf("Error in allocation of lines for source file.\n");
        exit(EXIT_FAILURE);
    }
    int source_line_count=read_lines(source_lines,source_file);
    int changes_line_count=read_changes(&changes_lines,changes_file);
    

    // Useful parameters for performing changes
    int line_drift=0,start_line,end_line,temp_line=1,completed_changes=0,lines_at_once,capacity=source_line_count,temp_offset;
    long offset_drift=0;
    bool flag=false,flag1,flag2;
    char previous_operation=changes_lines[0].operation;
    int previous_line=changes_lines[0].line_no;
    start_line=end_line=previous_line;
    int temp_line2,start2,end2,previous_line2,lines_at_once2;
    while(1){
        while((temp_line<changes_line_count) && (changes_lines[temp_line].operation==previous_operation) && (changes_lines[temp_line].line_no==previous_line+1)){
            end_line=changes_lines[temp_line].line_no;
            previous_line++;
            temp_line++;
        }
        if(temp_line==changes_line_count){
            if(previous_operation=='+'){
                lines_at_once=end_line-start_line+1;
                temp_offset=source_line_count-1;
                if(start_line==source_line_count+1){
                    for(int i=0;i<lines_at_once;i++){
                        source_lines[source_line_count+i]=changes_lines[completed_changes+i].content;
                    }
                }
                else{
                    while(temp_offset>=start_line-1){
                        if(capacity<temp_offset+lines_at_once){
                            capacity*=2;
                            char ** temp = realloc(source_lines, sizeof(char *) * capacity); 
                            if (!temp) {
                                printf("Error in memory allocation for storing lines pointer.");
                                exit(EXIT_FAILURE);
                            }
                            source_lines = temp;
                        }
                        source_lines[temp_offset+lines_at_once]=source_lines[temp_offset];
                        temp_offset--;
                    }
                    for(int i=0;i<lines_at_once;i++){
                        source_lines[start_line+i-1]=changes_lines[completed_changes+i].content;
                    }
                } 
                source_line_count+=lines_at_once;
                break;
            }
            else if(previous_operation=='-'){
                lines_at_once=end_line-start_line+1;
                temp_offset=end_line+1;
                if(end_line+line_drift==source_line_count){
                    source_line_count-=lines_at_once;
                }
                else{
                    while(temp_offset+line_drift<=source_line_count){
                        source_lines[temp_offset-lines_at_once+line_drift-1]=source_lines[temp_offset+line_drift-1];
                        temp_offset++;
                    }
                    source_line_count-=lines_at_once;
                }
                break;
            }
            else{
                printf("Invalid Operation Log in changes file.\n");
                exit(EXIT_FAILURE);
            }
        }
        else{
            if(previous_operation=='+'){
                lines_at_once=end_line-start_line+1;
                temp_offset=source_line_count-1;
                while(temp_offset>=start_line-1){
                    if(capacity<temp_offset+lines_at_once){
                        capacity*=2;
                        char ** temp = realloc(source_lines, sizeof(char *) * capacity); 
                        if (!temp) {
                            printf("Error in memory allocation for storing lines pointer.");
                            exit(EXIT_FAILURE);
                        }
                        source_lines = temp;
                    }
                    source_lines[temp_offset+lines_at_once]=source_lines[temp_offset];
                    temp_offset--;
                }
                for(int i=0;i<lines_at_once;i++){
                    source_lines[start_line+i-1]=changes_lines[completed_changes+i].content;
                }
                completed_changes+=lines_at_once;
                line_drift+=lines_at_once;
                source_line_count+=lines_at_once;
            }
            else if(previous_operation=='-'){
                lines_at_once=end_line-start_line+1;
                temp_offset=end_line+1;
                while(temp_offset+line_drift<=source_line_count){
                    source_lines[temp_offset-lines_at_once+line_drift-1]=source_lines[temp_offset+line_drift-1];
                    temp_offset++;
                }
                completed_changes+=lines_at_once;
                source_line_count-=lines_at_once;
                line_drift-=lines_at_once;
            }
            else{
                printf("Invalid Operation Log in changes file.\n");
                exit(EXIT_FAILURE);
            }
        }
        if(temp_line>=changes_line_count)
            break;
        previous_operation=changes_lines[temp_line].operation;
        previous_line=changes_lines[temp_line].line_no;
        start_line=end_line=previous_line;
        temp_line++;
    }
    for(int i=0;i<source_line_count;i++){
        printf("%s\n",source_lines[i]);
    }
    printf("File reverting completed.\n");
    return 0;
}