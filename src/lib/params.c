#include "params.h"

int verbose = 0;
int comment_skip_enabled = 1;
char skip_comments_character = '!';
unsigned char program_protection_404C = 0x10; // Ha ez 00, akkor van programvédelem. 0x10 esetén nincs.
unsigned char program_autostart_403E = 0; // Ha ez 1, akkor a megszakítás tiltva van, és automatikusan elindul a program
