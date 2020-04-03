#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#define BITSIZE 0x10
#define OFFSET 0x100
#define STARTDATA 0x10000000
#define STARTTEXT 0x00400000

/*******************************************************
 * Function Declaration
 *
 *******************************************************/
char *change_file_ext(char *str);
int decimal_to_biinary(int decimal);
int value_to_int(char num[]);
void change_to_binary();
void read_instruction(FILE* input);
void read_assemble(FILE* input);
void read_text(FILE* input);
void write_obj_file(FILE* output);

struct Instruction { //¡§«ÿ¡Æ ¿÷¿∏π«∑Œ πŸ≤Ó¡ˆ æ ¥¬ ∞ÕµÈ
    char name[BITSIZE] = { '\0', };
    char opcode[BITSIZE] = { '\0', };
    char format[BITSIZE] = { '\0', };
    char funct[BITSIZE] = { '\0', };
};

struct Instruction inst[20] = { //≥™¡ﬂø° 22 ª˝∑´«ÿ∫∏±‚
    { "addiu", "001001", 'I',"" }, //0
    { "addu", "000000", 'R',"100001" }, //1
    { "and", "000000", 'R',"100100" },//2
    { "andi", "001100", 'I', "" },//3
    { "beq","000100", 'I', "" },//4
    { "bne", "000101", 'I', "" },//5
    { "j", "000010", 'J', "" },//6
    { "jal", "000011", 'J', "" },//7
    { "jr", "000000", 'R',"001000" },//8
    { "lui", "001111", 'I', "" },//9
    { "lw", "100011", 'I', "" },//10
    { "nor", "000000", 'R' , "100111" },//11
    { "or", "000000", 'R', "100101" },//12
    { "ori", "001101", 'I', "" },//13
    { "sltiu", "001011",'I' ,"" },//14
    { "sltu", "000000", 'R', "101011" },//15
    { "sll", "000000", 'R' ,"000000" },//16
    { "srl", "000000", 'R', "000010" },//17
    { "sw", "101011", 'I', "" },//18
    { "subu", "000000", 'R', "100011" }//19
};

struct Data {
    char label[BITSIZE] = { '\0', };
    char value[BITSIZE] = { '\0', };
    unsigned int address = 0;//Ω«¡¶ ∏ﬁ∏∏Æ ¡÷º“
}__attribute__((packed));

struct Text { //instruction ¡ﬂ ¡§«ÿ¡Æ¿÷¡ˆ æ ¿∫ ∞ÕµÈ
    int inst_idx = 0;//∏Óπ¯¬∞ inst¿Œ¡ˆ
    int rs = 0;
    int rt = 0;
    int rd = 0;
    int shamt = 0;
    int immediate = 0;
    int j_address = 0;//≥™¡ﬂø° ¥Ÿ ¿–∞Ì ≥≠ »ƒimme∞˙ addΩœ ∏æ∆º≠ ¿˙¿Â«ÿµ– ¡÷º“∑Œ πŸ≤„¡÷¿⁄
    unsigned int address = 0;//Ω«¡¶ ∏ﬁ∏∏Æ ¡÷º“

}__attribute__((packed));

struct Label {
    char name[BITSIZE] = { '\0', };
    unsigned int address = 0;//Ω«¡¶ ∏ﬁ∏∏Æ ¡÷º“
}__attribute__((packed));

struct Label label[OFFSET] = { '\0', };//√π π¯¬∞ ¿–¿ª ∂ß ∏µÁ ∂Û∫ß ∫∞ ¡÷º“ ¿˙¿Â
struct Text text[OFFSET] = { '\0', };//µŒ π¯¬∞ ¿–¿ª ∂ß text instruction¿–æÓº≠ ¿¸∫Œ º˝¿⁄∑Œ
struct Data data[OFFSET] = { '\0', };//data

int datasize = 0;
int textsize = 0;

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


