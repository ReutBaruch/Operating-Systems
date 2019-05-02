//REut Baruch
//205467186

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_OF_CORRECT_ARG 3
#define EQUALS 1
#define SAME 3
#define DIFFERENT 2


int main(int argc, char** argv) {

    if (argc != NUM_OF_CORRECT_ARG){
        perror("incorrect number of argumants/n");
        return -1;
    }

    char buffer1[sizeof(char)];
    char buffer2[sizeof(char)];

    bool equalFlag = true;
    bool sameFlag = true;

    int file1 = open(argv[1],  O_RDONLY);
    int file2 = open(argv[2],  O_RDONLY);

    //opened both files successfully
    if ((file1 != -1) && (file2 != -1)){

        //read a char from both files
        size_t size1 = read(file1, buffer1, sizeof(buffer1));
        size_t size2 = read(file2, buffer2, sizeof(buffer2));

        //as long as its not the end of one of the files
        while ((size1 > 0) || (size2 > 0)){

            if (*buffer1 == *buffer2){
                //read next char
                size1 = read(file1, buffer1, sizeof(buffer1));
                size2 = read(file2, buffer2, sizeof(buffer2));

            } else if (*buffer1 != *buffer2){
                //it means that they are not equals
                equalFlag = false;

                //skip enter and space
                if ((*buffer1 == '\n') || (*buffer1 == ' ')){
                    size1 = read(file1, buffer1, sizeof(buffer1));
                } else if ((*buffer2 == '\n') || (*buffer2 == ' ')){
                    size2 = read(file2, buffer2, sizeof(buffer2));
                    //check if its capital letter deference
                } else if (*buffer1 > *buffer2){
                    if (*buffer1 == (*buffer2 + 32)){
                        size1 = read(file1, buffer1, sizeof(buffer1));
                        size2 = read(file2, buffer2, sizeof(buffer2));
                    } else {
                        //if not - they are different and stop
                        sameFlag = false;
                        break;
                    }
                    //check if its capital letter deference
                } else if (*buffer1 < *buffer2) {
                    if ((*buffer1 + 32) == *buffer2) {
                        size1 = read(file1, buffer1, sizeof(buffer1));
                        size2 = read(file2, buffer2, sizeof(buffer2));
                    } else {
                        //if not - they are different and stop
                        sameFlag = false;
                        break;
                    }
                    //any other case - they are different
                } else {
                    sameFlag = false;
                }
            }
        }

        close(file1);
        close(file2);

        if (equalFlag){
        //    printf("equals!\n");
            return EQUALS;
        } else if (sameFlag){
        //    printf("same!\n");
            return SAME;
        } else {
        //    printf("different!\n");
            return DIFFERENT;
        }
    }
}
