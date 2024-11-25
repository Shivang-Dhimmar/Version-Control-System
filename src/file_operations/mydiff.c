#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include "common.h"

// make global variavle that can be access by both the functions to avoid passsing more arguments
static int print_done=0;
int * v_front;
int * v_back;
int x_position_vector_size;
char buffer_for_file[150];
int length;
int changes_file_fd;
char * new_file_name;

// This functionis called recursively to solve the sub-problems using divide and conquer approach 
void diff(char ** old_source,char ** new_source,int oldlines_count,int newlines_count,int old_offset,int new_offset){
    int total_lines=oldlines_count+newlines_count;
    int line_diff=oldlines_count-newlines_count;
    x_position_vector_size= (oldlines_count<newlines_count)? 2*oldlines_count+2 : 2*newlines_count+2;


    // Handle situaltion when both old and new content is non-empty
    if((oldlines_count>0) && (newlines_count>0)){
        int * v1,*v2;
        int o,m;
        for (int i = 0; i < x_position_vector_size; i++) {
            v_front[i] = 0;
            v_back[i] = 0;
        }
        int iterations=(total_lines/2) + (total_lines%2);

        // Loop for atmost iterations differences in the old & new content
        for(int D=0;D<=iterations;D++){
            // serach differences from forward and backward both  
            for(int i=0;i<2;i++){
                // for forward scan
                if(i==0){
                    v1=v_front;
                    v2=v_back;
                    o=1,m=1;
                }
                //for backward scan
                else{
                    v1=v_back;
                    v2=v_front;
                    o=0,m=-1;
                }
                int limit1,limit2;
                if(D-newlines_count>0)
                    limit1=-(D-2*(D-newlines_count));
                else
                    limit1=-D;
                if(D-oldlines_count>0)
                    limit2=D-(2*(D-oldlines_count));
                else    
                    limit2=D;
                // For D changes find search all posibilities of differences
                for(int j=limit1;j<=limit2;j=j+2){
                    int a,b,a0,b0,z;
                    int temp1=(j-1)%x_position_vector_size,temp2=(j+1)%x_position_vector_size;
                    if(temp1<0)
                        temp1+=x_position_vector_size;
                    if(temp2<0)
                        temp2+=x_position_vector_size;
                    if(j==-D || (j!=D && v1[temp1]<v1[temp2])){
                        a=v1[temp2];
                    }
                    else{
                        a=v1[temp1]+1;
                    }
                    b=a-j;
                    a0=a,b0=b;
                    // increament temporary variables for matching lines
                    while(a<oldlines_count && b<newlines_count){
                        if(strcmp(old_source[(1-o)*oldlines_count + m*a+(o-1)],new_source[(1-o)*newlines_count+m*b+(o-1)])==0){
                            a++;
                            b++;
                        }
                        else{
                            break;
                        }
                    }
                    int temp3=j%x_position_vector_size;
                    if(temp3<0)
                        temp3+=x_position_vector_size;
                    v1[temp3]=a;
                    z=-(j-line_diff);
                    int temp4=z%x_position_vector_size;
                    if(temp4<0)
                        temp4+=x_position_vector_size;

                    // Condition for checking is middle snake is arrived
                    if(total_lines%2==o && z>=-(D-o) && z<=D-o && v1[temp3]+v2[temp4]>=oldlines_count){
                        int D1,x,y,x0,y0;
                        if(o==1){
                            D1=2*D-1;
                            x0=a0;
                            y0=b0;
                            x=a;
                            y=b;
                        }
                        else{
                            D1=2*D;
                            x0=oldlines_count-a;
                            y0=newlines_count-b;
                            x=oldlines_count-a0;
                            y=newlines_count-b0;
                        }

                        // Ignore common lines & call diff for upper portion & lower portion of old & new content(divide & conquer approach) 
                        if(D1>1 || (x!=x0 && y!=y0)){
                            diff(old_source,new_source,x0,y0,old_offset,new_offset);
                            diff(old_source+x,new_source+y,oldlines_count-x,newlines_count-y,old_offset+x,new_offset+y);
                            return;
                        }
                        // If old content is sub part of new content then call diff for remaining content of new content, It goes to Insertion handler
                        else if(newlines_count>oldlines_count){
                            diff(old_source+oldlines_count,new_source+oldlines_count,0,newlines_count-oldlines_count,old_offset+oldlines_count,new_offset+oldlines_count);
                            return;
                        }
                        // If new content is sub part of old content then call diff for remaining content of old content, It goes to Deletion handler
                        else if(newlines_count<oldlines_count){
                            diff(old_source+newlines_count,new_source+newlines_count,oldlines_count-newlines_count,0,old_offset+newlines_count,new_offset+newlines_count);
                            return;
                        }
                    }
                }
            }
        }
    }

    // Handle lines Deletion in old file & insert entries of deletion in changes.txt file
    else if(oldlines_count>0){
        if(!print_done){
            printf("%s\n",new_file_name);
            printf("%-5s %-10s %-80s\n", "Action", "Line Number", "Content");
            print_done=1;
        }
        for(int j=0;j<oldlines_count;j++){
            // Write formatted output in buffer first and then write it in file
            length=snprintf(buffer_for_file,sizeof(buffer_for_file),"%c,%d,%s\n", '-', old_offset+j+1 ,old_source[j]);
            if(write(changes_file_fd,buffer_for_file,length)<0){
                printf("Error in writting the diff in file.\n");
                exit(EXIT_FAILURE);
            }
            // Write content on terminal so user can see difference
            printf("\033[31m%-5s %10d %-80s\033[0m\n", "-", old_offset+j+1, old_source[j]);
        }
        printf("\n");
        return;
    }

    // Handle lines insertion in new file & insert entries of insertion in changes.txt file 
    else if(newlines_count>0){
        if(!print_done){
            printf("%s\n",new_file_name);
            printf("%-5s %-10s %-80s\n", "Action", "Line Number", "Content");
            print_done=1;
        }
        for(int j=0;j<newlines_count;j++){
            // Write formatted output in buffer first and then write it in file
            length=snprintf(buffer_for_file,sizeof(buffer_for_file),"%c,%d,%s\n", '+',new_offset + j+1, new_source[j]);
            if(write(changes_file_fd,buffer_for_file,length)<0){
                printf("Error in writting the diff in file.\n");
                exit(EXIT_FAILURE);
            }
            // Write content on terminal so user can see difference
            printf("\033[32m%-5s %10d %-80s\033[0m\n", "+", new_offset + j+1, new_source[j]);
        }
        printf("\n");
        return;
    }
}

