/**
 * Címkék kezelése
 */
#include <stdint.h>

#define MAX_LABEL_LENGTH 100

void set_label( const char* label, uint16_t line_number );
uint16_t get_line_number( const char* label );
void clear_labels();
