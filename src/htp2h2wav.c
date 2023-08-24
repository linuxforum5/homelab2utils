
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>

unsigned char verbose = 0;

/**************************************************************************************************************************************************
 * Wav methodes
 **************************************************************************************************************************************************/
const unsigned int defaultBaud = 44100; // 8000; // max 13000
// uint16_t full_bytes_size = 0; // A kiírt hasznos bájtok száma
const unsigned char  SILENCE = 0x80;
const unsigned char POS_PEAK = 0xff; // 8;
const unsigned char NEG_PEAK = 0x00; // 8;

/* WAV file header structure */
/* should be 1-byte aligned */
#pragma pack(1)
struct wav_header { // 44 bytes
    char           riff[ 4 ];       // 4 bytes
    uint32_t       rLen;            // 4 bytes
    char           WAVE[ 4 ];       // 4 bytes
    char           fmt[ 4 ];        // 4 bytes
    uint32_t       fLen;            /* 0x1020 */
    uint16_t       wFormatTag;      /* 0x0001 */
    uint16_t       nChannels;       /* 0x0001 */
    uint32_t       nSamplesPerSec;
    uint32_t       nAvgBytesPerSec; // nSamplesPerSec*nChannels*(nBitsPerSample/8)
    uint16_t       nBlockAlign;     /* 0x0001 */
    uint16_t       nBitsPerSample;  /* 0x0008 */
    char           datastr[ 4 ];    // 4 bytes
    unsigned int   data_size;       // 4 bytes
} waveHeader = {
    'R','I','F','F', //     Chunk ID - konstans, 4 byte hosszú, értéke 0x52494646, ASCII kódban "RIFF"
    0,               //     Chunk Size - 4 byte hosszú, a fájlméretet tartalmazza bájtokban a fejléccel együtt, értéke 0x01D61A72 (decimálisan 30808690, vagyis a fájl mérete ~30,8 MB)
    'W','A','V','E', //     Format - konstans, 4 byte hosszú,értéke 0x57415645, ASCII kódban "WAVE"
    'f','m','t',' ', //     SubChunk1 ID - konstans, 4 byte hosszú, értéke 0x666D7420, ASCII kódban "fmt "
    16,              //     SubChunk1 Size - 4 byte hosszú, a fejléc méretét tartalmazza, esetünkben 0x00000010
    1,               //     Audio Format - 2 byte hosszú, PCM esetében 0x0001
    1,               //     Num Channels - 2 byte hosszú, csatornák számát tartalmazza, esetünkben 0x0002
    defaultBaud,     //     Sample Rate - 4 byte hosszú, mintavételezési frekvenciát tartalmazza, esetünkben 0x00007D00 (decimálisan 32000)
    defaultBaud,     //     Byte Rate - 4 byte hosszú, értéke 0x0000FA00 (decmálisan 64000)
    1,               //     Block Align - 2 byte hosszú, az 1 mintában található bájtok számát tartalmazza - 0x0002
    8,               //     Bits Per Sample - 2 byte hosszú, felbontást tartalmazza bitekben, értéke 0x0008
    'd','a','t','a', //     Sub Chunk2 ID - konstans, 4 byte hosszú, értéke 0x64617461, ASCII kódban "data"
    0                //     Sub Chunk2 Size - 4 byte hosszú, az adatblokk méretét tartalmazza bájtokban, értéke 0x01D61A1E
};
#pragma pack()

static unsigned char    p_gain = 60;
// const unsigned char     p_silence = 0;
// static unsigned int     wav_sample_count = 0;

void write_peaks( FILE *wav, int us, unsigned char value ) {
    int sample_length_us = 1000000 / waveHeader.nSamplesPerSec;
    int cnt = us / sample_length_us;
    for( int i = 0; i < cnt; i++ ) fputc( value, wav );
}

void write_bit_into_wav( FILE *wav, unsigned char bit ) {
    int sync_us = 100; // 48000Hz esetén 50-200-ig ok
    int space_us = 1600 - sync_us;
    unsigned char nulla = SILENCE; // NEG_PEAK
    if ( bit ) {
        write_peaks( wav, ( space_us - sync_us ) / 2, nulla );
        write_peaks( wav, sync_us, POS_PEAK );
        write_peaks( wav, ( space_us - sync_us ) / 2, nulla );
    } else {
        write_peaks( wav, space_us, nulla );
    }
    write_peaks( wav, sync_us, POS_PEAK );
}

