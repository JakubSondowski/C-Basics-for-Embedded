#include <stdio.h>
#include <stdint.h>

// Constants for bit operations
#define LAST_BIT 0x1
#define BITS_IN_BYTE 8

// Define a custom number type for uniform formatting, 
// must be unsigned type (uint8_t, uint16_t, uint32_t, unsigned short int etc.) for correct binary output.
// Using signed types may lead to unexpected binary representations.
typedef uint16_t selectedNumberFormat;

// Function prototype
void print_in_binary(selectedNumberFormat number);

int main(void) {
    // Define decimal, hexadecimal, and binary number representations
    selectedNumberFormat decimalNumber = 2137;
    selectedNumberFormat hexNumber = 0x1660;
    selectedNumberFormat binNumber = 0b10111;

    // Display numbers in their native formats
    printf("All numbers in native formats:\n");
    printf("decimalNumber = %d\n", decimalNumber);
    printf("hexNumber     = 0x%x\n", hexNumber);
    printf("binNumber     = 0b");
    print_in_binary(binNumber);
    putchar('\n');

    // Display all numbers in binary
    printf("Other formats in binary format:\n");
    printf("decimalNumber = 0b");
    print_in_binary(decimalNumber);
    printf("hexNumber     = 0b");
    print_in_binary(hexNumber);
    putchar('\n');

    // Display other representations in decimal
    printf("Other formats in decimal:\n");
    printf("hexNumber = %d\n", hexNumber);
    printf("binNumber = %d\n", binNumber);
    putchar('\n');

    // Display all numbers in hexadecimal
    printf("Other formats in hexadecimal:\n");
    printf("decimalNumber = 0x%x\n", decimalNumber);
    printf("binNumber     = 0x%x\n", binNumber);
    //putchar('\n');

    return 0;
}

// Prints an 8, 16, or 32-bit number in binary format, depending on the chosen type
void print_in_binary(selectedNumberFormat number) {
    int totalBits = sizeof(number) * BITS_IN_BYTE;
    for (int i = 0; i < totalBits; i++) {
        // Shift the number to isolate each bit and print it
        printf("%d", (number >> (totalBits - 1 - i)) & LAST_BIT);
    }
    putchar('\n');
}
