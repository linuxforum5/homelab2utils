
#include <stdint.h>

void write_htp_header( FILE *htp, int counter, uint16_t load_address, const char *htp_name );
// void write_htp_load_address_and_size_placeholder( FILE *htp, uint16_t load_address );
void crc_fputc( char byte, FILE *f );
void crc_write_word( uint16_t word, FILE *htp );
void close_htp_block( FILE *htp, int is_last_block );
void close_htp_file( FILE *htp );
// void write_htp_block_name( FILE *htp, const char *htp_name );
void htp_dec_crc_and_byte_counter( unsigned char byte );
uint16_t htp_get_byte_counter();
