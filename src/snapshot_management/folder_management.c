#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <zlib.h>
#include <openssl/sha.h>
#include <errno.h>
#include <fcntl.h>

extern int mydiff(char * oldfile_content,char * newfile);
extern int myrevert(char * old_file_content,char * new_file,char *diff_content,char * permission);


#define MAX_PATH_LENGTH 1024
#define FILE_RECORD_PATH ".mygit/files"

#define MAX_PATH_LENGTH 1024

int split_string(char  ** lines,const char *input) {
    
    int len = strlen(input);

    int max_lines=100;
    
    int line_start = 0;
    int line_count = 0;
    
    for (int i = 0; i < len; i++) {
        
        if (input[i] == '\n') {
            
            int line_length = i - line_start + 1;  
            lines[line_count] = malloc(line_length + 1);  
            strncpy(lines[line_count], &input[line_start], line_length);
            lines[line_count][line_length] = '\0'; 
            line_count++;

             if (line_count >= max_lines) {
                max_lines *= 2;
                lines = realloc(lines, sizeof(char*) * max_lines);
                if (lines == NULL) {
                    perror("Failed to reallocate memory");
                    exit(1);
                }
            }
            line_start = i + 1;  
        }
    }
    if (line_start < len) {
        lines[line_count] = malloc(len - line_start + 1);
        strcpy(lines[line_count], &input[line_start]);
        line_count++;
    }

    return line_count;
}

// Helper function to read file content into a buffer
size_t read_file(const char *path, char **buffer) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        perror("Failed to open file");
        return 0;
    }

    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);

    *buffer = malloc(length);
    if (*buffer == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return 0;
    }

    fread(*buffer, 1, length, file);
    fclose(file);

    return length;
}

// Helper function to write content to a file
int write_file(const char *path, const char *content, size_t length) {
    FILE *file = fopen(path, "wb+");
    if (!file) {
        printf("%s",path);
        perror("Failed to open file for writing.....");
        return -1;
    }

    fwrite(content, 1, length, file);
    fclose(file);

    return 0;
}


// Function to read an object from the .mygit/objects directory
void read_object(const char *parent, const char *sha, char **content, size_t *content_length) {
    char path[MAX_PATH_LENGTH];
    snprintf(path, sizeof(path), "%s/.mygit/objects/%c%c/%s", parent, sha[0], sha[1], sha);

    // Read the compressed file
    char *compressed_content;
    size_t compressed_length = 0;
    if ((compressed_length =read_file(path, &compressed_content)) == 0) {
        return;
    }

    // Decompress the content
    uLongf decompressed_length = 1024 * 1024; // Max decompressed size
    *content = malloc(decompressed_length);
    int ret = uncompress((Bytef *)*content, &decompressed_length, (const Bytef *)compressed_content, compressed_length);
    if (ret != Z_OK) {
        perror("Failed to decompress object");
        //free(compressed_content);
        return;
    }

    *content_length = decompressed_length;
    //free(compressed_content);
}



void revert(char* dir,char* hash){


    char* content =NULL;
    size_t content_length;

    read_object(".",hash,&content,&content_length);

    char* tree_header = strtok_r(content, "\t",&content);
    char* entry=NULL;
    while(entry=strtok_r(content, " ",&content)){

    
    int mode = atoi(strtok_r(content," ",&content));
    //statbuf.st_mode & 07777, entry->d_name, entry_hash,diff_hash


    //mode = atoi(entry);

    
    char full_path[1024];
   
    char* name = strtok_r(content," ",&content);
    char* entry_hash=strtok_r(content," ",&content);

    char* diff_hash=strtok_r(content,"\n",&content);
    sprintf(full_path,"%s/%s",dir,name);
    if(mode){
        mkdir(full_path, 0765);
        revert(full_path,entry_hash);

    }


    else{

        char* content=NULL;
        read_object(".", entry_hash, &content, &content_length);
        char* header = strtok_r(content,"\t",&content);
        char* actual_content= strtok_r(content,"\0",&content);
        char* actual_diff=NULL;
        if(!strcmp(diff_hash,"NO")){
            actual_diff=NULL;
        }
        else{
        content=NULL;
        read_object(".", diff_hash, &content, &content_length);
        header = strtok_r(content,"\t",&content);
        actual_diff = strtok_r(content,"\0",&content);

        }
        myrevert(actual_content,full_path,actual_diff,entry);
    }

    }
}