int main(int argc, char *argv[])
{
    FILE *input, *output;
    char *filename;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <*.s>\n", argv[0]);//0π¯¿∫ assembler 1π¯¿∫ .s∆ƒ¿œ
        fprintf(stderr, "Example: %s sample_input/example?.s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    input = fopen(argv[1], "r");//.s ∆ƒ¿œø≠±‚
    if (input == NULL) {//.s ∆ƒ¿œæ¯¥¬∞ÊøÏ øπø‹√≥∏Æ
        perror("ERROR");
        exit(EXIT_FAILURE);
    }

    filename = strdup(argv[1]); //argv[1]∏¶ filename¿∏∑Œ ∫π¡¶ »ƒ ªÁ∫ª ¿Œº‚
    if (change_file_ext(filename) == NULL) {//.s ∆ƒ¿œ¿Ã æ¯¥¬ ∞ÊøÏ øπø‹√≥∏Æ
        fprintf(stderr, "'%s' file is not an assembly file.\n", filename);
        exit(EXIT_FAILURE);
    }

    output = fopen(filename, "w");//fliename.o∏¶ æ¥¥Ÿ.
    if (output == NULL) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }

    // process();
    read_assemble(input);
    change_to_binary();
    write_obj_file(output);


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
char *change_file_ext(char *str)//∆ƒ¿œ ¿Ã∏ß¿ªπﬁ¿∏∏È Ω««‡∆ƒ¿œ∑Œ ∫Ø∞Ê«œø© return ""¡ÿ¥Ÿ.
{
    char *dot = strrchr(str, '.');

    if (!dot || dot == str || (strcmp(dot, ".s") != 0)) {
        return NULL;
    }

    str[strlen(str) - 1] = 'o';
    return "";
}

int decimal_to_biinary(int decimal) {//10->2¡¯ºˆ∑Œ ∫Ø»Ø
    long binary, sum = 0, i = 1;

    while (decimal>0)
    {
        binary = decimal % 2;
        sum += binary * i;
        decimal = decimal / 2;
        i *= 10;
    }
    return sum;
}

int value_to_int(char num[]) {//$10, -> 10(int)
    char result[30] = { '\0', };
    int j = 0;
    int i;
    for (i = 0; num[i] != '\0'; i++) {
        if (isdigit(num[i])) {
            result[j++] = num[i];
        }
    }

    return atoi(result);
}

void read_text(FILE* input) {
    char text_temp[BITSIZE] = { '\0', };//16
    int text_address = STARTTEXT;
    int label_count = 0;

    while (!feof(input)) {//.text∫Œ≈Õ ∆ƒ¿œ¿« ≥°±Ó¡ˆ ¿–±‚

        fscanf(input, "%s", &text_temp);//.text¥Ÿ¿Ω ¿–¿Ω

        int text_label_size = strlen(text_temp);// πÆ¿⁄ø≠ «—∞≥¿« ≈©±‚ = string_size

        if (strcmp(text_temp[text_label_size - 1], ":") == 0) {//∫Øºˆ ∏Ì ∏∂¡ˆ∏∑¿Ã : .∂Û∫ß
                                                               //label[label_count].name = text_temp;//∂Û∫ß ¿Ã∏ß∫∞∑Œ
            memcpy(label[label_count].name, text_temp, text_label_size - 1);
            label[label_count].address = text_address;//¡÷º“¿˙¿Â
            label_count++;
        }
        else {//∂Û∫ß æ∆¥œ∏È,
            int i;
            for (i = 0; i < 22; i++) //instºˆ
                if (strcmp(text_temp, inst[i].name) == 0) {//instruction name¿œ∂ß∏∏
                    text_address += 4;//¡÷º“∞ËªÍ
                }

        }
    }
}

void read_instruction(FILE* input) {//¥ŸΩ√ ¿–¿∏∏Èº≠
    char text_temp[BITSIZE] = { '\0', };

    while (!feof(input)) {//.text∫Œ≈Õ ∆ƒ¿œ¿« ≥°±Ó¡ˆ ¿–±‚

        fscanf(input, "%s", &text_temp);//.text¥Ÿ¿Ω ¿–¿Ω

        int text_label_size = strlen(text_temp);// πÆ¿⁄ø≠ «—∞≥¿« ≈©±‚ = string_size

        if (strcmp(text_temp[text_label_size - 1], ":") == 0) {//∫Øºˆ ∏Ì ∏∂¡ˆ∏∑¿Ã : ∂Û∫ß
            continue;

        }
        else {//∂Û∫ß æ∆¥œ∏È,
            int current_address = STARTTEXT;
            int idx;
            int text_count = 0;
            char temp_1[30] = { '\0', };
            char temp_2[30] = { '\0', };
            char temp_3[30] = { '\0', };
            char temp_4[30] = { '\0', };

            for (idx = 0; idx < 20; idx++) {//instºˆ
                if (strcmp(text_temp, inst[idx].name) == 0) {//instruction structø° ¿÷¥¬∞ÊøÏ

                    text[text_count].inst_idx = idx;

                    switch (inst[idx].format) {
                    case 'R':

                        if (inst[idx] == 16 || inst[idx] == 17) {//shift
                            fscanf(input, "%s", &temp_1);//&text[text_count].rd);
                            fscanf(input, "%s", &temp_2);// &text[text_count].rt);
                            fscanf(input, "%d", &text[text_count].shamt);

                            text[text_count].rd = value_to_int(temp_1);
                            text[text_count].rt = value_to_int(temp_2);
                        }
                        else if (inst[idx] == 8) {//jr
                            fscanf(input, "%s", &temp_1);

                            text[text_count].rs = value_to_int(temp_1);
                        }
                        else {//not shift

                            fscanf(input, "%s", &temp_1);
                            fscanf(input, "%s", &temp_2);
                            fscanf(input, "%s", &temp_3);

                            text[text_count].rd = value_to_int(temp_1);
                            text[text_count].rs = value_to_int(temp_2);
                            text[text_count].rt = value_to_int(temp_3);


                        }

                        break;



                    case 'I':
                        if (inst[idx] == 9) { //lui¿œ∂ß
                            fscanf(input, "%s", &temp_1);
                            text[text_count].rt = value_to_int(temp_1);

                            fscanf(input, "%d", &text[text_count].immediate);//16¡¯ºˆ µÈæÓ∞°¿÷¥Ÿ
                        }
                        else if (inst[idx] == 4 || inst[idx] == 5) {//bne , beq
                            char label_temp[BITSIZE] = { '\0', };
                            fscanf(input, "%s", &temp_1);
                            fscanf(input, "%s", &temp_2);

                            text[text_count].rs = value_to_int(temp_1);
                            text[text_count].rt = value_to_int(temp_2);

                            fscanf(input, "%s", &label_temp);//labelπﬁæ∆øÕº≠
                            int j;
                            for (j = 0; label[j].name[0] != '\0'; j++) {//label∫∞ ¡÷º“ √£æ∆¡‡æﬂ«—¥Ÿ.
                                if (strcmp(label[j].name, label_temp) == 0) {//∂Û∫ß √£¿∏∏È
                                    text[text_count].immediate = label[j].address - (current_address + 4) / 4//xƒ≠ ¿Ãµø//10¡¯ºˆ µÈæÓ∞°¿÷¥Ÿ
                                }

                            }
                        }
                        else if (inst[idx] == 10 || inst[idx] == 18) {//lw, sw
                            fscanf(input, "%s", &temp_1);
                            text[text_count].rt = value_to_int(temp_1);

                            fscanf(input, "%d(%s", &text[text_count].immediate, &temp_1);//4($3) : 3¿Ã rs 4¥¬ imm
                            text[text_count].rs = value_to_int(temp_1);//∞Ê∞Ì
                        }
                        else {//≥™∏”¡ˆ
                            fscanf(input, "%s,", &temp_1);
                            fscanf(input, "%s", &temp_2);

                            text[text_count].rt = value_to_int(temp_1);
                            text[text_count].rs = value_to_int(temp_2);
                            fscanf(input, "%d", &text[text_count].immediate);
                        }

                        break;



                    case 'J':
                        int j;
                        fscanf(input, "%s", &label_temp);//labelπﬁæ∆øÕº≠
                        for (j = 0; label[j].name[0] != '\0'; j++) {//label∫∞ ¡÷º“ √£æ∆¡‡æﬂ«—¥Ÿ.
                            if (strcmp(label[j].name, label_temp) == 0) {//∂Û∫ß √£¿∏∏È
                                text[text_count].j_address = label[j].address;//∂Û∫ß ¡÷º“∏¶ ≥÷æÓµ–¥Ÿ.
                                
                            }
                        }

                        break;

                        text_count++;
                        current_address += 4;
                    }
                    break;
                }
                else if (strcmp(text_temp, "la") == 0) {//la¿Œ ∞ÊøÏ
                                                        //text_count++;+2«ÿ¡‡æﬂ«—¥Ÿ.
                    //la    $8, data1¿Ã∂Û«“∂ß

                    fscanf(input, "%s", &temp_1);//$8πﬁ¿Ω
                    fscanf(input, "%s", &label_temp);//data¿« ∂Û∫ß πﬁæ∆øÕº≠

                    int j;
                    for (j = 0; data[j].label[0] != '\0'; j++) {//label∫∞ ¡÷º“ √£æ∆¡‡æﬂ«—¥Ÿ.
                        if (strcmp(data[j].label, label_temp) == 0) {//∂Û∫ß √£¿∏∏È

                            sprintf(temp_4, "%08X\n", label[j].address);//∂Û∫ß ¡÷º“ 16¡¯ºˆ∑Œ πŸ≤ŸæÓº≠ temp_4 ¿˙¿Â
                            text[text_count].inst_idx = 9;//lui
                            text[text_count].rt = value_to_int(temp_1);
                            int temp_num = temp_4[0] * 1000 + temp_4[1] * 100 + temp_4[2] * 10 + temp_4[3] * 1;
                            text[text_count].immediate = temp_num;//æ’¿« 4¿⁄∏Æ ¿˙¿Â

                            text_count++;
                            current_address += 4;


                            if (temp_4[4] != '0' || temp_4[5] != '0' || temp_4[6] != '0' || temp_4[7] != '0') {//∂Û∫ß ¡÷º“ µﬁ¿⁄∏Æ 0æ∆¥œ∏È
                                text[text_count].inst_idx = 13;
                                text[text_count].rt = value_to_int(temp_1);
                                int temp_num = temp_4[4] * 1000 + temp_4[5] * 100 + temp_4[6] * 10 + temp_4[7] * 1;
                                text[text_count].immediate = temp_num;//µ⁄¿« 4¿⁄∏Æ ¿˙¿Â

                                text_count++;
                                current_address += 4;
                            }
                        }
                    }
                }
            }

            textsize = current_address - STARTTEXT;
        }
    }
}


void read_assemble(FILE* input) {
    char data_temp[BITSIZE] = { '\0', };//16
    int data_count = 0;
    int data_address = STARTDATA;
    //bool data_label_check = false;

    while (!feof(input)) {//∆ƒ¿œ¿« ≥°±Ó¡ˆ ¿–±‚
        fscanf(input, "%s", &data_temp);//πÆ¿⁄ø≠ «—∞≥æø ¿–¿∏∏Èº≠ data_tempø° ≥÷¿Ω

        int data_label_size = strlen(data_temp);// πÆ¿⁄ø≠ «—∞≥¿« ≈©±‚ = string_size

        if (strcmp(data_temp, ".text") == 0) { //.text
            datasize = data_address - STARTDATA;
            FILE *point;
            point = input;//∞Ê∞Ì
            read_text(point);
            read_instruction(point);
            break;
        }
        else {//.data
            if (strcmp(data_temp, ".word") == 0 || strcmp(data_temp, ".data") == 0) { //.word or .data∏È,

                continue;
            }
            else if (strcmp(data_temp[data_label_size - 1], ":") == 0) {//∫Øºˆ ∏Ì∏∂¡ˆ∏∑¿Ã : .∂Û∫ß ¿÷¿∏∏È,
                                                                        //data[data_count].label = data_temp;//±∏¡∂√ºø° ∂Û∫ß ¿˙¿Â
                memcpy(data[data_count].label, data_temp, data_label_size - 1);//data_label_check == true
            }
            else {//value
                  //if (!data_label_check)//¿¸ø° ∂Û∫ß¿Ã æ¯¿∏∏È
                  //    data[data_count].label = "";

                data[data_count].value = data_temp;//±∏¡∂√ºø° value¿˙¿Â
                data[data_count].address = data_address;//±∏¡∂√ºø° ¡÷º“ ¿˙¿Â

                data_count++;//±∏¡∂√º idx «œ≥™ ¡ı∞°Ω√≈≤¥Ÿ.
                data_address += 0x00000004;//¡÷º“∞™ ¡ı∞°Ω√≈≤¥Ÿ.
                                           //data_label_check = false;
            }
        }
    }
}

void change_to_binary(FILE* output) {//¿⁄∏Æºˆ ∏¬√Áº≠ 0µµ √ﬂ∞°«ÿæﬂ «—¥Ÿ + √‚∑¬«“ ∫Œ∫– binary∑Œ πŸ≤„¡‡æﬂ «—¥Ÿ.
    fprintf(output, "%032d", decimal_to_binary(textsize));
    fprintf(output, "%032d", decimal_to_binary(datasize));
    int i;
    for (i = 0; i < textsize / 4; i++) {
        switch (inst[text[i].inst_idx].format) {
        case 'R':
            fprintf(output, "%06d", decimal_to_binary(inst[text[i].inst_idx].opcode);
            fprintf(output, "%05d", decimal_to_binary(text[i].rs);
            fprintf(output, "%05d", decimal_to_binary(text[i].rt);
            fprintf(output, "%05d", decimal_to_binary(text[i].rd);
            fprintf(output, "%05d", decimal_to_binary(text[i].shamt);
            fprintf(output, "%06d", decimal_to_binary(inst[text[i].inst_idx].funct);
            break;
        case 'I':
            fprintf(output, "%06d", decimal_to_binary(inst[text[i].inst_idx].opcode);
            fprintf(output, "%05d", decimal_to_binary(text[i].rs);
            fprintf(output, "%05d", decimal_to_binary(text[i].rt);
            fprintf(output, "%016d", decimal_to_binary(text[i].immediate);
            break;
        case 'J':
            fprintf(output, "%06d", decimal_to_binary(inst[text[i].inst_idx].opcode);
            fprintf(output, "%026d", decimal_to_binary(text[i].j_address);
            break;
        default://la
            fprintf(output, "%06d", decimal_to_binary(inst[text[i].inst_idx].opcode);
            fprintf(output, "%05d", decimal_to_binary(text[i].rs);
            fprintf(output, "%05d", decimal_to_binary(text[i].rt);
            fprintf(output, "%016d", decimal_to_binary(text[i].immediate);
            break;
        }
    }

    for (i = 0; i < datasize / 4; i++)
        fprintf(output, "%032d", decimal_to_binary(data[i].value);
}




