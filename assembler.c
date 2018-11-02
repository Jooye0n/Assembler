#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

/*******************************************************
 * Function Declaration
 *
 *******************************************************/
char *change_file_ext(char *str);
char *dec_to_bin(int k, int n); //자리수, 숫자
int hex_to_dec(char *hex);
int find_op(char *inst);
int find_dataLabel(char *data);
int find_textLabel(char *label);
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
    char* name;
    char* code;
    char type;
    char* funct;
}opcode;

typedef struct data{
    char label[10];
    char value[20];
    int address; // recorded in decimal
}data;

typedef struct label{
    char name[10];
    int address; // recorded in decimal
}labels;

typedef int bool;
#define true 1
#define false 0

opcode opcode_list[21] = {
        {"addiu","001001",'i'},{"addu","000000",'r',"100001"},{"and","000000",'r',"100100"},{"andi","001100",'i'},{"beq","000100",'i'},{"bne","000101",'i'},{"j","000010",'j'},
        {"jal","000011",'j'},{"jr","000000",'r',"001000"},{"lui","001111",'i'},{"lw","100011",'i'},{"la","111111",'i',"001101"},{"nor","000000",'r',"100111"},{"or","000000",'r',"100101"},
        {"ori","001101",'i'},{"sltiu","001011",'i'},{"sltu","000000",'r',"101011"},{"sll","000000",'r',"000000"},{"srl","000000",'r',"000010"},{"sw","101011",'i'},{"subu","000000",'r',"100011"}
};
data data_list[6];
labels label_list[10];

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

    unsigned int text_pointer = hex_to_dec("0x400000");
    unsigned int data_pointer = hex_to_dec("0x10000000");
    bool dataSection = false;
    bool textSection = false;
    int row=0;
    char ISA[32][5][15]; // 줄당, 커맨드 개수, 커맨드 담는거
    char buf[128];
    char buf2[128];

    int index_data = 0; 
    int index_label = 0; 
    int whereTextstarts;
    int text_count=0;               // 명령어 개수 나중에 처음에 출력해야함

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
            whereTextstarts=row;
            continue;
        }
        if(dataSection){ // 
            if(strchr(ISA[row][0], ':')!=NULL){ //줄이 레이블형식일때 
                strcpy(data_list[index_data].label, ISA[row][0]);
                data_list[index_data].address = data_pointer;
                strcpy(data_list[index_data].value, ISA[row][2]);
                index_data++;
                data_pointer+=4;                    //나중에 출력해야함
            }
            else{ //딸려오는 인덱스일때
                strcpy(data_list[index_data].label, data_list[index_data-1].label);
                data_list[index_data].address = data_pointer;
                strcpy(data_list[index_data].value, ISA[row][1]);
                index_data++;
                data_pointer+=4;
            }
        }
        if(textSection){ // 텍스트섹션일때
            if(strchr(ISA[row][0], ':')!=NULL){ // 레이블일때
                strcpy(label_list[index_label].name, ISA[row][0]);
                label_list[index_label].address = text_pointer;
                index_label++;
            }
            else{
                sprintf(buf2, "%d", text_pointer);
                strcpy(ISA[row][4], buf2);
                text_pointer+=4;    
                text_count++;
                if(strcmp(ISA[row][0], "la")==0 && strcmp(ISA[row][2],"data1")!=0){
                    text_count++;
                    text_pointer+=4;
                }
            }
        }
        
        row++;   // DON'T MOVE, KEEP VERY BELOW
    }
    
    // for(int i=0;i<row; i++){
    //     for(int j=0; j<5; j++){
    //         printf("ISA[%d][%d]: %s ", i, j, ISA[i][j]);
    //     }
    //     printf("\n");
    // }

    textSection=false;
    int op_index;
    int foundData;
    fputs(dec_to_bin(32, text_count*4), output);   // 첫 두줄
    // fputs("\n", output);
    fputs(dec_to_bin(32, index_data*4), output);
    // fputs("\n", output);

    for(int i=0; i<row; i++){     //second pass
        if(i==whereTextstarts){ 
            textSection=true;
            continue;
        }   
        if(textSection){
            if(strchr(ISA[i][0], ':')==NULL){ //레이블아닐때
                op_index=find_op(ISA[i][0]);
                if(opcode_list[op_index].type=='r'){ //r type 일 때
                    if(strcmp(ISA[i][0], "sll")!=0 && strcmp(ISA[i][0], "srl")!=0 && strcmp(ISA[i][0], "jr")!=0){  //r type에서 jr ,srl, sll 아닐때

                        // fputs(ISA[i][0], output);
                        // fputs(": ", output);

                        fputs(opcode_list[op_index].code, output); //op
                        fputs(dec_to_bin(5,atoi(ISA[i][2]+1)), output); //rs
                        fputs(dec_to_bin(5,atoi(ISA[i][3]+1)), output); //rt
                        fputs(dec_to_bin(5,atoi(ISA[i][1]+1)), output); //rd
                        fputs(dec_to_bin(5,0), output); //shamt
                        fputs(opcode_list[op_index].funct, output); //funct

                        // fputs(" its r type!\n", output);
                        // fputs("\n", output);
                    }
                    else if(strcmp(ISA[i][0], "jr")==0){ //jr 일때

                        // fputs(ISA[i][0], output);
                        // fputs(": ", output);

                        fputs(opcode_list[op_index].code, output); //op
                        fputs(dec_to_bin(5,atoi(ISA[i][1]+1)), output); //rs
                        fputs(dec_to_bin(5,0), output); //rt 
                        fputs(dec_to_bin(5,0), output); //rd
                        fputs(dec_to_bin(5,0), output); //shmat
                        fputs(opcode_list[op_index].funct, output); //funct

                        // fputs(" its r type!\n", output);
                        // fputs("\n", output);

                    }
                    else{ // shift 일때

                        // fputs(ISA[i][0], output);
                        // fputs(": ", output);

                        fputs(opcode_list[op_index].code, output); //op
                        fputs(dec_to_bin(5,0), output); //rs
                        fputs(dec_to_bin(5,atoi(ISA[i][2]+1)), output); //rt
                        fputs(dec_to_bin(5,atoi(ISA[i][1]+1)), output); //rd
                        fputs(dec_to_bin(5,atoi(ISA[i][3])), output); //shamt
                        fputs(opcode_list[op_index].funct, output); //funct

                        // fputs(" its r type!\n", output);
                        // fputs("\n", output);
                    }
                }
                else if(opcode_list[op_index].type=='i'){ //i type 일 때
                    if(strcmp(ISA[i][0], "la")==0){         // special case1: la 일 때 
                        if(strcmp(ISA[i][2], "data1")==0){
                            //do lui only
                            // fputs("lui", output);
                            // fputs(": ", output);

                            op_index=find_op("lui");
                            fputs(opcode_list[op_index].code, output); //op
                            fputs(dec_to_bin(5,0), output); // rs
                            fputs(dec_to_bin(5, atoi(ISA[i][1]+1)), output); //rt
                            fputs(dec_to_bin(16, (data_list[0].address)/65536), output); //addr

                            // fputs(" its i type!\n", output);
                            // fputs("\n", output);
                        }
                        else{
                            //do lui 
                            // fputs("lui", output);
                            // fputs(": ", output);

                            op_index=find_op("lui");
                            fputs(opcode_list[op_index].code, output); //op
                            fputs(dec_to_bin(5,0), output); // rs
                            fputs(dec_to_bin(5, atoi(ISA[i][1]+1)), output); //rt
                            foundData=find_dataLabel(ISA[i][2]);
                            fputs(dec_to_bin(16, (data_list[foundData].address)/65536), output); //addr

                            // fputs(" its i type!\n", output);
                            // fputs("\n", output);

                            //do ori

                            // fputs("ori", output);
                            // fputs(": ", output);

                            op_index=find_op("ori");
                            fputs(opcode_list[op_index].code, output); //op
                            fputs(dec_to_bin(5, atoi(ISA[i][1]+1)), output); //rs
                            fputs(dec_to_bin(5, atoi(ISA[i][1]+1)), output); //rt
                            fputs(dec_to_bin(16, (data_list[foundData].address)-hex_to_dec("0x10000000")), output); //addr

                            // fputs(" its i type!\n", output);
                            // fputs("\n", output);
                        }
                    }
                    else if(strcmp(ISA[i][0], "bne")==0 || strcmp(ISA[i][0], "beq")==0){ // special case2: branch 일 때

                        // fputs(ISA[i][0], output);
                        // fputs(": ", output);

                        int offset=0;
                        op_index=find_op(ISA[i][0]);
                        fputs(opcode_list[op_index].code, output); //op
                        fputs(dec_to_bin(5, atoi(ISA[i][1]+1)), output); //rs
                        fputs(dec_to_bin(5, atoi(ISA[i][2]+1)), output); //rt
                        // pc addressing
                        foundData=find_textLabel(ISA[i][3]);
                        offset=(label_list[foundData].address-(atoi(ISA[i][4])+4))/4;
                        fputs(dec_to_bin(16, offset), output); //addr
                        // fputs(" its i type!\n", output);
                        // fputs("\n", output);
                    }
                    else if(strcmp(ISA[i][0], "lui")==0){ // special case3: lui 일 때

                        // fputs("lui", output);
                        // fputs(": ", output);

                        op_index=find_op("lui");
                        fputs(opcode_list[op_index].code, output); //op
                        fputs(dec_to_bin(5,0), output); // rs
                        fputs(dec_to_bin(5, atoi(ISA[i][1]+1)), output); //rt
                        if(strchr(ISA[i][2], 'x')!=NULL) // 16진수 주소값일때 (16진수일때) char 타입    
                            fputs(dec_to_bin(16, hex_to_dec(ISA[i][2])), output);
                        else //imm 값일 때
                            fputs(dec_to_bin(16, atoi(ISA[i][2])), output);

                        // fputs(" its i type!\n", output);
                        // fputs("\n", output);
                    }
                    else if(strcmp(ISA[i][0], "lw")==0 || strcmp(ISA[i][0], "sw")==0){

                        // fputs(ISA[i][0], output);
                        // fputs(": ", output);

                        op_index=find_op(ISA[i][0]);
                        fputs(opcode_list[op_index].code, output); //op

                        char tempo[10];
                        int in=0;
                        char *ptr3 = strtok(ISA[i][2], "(");
                        do{
                            strcpy(tempo, ptr3);
                            in++;
                        }while (ptr3 = strtok(NULL, "("));
                        fputs(dec_to_bin(5, atoi(tempo+1)), output); //rs
                        fputs(dec_to_bin(5, atoi(ISA[i][1]+1)), output); //rt
                        fputs(dec_to_bin(16, atoi(ISA[i][2])), output); //addr

                        // fputs(" its i type!\n", output);
                        // fputs("\n", output);
                    }
                    else{                   //그 외 i type 일 때

                        // fputs(ISA[i][0], output);
                        // fputs(": ", output);

                        op_index=find_op(ISA[i][0]);
                        fputs(opcode_list[op_index].code, output); //op
                        fputs(dec_to_bin(5, atoi(ISA[i][2]+1)), output); //rs
                        fputs(dec_to_bin(5, atoi(ISA[i][1]+1)), output); //rt

                        if(strchr(ISA[i][3], 'x')!=NULL) // 주소값일때 (16진수일때) char 타입    
                            fputs(dec_to_bin(16, hex_to_dec(ISA[i][3])), output);
                        else //imm 값일 때
                            fputs(dec_to_bin(16, atoi(ISA[i][3])), output);

                        // fputs(" its i type!\n", output);
                        // fputs("\n", output);
                    }
                }
                else if(opcode_list[op_index].type=='j'){

                    // fputs(ISA[i][0], output);
                    // fputs(": ", output);

                    op_index=find_op(ISA[i][0]); 
                    fputs(opcode_list[op_index].code, output); //op
                    //direct jump addressing
                    foundData=find_textLabel(ISA[i][1]);
                    fputs(dec_to_bin(26, (label_list[foundData].address)/4), output);

                    // fputs(" its j type!\n", output);
                    // fputs("\n", output);
                }
            }   
        }
    }

    for(int i=0; i<index_data; i++){
        if(strchr(data_list[i].value, 'x')!=NULL){ // 주소값일때 (16진수일때) char 타입
            fputs(dec_to_bin(32, hex_to_dec(data_list[i].value)), output);

            // fputs("\n", output);
        }
        else{ // 정수값일때 그래도 여전히 char 타입
            fputs(dec_to_bin(32, atoi(data_list[i].value)), output);

            // fputs("\n", output);
        }
    }

    fputs("\n", output);

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

