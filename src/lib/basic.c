/**
 * Homelab 2 (Aircomp 16) basic functions
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "htp.h"
#include "fs.h"
#include "basic.h"
#include "params.h"
#include "labels.h"
#include <ctype.h>

#define BASIC_START 0x40A0
#define MAX_LINE_LENGTH 1000

int last_line_number = 0; // For auto line number

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
// , 0x44, 0xc4, 0x43, 0xce, 0x50, 0xd0, 0x4f, 0xc4, 0x55, 0xc6, 0x53, 0xce, 0x54, 0xcd, 0x55, 0xd3, 0x2f, 0xb0, 0x49, 0xd1, 0x4f, 0xcd, 0x4f, 0xd6, 0x53, 0xcc, 0x42, 0xd3, 0x4f, 0xcb, 0x45, 0x52, 0x52, 0x4f, 0
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
    int current_token = 127; // A tokenszavak utolsó karakterének 7. bitje 1. A token azt mondja meg, hogy hányadik(+127) tokenszó a keresett. Az első szó az "^" Ennek kódja 128.
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

            if ( token_character == toupper( string[ match_length ] ) ) { // A keresett karakter jó
                match_length++; // Eggyel több illeszkedik
                if ( last_token_character ) { // Ez volt a token utolsó karaktere, megvagyunk
                    *string_length = match_length; // Az illesztett szó hossza
                    if ( verbose ) {
                        printf( "Found token '" );
                        for( int p=0; p<match_length; p++ ) printf( "%c", toupper( string[ p ] ) );
                        printf( "' code=%02X (%d)\n", current_token, current_token );
//printf( "Found new toke for '%s' (length=%d from pos=%d): ", string, match_length, i-match_length+1 );
//printf(" - %02X\n", current_token );
                    }
                    return current_token; // Ez a token kódja
                } else if ( match_length >= *string_length ) { // Nincs több illeszthető keresett karakter, ez nem token
//exit(1);
                    return 0;
                } // else { // Különben folytatjuk.
            } else { // Nem illeszkedő karakter
                match_length = 0; // Kezdjük előről
                if ( !last_token_character ) skip = 1;
            }
        }
    }
//exit(1);
    return 0;
}

int drop_comment( char * line, int line_length ) {
    for( int i=0; i<line_length; i++ ) {
        if ( comment_skip_enabled && ( skip_comments_character == 0 ) && ( line[i] == 0x27 ) && ( line[ i+1 ]== 0x22 ) ) { // 0x27=' és 0x22="
            line_length = i;
        } else if ( comment_skip_enabled && ( line[i] == 0x27 ) && ( line[ i+1 ]== 0x22 ) && ( line[ i+2 ]== skip_comments_character ) ) { // '"! mindenképp kihagyásra kerül!
            line_length = i;
        }
    }
    line[ line_length ] = 0;
    return line_length;
}

int get_line_from( FILE *f, unsigned char* line, uint16_t *line_number, char* label, int max_line_length ) {
    *line_number = 0;
    label[0] = 0;
    int label_length = 0;
    int read_number = 1; // 1=read line number, 2=read label
    int line_length = 0;
    for ( unsigned char c = fgetc( f ); !feof( f ) && c!=10 && c!=13; c = fgetc( f ) ) {
        if ( read_number == 1 ) {
            if ( c>='0' && c<='9' ) {
                *line_number = *line_number * 10 + c - '0';
            } else if ( !*line_number && c <=' ' ) { // SKIP prefix spaces
            } else if ( c==' ' || c==9 ) { // SKIP line basic prefix spaces
            } else if ( c=='{' ) { // Begin label
                read_number = 2;
            } else { // BASIC text line
                read_number = 0;
            }
        } else if ( read_number == 2 ) { // Read label
            if ( c=='}' ) { // End label
                read_number = 1;
            } else if ( c > 32 ) {
                label[ label_length++ ] = c;
                if ( label_length >= MAX_LABEL_LENGTH ) label_length--;
                label[ label_length ] = 0;
            }
        }
        if ( !read_number ) {
            line[ line_length++ ] = c;
            if ( line_length == max_line_length ) {
                fprintf( stderr, "Line to length!\n" ); exit(1);
            }
        }
    }
    line_length = drop_comment( line, line_length );
    while( line_length && ( line[ line_length-1 ]==32 || line[ line_length-1 ]==9 ) ) line[ --line_length ] = 0; // Rtrim
    return line_length;
}

void dollar1( FILE *htp, unsigned char byte1 ) {
    long int pos = ftell( htp );
    long int back_pos = pos-1;
    fseek( htp, back_pos, SEEK_SET );
    htp_dec_crc_and_byte_counter( byte1 );
    byte1 = byte1 | 32;
    crc_fputc( byte1, htp );
    fseek( htp, pos, SEEK_SET );
}

void dollar2( FILE *htp, unsigned char byte2 ) {
    long int pos = ftell( htp );
    long int back_pos = pos-2;
    fseek( htp, back_pos, SEEK_SET );
    htp_dec_crc_and_byte_counter( byte2 );
    byte2 = byte2 | 32;
    crc_fputc( byte2, htp );
    fseek( htp, pos, SEEK_SET );
}

void bol64( FILE *htp, unsigned char byte1 ) {
    long int pos = ftell( htp );
    long int back_pos = pos-1;
    fseek( htp, back_pos, SEEK_SET );
    htp_dec_crc_and_byte_counter( byte1 );
    byte1 -= 64;
    crc_fputc( byte1, htp );
    fseek( htp, pos, SEEK_SET );
}

int read_label_write_number( const char* line, int i, int line_length, FILE *htp ) { // Címke beolvasása, és a htp fájlba a hozzá tartozó sorszám kiírása
    if ( line[ i ] == '{' ) {
        char label[ MAX_LABEL_LENGTH ];
        int j = 0;
        for( i++; ( i < line_length ) && ( line[i] != '}' ); i++ ) { // i = next_char start position
            label[ j++ ] = line[i];
        }
        label[j]=0;
        uint16_t line_number = get_line_number( label );
        char numstr[10] = { 0 };
        sprintf( numstr, "%d", line_number );
        for( int i=0; numstr[i] && ( i < 10 ); i++ ) crc_fputc( numstr[i], htp );
        return i;
    } else {
        fprintf( stderr, "Internal error 1" );
        exit(4);
    }
}

#define CODE_MODE 0
#define SPACE_MODE 1
/**
 * Egy sor elkódolás sorfolytonos. Ha van TOKEN, akkor annak heylére a kódja kerül, különben maradnak a karakterek.
 * A karakterek 7. bitje mindig 0, a tokenek 7. bitje mindig 1.
 * A string változók első karakterének 5. bitje is 1, cserébe a $ nem kerül tárolásra.
 */
