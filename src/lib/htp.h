#include <stdint.h>

/**
 * A HTP fájl szerkezete:
 * - Első htp rekord
 *     256 x 0 bájt
 *     0xA5
 *     név karakterek, maximum 65, de lehet 0 darab is, azonban nem tartalmazhat 0 értékű bájtot
 *     név rekordot lezáró 0 bájt
 *     loadAddress low
 *     loadAddress high
 *     byte_counter low
 *     byte_counter high
 *     /// Innentől kezdődően a kiírt karakterek értékét egy CRC-ben összegezzük, aminek az alsó bájtját a végén kiírjuk
 *     bytes payload
 *     crc byte
 *     0, ha ez volt az utolsó rekord, és 1, ha jönnek még tovűábbi rekordok
 * - Következő rekord
 *     kevesebb bevezető 0, és a név lehet üres, de a lezáró 0 kell. különben ugyanaz
 */


void write_fix_htp_header( FILE *htp, int prefix_counter, uint16_t load_address, uint16_t byte_counter, const char *htp_name );
void write_htp_header( FILE *htp, int counter, uint16_t load_address, const char *htp_name );
// void write_htp_load_address_and_size_placeholder( FILE *htp, uint16_t load_address );
void crc_fputc( char byte, FILE *f );
void crc_write_word( uint16_t word, FILE *htp );
void close_fix_htp_block( FILE *htp, int is_last_block ); // crc and next block bytes only
void close_htp_block( FILE *htp, int is_last_block ); // with fseek
void close_htp_file( FILE *htp );
// void write_htp_block_name( FILE *htp, const char *htp_name );
void htp_dec_crc_and_byte_counter( unsigned char byte );
uint16_t htp_get_byte_counter();
