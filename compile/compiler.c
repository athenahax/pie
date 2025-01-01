#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_ARRAY_SIZE 1000

int count_numbers(const char *str) {
    int count = 0;
    const char *ptr = str;
    
    while (*ptr) {
        while (*ptr && !isdigit(*ptr)) ptr++;
        if (!*ptr) break;
        
        count++;
        
        while (*ptr && (isdigit(*ptr))) ptr++;
    }
    
    return count;
}

void parse_array(const char *input, uint16_t *arr, int *size) {
    const char *ptr = input;
    int index = 0;
    
    while (*ptr && index < MAX_ARRAY_SIZE) {
        while (*ptr && !isdigit(*ptr)) ptr++;
        if (!*ptr) break;
        
        char *end;
        unsigned long num = strtoul(ptr, &end, 10);
        if (num > UINT16_MAX) {
            fprintf(stderr, "ERROR: integer too large for uint16_t: %lu\n", num);
            exit(1);
        }
        arr[index++] = (uint16_t)num;
        
        ptr = end;
    }
    
    *size = index;
}

char* create_array_string(uint16_t *arr, int size) {
    char *result = malloc(MAX_LINE_LENGTH * ((size / 8) + 1));
    char temp[32];
    
    strcpy(result, "    uint16_t locations[] = {\n        ");
    
    for (int i = 0; i < size; i++) {
        sprintf(temp, "%hu", arr[i]);
        strcat(result, temp);
        
        if (i < size - 1) {
            strcat(result, ", ");
            if ((i + 1) % 12 == 0) {
                strcat(result, "\n        ");
            }
        }
    }
    
    strcat(result, "\n    };\n");
    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <array_elements>\n", argv[0]);
        fprintf(stderr, "EXAMPLE: %s \"145, 117, 97, 85, 86, 86\"\n", argv[0]);
        return 1;
    }
    
    FILE *file, *temp;
    char line[MAX_LINE_LENGTH];
    uint16_t arr[MAX_ARRAY_SIZE];
    int size;
    int in_array = 0;
    
    size = count_numbers(argv[1]);
    if (size > MAX_ARRAY_SIZE || size == 0) {
        fprintf(stderr, "ERROR: invalid array size: %d\n", size);
        return 1;
    }
    
    parse_array(argv[1], arr, &size);
    
    char *new_array = create_array_string(arr, size);
    
    file = fopen("stub.c", "r");
    if (!file) {
        fprintf(stderr, "ERROR: couldn't open stub.c\n");
        return 1;
    }
    
    temp = fopen("temp.c", "w");
    if (!temp) {
        fprintf(stderr, "ERROR: couldn't create temporary file temp.c\n");
        fclose(file);
        return 1;
    }
    
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        if (strstr(line, "uint16_t locations[] = {")) {
            in_array = 1;
            fputs(new_array, temp);
            
            while (fgets(line, MAX_LINE_LENGTH, file)) {
                if (strstr(line, "};")) {
                    in_array = 0;
                    break;
                }
            }
            continue;
        }
        
        if (!in_array) {
            fputs(line, temp);
        }
    }
    
    free(new_array);
    fclose(file);
    fclose(temp);
    
    remove("stub.c");
    rename("temp.c", "stub.c");
    
    printf("Array has been written to stub.c\n");
    printf("Compiling...\n");
    
    system("gcc stub.c -o stub -lm");
    
    return 0;
}