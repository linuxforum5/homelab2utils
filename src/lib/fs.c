/**
 * Filesystem
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"
#include <libgen.h>
#include "fs.h"

void dropExtIfExists( char *str ) {
    while( *str!=0 ) {
        if ( *str == '.' ) {
            *str = 0;
        } else {
            str++;
        }
    }
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

int is_dir( const char *path ) {
    struct stat path_stat;
    stat( path, &path_stat );
    return S_ISDIR( path_stat.st_mode );
}

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

char is_ext( const char *filename, const char *ext ) {
    int fileNameSize = 0;
    while( filename[ fileNameSize ] ) fileNameSize++;
    int extSize = 0;
    while( ext[ extSize ] ) extSize++;
    while( extSize && fileNameSize && ext[ extSize ] == filename[ fileNameSize ] ) {
        extSize--;
        fileNameSize--;
    }
    return !extSize;
}

const char* insert_str_before_last_point( const char *filename, const char *str ) {
    int fnSize = strlen( filename );
    int strSize = strlen( str );
    if ( fnSize && strSize ) {
        int newSize = fnSize + strSize + 2; // characters + prefix selector + end of string
        char *newStr = malloc( newSize );
        int newPos = newSize - 1;
        newStr[ newPos-- ] = 0; // End of new string
        int fnPos = fnSize - 1;
        int strPos = strSize - 1;
        while( fnPos>=0 && filename[ fnPos ] != '.' ) {
            newStr[ fnPos-- + strPos + 2 ] = filename[ fnPos ];
        }
        newStr[ fnPos-- + strPos + 2 ] = filename[ fnPos ]; // . másolása
        while( strPos>=0 ) {
            newStr[ fnPos + strPos-- + 2 ] = str[ strPos ];
        }
        newStr[ fnPos + 1 ] = '.'; // prefix selector
        while( fnPos>=0 ) {
            newStr[ fnPos-- ] = filename[ fnPos ];
        }
        return newStr;
    } else {
        return filename;
    }
}