char *dec_to_bin(int k, int n)
{
    int c, d, count;
    char *pointer;
    
    count = 0;
    pointer = (char*)malloc(32+1);
    
    if(pointer == NULL)
        exit(EXIT_FAILURE);
    
    for(c = k-1; c >= 0; c--){
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

int hex_to_dec(char* hex){
    int decimal = 0;

    int position = 0;
    for (int i = strlen(hex) - 1; i >= 0; i--)
    {
        char ch = hex[i];
        if (ch >= 48 && ch <= 57)
        {
            decimal += (ch - 48) * pow(16, position);
        }
        else if (ch >= 65 && ch <= 70){
            decimal += (ch - (65 - 10)) * pow(16, position);
        }
        else if (ch >= 97 && ch <= 102)
        {
            decimal += (ch - (97 - 10)) * pow(16, position);
        }

        position++;
    }

    return decimal;

}

int find_op(char* inst){
    for(int i=0; i<21; i++){
        if(strcmp(inst, opcode_list[i].name)==0)
            return i;
    }
    printf("couldn't find_op\n");
    return -1;
}

int find_dataLabel(char* data){
    strcat(data, ":");
    for(int i=0; i<6; i++){
        if(strcmp(data, data_list[i].label)==0)
            return i;
    }
    printf("couldn't find_dataLabel\n");
    return -1;
}

int find_textLabel(char *label){
    strcat(label, ":");
    for(int i=0; i<9; i++){
        if(strcmp(label, label_list[i].name)==0)
            return i;
    }
    printf("couldn't find_textLabel\n");
    return -1;
}