/*  This function writes the difference of two files in temporary changes.txt file.
    It takes old file content and new file name as an input.  */
int mydiff(char * oldfile_content,char * newfile){   
    FILE * new_sourcefile=fopen(newfile,"r");
    if(!new_sourcefile){
        printf("Error in openning new source file %s",newfile);
        exit(EXIT_FAILURE);
    }
    new_file_name=newfile;
    changes_file_fd=open("changes.txt",O_WRONLY | O_CREAT | O_TRUNC,0644);
    if(changes_file_fd<0){
        printf("Error in creating or opening the changes file.\n");
        exit(EXIT_FAILURE);
    }
    // Allocate memory for c-string array for old and new file content
    char ** oldlines=malloc(sizeof(char *)*MAX_LINES);
    char ** newlines=malloc(sizeof(char *)*MAX_LINES);
    if(!oldlines){
        printf("Error in memory allocation for old source file.\n");
        exit(EXIT_FAILURE);
    }
    if(!newlines){
        printf("Error in memory allocation for new source file.\n");
        exit(EXIT_FAILURE);
    }

    // read the file and formatted string into array of c-string
    int oldlines_count = read_lines_string(oldlines,oldfile_content);
    int newlines_count = read_lines_file(newlines,new_sourcefile);
    
    
    x_position_vector_size= (oldlines_count<newlines_count)? 2*oldlines_count+2 : 2*newlines_count+2;
    // Initialized array for storing trace of progress of diff calculation
    v_front=(int *)calloc(x_position_vector_size,sizeof(int));
    v_back=(int *)calloc(x_position_vector_size,sizeof(int));
    if(v_back==NULL || v_front==NULL){
        printf("Error in allocationg space.\n");
        exit(EXIT_FAILURE);
    }
    if(fclose(new_sourcefile)!=0){
        printf("Error in closing new source file.\n");
        exit(EXIT_FAILURE);
    }
    // this function compute difference between two files and write it into the changes.txt file
    diff(oldlines,newlines,oldlines_count,newlines_count,0,0);
    if(!print_done){
        printf("No difference in files.\n");
    }
    free(v_front);
    free(v_back);
    free(oldlines);
    free(newlines);
    return 0;
}
