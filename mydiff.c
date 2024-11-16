#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1024

int print_done=0;

void diff(char ** old_source,char ** new_source,int oldlines_count,int newlines_count,int old_offset,int new_offset){
    int total_lines=oldlines_count+newlines_count;
    int line_diff=oldlines_count-newlines_count;
    int x_position_vector=2*total_lines+1;
    if(oldlines_count>0 && newlines_count>0){
        int * v_front=(int *)calloc(x_position_vector,sizeof(int));
        int * v_back=(int *)calloc(x_position_vector,sizeof(int));
        int * v1,*v2;
        int o,m;
        if(v_back==NULL || v_front==NULL){
            printf("Error in allocationg space.\n");
            exit(EXIT_FAILURE);
        }
        // for (int i = 0; i < x_position_vector; i++) {
        //     v_front[i] = i-total_lines;
        //     v_back[i] = i-total_lines;
        // }
        v_front[total_lines+1]=0;
        v_back[total_lines+1]=0;
        for(int D=0;D<oldlines_count+newlines_count;D++){
            for(int i=0;i<2;i++){
                if(i==0){
                    v1=v_front;
                    v2=v_back;
                    o=1,m=1;
                }
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
                for(int j=limit1;j<=limit2;j=j+2){
                    int a,b,a0,b0,z;
                    if((j==-D || j!=D) && v1[total_lines+j-1]<v1[total_lines+j+1]){
                        a=v1[total_lines+j+1];
                    }
                    else{
                        a=v1[total_lines+j-1]+1;
                    }
                    if(D==0)
                        a=-1;
                    b=a-j;
                    a0=a,b0=b;
                    while(a<oldlines_count-1 && b<newlines_count-1){
                        if(strcmp(old_source[old_offset+(1-o)*oldlines_count + m*(a+1)+(o-1)],new_source[new_offset+(1-o)*newlines_count+m*(b+1)+(o-1)])==0){
                            a++;
                            b++;
                        }
                        else{
                            break;
                        }
                    }
                    v1[total_lines+j]=a;
                    z=-(j-line_diff);
                    if(total_lines%2==o && z>=-(D-o) && z<=D-o && v1[total_lines+j]+v2[total_lines+z]+2>=oldlines_count){
                        int D1,x,y,x0,y0;
                        if(o==1){
                            D1=2*D-1;
                            x=a+1;
                            y=b+1;
                            x0=a0+1;
                            y0=b0+1;
                        }
                        else{
                            D1=2*D;
                            x=oldlines_count-(a0+1);
                            y=newlines_count-(b0+1);
                            x0=oldlines_count-(a+1);
                            y0=newlines_count-(b+1);
                        }
                        if(D1>1 || (x!=x0 && y!=y0)){
                            diff(old_source,new_source,x0,y0,old_offset,new_offset);
                            diff(old_source,new_source,oldlines_count-x,newlines_count-y,old_offset+x,new_offset+y);
                            free(v_front);
                            free(v_back);
                            return;
                        }
                        else if(newlines_count>oldlines_count){
                            diff(old_source,new_source,0,newlines_count-oldlines_count,old_offset+oldlines_count,new_offset+oldlines_count);
                            free(v_front);
                            free(v_back);
                            return;
                        }
                        else if(newlines_count<oldlines_count){
                            diff(old_source,new_source,oldlines_count-newlines_count,0,old_offset+newlines_count,new_offset+newlines_count);
                            free(v_front);
                            free(v_back);
                            return;
                        }
                    }
                }
            }
        }
    }
    // Lines Deleted in old file
    else if(oldlines_count>0){
        if(!print_done){
            printf("%-5s %-10s %-10s %-80s\n", "Action", "Position Old", "Position New", "Content");
            print_done=1;
        }
        for(int j=0;j<oldlines_count;j++){
            printf("%-5s %10d %10d %-80s\n", "-", old_offset+j+1,new_offset+1 , old_source[old_offset+j]);

            // printf("- , position old: %d , content: %s\n",old_offset+j,old_source[old_offset+j]);
        }
        return;
    }
    // Lines inserted in new file
    else if(newlines_count>0){
        if(!print_done){
            printf("%-5s %-10s %-10s %-80s\n", "Action", "Position Old", "Position New", "Content");
            print_done=1;
        }
        for(int j=0;j<newlines_count;j++){
            printf("%-5s %10d %10d %-80s\n", "+", old_offset+1, new_offset + j+1, new_source[new_offset + j]);
            // printf("+ , position old: %d , position new: %d , content: %s\n",old_offset,new_offset+j,new_source[new_offset+j]);
        }
        return;
    }
}

// Read File line by line & store it in lines c-string array & return the number of lines readed
int read_lines(char ** lines,FILE * source){
    char * buffer=(char *)malloc(sizeof(char)*MAX_LINE_LENGTH);
    if(!buffer){
        printf("Error in memory allocation for buffer.\n");
        exit(EXIT_FAILURE);
    }
    int no_lines=0,capacity=MAX_LINES;
    while(fgets(buffer,MAX_LINE_LENGTH,source)){
        if(no_lines==capacity){
            capacity*=2;
            char **temp = realloc(lines, sizeof(char*) * capacity); 
            if (!temp) {
                perror("Error in memory allocation for storing lines pointer.");
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

int main(int argc,char * argv[]){
    if(argc!=3){
        printf("Invalid Command. Use ./mydiff <source_file> <destination_file>\n");
        exit(EXIT_FAILURE);
    }    
    FILE * old_sourcefile=fopen(argv[1],"r");
    if(!old_sourcefile){
        printf("Error in opening old source file %s!",argv[1]);
        exit(EXIT_FAILURE);
    }
    FILE * new_sourcefile=fopen(argv[2],"r");
    if(!new_sourcefile){
        printf("Error in openning new source file %s",argv[2]);
        exit(EXIT_FAILURE);
    }
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
    int oldlines_count = read_lines(oldlines,old_sourcefile);
    int newlines_count = read_lines(newlines,new_sourcefile);
    if(fclose(old_sourcefile)!=0){
        printf("Error in closing old source file.\n");
        exit(EXIT_FAILURE);
    }
    if(fclose(new_sourcefile)!=0){
        printf("Error in closing new source file.\n");
        exit(EXIT_FAILURE);
    }
    diff(oldlines,newlines,oldlines_count,newlines_count,0,0);
    if(!print_done){
        printf("No difference in files.\n");
    }
    free(oldlines);
    free(newlines);
    return 0;
}