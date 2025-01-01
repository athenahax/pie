#include "pi.h"
#include "asm_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned char *bytes;
    int total_size;
    int capacity;
    uint16_t *pi_locations;
} EncodedProgram;

void init_encoded_program(EncodedProgram *program, int initial_capacity) {
    program->bytes = (unsigned char *)malloc(initial_capacity);
    program->pi_locations = (uint16_t *)malloc(initial_capacity * sizeof(uint16_t));
    program->total_size = 0;
    program->capacity = initial_capacity;
}

void add_encoded_bytes(EncodedProgram *program, unsigned char *bytes, int size) {
    while (program->total_size + size > program->capacity) {
        program->capacity *= 2;
        program->bytes = (unsigned char *)realloc(program->bytes, program->capacity);
        program->pi_locations = (uint16_t *)realloc(program->pi_locations, 
                                                   program->capacity * sizeof(uint16_t));
    }
    
    memcpy(program->bytes + program->total_size, bytes, size);
    program->total_size += size;
}

void print_encoded_array(EncodedProgram *program) {
    printf("\nORIGINAL BYTES[] = {\n    ");
    
    for (int i = 0; i < program->total_size; i++) {
        printf("0x%02X", program->bytes[i]);
        if (i < program->total_size - 1) {
            printf(", ");
        }
        
        if ((i + 1) % 12 == 0 && i < program->total_size - 1) {
            printf("\n    ");
        }
    }
    
    printf("\n};\n");

    printf("\nLOCATIONS: ");
    for (int i = 0; i < program->total_size; i++) {
        printf("%u", program->pi_locations[i]);
        if (i < program->total_size - 1) {
            printf(", ");
        }
    }
    printf("\n");
    
    printf("\nTOTAL SIZE: %d bytes\n", program->total_size);
}

void encode_program_in_pi(EncodedProgram *program) {
    PiEncoder encoder;
    pi_encoder_init(&encoder);
    
    for (int i = 0; i < program->total_size; i++) {
        program->pi_locations[i] = pi_encoder_get(&encoder, program->bytes[i]);
    }
}

void print_debug_bytes(unsigned char* bytes, int size, int line_number, const char* instruction) {
    printf("LINE %d: %s", line_number, instruction);
    printf(" DEBUG ENCODING: ");
    for (int i = 0; i < size; i++) {
        printf("%02X ", bytes[i]);
    }
    printf("\n");
}

void trim(char* str) {
    char* start = str;
    char* end = str + strlen(str) - 1;
    
    while(isspace(*start)) start++;
    while(end > start && isspace(*end)) end--;
    *(end + 1) = '\0';
    
    if(start > str) {
        memmove(str, start, strlen(start) + 1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <assembly_file>\n", argv[0]);
        return 1;
    }

    InstructionMapping instructions[MAX_OPCODES];
    int num_instructions;
    init_instruction_set(instructions, &num_instructions);

    EncodedProgram program;
    init_encoded_program(&program, 1024);

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "ERROR: couldn't open file %s\n", argv[1]);
        free(program.bytes);
        free(program.pi_locations);
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    int line_number = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        
        trim(line);
        if (strlen(line) == 0 || line[0] == ';' || line[0] == '#') {
            continue;
        }

        char mnemonic[MAX_MNEMONIC_LENGTH];
        Operand operands[3];
        int num_operands;

        if (parse_instruction(line, mnemonic, operands, &num_operands) < 0) {
            fprintf(stderr, "ERROR: couldn't parse line %d: %s\n", line_number, line);
            continue;
        }

        int found = 0;
        for (int i = 0; i < num_instructions; i++) {
            if (strcasecmp(instructions[i].mnemonic, mnemonic) == 0) {
                unsigned char encoded[15];
                int encoded_size;

                encode_instruction(&instructions[i], operands, num_operands, 
                                 encoded, &encoded_size);

                add_encoded_bytes(&program, encoded, encoded_size);
                print_debug_bytes(encoded, encoded_size, line_number, line);
                
                found = 1;
                break;
            }
        }

        if (!found) {
            fprintf(stderr, "WARNING: unknown instruction at line %d: %s\n", 
                    line_number, mnemonic);
        }
    }
    
    encode_program_in_pi(&program);

    print_encoded_array(&program);

    free(program.bytes);
    free(program.pi_locations);
    fclose(file);
    return 0;
}