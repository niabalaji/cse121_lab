#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "esp_timer.h"

#define UNIT_MS 100
#define DOT_MS (UNIT_MS)
#define DASH_MS (3 * UNIT_MS)
#define LETTER_SPACE_MS 280
#define WORD_SPACE_MS 650
#define MAX_MORSE_LEN 5
#define THRESHOLD 700

static const char *TAG = "MORSE_DECODER";

char morse_letter[MAX_MORSE_LEN + 1] = {0};
int letter_index = 0;
char message_buffer[256] = {0};
int message_index = 0;
int prev_state = -1;
int64_t last_change_time = 0;

// Morse code table
typedef struct {
    const char *morse;
    char letter;
} morse_entry_t;

static const morse_entry_t morse_table[] = {
    {".-", 'A'},   {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},
    {".", 'E'},    {"..-.", 'F'}, {"--.", 'G'},  {"....", 'H'},
    {"..", 'I'},   {".---", 'J'}, {"-.-", 'K'},  {".-..", 'L'},
    {"--", 'M'},   {"-.", 'N'},   {"---", 'O'},  {".--.", 'P'},
    {"--.-", 'Q'}, {".-.", 'R'},  {"...", 'S'},  {"-", 'T'},
    {"..-", 'U'},  {"...-", 'V'}, {".--", 'W'},  {"-..-", 'X'},
    {"-.--", 'Y'}, {"--..", 'Z'},
    {"-----", '0'}, {".----", '1'}, {"..---", '2'}, {"...--", '3'},
    {"....-", '4'}, {".....", '5'}, {"-....", '6'}, {"--...", '7'},
    {"---..", '8'}, {"----.", '9'},
    {NULL, 0}
};

char decode_morse(const char *morse) {
    for (int i = 0; morse_table[i].morse != NULL; i++) {
        if (strcmp(morse, morse_table[i].morse) == 0) {
            return morse_table[i].letter;
        }
    }
    ESP_LOGW(TAG, "Unknown Morse sequence: [%s]", morse);
    return '?';
}

void app_main(void) {
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_2, &chan_config));

    last_change_time = esp_timer_get_time();

    while (1) {
        int adc_val = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_2, &adc_val));
        int light_on = adc_val > THRESHOLD ? 1 : 0;
        int64_t now = esp_timer_get_time();
        int64_t duration = (now - last_change_time) / 1000;

        if (prev_state == -1) {
            prev_state = light_on;
            last_change_time = now;
        }

        if (light_on != prev_state) {
            ESP_LOGI(TAG, "Transition: %d -> %d | Duration: %lld ms", prev_state, light_on, duration);

            if (prev_state == 1 && light_on == 0) {
                // LED just turned OFF
                if (duration < DOT_MS * 1.5) {
                    if (letter_index < MAX_MORSE_LEN) {
                        morse_letter[letter_index++] = '.';
                    } else {
                        ESP_LOGW(TAG, "Morse buffer full. Resetting.");
                        letter_index = 0;
                        memset(morse_letter, 0, sizeof(morse_letter));
                    }
                } else {
                    if (letter_index < MAX_MORSE_LEN) {
                        morse_letter[letter_index++] = '-';
                    } else {
                        ESP_LOGW(TAG, "Morse buffer full. Resetting.");
                        letter_index = 0;
                        memset(morse_letter, 0, sizeof(morse_letter));
                    }
                }
            } else if (prev_state == 0 && light_on == 1) {
                // LED just turned ON (gap just ended)
                if (duration > WORD_SPACE_MS) {
                    // Decode letter before word break
                    if (letter_index > 0) {
                        morse_letter[letter_index] = '\0';
                        char decoded = decode_morse(morse_letter);
                        message_buffer[message_index++] = decoded;
                    }
                    message_buffer[message_index++] = ' ';
                    message_buffer[message_index] = '\0';
                    printf("Message: %s\n", message_buffer);
                    letter_index = 0;
                    memset(morse_letter, 0, sizeof(morse_letter));
                
                } else if (duration > LETTER_SPACE_MS) {
                    if (letter_index > 0) {
                        morse_letter[letter_index] = '\0';
                        char decoded = decode_morse(morse_letter);
                        message_buffer[message_index++] = decoded;
                        message_buffer[message_index] = '\0';
                        ESP_LOGI(TAG, "Decoded [%s] = %c", morse_letter, decoded);
                        letter_index = 0;
                        memset(morse_letter, 0, sizeof(morse_letter));
                    }
                }
                
            }

            prev_state = light_on;
            last_change_time = now;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
