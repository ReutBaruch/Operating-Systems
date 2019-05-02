//Reut Baruch
//205467186

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>


#define NUM_OF_CORRECT_ARG 2
#define NUM_CHARS 150
#define NUM_LINES 3

const char* ERROR_TYPE = "Error in system call\n";
const int ERROR_LEN = 250;

void listDir(char param[NUM_LINES][NUM_CHARS]);
int splitFile(char file[]);
void compareFiles(int out, char param[NUM_LINES][NUM_CHARS], char dirName[NUM_CHARS], char path[NUM_CHARS]);

void extractDirName(char *fullPath, char *onlyDir);

int main(int argc, char** argv) {
    //should recieve a path
    if (argc != NUM_OF_CORRECT_ARG){
        write(2, ERROR_TYPE, ERROR_LEN);
    } else {
        char send[NUM_CHARS];
        //copy to temp array
        strcpy(send, argv[1]);

        splitFile(send);
    }

    return 0;
}


int splitFile(char file[]){

    int readID;
    char buffer[2];
    char param[NUM_LINES][NUM_CHARS];
    char tempParam[NUM_LINES][NUM_CHARS];
    int fileID = open(file, O_RDONLY);

    //cant open file
    if (fileID < 0){
        write(2, ERROR_TYPE, ERROR_LEN);
        exit(-1);
    }

    bzero(buffer, sizeof(buffer));
    bzero(param, sizeof(param));

    //separate the file to NUM_LINES lines
    int i = 0;
    for (; i < NUM_LINES; ++i) {
        //reading chars till the end of the first line
        readID = read(fileID, buffer, sizeof(char));

        while ((readID > 0) && (*buffer != '\n')){
            strcat(param[i], buffer);
            bzero(buffer, sizeof(buffer));

            //read the next char
            readID = read(fileID, buffer, sizeof(char));
        }
    } //end of for loop

    strcpy(tempParam[0], param[0]);
    strcpy(tempParam[1], param[1]);
    strcpy(tempParam[2], param[2]);

    listDir(tempParam);
}

/**
 * goes recursevly on all folders and files in a given path
 * @param param path
 */
