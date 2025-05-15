#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for sleep()
#include <string.h>
#include <ctype.h>

#define GPIO_CHIP "gpiochip0" // default chip on Pi
#define GPIO_LINE 17          // BCM pin number (GPIO17)
#define UNIT_TIME 150000 // 150 ms

struct gpiod_chip *chip;
struct gpiod_line *line;

// Morse Code Dictionary
const char* morse_table[36] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..",     // A-I
    ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.",   // J-R
    "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",          // S-Z
    "-----", ".----", "..---", "...--", "....-", ".....",              // 0-5
    "-....", "--...", "---..", "----."                                 // 6-9
};

// Letter to Morse
const char* get_morse(char c) {
    if (isalpha(c)) return morse_table[toupper(c) - 'A'];
    if (isdigit(c)) return morse_table[c - '0' + 26];
    if (c == ' ') return "/";
    return "";  // Unknown character
}

// LED blink
void blink_dot() {
    gpiod_line_set_value(line, 1);
    usleep(UNIT_TIME);
    gpiod_line_set_value(line, 0);
    usleep(UNIT_TIME);
}

void blink_dash() {
    gpiod_line_set_value(line, 1);
    usleep(UNIT_TIME * 3);
    gpiod_line_set_value(line, 0);
    usleep(UNIT_TIME);
}

void blink_space_between_letters() {
    usleep(UNIT_TIME * 2);  // Already waited 1 unit after symbol
}

void blink_space_between_words() {
    usleep(UNIT_TIME * 6);  // Already waited 1 unit after symbol
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <repetitions> \"message\"\n", argv[0]);
        return 1;
    }

    int repetitions = atoi(argv[1]);
    char *message = argv[2];

    // Open GPIO
    chip = gpiod_chip_open_by_name(GPIO_CHIP);
    if (!chip) { perror("Failed to open chip"); return 1; }

    line = gpiod_chip_get_line(chip, GPIO_LINE);
    if (!line) { perror("Failed to get line"); gpiod_chip_close(chip); return 1; }

    if (gpiod_line_request_output(line, "morse", 0) < 0) {
        perror("Failed to request output");
        gpiod_chip_close(chip);
        return 1;
    }

    // Repeat message
    for (int r = 0; r < repetitions; r++) {
        for (size_t i = 0; i < strlen(message); i++) {
            const char *morse = get_morse(message[i]);

            if (strcmp(morse, "/") == 0) {
                blink_space_between_words();
                continue;
            }

            for (size_t j = 0; j < strlen(morse); j++) {
                if (morse[j] == '.') blink_dot();
                else if (morse[j] == '-') blink_dash();
            }

            blink_space_between_letters();
        }

        // Space between repeated messages
        usleep(UNIT_TIME * 7);
    }

    // Clean up
    gpiod_line_release(line);
    gpiod_chip_close(chip);

    return 0;
}


