/**
 * Create homelab 2 .htp file
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
#include "lib/fs.h"
#include "lib/htp.h"
#include "lib/basic.h"
#include "lib/params.h"

#define VM 0
#define VS 2
#define VB 'b'

#define BASIC_START 0x40A0
#define MAX_LINE_LENGTH 1000

char htp_name[65] = { 0 };

/********************************************************************************************************************
 * HTP functions
 ********************************************************************************************************************/
void create_bin_htp_block( FILE *htp, FILE *bin, uint16_t bin_load_address, const char* htp_name, int is_last ) {
    write_htp_header( htp, 128, bin_load_address, htp_name );
    for( unsigned char c = fgetc( bin ); !feof( bin ); c = fgetc( bin ) ) {
        crc_fputc( c, htp );
    }
    close_htp_block( htp, is_last );
}

void create_bas_htp_block( FILE *htp, FILE *txt, const char* htp_name, int is_last, FILE *BAS ) {
    uint16_t bas_load_address = 0x4016;
    write_htp_header( htp, 256, bas_load_address, htp_name );
    write_htp_basic_payload( htp, txt, bas_load_address, BAS );
    close_htp_block( htp, is_last );
}
/********************************************************************************************************************
 * String functions
 ********************************************************************************************************************/
void copy_to_name( char* basename ) {
    int i = 0;
    for( i = 0; i<16 && basename[ i ]; i++ ) htp_name[ i ] = basename[ i ];
    for( int j = i; j < 17; j++ ) htp_name[ j ] = 0;
}
/********************************************************************************************************************
 * Main functions
 ********************************************************************************************************************/

void print_usage() {
    printf( "H2CreateHtp v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "Copyright 2023 by László Princz\n");
    printf( "Create a new .htp file from Homelab 2 (Aircomp 16) BASIC txt source file.\n");
    printf( "Usage:\n");
    printf( "h2CreateHtp [-svl] -i <source_bas_txt_file> -n <name_on_tape> -o <htp_filename>\n");
    printf( "-t source_bas_txt_file : TXT file contains BASIC source code with line numbers.\n");
    printf( "-B basic_output_fie    : Savce the final BASIC source code int htp file.\n");
    printf( "-b z80_bin_file        : z80 system binary block.\n");
    printf( "-L bin_load_address    : load address for binary block. Default value is 0x6000 (24576 in decimal).\n");
    printf( "-o htp_file_name       : Name of the new htp file.\n");
    printf( "-n name_on_tape        : The tape name. Default name created from ptp filename.\n");
    printf( "-s character           : skip BASIC comments with this first character. ''=skip all comments. 'no'=disable comment skipping. Default value is '!'\n");
    printf( "-p                     : with program protection\n");
    printf( "-a                     : autostart and program break disabled (only in wav format)\n");
    printf( "-l                     : List tokens only.\n");
    printf( "-v                     : Verbose output.\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    int opt = 0;
    char *srcBasename = 0;
    char *destDir = 0;
    FILE *htp = 0;
    FILE *txt = 0;
    FILE *BAS = 0;
    FILE *bin = 0;
    uint16_t bin_load_address = 0x6000;
    while ( ( opt = getopt (argc, argv, "aplv?h:n:s:t:B:b:L:o:") ) != -1 ) {
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

            case 's':
                if ( !optarg[0] || ( optarg[0] && !optarg[1] ) ) {
                    skip_comments_character = optarg[0];
                } else if ( optarg[0]=='n' && optarg[1]=='o' && optarg[2]==0 ) {
                    comment_skip_enabled = 0;
                } else {
                    fprintf( stderr, "Error in comment skip parameter. Only one character enabled!" ); exit(4);
                }
                break;

            case 'l':
                list_tokens();
                exit(0);
                break;

            case 'n': // define ptp name
                copy_to_name( optarg );
                break;

            case 'p': // Program protection
                program_protection_404C = 0;
                break;

            case 'a': // Autostart and no interrupt
                program_autostart_403E = 1;
                break;

            case 't': // open txt file
                if ( !( txt = fopen( optarg, "rb" ) ) ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
                }
                srcBasename = copyStr( basename( optarg ), 4 );
                destDir = copyStr( dirname( optarg ), 0 );
                if ( !htp_name[ 0 ] ) copy_to_name( srcBasename );
                break;

            case 'b': // open bin file
                if ( !( bin = fopen( optarg, "rb" ) ) ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
                }
                srcBasename = copyStr( basename( optarg ), 4 );
                destDir = copyStr( dirname( optarg ), 0 );
                if ( !htp_name[ 0 ] ) copy_to_name( srcBasename );
                break;

            case 'L':
                if ( !sscanf( optarg, "%u", &bin_load_address ) ) {
                    fprintf( stderr, "Error parsing argument for '-L'.\n");
                    exit(2);
                }
                break;

            case 'B': // Create BAS output file
                BAS = fopen( optarg, "wb" );
                if ( !BAS ) {
                    fprintf( stderr, "Error creating %s.\n", optarg );
                    exit(4);
                }
                break;

            case 'o': // Create htp file
                htp = fopen( optarg, "wb" );
                if ( !htp ) {
                    fprintf( stderr, "Error creating %s.\n", optarg );
                    exit(4);
                }
                break;
        }
    }
    if ( htp && txt ) {
        dropExtIfExists( htp_name );
        create_bas_htp_block( htp, txt, htp_name, !bin, BAS );
        if ( BAS ) fclose( BAS );
        if ( bin ) create_bin_htp_block( htp, bin, bin_load_address, 0, 1 );
        close_htp_file( htp );
    } else {
        print_usage();
    }
    return 0;
}
