#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*******************************************************
 * Function Declaration
 *
 *******************************************************/
char *change_file_ext(char *str);
char *dec_to_bin(int n);
char *dec_to_hex(int n_bit, int dec);

/*******************************************************
 * Function: main
 *
 * Parameters:
 *  int
 *      argc: the number of argument
 *  char*
 *      argv[]: array of a sting argument
 *
 * Return:
 *  return success exit value
 *
 * Info:
 *  The typical main function in C language.
 *  It reads system arguments from terminal (or commands)
 *  and parse an assembly file(*.s).
 *  Then, it converts a certain instruction into
 *  object code which is basically binary code.
 *
 *******************************************************/

typedef struct opcode{
    char name[10];
    char* code;
    char type;
    char* funct;
}opcode;

typedef int bool;
#define true 1
#define false 0

int
main(int argc, char *argv[])
{
    FILE *input, *output;
    char *filename;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <*.s>\n", argv[0]);
        fprintf(stderr, "Example: %s sample_input/example?.s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    input = fopen(argv[1], "r");
    if (input == NULL) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }

    filename = strdup(argv[1]); // strdup() is not a standard C library but fairy used a lot.
    if(change_file_ext(filename) == NULL) {
        fprintf(stderr, "'%s' file is not an assembly file.\n", filename);
        exit(EXIT_FAILURE);
    }

    output = fopen(filename, "w");
    if (output == NULL) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }
//=========================================================

    // text starts from 0x400000
    // data starts from 0x10000000

    opcode opcode_list[21] = {
        {"addiu","001001",'i'},{"addu","000000",'r',"100001"},{"and","000000",'r',"100100"},{"andi","001100",'i'},{"beq","000100",'i'},{"bne","000110",'i'},{"j","000010",'j'},
        {"jal","000011",'j'},{"jr","000000",'j',"001000"},{"lui","111111",'i'},{"lw","100011",'i'},{"la","111111",'s',"001101"},{"nor","000000",'r',"100111"},{"or","000000",'r',"100101"},
        {"ori","001101",'i'},{"sltiu","001011",'i'},{"sltu","000000",'r',"101011"},{"sll","000000",'r',"000000"},{"srl","000000",'r',"000010"},{"sw","101011",'i'},{"subu","000000",'r',"100011"}
    };

    int reg[32] = {0, };
    unsigned int text_pointer = 400000;
    unsigned int pc = 400000;
    unsigned int data_pointer = 10000000;
    bool dataSection = false;
    bool textSection = false;
    int row=0;
    char ISA[32][5][15]; // 줄당, 커맨드 개수, 커맨드 담는거
    char buf[128];
    char buf2[128];

    char label[10][10][10];
    char data[10][10][10]; //data1 : 100 --> data[1][1] = 100, data[1][2] = 10000000
    int index_data = 1; //data는 1부터 시작, ISA는 0부터 시작
    int index_label = 1;


    while(fgets(buf, sizeof(buf), input)){  // reads 1 line per 1 loop
        char temp[128];
        strcpy(temp, buf);
        int col=0;



        char *ptr = strtok(temp, " \t\n");
        do{
            strcpy(ISA[row][col], ptr);
            col++;
        }while (ptr = strtok(NULL, " \t\n"));


       
        if(strcmp(ISA[row][0], ".data")==0){
            dataSection=true;
            continue;  //컨티뉴하면 ISA가 안넣음 --> 안넣어도된다고 가정
        }
        if(strcmp(ISA[row][0], ".text")==0){
            textSection=true;
            dataSection=false;
            continue;
        }
        if(dataSection){ // 데이터 섹션일때 @@@@@@@@@@@@@ 아직 array형식은 지원안함
            if(strchr(ISA[row][0], ':')!=NULL){ //줄이 레이블형식일때 (array, data)
                //if(strchr(ISA[row][1], 'w')!=NULL)
                    strcpy(data[index_data][1], ISA[row][2]);
                    sprintf(buf2, "%d", data_pointer);
                    strcpy(data[index_data][2], buf2);
                    index_data++;
                    data_pointer+=4;
            }
            // else{
            //     strcpy(data[index_data][1], ISA[row][1]);
            //     sprintf(buf2, "%d", data_pointer);
            //     strcpy(data[index_data][2], buf2);
            //     index_data++;
            //     data_pointer+=4;
            // }
        }
        if(textSection){ // 텍스트섹션일때
            if(strchr(ISA[row][0], ':')!=NULL){ // 레이블일때
                strcpy(label[index_label][1], ISA[row][0]);
                sprintf(buf2, "%d", text_pointer);
                strcpy(label[index_label][2], buf2);
                index_label++;
                text_pointer+=4;
            }
            // find instruction
        }



        row++;   // DON'T MOVE, KEEP BELOWEST
    }

    unsigned int datanum=0;       //둘째 output을 위해 남겨놓음
    datanum=data_pointer-10000000;
    unsigned int textnum=0;        // 첫 output을 위해 남겨놓음
    textnum=text_pointer-400000;

    // printf("%s", label[2][1]);
    // printf(" %s", label[2][2]);

    // char *bin;
    // bin = dec_to_bin(10);
    // printf("%s", bin);



    // for(int i=0; i<5; i++){
    //     if(strcmp(ISA[1][i], "")==0){
    //         printf("%d\n", i);                   measure each size
    //         break;
    //     }
    // }

    char buf3[5]="abcde";

    fputs(buf3, output);

    

    

    


























//=========================================================
    fclose(input);
    fclose(output);
    exit(EXIT_SUCCESS);
}


/*******************************************************
 * Function: change_file_ext
 *
 * Parameters:
 *  char
 *      *str: a raw filename (without path)
 *
 * Return:
 *  return NULL if a file is not an assembly file.
 *  return empty string
 *
 * Info:
 *  This function reads filename and converst it into
 *  object extention name, *.o
 *
 *******************************************************/
char
*change_file_ext(char *str)
{
    char *dot = strrchr(str, '.');

    if (!dot || dot == str || (strcmp(dot, ".s") != 0)) {
        return NULL;
    }

    str[strlen(str) - 1] = 'o';
    return "";
}

char 
*dec_to_bin(int n)
{
    int c, d, count;
    char *pointer;
    
    count = 0;
    pointer = (char*)malloc(32+1);
    
    if(pointer == NULL)
        exit(EXIT_FAILURE);
    
    for(c = 31; c >= 0; c--){
        d = n >> c;
        
        if(d & 1)
            *(pointer+count) = 1 + '0';
        else
            *(pointer+count) = 0 + '0';

        count++;
    }
    *(pointer+count) = '\0';
    
    return pointer;
}
