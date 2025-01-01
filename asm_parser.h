/*

Author: https://github.com/athenahax

Assembly parser, x86-64 instruction set
Not all instructions are included, only
the most commonly used operators : )

*/

#ifndef ASM_PARSER_H
#define ASM_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 256
#define MAX_OPCODES 1024
#define MAX_MNEMONIC_LENGTH 10

typedef enum {
    RAX = 0, RCX = 1, RDX = 2, RBX = 3,
    RSP = 4, RBP = 5, RSI = 6, RDI = 7,
    R8 = 8,  R9 = 9,  R10 = 10, R11 = 11,
    R12 = 12, R13 = 13, R14 = 14, R15 = 15
} Register;

typedef enum {
    OP_NONE,
    OP_REG,
    OP_IMM,
    OP_MEM,
    OP_REL
} OperandType;

typedef struct {
    unsigned char rex;
    unsigned char operand;
    unsigned char address;
    unsigned char segment;
} InstructionPrefix;

typedef struct {
    OperandType type;
    union {
        Register reg;
        int64_t immediate;
        struct {
            Register base;
            Register index;
            int scale;
            int32_t displacement;
        } mem;
    } value;
} Operand;

typedef struct {
    char mnemonic[MAX_MNEMONIC_LENGTH];
    unsigned char opcode;
    unsigned char opcode2;
    unsigned char is_two_byte;
    unsigned char has_modrm;
    unsigned char op_extended;
    unsigned char op_ext;
    unsigned char operand_size;
} InstructionMapping;

int get_register_encoding(const char* reg_name) {
    char reg_upper[5];
    strncpy(reg_upper, reg_name + 1, 4);
    reg_upper[4] = '\0';
    for (int i = 0; reg_upper[i]; i++) {
        reg_upper[i] = toupper(reg_upper[i]);
    }
    
    if (strcmp(reg_upper, "AX") == 0) return RAX;
    if (strcmp(reg_upper, "CX") == 0) return RCX;
    if (strcmp(reg_upper, "DX") == 0) return RDX;
    if (strcmp(reg_upper, "BX") == 0) return RBX;
    if (strcmp(reg_upper, "SP") == 0) return RSP;
    if (strcmp(reg_upper, "BP") == 0) return RBP;
    if (strcmp(reg_upper, "SI") == 0) return RSI;
    if (strcmp(reg_upper, "DI") == 0) return RDI;
    
    if (reg_upper[0] == '8') return R8;
    if (reg_upper[0] == '9') return R9;
    if (strncmp(reg_upper, "10", 2) == 0) return R10;
    if (strncmp(reg_upper, "11", 2) == 0) return R11;
    if (strncmp(reg_upper, "12", 2) == 0) return R12;
    if (strncmp(reg_upper, "13", 2) == 0) return R13;
    if (strncmp(reg_upper, "14", 2) == 0) return R14;
    if (strncmp(reg_upper, "15", 2) == 0) return R15;
    
    return -1;
}

unsigned char generate_modrm(int mod, int reg, int rm) {
    return ((mod & 0x03) << 6) | ((reg & 0x07) << 3) | (rm & 0x07);
}

unsigned char generate_rex(int w, int r, int x, int b) {
    return 0x40 | (w << 3) | (r << 2) | (x << 1) | b;
}