static void close_wav( FILE *wav ) {
    int full_size = ftell( wav );
    fseek( wav, 4, SEEK_SET );
    fwrite( &full_size, sizeof( full_size ), 1, wav ); // Wave header 2. field : filesize with header. First the lowerest byte

    int data_size = full_size - sizeof( waveHeader );
    fseek( wav, sizeof( waveHeader ) - 4 ,SEEK_SET ); // data chunk size position: 40
    fwrite( &data_size, sizeof( data_size ), 1, wav );
    fclose( wav );
}

void write_byte_into_wav( FILE* wav, unsigned char c ) { // Bitkiírási sorrend: először a 7. bit legvégül a 0. bit
    for( int i = 0; i < 8; i++ ) {
        unsigned char bit = ( c << i ) & 128; // 7. bit a léptetés után. 1. lépés után az eredeti 6. bitje.
        write_bit_into_wav( wav, bit );
    }
}

static void init_wav( FILE *wavfile ) {
    unsigned int i;

    fwrite( &waveHeader, sizeof( waveHeader ), 1, wavfile );
    /* Lead in silence */
    write_peaks( wavfile, 2000, SILENCE );
}

static void process_htp( FILE *input, FILE* output ) {
    while ( !feof( input ) ) {
        unsigned char c = fgetc( input );
        // if ( !feof( input ) ) 
        write_byte_into_wav( output, c );
    }
    fclose( input );
//    write_byte_into_wav( output, 0 );
    write_peaks( output, 2000, SILENCE );
}    

static void print_usage() {
    printf( "Htp2A16Wav v1.0\n");
    printf( "Aircomp 16 Htp2Wav v1.0\n");
    printf( "Copyright 2023 by Laszlo Princz\n");
    printf( "Usage:\n");
    printf( "htp2a16wav -i <input_filename> -o <output_filename>\n");
    printf( "Command line option:\n");
    printf( "-f <freq> : Supported sample rates are: 48000, 44100, 22050, 11025 and 8000. Defaut is 44100Hz\n");
    printf( "-h        : prints this text\n");
    exit(1);
}    

int main(int argc, char *argv[]) {
    int finished = 0;
    int arg1, arg2;
    FILE *htp = 0, *wav = 0;

    while (!finished) {
        switch (getopt (argc, argv, "?hf:i:o:g:")) {
            case -1:
            case ':':
                finished = 1;
                break;
            case '?':
            case 'h':
                print_usage();
                break;
            case 'f':
                if ( !sscanf( optarg, "%i", &arg1 ) ) {
                    fprintf( stderr, "Error parsing argument for '-f'.\n");
                    exit(2);
                } else {
                    if ( arg1!=48000 && arg1!=44100 && arg1!=22050 && arg1!=11025 && arg1!=8000 ) {
                        fprintf( stderr, "Unsupported sample rate: %i.\n", arg1);
                        fprintf( stderr, "Supported sample rates are: 48000, 44100, 22050, 11025 and 8000.\n");
                        exit(3);
                    }
                    waveHeader.nSamplesPerSec = arg1;
                    waveHeader.nAvgBytesPerSec = arg1; // waveHeader.nSamplesPerSec*waveHeader.nChannels*(waveHeader.nBitsPerSample%8);
                }    
                break;
            case 'g':
                if ( !sscanf( optarg, "%i", &arg1 ) ) {
                    fprintf( stderr, "Error parsing argument for '-g'.\n");
                    exit(2);
                } else {
                    if ( arg1<0 || arg1>7 ) {
                        fprintf( stderr, "Illegal gain value: %i.\n", arg1);
                        fprintf( stderr, "Gain must be between 1 and 7.\n");
                    }
                    p_gain = arg1*0x0f;
                }    
                break;
            case 'i':
                if ( !(htp = fopen( optarg, "rb")) ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
                }
                break;
            case 'o':
                if ( !(wav = fopen( optarg, "wb")) ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
                }
                break;                
            default:
                break;
        }
    }
   
    if ( !htp ) {
        print_usage();
    } else if ( !wav ) {
        print_usage();
    } else {
        init_wav(wav);
        process_htp(htp, wav);
        close_wav( wav );
    }    
   
    return 0;
}