// Function to write an object to the .mygit/objects directory
char *write_object(const char *parent, const char *type, const char *content, size_t content_length) {
    // Prepare the content
    char *header;
    size_t header_length = snprintf(NULL, 0, "%s %zu\t", type, content_length);
    header = malloc(header_length + content_length);
    snprintf(header, header_length + content_length, "%s %zu\t", type, content_length);
    memcpy(header + header_length, content, content_length);

    // Hash the content
    unsigned char sha1_hash[SHA_DIGEST_LENGTH];
    char* hex_string = (char*) malloc( 2*SHA_DIGEST_LENGTH+1);
    SHA1((unsigned char *)header, header_length + content_length, sha1_hash);
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf(hex_string+2*i, "%02x", sha1_hash[i]);
    }
    // Create the file path
    char path[MAX_PATH_LENGTH];

    snprintf(path, sizeof(path), "%s/.mygit/objects/%c%c", parent, hex_string[0],hex_string[1]);

    DIR* dir = opendir(path);

    if(ENOENT == errno){
        mkdir(path, 0765);
    }

    
    snprintf(path, sizeof(path), "%s/.mygit/objects/%c%c/%s", parent, hex_string[0],hex_string[1], hex_string);

    // Compress the content
    uLongf compressed_length = compressBound(header_length + content_length);
    char *compressed_content = malloc(compressed_length);
    int ret = compress((Bytef *)compressed_content, &compressed_length, (const Bytef *)header, header_length + content_length);
    if (ret != Z_OK) {
        perror("Failed to compress content");
        //free(header);
        //free(compressed_content);
        return NULL;
    }

    // Write the compressed content to the file
    write_file(path, compressed_content, compressed_length);

    // Clean up
    //free(header);
    //free(compressed_content);

    // Return the object hash as a hex string

    //char* hash=malloc(sizeof(sha1_hash)+1);


    return hex_string;
}




char* get_blob_hash_for_file(const char* file_path) {
    FILE* file = fopen(FILE_RECORD_PATH, "r");
    if (!file) {
        if (errno == ENOENT) {
            // .mygit/files doesn't exist, return NULL
            return NULL;
        } else {
            perror("fopen");
            exit(1);
        }
    }

    char line[MAX_PATH_LENGTH + 41];
    char* return_hash = (char*)malloc(41); // Max path length + hash length
    while (fgets(line, sizeof(line), file)) {
        char* path = strtok(line, " ");
        char* hash = strtok(NULL, "\n");
        if (path && hash && strcmp(path, file_path) == 0) {
            fclose(file);
            snprintf(return_hash,41,hash);
            return return_hash;  // Return the hash if found
        }
    }

    fclose(file);
    return NULL;  // File hash not found
}

// Function to add a file and its hash to .mygit/files
void add_file_to_git(const char* file_path, const char* hash) {
    FILE* file = fopen(FILE_RECORD_PATH, "a+");
    if (!file) {
        perror("fopen");
        exit(1);
    }

    fprintf(file, "%s %s\n", file_path, hash);
    fclose(file);
}