void init_instruction_set(InstructionMapping *instructions, int *num_instructions) {
    *num_instructions = 0;
    
    strcpy(instructions[*num_instructions].mnemonic, "MOV");
    instructions[*num_instructions].opcode = 0x89;
    instructions[*num_instructions].has_modrm = 1;
    instructions[*num_instructions].op_extended = 0;
    instructions[*num_instructions].operand_size = 8;
    (*num_instructions)++;
    
    strcpy(instructions[*num_instructions].mnemonic, "MOV");
    instructions[*num_instructions].opcode = 0xB8;
    instructions[*num_instructions].has_modrm = 0;
    instructions[*num_instructions].op_extended = 0;
    instructions[*num_instructions].operand_size = 8;
    (*num_instructions)++;
    
    strcpy(instructions[*num_instructions].mnemonic, "ADD");
    instructions[*num_instructions].opcode = 0x01;
    instructions[*num_instructions].has_modrm = 1;
    instructions[*num_instructions].op_extended = 0;
    instructions[*num_instructions].operand_size = 8;
    (*num_instructions)++;
    
    strcpy(instructions[*num_instructions].mnemonic, "SUB");
    instructions[*num_instructions].opcode = 0x29;
    instructions[*num_instructions].has_modrm = 1;
    instructions[*num_instructions].op_extended = 0;
    instructions[*num_instructions].operand_size = 8;
    (*num_instructions)++;
    
    strcpy(instructions[*num_instructions].mnemonic, "AND");
    instructions[*num_instructions].opcode = 0x21;
    instructions[*num_instructions].has_modrm = 1;
    instructions[*num_instructions].op_extended = 0;
    instructions[*num_instructions].operand_size = 8;
    (*num_instructions)++;
    
    strcpy(instructions[*num_instructions].mnemonic, "OR");
    instructions[*num_instructions].opcode = 0x09;
    instructions[*num_instructions].has_modrm = 1;
    instructions[*num_instructions].op_extended = 0;
    instructions[*num_instructions].operand_size = 8;
    (*num_instructions)++;
    
    strcpy(instructions[*num_instructions].mnemonic, "XOR");
    instructions[*num_instructions].opcode = 0x31;
    instructions[*num_instructions].has_modrm = 1;
    instructions[*num_instructions].op_extended = 0;
    instructions[*num_instructions].operand_size = 8;
    (*num_instructions)++;
    
    strcpy(instructions[*num_instructions].mnemonic, "CMP");
    instructions[*num_instructions].opcode = 0x39;
    instructions[*num_instructions].has_modrm = 1;
    instructions[*num_instructions].op_extended = 0;
    instructions[*num_instructions].operand_size = 8;
    (*num_instructions)++;
    
    strcpy(instructions[*num_instructions].mnemonic, "PUSH");
    instructions[*num_instructions].opcode = 0x50;
    instructions[*num_instructions].has_modrm = 0;
    instructions[*num_instructions].op_extended = 0;
    instructions[*num_instructions].operand_size = 8;
    (*num_instructions)++;
    
    strcpy(instructions[*num_instructions].mnemonic, "POP");
    instructions[*num_instructions].opcode = 0x58;
    instructions[*num_instructions].has_modrm = 0;
    instructions[*num_instructions].op_extended = 0;
    instructions[*num_instructions].operand_size = 8;
    (*num_instructions)++;
    
    strcpy(instructions[*num_instructions].mnemonic, "LEA");
    instructions[*num_instructions].opcode = 0x8D;
    instructions[*num_instructions].has_modrm = 1;
    instructions[*num_instructions].op_extended = 0;
    instructions[*num_instructions].operand_size = 8;
    (*num_instructions)++;

    strcpy(instructions[*num_instructions].mnemonic, "SYSCALL");
    instructions[*num_instructions].opcode = 0x0F;
    instructions[*num_instructions].opcode2 = 0x05;
    instructions[*num_instructions].is_two_byte = 1;
    instructions[*num_instructions].has_modrm = 0;
    instructions[*num_instructions].op_extended = 0;
    instructions[*num_instructions].operand_size = 0;
    (*num_instructions)++;
}

void encode_instruction(const InstructionMapping *inst, const Operand *operands, 
                          int num_operands, unsigned char *output, int *output_size) {
    *output_size = 0;
    
    int need_rex = 0;
    int rex_w = (inst->operand_size == 8) ? 1 : 0;
    int rex_r = 0;
    int rex_x = 0;
    int rex_b = 0;
    
    if (rex_w || 
        (num_operands > 0 && operands[0].type == OP_REG && operands[0].value.reg >= R8) ||
        (num_operands > 1 && operands[1].type == OP_REG && operands[1].value.reg >= R8)) {
        need_rex = 1;
        if (num_operands > 0 && operands[0].type == OP_REG)
            rex_r = operands[0].value.reg >= R8 ? 1 : 0;
        if (num_operands > 1 && operands[1].type == OP_REG)
            rex_b = operands[1].value.reg >= R8 ? 1 : 0;
    }
    
    if (need_rex) {
        output[(*output_size)++] = generate_rex(rex_w, rex_r, rex_x, rex_b);
    }
    
    output[(*output_size)++] = inst->opcode;
    
    if (inst->has_modrm) {
        int mod = 0x3;
        int reg = (num_operands > 0 && operands[0].type == OP_REG) ? 
                 (operands[0].value.reg & 0x7) : 0;
        int rm = (num_operands > 1 && operands[1].type == OP_REG) ? 
                (operands[1].value.reg & 0x7) : 0;
        
        output[(*output_size)++] = generate_modrm(mod, reg, rm);
    }
    
    for (int i = 0; i < num_operands; i++) {
        if (operands[i].type == OP_IMM) {
            switch (inst->operand_size) {
                case 8:
                    *(int64_t*)(&output[*output_size]) = operands[i].value.immediate;
                    *output_size += 8;
                    break;
                case 4:
                    *(int32_t*)(&output[*output_size]) = (int32_t)operands[i].value.immediate;
                    *output_size += 4;
                    break;
                case 2:
                    *(int16_t*)(&output[*output_size]) = (int16_t)operands[i].value.immediate;
                    *output_size += 2;
                    break;
                case 1:
                    output[(*output_size)++] = (unsigned char)operands[i].value.immediate;
                    break;
            }
        }
    }
}

