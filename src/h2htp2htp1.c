/**
 * Convert multi block htp file into one htp block
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
#include "lib/htp.h"
// #include "lib/fs.h"

#define VM 0
#define VS 1
#define VB 'b'

#define MAX_RECORD_COUNTER 100

int verbose = 0;
char htp_name[65] = { 0 };
int htp_name_length = 0;
uint16_t first_address = 0;
uint16_t last_address = 0;
long int positions[ MAX_RECORD_COUNTER ] = {};
uint16_t load_addresses[ MAX_RECORD_COUNTER ] = {};
int record_counter = 0;
char dummy_byte = 0; // For space between records

/********************************************************************************************************************
 * HTP functions
 ********************************************************************************************************************/
uint16_t mapping_htp( FILE *htp ) {
    fseek( htp, 0, SEEK_SET );
    while( !feof( htp ) ) {
        while( fgetc( htp ) != 0xA5 && !feof( htp ) );
        if ( !feof( htp ) ) {
            for( char c = fgetc( htp ); c != 0 ; c = fgetc( htp ) ) {
                htp_name[ htp_name_length++ ] = c;
                htp_name[ htp_name_length ] = 0;
            }
            positions[ record_counter ] = ftell( htp );
            uint16_t address0 = fgetc( htp ) + 256 * fgetc( htp );
            load_addresses[ record_counter++ ] = address0;
            if ( record_counter >= MAX_RECORD_COUNTER ) {
                fprintf( stderr, "Too many records in source htp file!\n" );
                exit( 4 );
            }
            uint16_t byte_counter = fgetc( htp ) + 256 * fgetc( htp );
            uint16_t address1 = address0 + byte_counter - 1;
            if ( verbose ) fprintf( stdout, "Found %d. htp record from 0x%04X to 0x%04X (%d bytes)\n", record_counter, address0, address1, byte_counter );
            if ( !first_address ) {
                first_address = address0;
                last_address = address1;
            }
            if ( address0<first_address ) first_address = address0;
            if ( address1>last_address ) last_address = address1;
            for( int i = 0; i < byte_counter; i++ ) fgetc( htp ); // payload
            fgetc( htp ); // CRC
        }
    }
    if ( verbose ) fprintf( stdout, "Required %d bytes total\n", last_address - first_address + 1 );
    return last_address - first_address + 1;
}

int merge_htp( FILE *srcHtp, FILE *destHtp ) {
    uint16_t total_byte_counter = mapping_htp( srcHtp );
    write_fix_htp_header( destHtp, 256, first_address, total_byte_counter, htp_name );
    uint16_t last_written_address = 0;
    for( int i = 0; i < record_counter; i++ ) {
        fseek( srcHtp, positions[ i ], SEEK_SET );
        uint16_t address0 = fgetc( srcHtp ) + 256 * fgetc( srcHtp );
        uint16_t byte_counter = fgetc( srcHtp ) + 256 * fgetc( srcHtp );
        uint16_t address1 = address0 + byte_counter - 1;
        if ( last_written_address ) {
            int dummy_counter = address0 - last_written_address - 1;
            for( int j = 0; j < dummy_counter; j++ ) crc_fputc( dummy_byte, destHtp );
            if ( verbose ) fprintf( stdout, "Written %d dummy bytes\n", dummy_counter );
        }
        for ( int j = 0; j < byte_counter; j++ ) {
            char c = fgetc( srcHtp );
            crc_fputc( c, destHtp );
        }
        if ( verbose ) fprintf( stdout, "Written %d payload bytes from 0x%04X\n", byte_counter, address0 );
        last_written_address = address1;
    }
    close_fix_htp_block( destHtp, 0 );
}

/********************************************************************************************************************
 * Main functions
 ********************************************************************************************************************/

void print_usage() {
    printf( "H2Htp2Htp1 v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "Copyright 2023 by László Princz\n");
    printf( "Create a one record .htp file from Aircomp 16 (Homelab 2) multi record .htp source file.\n");
    printf( "Usage:\n");
    printf( "h2htp2htp1 [-svl] -i <source_htp_file> -o <one_record_htp_filename>\n");
    printf( "-i source_htp_file : TXT file contains BASIC source code with line numbers.\n");
    printf( "-o htp_file_name       : Name of the new htp file.\n");
    printf( "-v                     : Verbose output.\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    int opt = 0;
    char *srcBasename = 0;
    char *destDir = 0;
    FILE *srcHtp = 0;
    FILE *destHtp = 0;
    while ( ( opt = getopt (argc, argv, "aplv?h:n:s:i:o:") ) != -1 ) {
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
                if ( !( srcHtp = fopen( optarg, "rb" ) ) ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
                }
                break;

            case 'o': // Create htp file
                destHtp = fopen( optarg, "wb" );
                if ( !destHtp ) {
                    fprintf( stderr, "Error creating %s.\n", optarg );
                    exit(4);
                }
                break;
        }
    }
    if ( srcHtp && destHtp ) {
        merge_htp( srcHtp, destHtp );
        fclose( srcHtp );
        fclose( destHtp );
    } else {
        print_usage();
    }
    return 0;
}