char encode_line( FILE *htp, char* line, int line_length ) {
    unsigned char token = 0;
    int mode = CODE_MODE; // CODE módban a kód értelmeződik tokenekre és egyéb paraméterekre és a space lenyelődik, de SPACE modban nincs értelmezés, olyankor a SPACE is kiírásra kerül.
    int varNameCharCounter = 0; // Azt számolja, hogy az utolsó token óta hány konverziómentes karakter került kiírásra
    char back_chars[2] = { 0 }; // Az utoljára kiírt nem konvertált karakter az 1. indexen van, az előtte kiírt a 0. indexen
    for( int i = 0; i < line_length; i++ ) { // i = next_char start position
        char current_char = line[ i ];
        if ( mode == CODE_MODE ) {
            current_char = toupper( current_char );
            int current_char_start_position = i;
            int length = line_length-i; // Hátralévő hossz
            if ( token = get_token( line+current_char_start_position, &length ) ) { // Az i. indextől kezdve van egy token
                    crc_fputc( token, htp );
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
            } else {  // i. index egy az egyben másolandó, mivel nem token. Kivéve, ha valamilyen speciális karakter, mint a $ vagy a (
                    if ( current_char == 0x24 ) { // '$' character. Ez speciális jel, mivel nem kerül karakterként tárolásra, de módosítja a változó eltárolt azonosítóját
                        if ( varNameCharCounter == 1 ) {
                            dollar1( htp, back_chars[ 1 ] );
                        } else if ( varNameCharCounter > 1 ) {
                            if ( ( line[ i-2 ] < 'A' ) || ( line[i-2] > 'Z' ) ) {
                                fprintf( stderr, "Invalid string variable name char: '%c'!\n", line[i-2] );
                                exit(4);
                            }
                            dollar2( htp, back_chars[ 0 ] );
                        } else {
                            fprintf( stderr, "Invalid string start!\n" );
                            exit(4);
                        }
                    } else if ( current_char == 0x28 ) { // '(' Ez speciális jel, mivel nem kerül tárolásra, de listázáskor megjelenik
                        if ( varNameCharCounter == 1 ) {
                            if ( ( back_chars[ 1 ] < 'A' ) || ( back_chars[ 1 ] > 'Z' ) ) {
                                fprintf( stderr, "Invalid DIM variable name char: '%c'!\n", back_chars[ 1 ] );
                                exit(4);
                            }
                            crc_fputc( 31, htp );
                            varNameCharCounter = 0;
                        } else if ( varNameCharCounter > 1 ) {
                            bol64( htp, back_chars[ 1 ] );
                        } else {
                            crc_fputc( current_char, htp );
                        }
                    } else if ( current_char == '{' ) { // Label kezdete
                        i = read_label_write_number( line, i, line_length, htp ); // Címke beolvasása, és a htp fájlba a hozzá tartozó sorszám kiírása
                        varNameCharCounter = 0;
                    } else if ( current_char != 32 ) {
                        crc_fputc( current_char, htp );
                        back_chars[ 0 ] = back_chars[ 1 ];
                        back_chars[ 1 ] = current_char;
                        if ( ( current_char >= 'A' ) && ( current_char <= 'Z' ) ) {
                            varNameCharCounter++;
                        } else if ( current_char >= 'a' ) {
                            fprintf( stderr, "Invalid source character: '%c'\n", current_char );
                            exit(4);
                        } else {
                            varNameCharCounter = 0;
                        }
                        if ( current_char == 0x22 ) mode = SPACE_MODE;
                    }
            }
        } else if ( mode == SPACE_MODE ) {
            crc_fputc( current_char, htp );
            if ( current_char == 0x22 ) mode = CODE_MODE;
        }
    }
}

