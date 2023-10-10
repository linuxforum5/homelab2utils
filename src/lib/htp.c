/**
 * Homelab 2 (Aircomp 16) htp functions
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "htp.h"

unsigned char crc = 0;
uint16_t byte_counter = 0;
long int payload_length_position = 0;

void write_fix_htp_header( FILE *htp, int prefix_counter, uint16_t load_address, uint16_t byte_counter, const char *htp_name ) {
    // Write header
    // if ( htp_name ) { // If name not null ( first block )
    for( int i=0; i<prefix_counter; i++ ) fputc( 0x00, htp );
    fputc( 0xA5, htp );
    crc = 0;
    if ( htp_name ) { // If name not null ( first block )
        // Write name
        for( int i=0; (i<65) && htp_name[i]; i++ ) fputc( htp_name[ i ], htp );
    }
    fputc( 0x0, htp );
    // Write load address
    fputc( load_address % 256, htp );
    fputc( load_address / 256, htp );
    // Write length placeholder
    fputc( byte_counter % 256, htp );
    fputc( byte_counter / 256, htp );
}


void write_htp_header( FILE *htp, int counter, uint16_t load_address, const char *htp_name ) {
    // Write header
    // if ( htp_name ) { // If name not null ( first block )
    for( int i=0; i<counter; i++ ) fputc( 0x00, htp );
    fputc( 0xA5, htp );
    crc = 0;
    byte_counter = 0;
    payload_length_position = 0;
    if ( htp_name ) { // If name not null ( first block )
        // Write name
        for( int i=0; (i<65) && htp_name[i]; i++ ) fputc( htp_name[ i ], htp );
    }
    fputc( 0x0, htp );
    // Write load address
    fputc( load_address % 256, htp );
    fputc( load_address / 256, htp );
    // Write length placeholder
    payload_length_position = ftell( htp );
    fputc( byte_counter % 256, htp );
    fputc( byte_counter / 256, htp );
}
/*
void write_htp_block_name( FILE *htp, const char *htp_name ) {
    // Write name
    for( int i=0; (i<65) && htp_name[i]; i++ ) fputc( htp_name[ i ], htp );
    fputc( 0x0, htp );
}

void write_htp_load_address_and_size_placeholder( FILE *htp, uint16_t load_address ) {
    // Write load address
    fputc( load_address % 256, htp );
    fputc( load_address / 256, htp );
    // Write length placeholder
    payload_length_position = ftell( htp );
    fputc( byte_counter % 256, htp );
    fputc( byte_counter / 256, htp );
}
*/
void crc_fputc( char byte, FILE *htp ) {
    crc += byte;
    byte_counter++;
    fputc( byte, htp );
}

void crc_write_word( uint16_t word, FILE *htp ) {
    crc_fputc( word % 256, htp );
    crc_fputc( word / 256, htp );
}

void close_fix_htp_block( FILE *htp, int is_last_block ) {
    fputc( crc, htp );
    // fputc( is_last_block ? crc+1 : crc, htp );
    fputc( is_last_block ? 0x00 : 0xD8, htp );
}

void close_htp_block( FILE *htp, int is_last_block ) {
    fseek( htp, 0, SEEK_END );
    long int end_position = ftell( htp );
    // Back to the payload length_
    fseek( htp, payload_length_position, SEEK_SET );
    fputc( byte_counter % 256, htp );
    fputc( byte_counter / 256, htp ); // Ezek után a byte_counter értke már nem számít, a későbbi write_word már módosíthatja. Ott már csak a crc a fontos!
    // Back to end of file
    fseek( htp, end_position, SEEK_SET );
    close_fix_htp_block( htp, is_last_block );
}

void close_htp_file( FILE *htp ) {
    // Write the last 0 byte
    // fputc( 0, htp );
    fclose( htp );
}

void htp_dec_crc_and_byte_counter( unsigned char byte ) {
    crc -= byte;
    byte_counter--;
}

uint16_t htp_get_byte_counter() { return byte_counter; }
