/**
 * Címkék kezelése
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "labels.h"
#include "params.h"

#define MAX_LABEL_COUNTER 1000

int label_counter = 0;
char* labels[ MAX_LABEL_COUNTER ] = { 0 };
uint16_t numbers[ MAX_LABEL_COUNTER ] = { 0 };

int _get_label_index( const char* label ) {
    int index = 0;
    while( index < label_counter && strcmp( labels[ index ], label ) ) index++;
    return index;
}

void set_label( const char* label, uint16_t line_number ) {
    if ( _get_label_index( label ) < label_counter ) {
        fprintf( stderr, "Label already defined: '%s'\n", label );
        exit(4);
    } else {
        labels[ label_counter ] = malloc( sizeof(char) * strlen( label ) );
        strcpy( labels[ label_counter ], label );
        numbers[ label_counter ] = line_number;
    printf( "*** SET LABEL '%s' = %d (index=%d)\n", label, line_number, label_counter );
//printf( "First label = '%s'\n", labels[ 0 ] );
        label_counter++;
        if ( label_counter >= MAX_LABEL_COUNTER ) {
            printf( "Too many labels!\n" );
            exit(4);
        }
    }
}

uint16_t get_line_number( const char* label ) {
    int index = _get_label_index( label );
    if ( index < label_counter ) {
//	printf( "*** GET LABEL '%s' = %d (index=%d)\n", label, numbers[ index ], index );
        return numbers[ index ];
    } else {
        printf( "Label not found: '%s'\n", label );
        exit(4);
    }
}

void clear_labels() { label_counter = 0; }