// Function to recursively generate a tree object from the directory structure
char *write_tree(const char *parent, const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Failed to open directory....");
        return NULL;
    }

    // List of entries: (mode, name, hash)
    char *entries = malloc(0);
    size_t entries_length = 0;
    int mode=0;

    struct dirent *entry;
    while ((entry = readdir(dir))) {


        // Skip .mygit directory
        if (strcmp(entry->d_name, ".mygit") == 0 || strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[MAX_PATH_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        // Check if it's a directory or file
        struct stat statbuf;
        if (stat(full_path, &statbuf) == -1) {
            perror("Failed to stat file");
            continue;
        }

        // If it's a directory, create a tree entry
        char *entry_hash = NULL;
        char *diff_hash=NULL;
        size_t entry_length = 0;
        size_t diff_len;
        printf("%s",entry->d_name);
        if (S_ISDIR(statbuf.st_mode)) {
            mode=1;
            entry_hash = write_tree(parent, full_path);
            entry_length = strlen(entry_hash);
        } else {
            // If it's a file, create a blob entry
            mode=0;
            char* existing_hash = get_blob_hash_for_file(full_path);
            if (existing_hash) {
                entry_hash = existing_hash;
                entry_length = strlen(entry_hash);
                char *content = NULL;
                size_t content_length = 0;
                read_object(".", existing_hash, &content, &content_length);
                char* header = strtok(content,"\t");
                char* actual_content= strtok(NULL,"\0");
                char *file_content = NULL;
                //size_t file_content_length = read_file(full_path, &file_content);

              


                mydiff(actual_content,full_path);

               
                //*file_content = NULL;
                size_t file_content_length = read_file("changes.txt", &file_content);


                remove("changes.txt");

                diff_hash = write_object(parent, "blob", file_content, file_content_length);
                diff_len = strlen(entry_hash);



            }

            else{
                 
                char *file_content = NULL;
                size_t file_content_length = read_file(full_path, &file_content);

                entry_hash = write_object(parent, "blob", file_content, file_content_length);
                //free(file_content);
                entry_length = strlen(entry_hash);
                 add_file_to_git(full_path, entry_hash); 
            }
            
           
        }

        // Append the entry to the list
        size_t mode_length = snprintf(NULL, 0, "%u", statbuf.st_mode & 07777);
        if(!diff_hash){
            diff_hash=(char*)malloc(3);
            diff_hash="NO";
            diff_len=2;
        }
        entries = realloc(entries, entries_length + mode_length + strlen(entry->d_name) + 6 + entry_length+diff_len+1);
        snprintf(entries + entries_length, mode_length + strlen(entry->d_name) + 6 + entry_length +diff_len+1, "%u %d %s %s %s\n", statbuf.st_mode & 07777,mode, entry->d_name, entry_hash,diff_hash);
        
        //sprintf(entries + entries_length,"%u %s %s %s\n",statbuf.st_mode & 07777, entry->d_name, entry_hash,diff_hash);
        entries_length += mode_length + strlen(entry->d_name) + 6 + entry_length + diff_len;
        //free(entry_hash);
    }
    closedir(dir);

    // Write the tree object
    char *tree_hash = write_object(parent, "tree", entries, entries_length);
    //free(entries);
    return tree_hash;
}

// Function to create a commit object
char *commit_tree(const char *parent, const char *tree_sha, const char *parent_sha, const char *message) {
    // Prepare commit content (author, committer, parent, tree, message)
    const char *author = "<30713864+ggzor@shivang.noreply.mygithub.com> 1714599041 -0600\n";
    const char *committer = "<30713864+ggzor@ishan.noreply.mygithub.com> 1714599041 -0600\n";

    // Format the commit message content
    size_t content_length = snprintf(NULL, 0, "tree %s\n", tree_sha) + 
                            (parent_sha ? snprintf(NULL, 0, "parent %s\n", parent_sha) : 0) +
                            strlen(author) +
                            strlen(committer) +
                            strlen(message) +
                            2; // For the newline characters at the end

    char *commit_content = malloc(content_length + 1);
    char *ptr = commit_content;

    ptr += sprintf(ptr, "tree %s\n", tree_sha);
    if (parent_sha) {
        ptr += sprintf(ptr, "parent %s\n", parent_sha);
    }
    ptr += sprintf(ptr, "%s", author);
    ptr += sprintf(ptr, "%s", committer);
    ptr += sprintf(ptr, "\n%s\n", message);

    // Write the commit object
    char *commit_hash = write_object(parent, "commit", commit_content, content_length);
    //free(commit_content);

    return commit_hash;
}

// Main function
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "init") == 0) {
        // Initialize the git repository
        mkdir(".mygit", 0765);
        mkdir(".mygit/objects", 0765);
        printf("Initialized mygit directory\n");
    } else if (strcmp(argv[1], "cat-file") == 0 && argc == 4 && strcmp(argv[2], "-p") == 0) {
        // Display object content
        char *content = NULL;
        size_t content_length = 0;
        read_object(".", argv[3], &content, &content_length);
        if (content) {
            fwrite(content, 1, content_length, stdout);
            //free(content);
        }
    } else if (strcmp(argv[1], "hash-object") == 0 && argc == 4 && strcmp(argv[2], "-w") == 0) {
        // Hash and store object
        char *content = NULL;
        size_t content_length = 0;
        read_file(argv[3], &content);
        char *hash = write_object(".", "blob", content, content_length);
        printf("%s\n", hash);
        //free(hash);
        //free(content);
    } else if (strcmp(argv[1], "write-tree") == 0 && argc == 3) {
        // Write the tree object for the directory
        char *tree_hash = write_tree(".", argv[2]);
        printf("Tree Hash: %s\n", tree_hash);
        //free(tree_hash);
    } else if (strcmp(argv[1], "commit-tree") == 0 && argc == 4) {
        // Create a commit object
        const char *tree_sha = argv[2];
        const char *parent_sha = NULL;
        const char *message = argv[3];

        char *commit_hash = commit_tree(".", tree_sha, NULL, message);
        printf("Commit Hash: %s\n", commit_hash);
        //free(commit_hash);
    } 
    
    else if (strcmp(argv[1], "goto") == 0 && argc == 3) {
        // Create a commit object
        system("rm -r *");
        const char *commit_sha = argv[2];
        const char *parent_sha = NULL;
        char* content=NULL;
        size_t content_length=0;
        read_object(".",commit_sha,&content,&content_length);

        char* commit_header = strtok(content, "\t");
        char* tree = strtok(NULL, " ");
        char* tree_sha = strtok(NULL, "\n");

        revert(".",tree_sha);
        
        //free(commit_sha);
    }
    
    
    else {
        fprintf(stderr, "Unknown command or invalid arguments.\n");
        return 1;
    }

    return 0;
}
