#ifndef CRYPTO_H
#define CRYPTO_H

// Shared XOR encryption key
static const char XOR_KEY[] = "6sVKlW/RqcfH5ZbOWF/jjkb7jBbfTZJmgU8fNX/+tpH+b8BXfy08f0RjwMZLvLI/okeC/fZlWpZiNjunOSVfFnHp00vJRcejZ9OuBHJa0M2FO/kQqAU6WaYlPM6CJh5WCtMaVHMSCa64Z0cxQs1RWXuPoERikrMY8KhvANc7lood9IhUd+ZvJ4RfD4rgaBev";

// Shared encryption/decryption function
static void xor_encrypt_decrypt(const char* input, char* output, int length) {
    int key_len = strlen(XOR_KEY);
    for (int i = 0; i < length; i++) {
        output[i] = input[i] ^ XOR_KEY[i % key_len];
    }
}

// Shared hex conversion functions
static void binary_to_hex(const char* binary, char* hex, int length) {
    for (int i = 0; i < length; i++) {
        sprintf(hex + (i * 2), "%02X", (unsigned char)binary[i]);
    }
    hex[length * 2] = '\0';
}

static int hex_to_binary(const char* hex, char* binary) {
    int len = strlen(hex);
    for (int i = 0; i < len; i += 2) {
        char hex_byte[3] = {hex[i], hex[i + 1], '\0'};
        binary[i / 2] = (char)strtol(hex_byte, NULL, 16);
    }
    return len / 2;
}

#endif // CRYPTO_H