int parse_instruction(const char *line, char *mnemonic, Operand *operands, int *num_operands) {
    char temp_line[MAX_LINE_LENGTH];
    strncpy(temp_line, line, MAX_LINE_LENGTH);
    trim(temp_line);
    
    char *token = strtok(temp_line, " \t,");
    if (!token) return -1;
    
    strncpy(mnemonic, token, MAX_MNEMONIC_LENGTH);
    *num_operands = 0;
    
    while ((token = strtok(NULL, " \t,")) != NULL && *num_operands < 3) {
        if (token[0] == 'r' || token[0] == 'R') {
            operands[*num_operands].type = OP_REG;
            operands[*num_operands].value.reg = get_register_encoding(token);
            if (operands[*num_operands].value.reg == -1) {
                return -1;
            }
            (*num_operands)++;
        }
        else if (token[0] == '#' || (token[0] >= '0' && token[0] <= '9') || token[0] == '-') {
            operands[*num_operands].type = OP_IMM;
            const char *value_start = (token[0] == '#') ? token + 1 : token;
            operands[*num_operands].value.immediate = strtoll(value_start, NULL, 0);
            (*num_operands)++;
        }
        else if (token[0] == '[') {
            operands[*num_operands].type = OP_MEM;
            char *mem_expr = strdup(token + 1);
            mem_expr[strlen(mem_expr) - 1] = '\0';
            
            operands[*num_operands].value.mem.base = -1;
            operands[*num_operands].value.mem.index = -1;
            operands[*num_operands].value.mem.scale = 1;
            operands[*num_operands].value.mem.displacement = 0;
            
            char *component = strtok(mem_expr, "+");
            while (component != NULL) {
                while (*component == ' ') component++;
                char *end = component + strlen(component) - 1;
                while (end > component && *end == ' ') *end-- = '\0';
                
                if (component[0] == 'r' || component[0] == 'R') {
                    if (operands[*num_operands].value.mem.base == -1) {
                        operands[*num_operands].value.mem.base = get_register_encoding(component);
                    } else if (operands[*num_operands].value.mem.index == -1) {
                        char *scale = strchr(component, '*');
                        if (scale) {
                            *scale = '\0';
                            scale++;
                            operands[*num_operands].value.mem.scale = atoi(scale);
                            if (operands[*num_operands].value.mem.scale != 1 && 
                                operands[*num_operands].value.mem.scale != 2 && 
                                operands[*num_operands].value.mem.scale != 4 && 
                                operands[*num_operands].value.mem.scale != 8) {
                                operands[*num_operands].value.mem.scale = 1;
                            }
                        }
                        operands[*num_operands].value.mem.index = get_register_encoding(component);
                    }
                } else {
                    char *endptr;
                    long displacement = strtol(component, &endptr, 0);
                    if (endptr != component) {
                        operands[*num_operands].value.mem.displacement = (int32_t)displacement;
                    }
                }
                component = strtok(NULL, "+");
            }
            
            if (operands[*num_operands].value.mem.base == -1) {
                operands[*num_operands].value.mem.base = RAX;
            }
            
            free(mem_expr);
            (*num_operands)++;
        }
    }
    
    return 0;
}

#endif

