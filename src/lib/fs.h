/**
 * Filesystem
 */

#include <stdint.h>

/**
 * Lemásolja a forrás stringet egy új memóriaterületre, úgy, hogy levágja a string végét
 */
void dropExtIfExists( char *str );
char* copyStr( char *str, int chunkPos );
char* copyStr3( char *str1, char *str2, char *str3 );
int is_dir( const char *path );
char is_ext( const char *filename, const char *ext );
const char* insert_str_before_last_point( const char *filename, const char *str );
