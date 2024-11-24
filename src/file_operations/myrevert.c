#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include<string.h>
#include<fcntl.h>
#include "common.h"

int myrevert(char * old_file_content,char * new_file,char *diff_content,char * permission){
    int new_file_fd=open(new_file,O_WRONLY | O_CREAT | O_TRUNC,permission);
    char ** source_lines=malloc(sizeof(char *)*MAX_LINES);
    changes * changes_lines= (changes *)malloc(sizeof(changes)*MAX_CHANGES);

    if(new_file_fd<0){
        printf("Error in opening changes file.\n");
        exit(EXIT_FAILURE);
    }
    if(!source_lines || !changes_lines){
        printf("Error in allocation of lines for source file.\n");
        exit(EXIT_FAILURE);
    }
    int source_line_count=read_lines_string(source_lines,old_file_content);
    if(diff_content){
        int changes_line_count=read_changes(&changes_lines,diff_content);
        // Useful parameters for performing changes
        int line_drift=0,start_line,end_line,temp_line=1,completed_changes=0,lines_at_once,capacity=source_line_count,temp_offset;
        char previous_operation=changes_lines[0].operation;
        int previous_line=changes_lines[0].line_no;
        start_line=end_line=previous_line;
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
    }
    for(int i=0;i<source_line_count;i++){
        printf("%s\n",source_lines[i]);
        if(write(new_file_fd,strcat(source_lines[i],"\n"),strlen(source_lines[i])+1)<0){
            printf("Error in writting content to the new source file.\n");
            exit(EXIT_FAILURE);
        }
    }
    printf("File reverting completed.\n");
    return 0;
}