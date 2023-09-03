/**
 * Homelab 2 (Aircomp 16) basic functions
 */
#include <stdint.h>
#include <stdio.h>

//#define BASIC_BLOCK_LOAD_ADDRESS 0x4002
#define BASIC_BLOCK_LOAD_ADDRESS 0x4016

void list_tokens();
void write_htp_basic_payload( FILE *htp, FILE *txt, uint16_t load_address, FILE *BAS );