void preload_labels( FILE *txt ) {
    clear_labels();
    unsigned char line[ MAX_LINE_LENGTH+1 ]; // +1 a lezáró 0 miatt, amire nem lenne szükség, de a biztosnág kedvéért inkább mindig odatesszük.
    unsigned char label[ MAX_LABEL_LENGTH ];
    uint16_t line_number = 0;
    while( !feof( txt ) ) {
        int line_length = 0;
        if ( line_length = get_line_from( txt, line, &line_number, label, MAX_LINE_LENGTH ) ) {
            if ( !line_number ) line_number = last_line_number + 10;
            if ( label[0] ) set_label( label, line_number );
            last_line_number = line_number;
        }
    }
    fseek( txt, 0, SEEK_SET );
    last_line_number = 0;
}

// Line: NextRowAddrL NextRowAddrH NumL NumH tokenized 0
// Program end: 0 0
void encodeBasicFrom40A0( FILE *htp, FILE *txt ) { // Encode text into src.bytes
    preload_labels( txt );
    uint16_t next_line_addr = BASIC_START;
    uint16_t line_number = 0;
    unsigned char line[ MAX_LINE_LENGTH+1 ];
    unsigned char encoded_line[ MAX_LINE_LENGTH ];
    unsigned char label[ MAX_LABEL_LENGTH ];
    while( !feof( txt ) ) {
        int line_length = 0;
        if ( line_length = get_line_from( txt, line, &line_number, label, MAX_LINE_LENGTH ) ) {
            if ( label[0] ) line_number = get_line_number( label );
            if ( !line_number ) line_number = last_line_number + 10;
// printf( "%d %s\n", line_number, line );
            last_line_number = line_number;
            crc_fputc( line_number / 256, htp );
            crc_fputc( line_number % 256, htp );
            line[ line_length ] = 0;
            if ( verbose ) printf( "Convert line %d '%s'\n", line_number, line );
            encode_line( htp, line, line_length );
            crc_fputc( 0x60, htp );
        } else {
            if ( verbose ) printf( "Skip empty source line\n" );
        }
    }
}

