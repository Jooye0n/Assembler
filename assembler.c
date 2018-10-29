#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*******************************************************
 * Function Declaration
 *
 *******************************************************/
char *change_file_ext(char *str);
char *dec_to_bin(int k, int n); //자리수, 숫자
int hex_to_dec(char* hex);
char *bin_to_hex(char* bin);
int find_op(char* inst);
void Eliminate(char *str, char ch);


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

opcode opcode_list[21] = {
        {"addiu","001001",'i'},{"addu","000000",'r',"100001"},{"and","000000",'r',"100100"},{"andi","001100",'i'},{"beq","000100",'i'},{"bne","000101",'i'},{"j","000010",'j'},
        {"jal","000011",'j'},{"jr","000000",'r',"001000"},{"lui","001111",'i'},{"lw","100011",'i'},/*{"la","111111",'s',"001101"},*/{"nor","000000",'r',"100111"},{"or","000000",'r',"100101"},
        {"ori","001101",'i'},{"sltiu","001011",'i'},{"sltu","000000",'r',"101011"},{"sll","000000",'r',"000000"},{"srl","000000",'r',"000010"},{"sw","101011",'i'},{"subu","000000",'r',"100011"}
};
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

   

    int reg[32] = {0, };
    unsigned int text_pointer = hex_to_dec("0x400000");
    unsigned int pc = hex_to_dec("0x400000");
    unsigned int data_pointer = hex_to_dec("0x10000000");
    bool dataSection = false;
    bool textSection = false;
    int row=0;
    char ISA[32][4][15]; // 줄당, 커맨드 개수, 커맨드 담는거
    char buf[128];
    char buf2[128];

    char label[10][10][10];
    char data[10][10][10]; //data1 : 100 --> data[1][1] = 100, data[1][2] = 10000000
    int index_data = 1; //data는 1부터 시작, ISA는 0부터 시작
    int index_label = 1;
    int whereTextstarts;

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
            else{
                text_pointer+=4;
            }
            // find instruction
        }



        row++;   // DON'T MOVE, KEEP VERY BELOW
    }

    textSection=false;
    int op_index;
    // printf("%s ", ISA[4][1]);

    // printf("%d", atoi(ISA[4][2]+1));


    for(int i=0; i<32; i++){     //second pass
        if(i==whereTextstarts){ 
            textSection=true;
            continue;
        }        
        // for(int j=0; j<4; j++){
        if(textSection){
            if(strchr(ISA[i][0], ':')==NULL){ //레이블아닐때
                op_index=find_op(ISA[i][0]);
                if(opcode_list[op_index].type=='r'){ //r type 일 때
                    if(strcmp(ISA[i][0], "sll")!=0 && strcmp(ISA[i][0], "srl")!=0){  //r type에서도 srl, sll 아닐때
                        fputs(ISA[i][0], output);
                        fputs(": ", output);
                        fputs(opcode_list[op_index].code, output); //op
                        fputs(dec_to_bin(5,atoi(ISA[i][2]+1)), output); //rs
                        fputs(dec_to_bin(5,atoi(ISA[i][3]+1)), output); //rt
                        fputs(dec_to_bin(5,atoi(ISA[i][1]+1)), output); //rd
                        fputs(dec_to_bin(5,0), output); //shamt
                        fputs(opcode_list[op_index].funct, output); //op

                        fputs("\n", output);
                    }
                    else{
                        fputs(ISA[i][0], output);
                        fputs(": ", output);
                        fputs(opcode_list[op_index].code, output); //op
                        fputs(dec_to_bin(5,0), output); //rs
                        fputs(dec_to_bin(5,atoi(ISA[i][2]+1)), output); //rt
                        fputs(dec_to_bin(5,atoi(ISA[i][1]+1)), output); //rd
                        fputs(dec_to_bin(5,atoi(ISA[i][3])), output); //shamt
                        fputs(opcode_list[op_index].funct, output); //op
                        fputs("\n", output);
                    }
                        // fputs(atoi(ISA[i][2]+1), output);
                        // fputs(atoi(ISA[i][3]+1), output);
                        //fputs(opcode_list[op_index].code, output);
                    }
                }   
            }
        // }
        printf("\n");
    }
    // fputs(opcode_list[0].code, output);
    // printf("%s\n", data[3][2]);
    // printf("%d\n", data_pointer);
    // printf("%s\n", dec_to_bin(32, atoi(data[3][2])));
    // printf("%s\n", dec_to_bin(32, 268435464));
    // printf("%s\n", dec_to_bin(32, data_pointer));
    

    // unsigned int datanum=0;       //둘째 output을 위해 남겨놓음
    // datanum=data_pointer-hex_to_dec("0x10000000");
    // unsigned int textnum=0;        // 첫 output을 위해 남겨놓음
    // textnum=text_pointer-hex_to_dec("0x400000");
    // printf("%u\n", datanum);



    // printf("%s\n", dec_to_bin(10,15));              
    // printf("%s\n", dec_to_bin(10,hex_to_dec("f")));
    // printf("%s\n",bin_to_hex("1101"));






    // for(int i=0; i<5; i++){
    //     if(strcmp(ISA[1][i], "")==0){
    //         printf("%d\n", i);                   measure each size
    //         break;
    //     }
    // }

    // char buf3[5]="abcde";

    // fputs(buf3, output);
    // fputs(buf3, output);

    

    

    







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
*dec_to_bin(int k, int n)
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

char *bin_to_hex(char* bin){
    int value = (int)strtol(bin, NULL, 2);
    char hexString[12];
    sprintf(hexString, "%x", value);
    char *hex;
    strcpy(hex, hexString);

    return hex;
}

int find_op(char* inst){
    for(int i=0; i<21; i++){
        if(strcmp(inst, opcode_list[i].name)==0)
            return i;
    }
}

void Eliminate(char *str, char ch)
{
    for (; *str != '\0'; str++)//종료 문자를 만날 때까지 반복
    {
        if (*str == ch)//ch와 같은 문자일 때
        {
            strcpy(str, str + 1);
            str--;
        }
    }
}