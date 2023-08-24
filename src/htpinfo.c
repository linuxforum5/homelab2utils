/**
 * Show htp information
 * HTP fájlformátum
 * - 256 x 0x00
 * - 0xA5
 * - name with ending 0 character max 64 bytes length
 * - start address 2 bytes ( low, high ordered )
 * - length 2 bytes ( low, high ordered )
 * - data with 1 byte crc sum code
 * - 1 byte crc
 * - 0x00
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"
#include <libgen.h>

#define VM 0
#define VS 1
#define VB 'b'

int verbose = 0;

/********************************************************************************************************************
 * HTP functions
 ********************************************************************************************************************/

void show_htp_info( FILE *htp ) {
    for( int i=0; i<256; i++ ) {
        if ( fgetc( htp ) != 0 ) {
            fprintf( stderr, "Invalid htp header leading zeros\n" ); exit(4);
        }
    }
    if ( fgetc( htp ) != 0xA5 ) {
        fprintf( stderr, "Invalid htp header 0xA5 code\n" ); exit(4);
    }
    char c = 0;
    printf( "Name: " );
    int name_length = 0;
    while( c = fgetc( htp ) ) {
        printf( "%c", c );
        name_length++;
    }
    printf( " (length = %d)\n", name_length );
    int start_address = fgetc( htp ) + 256*fgetc( htp );
    printf( "Start address: %04X (%d)\n", start_address, start_address );
    int payload_length = fgetc( htp ) + 256*fgetc( htp );
    printf( "Payload length: %04X (%d)\n", payload_length, payload_length );
    int first_free_address = start_address + payload_length;
    printf( "First free address: %04X (%d)\n", first_free_address, first_free_address );
    int cols = 16;
    for( int i=start_address; i<0x40A0; i+=cols ) {
        printf( "%04X :", i );
        for( int j=0; j<cols; j++ ) printf( " %02X", fgetc( htp ) );
        printf( "\n" );
    }
}

/********************************************************************************************************************
 * Main functions
 ********************************************************************************************************************/
void print_usage() {
    printf( "htpinfo v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "Copyright 2023 by László Princz\n");
    printf( "Show htp informations.\n");
    printf( "Usage:\n");
    printf( "htpinfo [-v] -i <htp_filename>\n");
    printf( "-i htp_file_name       : Name of the new htp file.\n");
    printf( "-h                     : This screen.\n");
    printf( "-v                     : Verbose output.\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    int opt = 0;
    FILE *htp = 0;
    while ( ( opt = getopt (argc, argv, "v?h:i:") ) != -1 ) {
        switch ( opt ) {
            case -1:
            case ':':
                break;

            case '?':
            case 'h':
                print_usage();
                break;

            case 'v':
                verbose = 1;
                break;

            case 'i': // open pp file
                if ( !( htp = fopen( optarg, "rb" ) ) ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
                }
                break;
        }
    }
    if ( htp ) {
        show_htp_info( htp );
        fclose( htp );
    } else {
        print_usage();
    }
    return 0;
}