void listDir(char param[NUM_LINES][NUM_CHARS])
{
    struct dirent *entry;
    int in;
    int out;
    bool noCFile = true;
    pid_t child;
    char path[NUM_CHARS];
    bzero(path, NUM_CHARS);

    DIR *dr = opendir(param[0]);

    //end of recursion
    if (dr == NULL){
        return;
    }

    //write to "results", if doesnt exists - create the file
    if ((out = open("results.csv",O_CREAT | O_WRONLY | O_APPEND, 0644)) < 0) {//} O_CREAT | O_WRONLY | O_APPEND | O_TRUNC, 0644)) < 0){
        write(2, ERROR_TYPE, ERROR_LEN);
        exit(-1);
    }

    strcpy(path, param[0]);

    while ((entry = readdir(dr)) != NULL){

        //if its a directory
        if(entry->d_type == DT_DIR){

            if((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)){
                continue;
            }

            char newPath[NUM_CHARS];
            char newParam[NUM_LINES][NUM_CHARS];

            strcpy(newPath, path);

            strcat(newPath, "/");

            strcat(newPath, entry->d_name);

            strcpy(newParam[0], newPath);
            strcpy(newParam[1], param[1]);
            strcpy(newParam[2], param[2]);

            listDir(newParam);
            //if its a file
        } else {

            char *ending = strrchr(entry->d_name, '.');

            if (ending != NULL) {

                if (strcmp(ending, ".c") == 0) {
                    noCFile = false;
                    char compileFile[NUM_CHARS];

                    strcpy(compileFile, path);
                    strcat(compileFile, "/");

                    strcat(compileFile, entry->d_name);

                    child = fork();

                    //its the father
                    if (child != 0) {
                        int status;

                        while (waitpid(child, &status, 0) < 0);

                        int childStatus = WEXITSTATUS(status);

                        if(childStatus == EXIT_FAILURE){
                            char *buffer = path;
                            extractDirName(path, buffer);
                            strcat(buffer, ",20,COMPILATION_ERROR\n");
                            write(out, buffer, strlen(buffer));
                            return;
                        }

                        pid_t secondChild;
                        secondChild = fork();

                        //its the father
                        if (secondChild != 0) {
                            bool time = false;

                            int childSleep = 5;

                            //waiting for the second child to end
                            while (waitpid(secondChild, 0, WNOHANG) == 0) {
                                if (childSleep == 0) {
                                    time = true;
                                    break;
                                }
                                sleep(1);
                                childSleep--;
                            }

                            if (time) {
                                char *dirName;
                                dirName = strrchr(path, '/');
                                char newDirName[NUM_CHARS];
                                strcpy(newDirName,strtok(dirName, "/"));

                                strcat(newDirName, ",40,TIMEOUT\n");
                                write(out, newDirName, strlen(newDirName));
                                return;

                            } else {
                                char newParam2[NUM_LINES][NUM_CHARS];
                                char tempPath[NUM_CHARS];

                                strcpy(newParam2[0], param[0]);
                                strcpy(newParam2[1], param[1]);
                                strcpy(newParam2[2], param[2]);

                                strcpy(tempPath, path);

                                compareFiles(out, newParam2, compileFile, tempPath);
                                return;
                            }
                            //its the second child
                        } else {

                            char *send[NUM_CHARS];
                            int inChild, outChild;

                            if ((outChild = open("output.txt", O_CREAT | O_WRONLY | O_APPEND | O_TRUNC, 0644)) < 0) {
                                write(2, ERROR_TYPE, ERROR_LEN);
                                exit(-1);
                            }

                            //open the file to compare
                            inChild = open(param[1], O_RDONLY);

                            dup2(inChild, 0);
                            dup2(outChild, 1);

                            send[0] = "./a.out";
                            send[1] = NULL;

                            //run the compiled file
                            int ret_exe = execve("a.out", send, NULL);

                            if (ret_exe == -1) {
                                write(2, ERROR_TYPE, ERROR_LEN);
                                exit(-1);
                            }
                            //close the file
                            close(outChild);
                        }
                        //its the first child
                    } else {

                        char *outputCompile[NUM_LINES];

                        outputCompile[0] = "gcc";
                        outputCompile[1] = compileFile;
                        outputCompile[2] = NULL;

                        int ret_exec = execvp(outputCompile[0], outputCompile);

                        if (ret_exec == -1) {
                            write(2, ERROR_TYPE, ERROR_LEN);
                            exit(-1);
                        }
                    }

                }

            }
        }
    }
    if (noCFile){
        char *dirName;
        char dirtemp[NUM_CHARS];
        bzero(dirtemp, NUM_CHARS);

        strcpy(dirtemp, path);
        char onlyDirName[NUM_CHARS];
        extractDirName(dirtemp, onlyDirName);
        strcat(onlyDirName, ",0,NO_C_FILE\n");
        write(out, onlyDirName, strlen(onlyDirName));

    }


    //close the file
    close(out);

    //delete the file
    char* deleteFile = "output.txt";
    unlink(deleteFile);

    //close the dir
    closedir(dr);
}

void extractDirName(char *fullPath, char* onlyDir) {
    fullPath = strrchr(fullPath, '/');
    strcpy(onlyDir, strtok(fullPath, "/"));
}


void compareFiles(int out, char param[NUM_LINES][NUM_CHARS], char dirName[NUM_CHARS], char path[NUM_CHARS]){

    char *send[NUM_CHARS];
    pid_t childID;
    int status;
    dup2(out,1);

    send[0] = "./comp.out";
    send[1] = "output.txt";
    send[2] = param[2];
    send[3] = NULL;

    childID = fork();

    //its the father
    if (childID != 0){
        while (waitpid(childID, &status, 0) < 0);
        int result = WEXITSTATUS(status);

        char dirTemp[NUM_CHARS];
        strcpy(dirTemp, path);
        char onlyDirName[NUM_CHARS];
        extractDirName(dirTemp, onlyDirName);


        if(result == 1){
            strcat(onlyDirName, ",100,GREAT_JOB\n");
            write(out, onlyDirName, strlen(onlyDirName));
        } else if (result == 2){
            strcat(onlyDirName, ",60,BAD_OUTPUT\n");
            write(out, onlyDirName, strlen(onlyDirName));
        } else if (result == 3){
            strcat(onlyDirName, ",80,SIMILAR_OUTPUT\n");
            write(out, onlyDirName, strlen(onlyDirName));
        }
        //its the child
    } else {
        int ret_exe = execve("comp.out", send, NULL);

        if (ret_exe == -1) {
            write(2, ERROR_TYPE, ERROR_LEN);
            exit(-1);
        }
    }
}