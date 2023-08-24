/**
 * Create homelab BASIC .htp file
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
// #include "lib/fs.h"

#define VM 0
#define VS 1
#define VB 'b'

#define BASIC_START 0x40A0
#define MAX_LINE_LENGTH 1000

int verbose = 0;
int comment_skip_enabled = 1;
char skip_comments_character = '!'; 
char htp_name[65] = { 0 };
unsigned char program_protection_404C = 0x10; // Ha ez 00, akkor van programvédelem. 0x10 esetén nincs.
unsigned char program_autostart_403E = 0; // Ha ez 1, akkor a megszakítás tiltva van, és automatikusan elindul a program
uint16_t load_address = 0x4016;
uint16_t byte_counter = 0;

/********************************************************************************************************************
 * FS functions
 ********************************************************************************************************************/
int ext_is( const char* filename, const char* ext ) {
    int ext_length = strlen( ext );
    int fn_length = strlen( filename );
    if ( fn_length > ext_length ) {
        int separator_pos = fn_length - ext_length - 1;
        if ( filename[ separator_pos ] == '.' ) { // ext separator
            int match = 1;
            for( int i=0; match && i<ext_length; i++ ) match = ext[i]==filename[ separator_pos + 1 + i ];
            return match;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
} 

/********************************************************************************************************************
 * File functions
 ********************************************************************************************************************/

int is_dir( const char *path ) {
    struct stat path_stat;
    stat( path, &path_stat );
    return S_ISDIR( path_stat.st_mode );
}

int get_line_from( FILE *f, unsigned char* line, uint16_t *line_number ) {
    *line_number = 0;
    int read_number = 1;
    int line_length = 0;
    for ( unsigned char c = fgetc( f ); !feof( f ) && c!=10 && c!=13; c = fgetc( f ) ) {
        if ( read_number ) {
            if ( c>='0' && c<='9' ) {
                *line_number = *line_number * 10 + c - '0';
            } else if ( !*line_number && c <=' ' ) { // SKIP prefix
            } else if ( c==' ' || c==9 ) { // SKIP line prefix spaces
            } else {
                read_number = 0;
            }
        }
        if ( !read_number ) {
            line[ line_length++ ] = c;
            if ( line_length == MAX_LINE_LENGTH ) {
                printf( "Line to length!\n" ); exit(1);
            }
        }
    }
    return line_length;
}

/********************************************************************************************************************
 * BASIC functions
 ********************************************************************************************************************/
/**
 * A tokenek a 0x0411 - 0x51B címen kezdődnek. 225 byte
 * A token utolsó karakterének a 7. bitje 1-es, a többi karakternél ez a bit 0.
 * A token kódja az ebben a táblázabna elfoglalt indexe 0-ától számozva + 128.
 * Például a ^ token kódja : 0x80. Ez az első token a listában, tehát az indexe 0. Ehhez jön még a +128.
 *         a + token kódja : 0x81. Ez a második token a listában, tehát az indexe 1. Ehhez jön még a +128.
 */
const unsigned char tokens[] = { 
      0xde, 0xab, 0xad, 0xaa, 0xaf, 0xbe, 0xbc, 0xbd, 0x44, 0xcc, 0x47, 0xcc, 0x43, 0xd2, 0x48, 0xcd, 0x46, 0xce, 0x41, 0x4e, 0xc4, 0x4f, 0xd2, 0x43, 0x4c, 0xd2, 0x44, 0x49, 0xcd, 0x45, 0x4e,
0xc4, 0x46, 0x4f, 0xd2, 0x47, 0x4f, 0x54, 0xcf, 0x47, 0x4f, 0x53, 0x55, 0xc2, 0x49, 0xc6, 0x43, 0x4f, 0x4e, 0xd4, 0x44, 0x41, 0x54, 0xc1, 0x4c, 0x49, 0x53, 0xd4, 0x52, 0x45, 0x41, 0xc4, 0x4e,
0x45, 0x58, 0xd4, 0x4f, 0xce, 0x50, 0x4f, 0x4b, 0xc5, 0x44, 0x45, 0x46, 0x46, 0xce, 0x52, 0x45, 0x54, 0x55, 0x52, 0xce, 0x53, 0x41, 0x56, 0xc5, 0x54, 0x48, 0x45, 0xce, 0x4c, 0x4f, 0x41, 0xc4,
0x52, 0x45, 0x53, 0x54, 0x4f, 0x52, 0xc5, 0x53, 0x54, 0x45, 0xd0, 0x50, 0x4f, 0xd0, 0x4e, 0x45, 0xd7, 0x54, 0xcf, 0x20, 0x20, 0x0a, 0x8a, 0x50, 0x52, 0x49, 0x4e, 0xd4, 0x52, 0x55, 0xce, 0x54,
0x52, 0xc3, 0x4e, 0x4f, 0xd4, 0x50, 0x4c, 0x4f, 0xd4, 0x43, 0x41, 0x4c, 0xcc, 0x49, 0x4e, 0x50, 0x55, 0xd4, 0x53, 0x54, 0x52, 0xa4, 0x4c, 0x46, 0x54, 0xa4, 0x52, 0x47, 0x48, 0xa4, 0x43, 0x48,
0x52, 0xa4, 0x4d, 0x49, 0x44, 0xa4, 0x49, 0x4e, 0xd4, 0x41, 0x53, 0xc3, 0x4c, 0x45, 0xce, 0x41, 0x42, 0xd3, 0x53, 0x47, 0xce, 0x43, 0x4f, 0xd3, 0x50, 0x45, 0x45, 0xcb, 0x53, 0x51, 0xd2, 0x52,
0x4e, 0xc4, 0x53, 0x49, 0xce, 0x54, 0x41, 0xce, 0x55, 0x53, 0xd2, 0x56, 0x41, 0xcc, 0x41, 0x54, 0xce, 0x50, 0x4f, 0x49, 0x4e, 0xd4, 0x45, 0x58, 0xd0, 0x4c, 0x4f, 0xc7, 0x8d, 0x8d, 0x8d, 0x46,
0x52, 0xc5
// , 0x44, 0xc4, 0x43, 0xce, 0x50, 0xd0, 0x4f, 0xc4, 0x55, 0xc6, 0x53, 0xce, 0x54, 0xcd, 0x55, 0xd3, 0x2f, 0xb0, 0x49, 0xd1, 0x4f, 0xcd, 0x4f, 0xd6, 0x53, 0xcc, 0x42, 0xd3, 0x4f, 0xcb, 0x45, 0x52, 0x52, 0x4f, 0x52, 0x8d, 0x42, 0x52, 0x45, 0x41, 0x4b, 0x8d 
};

void list_tokens() {
    int code=128;
    int i=0;
    while( i<sizeof(tokens) ) {
        printf( "%d.\t%02X %d\t'", i, code, code );
        while( ( !(tokens[i] & 128) ) && ( i<sizeof(tokens) ) ) printf( "%c", tokens[i++] );
        printf( "%c' (%d)\n", tokens[i] & 127, tokens[i] );
        i++;
        code++;
    }
}

unsigned char get_token( unsigned char *string, int *string_length ) { // Visszaadja a szóhoz tartozó token kódot, vagy 0 kódot, ha nem tokenizálható.
    int current_token = 127; // A tokenszavak utolsó karakterének 7. bitje 1. A token azt mondja meg, hogy hányadik(+127) tokenszó a keresett. Az első szó az "^" Ennek kódja 128 
    int match_length = 0;
    int skip = 0; // Ha ez 1, akkor nem ellenőrzünk, hanem várjuk, míg vége lesz az aktuális tokennek, amiről tudjuk, hogy rossz
    for( int i=0; i<sizeof( tokens ); i++ ) {
        int last_token_character = tokens[ i ] > 127;
        if ( last_token_character ) current_token++; // Token last char. Increment token counter
        if ( skip ) {
            if ( last_token_character ) skip = 0;
        } else { // Ellenőrzésben vagyunk
            char token_character = last_token_character ? tokens[ i ] - 128 : tokens[ i ];

//printf( "Checkv %c%c for '%s' (length=%d from pos=%d): '", token_character, last_token_character ? '_':' ', string, match_length, i-match_length+1 );
//for( int p=0; p<match_length; p++ ) printf( "%c", string[p] );
//printf("' - %02X\n", current_token );

            if ( token_character == string[ match_length ] ) { // A keresett karakter jó
                match_length++; // Eggyel több illeszkedik
                if ( last_token_character ) { // Ez volt a token utolsó karaktere, megvagyunk
                    *string_length = match_length; // Az illesztett szó hossza
                    if ( verbose ) {
                        printf( "Found token '" );
                        for( int p=0; p<match_length; p++ ) printf( "%c", string[p] );
                        printf( "' code=%02X (%d)\n", current_token, current_token );
//printf( "Found new toke for '%s' (length=%d from pos=%d): ", string, match_length, i-match_length+1 );
//printf(" - %02X\n", current_token );
                    }
                    return current_token; // Ez a token kódja
                } else if ( match_length >= *string_length ) { // Nincs több illeszthető keresett karakter, ez nem token
//exit(1);
                    return 0;
                } // else { // Különben folytatjuk 
            } else { // Nem illeszkedő karakter
                match_length = 0; // Kezdjük előről
                if ( !last_token_character ) skip = 1;
            }
        }
    }
//exit(1);
    return 0;
}

char crc_fputc( char byte, FILE *f, char crc ) {
    crc += byte;
    byte_counter++;
    fputc( byte, f );
    return crc;
}

char dollar1( FILE *htp, unsigned char byte1, char crc ) {
    long int pos = ftell( htp );
    long int back_pos = pos-1;
    fseek( htp, back_pos, SEEK_SET );
    crc -= byte1;
    byte_counter--;
    byte1 = byte1 | 32;
    crc = crc_fputc( byte1, htp, crc );
    fseek( htp, pos, SEEK_SET );
    return crc;
}

char dollar2( FILE *htp, unsigned char byte2, char crc ) {
    long int pos = ftell( htp );
    long int back_pos = pos-2;
    fseek( htp, back_pos, SEEK_SET );
    crc -= byte2;
    byte_counter--;
    byte2 = byte2 | 32;
    crc = crc_fputc( byte2, htp, crc );
    fseek( htp, pos, SEEK_SET );
    return crc;
}

char bol64( FILE *htp, unsigned char byte1, char crc ) {
    long int pos = ftell( htp );
    long int back_pos = pos-1;
    fseek( htp, back_pos, SEEK_SET );
    crc -= byte1;
    byte_counter--;
    byte1 -= 64;
    crc = crc_fputc( byte1, htp, crc );
    fseek( htp, pos, SEEK_SET );
    return crc;
}

#define CODE_MODE 0
#define SPACE_MODE 1
#define DROP_MODE 2
/**
 * Egy sor elkódolás sorfolytonos. Ha van TOKEN, akkor annak heylére a kódja kerül, különben maradnak a karakterek.
 * A karakterek 7. bitje mindig 0, a tokenek 7. bitje mindig 1.
 * A string változók első karakterének 5. bitje is 1, cserébe a $ nem kerül tárolásra.
 */
char encode_line( FILE *htp, char* line, int line_length, char crc ) {
    unsigned char token = 0;
    int mode = CODE_MODE;
    int varNameCharCounter = 0; // Azt számolja, hogy az utolsó token óta hány konverziómentes karakter került kiírásra
    char back_chars[2] = { 0 }; // Az utoljára kiírt nem konvertált karakter az 1. indexen van, az előtte kiírt a 0. indexen
    for( int i=0; (i<line_length) && ( mode != DROP_MODE ); i++ ) { // i = next_char start position
        char current_char = line[ i ];
        if ( mode == CODE_MODE ) {
            if ( comment_skip_enabled && ( skip_comments_character == 0 ) && ( current_char == 0x27 ) && ( line[ i+1 ]== 0x22 ) ) { // 0x27=' és 0x22="
                mode = DROP_MODE;
            } else if ( comment_skip_enabled && ( current_char == 0x27 ) && ( line[ i+1 ]== 0x22 ) && ( line[ i+2 ]== skip_comments_character ) ) { // '"! mindenképp kihagyásra kerül!
                mode = DROP_MODE;
            } else {
                int current_char_start_position = i;
                int length = line_length-i; // Hátralévő hossz
                if ( token = get_token( line+current_char_start_position, &length ) ) { // Az i. indextől kezdve van egy token
                    crc = crc_fputc( token, htp, crc );
                    i += length - 1;
                    if ( ( token >= 0xAF ) && ( token <= 0xC8 ) ) { // Function
                        if ( line[ i + 1 ] == '(' ) {
                            i++; // Skip first '('
                        } else {
                            fprintf( stderr, "Missign '(' after funciton!\n" );
                            exit(4);
                        }
                    }
                    varNameCharCounter = 0;
                } else {  // i. index egy az egyben másolandó, mivel nem token
                    if ( current_char == 0x24 ) { // '$' character
                        if ( varNameCharCounter == 1 ) {
                            crc = dollar1( htp, back_chars[ 1 ], crc );
                        } else if ( varNameCharCounter > 1 ) {
                            if ( ( line[ i-2 ] < 'A' ) || ( line[i-2] > 'Z' ) ) {
                                fprintf( stderr, "Invalid string variable name char: '%c'!\n", line[i-2] );
                                exit(4);
                            }
                            crc = dollar2( htp, back_chars[ 0 ], crc );
                        } else {
                            fprintf( stderr, "Invalid string start!\n" );
                            exit(4);
                        }
                    } else if ( current_char == 0x28 ) { // '('
                        if ( varNameCharCounter == 1 ) {
                            if ( ( back_chars[ 1 ] < 'A' ) || ( back_chars[ 1 ] > 'Z' ) ) {
                                fprintf( stderr, "Invalid DIM variable name char: '%c'!\n", back_chars[ 1 ] );
                                exit(4);
                            }
                            crc = crc_fputc( 31, htp, crc );
                            varNameCharCounter = 0;
                        } else if ( varNameCharCounter > 1 ) {
                            crc = bol64( htp, back_chars[ 1 ], crc );
                        } else {
                            crc = crc_fputc( current_char, htp, crc );
                        }
                    } else if ( current_char != 32 ) {
                        crc = crc_fputc( current_char, htp, crc );
                        back_chars[ 0 ] = back_chars[ 1 ];
                        back_chars[ 1 ] = current_char;
                        if ( ( current_char >= 'A' ) && ( current_char <= 'Z' ) ) {
                            varNameCharCounter++;
                        } else {
                            varNameCharCounter = 0;
                        }
                        if ( current_char == 0x22 ) mode = SPACE_MODE;
                    }
                }
            }
        } else if ( mode == SPACE_MODE ) {
            crc = crc_fputc( current_char, htp, crc );
            if ( current_char == 0x22 ) mode = CODE_MODE;
        }
    }
    return crc;
}

// Line: NextRowAddrL NextRowAddrH NumL NumH tokenized 0
// Program end: 0 0
char encodeBasicFrom40A0( FILE *htp, FILE *txt, char crc ) { // Encode text into src.bytes
    uint16_t next_line_addr = BASIC_START;
    uint16_t line_number = 0;
    unsigned char line[ MAX_LINE_LENGTH ];
    unsigned char encoded_line[ MAX_LINE_LENGTH ];
    while( !feof( txt ) ) {
        int line_length = 0;
        if ( line_length = get_line_from( txt, line, &line_number ) ) {
            if ( line_number ) {
                crc = crc_fputc( line_number / 256, htp, crc );
                crc = crc_fputc( line_number % 256, htp, crc );
                line[ line_length ] = 0;
                if ( verbose ) printf( "Convert line %d '%s'\n", line_number, line );
                crc = encode_line( htp, line, line_length, crc );
                crc = crc_fputc( 0x60, htp, crc );
            } else {
                if ( verbose ) printf( "Skip line without line number\n" );
            }
        } else {
            if ( verbose ) printf( "Skip empty source line\n" );
        }
    }
    return crc;
}

/********************************************************************************************************************
 * HTP functions
 ********************************************************************************************************************/
char write_word( uint16_t word, FILE *htp, char crc ) {
    crc = crc_fputc( word % 256, htp, crc );
    crc = crc_fputc( word / 256, htp, crc );
    return crc;
}

int create_htp_from( FILE *htp, FILE *txt ) {
    // Write header
    for( int i=0; i<256; i++ )  fputc( 0x00, htp );
    fputc( 0xA5, htp );
    // Write name
    for( int i=0; (i<65) && htp_name[i]; i++ )  fputc( htp_name[ i ], htp );
    fputc( 0x0, htp );
    // Write load address
    fputc( load_address % 256, htp );
    fputc( load_address / 256, htp );
    // Write length placeholder
    long int payload_length_position = ftell( htp );
    fputc( byte_counter % 256, htp );
    fputc( byte_counter / 256, htp );

    char crc = 0;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Write DCB header
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    crc = write_word( 0x8000, htp, crc );                           // 4016 - 4017 : 8000		// HM
    long int dcb18_position = ftell( htp );                         //
    // Write 0x4018-0x40A0 DCB18- placeholder                       //
    crc = write_word( 0, htp, crc );                                // 4018-4019   :    0		// Start variables
    crc = write_word( 0, htp, crc );                                // 401A-401B   :    0		// Start string variables
    crc = write_word( 0, htp, crc );                                // 401C-401D   :    0		// End of string space
    // crc = write_word( 0x0004, htp, crc );                           // 401E-401F   : 0004		// Primary acc
    // crc = write_word( 0x4A85, htp, crc );                           // 4020-4021   : 4A85		// Primary acc
    // crc = write_word( 0xFE9B, htp, crc );                           // 4022-4023   : FE9B		// Random number
    // crc = write_word( 0x2F71, htp, crc );                           // 4024-4025   : 2F71		// Random number
    // crc = write_word( 0xFEFE, htp, crc );                           // 4026-4027   : FEFE // ???
    // crc = write_word( 0x0009, htp, crc );                           // 4028-4029   : 0009		// Auxiliary Accumulator
    // crc = write_word( 0x8133, htp, crc );                           // 402A-402B   : 8133		// Auxiliary Accumulator
    // crc = write_word( 0x5D4F, htp, crc );                           // 402C-402D   : 5DF4		// Temporary register
    // crc = write_word( 0x7FFF, htp, crc );                           // 402E-402F   : 7FFF		// Current BASIC Line
for( int i=0x401E; i<0x4030; i++ ) crc_fputc( 0x00, htp, crc );
    crc = write_word( 0x40A0, htp, crc );                           // 4030 - 4031 : 40A0		// Start of BASIC program
    // crc = crc_fputc( 0x01, htp, crc );                              // 4032        :   01		// BASIC FLAG 5.bit=CR
    // crc = write_word( 0xFEFE, htp, crc );                           // 4033-4034   : FEFE		// Start of graphics
    // crc = write_word( 0x41A3, htp, crc );                           // 4035-4036   : 41A3		// Statement pointer for CONT
    // crc = write_word( 0x40A0, htp, crc );                           // 4037-4038   : 40A0		// Next item for READ
    // crc = write_word( 0x00DC, htp, crc );                           // 4039-403A   : 00DC		// Current line for CONT
    // crc = write_word( 0x8000, htp, crc );                           // 403B-403C   : 8000		// Start of graphics area in RAM
for( int i=0x4032; i<0x403D; i++ ) crc_fputc( 0x00, htp, crc );
    crc = crc_fputc( 0x80, htp, crc );                              // 403D        :   80		// HM Page : HM high byte
    crc = crc_fputc( program_autostart_403E, htp, crc );                              // 403E        :    0		// BASIC interrupt és autostart? 1=tilt, 0=engedélyez
    // crc = crc_fputc( 0*0x1D, htp, crc );                              // 403F        :   1D		// Counter for cursor flashing
    // crc = crc_fputc( 0*0x00, htp, crc );                              // 4040        :    0 // ???
    // crc = write_word( 0x7FA4, htp, crc );                           // 4041-4042   : 7FA4		// Auxiliary stack pointer
    // crc = crc_fputc( 0*0x01, htp, crc );                              // 4043        :   01		// RST 8
    // crc = crc_fputc( 0*0x00, htp, crc );                              // 4044        :    0		// RST 8
    // crc = crc_fputc( 0*0x00, htp, crc );                              // 4045        :    0		// RST 8
    // crc = crc_fputc( 0*0x00, htp, crc );                              // 4046        :    0		// RST 10
    // crc = crc_fputc( 0*0x01, htp, crc );                              // 4047        :   01		// RST 10
    // crc = crc_fputc( 0*0x00, htp, crc );                              // 4048        :    0		// RST 10
    // crc = write_word( 0x1581, htp, crc );                           // 4049 - 404A : 1581 vagy 1582?	// Error vector
    // crc = crc_fputc( 0*0x0D, htp, crc );                              // 404B        :   0D		// Previous character
for( int i=0x403F; i<0x404C; i++ ) crc_fputc( 0x00, htp, crc );
    crc = crc_fputc( program_protection_404C, htp, crc );                // 404C        :   10		// Security flag : Programvédelem 00=védi a programot, 10=nem védi
    // crc = write_word( 0*0x0, htp, crc );                              // 404D - 404E :    0 // ???
    // crc = crc_fputc( 0*0xD1, htp, crc );                              // 404F        :   D1		// Error code
    // crc = write_word( 0*0xC350, htp, crc );                           // 4050 - 4051 : C350		// Line number at Error
for( int i=0x404D; i<0x4052; i++ ) crc_fputc( 0x00, htp, crc );

    crc = write_word( 0x0CA9, htp, crc );                           // 4052 - 4053 : 0CA9		// Vector for interpreter 
    crc = write_word( 0x0E18, htp, crc );                           // 4054 - 4055 : 0E18		// Vector for listing
    crc = write_word( 0x1581, htp, crc );                           // 4056 - 4057 : 1581		// Vector for Error              // 4056 : 81 15 
    for( int i=0x4058; i<0x40A0; i++ ) crc_fputc( 0x00, htp, crc ); // 4058 - 406F :    0		// Stack for display generator   // 4058 : E9 FF E9 FB E9 F7 E9 F3 E9 EF E9 EB E9 E7 E9 E3 85 4D F2 02 76 4D 04 60 

    crc = encodeBasicFrom40A0( htp, txt, crc );
    // Byte counter contains correct value

    crc = crc_fputc( 0x80, htp, crc );
    crc = crc_fputc( 0x60, htp, crc );
    crc = crc_fputc( 0x0, htp, crc );

    long int end_position = ftell( htp );

    // Back to the payload length_
    fseek( htp, payload_length_position, SEEK_SET );
    fputc( byte_counter % 256, htp );
    fputc( byte_counter / 256, htp ); // Ezek után a byte_counter értke már nem számít, a későbbi write_word már módosíthatja. Ott már csak a crc a fontos!

    // Back to dcb header
    uint16_t end_address = load_address + byte_counter - 1;
    fseek( htp, dcb18_position, SEEK_SET );

    crc = write_word( end_address, htp, crc ); // Write DCB+0x18	// Start variables
    crc = write_word( end_address+1, htp, crc ); // Write DCB+0x1A	// Start string variables
    crc = write_word( end_address+2, htp, crc ); // Write DCB+0x1C	// End of string space

    // Back to end of file
    fseek( htp, end_position, SEEK_SET );
    fputc( crc, htp );
    // Write the last 0 byte
    fputc( 0, htp );
}

/********************************************************************************************************************
 * String functions
 ********************************************************************************************************************/

void copy_to_name( char* basename ) {
    int i = 0;
    for( i = 0; i<16 && basename[ i ]; i++ ) htp_name[ i ] = basename[ i ];
    for( int j = i; j < 17; j++ ) htp_name[ j ] = 0;
}

char* copyStr( char *str, int chunkPos ) {
    int size = 0;
    while( str[size++] );
    char *newStr = malloc( size );
    for( int i=0; i<size; i++ ) newStr[i] = str[ i ];
    if ( chunkPos>0 && chunkPos<size ) { // Chunk extension
        if ( newStr[ size - chunkPos ] == '.' ) newStr[ size - chunkPos ] = 0;
    }
    return newStr;
}

char* copyStr3( char *str1, char *str2, char *str3 ) {
    int size1 = 0; while( str1[size1++] );
    int size2 = 0; while( str2[size2++] );
    size2--;
    int size3 = 0; while( str3[size3++] );
    char *newStr = malloc( size1 + size2 + size3 );
    for( int i=0; i<size1; i++ ) newStr[ i ] = str1[ i ];
    if ( size1 ) newStr[ size1 - 1 ] = '/';
    for( int i=0; i<size2; i++ ) newStr[ size1 + i ] = str2[ i ];
    for( int i=0; i<size3; i++ ) newStr[ size1 + size2 + i ] = str3[ i ];
    return newStr;
}
                
/********************************************************************************************************************
 * Main functions
 ********************************************************************************************************************/

void print_usage() {
    printf( "H2Bas2Htp v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "Copyright 2023 by László Princz\n");
    printf( "Create a new .htp file from Aircomp 16 (Homelab 2) BASIC txt source file.\n");
    printf( "Usage:\n");
    printf( "h2bas2htp [-svl] -i <source_bas_txt_file> -n <name_on_tape> -o <htp_filename>\n");
    printf( "-i source_bas_txt_file : TXT file contains BASIC source code with line numbers.\n");
    printf( "-o htp_file_name       : Name of the new htp file.\n");
    printf( "-n name_on_tape        : The tape name. Default name created from ptp filename.\n");
    printf( "-s                     : skip BASIC comments with this first character. ''=skip all comments. 'no'=disable comment skipping. Default value is '!'\n");
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

            case 'i': // open pp file
                if ( !( txt = fopen( optarg, "rb" ) ) ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
                }
                srcBasename = copyStr( basename( optarg ), 4 );
                destDir = copyStr( dirname( optarg ), 0 );
                if ( !htp_name[ 0 ] ) copy_to_name( srcBasename );
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
        create_htp_from( htp, txt );
        fclose( htp );
    } else {
        print_usage();
    }
    return 0;
}