void write_htp_basic_payload( FILE *htp, FILE *txt, uint16_t load_address ) {
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Write DCB header
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    crc_write_word( 0x8000, htp )   ;                           // 4016 - 4017 : 8000<-><------>// HM
    long int dcb18_position = ftell( htp );                     //
    // Write 0x4018-0x40A0 DCB18- placeholder                   //
    for( int i=0; i<6; i++ ) fputc( 0, htp );       // Placeholder for:
                                                                // 4018-4019   :    0<-><------>// Start variables
                                                                // 401A-401B   :    0<-><------>// Start string variables
                                                                // 401C-401D   :    0<-><------>// End of string space

//     crc_write_word( 0x0004, htp );                           // 401E-401F   : 0004<-><------>// Primary acc
//     crc_write_word( 0x4A85, htp );                           // 4020-4021   : 4A85<-><------>// Primary acc
//     crc_write_word( 0xFE9B, htp );                           // 4022-4023   : FE9B<-><------>// Random number
//     crc_write_word( 0x2F71, htp );                           // 4024-4025   : 2F71<-><------>// Random number
//     crc_write_word( 0xFEFE, htp );                           // 4026-4027   : FEFE // ???
//     crc_write_word( 0x0009, htp );                           // 4028-4029   : 0009<-><------>// Auxiliary Accumulator
//     crc_write_word( 0x8133, htp );                           // 402A-402B   : 8133<-><------>// Auxiliary Accumulator
//     crc_write_word( 0x5D4F, htp );                           // 402C-402D   : 5DF4<-><------>// Temporary register
    for( int i=0x401E; i<0x402E; i++ ) crc_fputc( 0x00, htp );

    crc_write_word( 0x7FFF, htp );                           // 402E-402F   : 7FFF<-><------>// Current BASIC Line
    crc_write_word( 0x40A0, htp );                           // 4030 - 4031 : 40A0<----><------>// Start of BASIC program

//    crc_fputc( 0x00, htp );                                  // 4032        :   01<-><------>// BASIC FLAG 5.bit=CR
//     crc_write_word( 0xFEFE, htp );                           // 4033-4034   : FEFE<-><------>// Start of graphics
//     crc_write_word( 0x41A3, htp );                           // 4035-4036   : 41A3<-><------>// Statement pointer for CONT
//     crc_write_word( 0x40A0, htp );                           // 4037-4038   : 40A0<-><------>// Next item for READ
//     crc_write_word( 0x00DC, htp );                           // 4039-403A   : 00DC<-><------>// Current line for CONT
//     crc_write_word( 0x8000, htp );                           // 403B-403C   : 8000<-><------>// Start of graphics area in RAM
    for( int i=0x4032; i<0x403D; i++ ) crc_fputc( 0x00, htp );
    crc_fputc( 0x80, htp );                              // 403D        :   80<><------>// HM Page : HM high byte
    crc_fputc( program_autostart_403E, htp );                              // 403E        :    0<------><------>// BASIC interrupt és autostart? 1=tilt, 0=engedélyez

//     crc_fputc( 0*0x1D, htp );                              // 403F        :   1D<---><------>// Counter for cursor flashing
//     crc_fputc( 0*0x00, htp );                              // 4040        :    0 // ???
//     crc_write_word( 0x7FA4, htp );                           // 4041-4042   : 7FA4<-><------>// Auxiliary stack pointer
//     crc_fputc( 0*0x01, htp );                              // 4043        :   01<---><------>// RST 8
//     crc_fputc( 0*0x00, htp );                              // 4044        :    0<---><------>// RST 8
//     crc_fputc( 0*0x00, htp );                              // 4045        :    0<---><------>// RST 8
//     crc_fputc( 0*0x00, htp );                              // 4046        :    0<---><------>// RST 10
//     crc_fputc( 0*0x01, htp );                              // 4047        :   01<---><------>// RST 10
//     crc_fputc( 0*0x00, htp );                              // 4048        :    0<---><------>// RST 10
//     crc_write_word( 0x1581, htp );                           // 4049 - 404A : 1581 vagy 1582?<------>// Error vector
//     crc_fputc( 0*0x0D, htp );                              // 404B        :   0D<---><------>// Previous character
    for( int i=0x403F; i<0x404C; i++ ) crc_fputc( 0x00, htp );

    crc_fputc( program_protection_404C, htp );                // 404C        :   10<---><------>// Security flag : Programvédelem 00=védi a programot, 10=nem védi

//     crc_write_word( 0*0x0, htp );                              // 404D - 404E :    0 // ???
//     crc_fputc( 0*0xD1, htp );                              // 404F        :   D1<---><------>// Error code
//     crc_write_word( 0*0xC350, htp );                           // 4050 - 4051 : C350><------>// Line number at Error
    for( int i=0x404D; i<0x4052; i++ ) crc_fputc( 0x00, htp );

    crc_write_word( 0x0CA9, htp );                           // 4052 - 4053 : 0CA9<----><------>// Vector for interpreter.
    crc_write_word( 0x0E18, htp );                           // 4054 - 4055 : 0E18<----><------>// Vector for listing
    crc_write_word( 0x1581, htp );                           // 4056 - 4057 : 1581<----><------>// Vector for Error              // 4056 : 81 15.
    for( int i=0x4058; i<0x40A0; i++ ) crc_fputc( 0x00, htp ); // 4058 - 406F :    0<--><------>// Stack for display generator   // 4058 : E9 FF E9 FB E9 F7 E9 F3 E9 EF E9 EB E9 E7 E9 E3 85 4D F2 02 76 4D 04 60

    encodeBasicFrom40A0( htp, txt );

    // Next the BASIC close line. Size is 3 bytes:
    crc_fputc( 0x80, htp );
    crc_fputc( 0x60, htp );
    crc_fputc( 0x0, htp );
    // Back to dcb header
    uint16_t end_address = load_address + htp_get_byte_counter() - 1 + 4;
    fseek( htp, dcb18_position, SEEK_SET );
    crc_write_word( end_address, htp );		// 4018 Write DCB+0x18<----->// Start variables
    crc_write_word( end_address+1, htp );	// 401A Write DCB+0x1A<--->// Start string variables
    crc_write_word( end_address+2, htp );	// 401C Write DCB+0x1C<--->// End of string space
// B644 B744 B844
// BC44 E744 E844

// B240 B340 B440 -> B840 B940 BA40
}
