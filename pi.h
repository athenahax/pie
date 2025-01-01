#ifndef PI_H
#define PI_H

#include <stdint.h>
#include <math.h>
#include <stdlib.h>

uint64_t binpow(uint64_t a, uint64_t b, uint64_t mod) {
    a %= mod;
    uint64_t result = 1;
    while (b > 0) {
        if (b & 1) result = (result * a) % mod;
        a = (a * a) % mod;
        b >>= 1;
    }
    return result;
}

double series(uint16_t d, int j) {
    double sum = 0;

    for (uint16_t k = 0; k <= d; ++k) {
        sum += (double)binpow(16, d - k, 8 * k + j) / (8 * k + j);
        sum -= floor(sum);
    }

    for (uint16_t k = d + 1;; ++k) {
        double inc = pow(16., (double)(d - k)) / (8 * k + j);

        if (inc < 1e-7) {
            break;
        }

        sum += inc;
        sum -= floor(sum);
    }

    return sum;
}

uint8_t pi_digit(uint16_t digit) {
    double s1 = series(digit, 1);
    double s4 = series(digit, 4);
    double s5 = series(digit, 5);
    double s6 = series(digit, 6);

    double pi_val = 4 * s1 - 2 * s4 - s5 - s6;
    pi_val -= floor(pi_val);

    return 16 * pi_val;
}

typedef struct {
    uint16_t byte_to_idx[0x100];
} PiEncoder;

void pi_encoder_init(PiEncoder* encoder) {
    uint8_t bitset[0x100] = {0};

    uint16_t idx = 0;
    uint16_t bits = pi_digit(idx + 1) << 12 | pi_digit(idx) << 8;
    size_t filled = 0;

    while (filled < 0x100) {
        bits = (pi_digit(idx + 3) << 12 | pi_digit(idx + 2) << 8 | bits >> 8);

        for (int bit = 0; bit < 8; ++bit) {
            uint8_t byte = (bits >> bit);
            if (!bitset[byte]) {
                bitset[byte] = 1;
                encoder->byte_to_idx[byte] = 8 * idx + bit;
                if (++filled >= 0x100) {
                    break;
                }
            }
        }
        idx += 2;
    }
}

uint16_t pi_encoder_get(const PiEncoder* encoder, uint8_t byte) {
    if (byte >= 0x100) {
        exit(1);
    }
    return encoder->byte_to_idx[byte];
}

typedef struct {
    uint8_t idx_to_byte[0x800];
    size_t size;
} PiDecoder;

void pi_decoder_init(PiDecoder* decoder, const PiEncoder* encoder) {
    decoder->size = 0x100;
    for (size_t i = 0; i < 0x800; i++) {
        decoder->idx_to_byte[i] = 0xFF;
    }
    
    for (uint16_t byte = 0; byte < 0x100; ++byte) {
        uint16_t idx = pi_encoder_get(encoder, byte);
        decoder->idx_to_byte[idx] = byte;
    }
}

uint8_t pi_decoder_get(const PiDecoder* decoder, uint16_t idx) {
    if (idx >= 0x800 || decoder->idx_to_byte[idx] == 0xFF) {
        exit(1);
    }
    return decoder->idx_to_byte[idx];
}

size_t pi_encoder_size(const PiEncoder* encoder) {
    return 0x100;
}

size_t pi_decoder_size(const PiDecoder* decoder) {
    return decoder->size;
}

#endif