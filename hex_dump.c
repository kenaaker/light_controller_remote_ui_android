/* File hex_dump.c created by Ken Aaker on Fri Aug  8 2003. */
#include <stdio.h>
#include "hex_dump.h"

#ifdef __cplusplus
"C" {
#endif
    void hex_dump(const char *function_name, const int line_number, const char *title, const void *mem, const int len) {

        unsigned short i;
        unsigned char *memByte;
        unsigned char *printBytes;
        int print_column_start;
        int cur_print_column;

        printf("%s:%d %s at %p for %d bytes\n", function_name, line_number, title, mem, len);
        if (mem != NULL) {
            printBytes = (void *)mem;
            print_column_start = (16 * 2) + 4;

            cur_print_column = print_column_start;
            for (i = 0, memByte = (unsigned char *)mem; i < len; ++i, ++memByte) {
                if ((i & 15) == 0) { /* new line each 16 bytes */
                    if (i != 0) {    /* But skip the line end for 0. */
                        printf(" |");
                        while (printBytes < memByte) {
                            if ((*printBytes > ' ') && (*printBytes < 'z')) {
                                printf("%c", *printBytes);
                            } else {
                                printf("%c", '.');
                            } /* endif */
                            ++printBytes;
                        } /* endwhile */
                        printf("|\n");
                    } /* endif */
                    printf("%p 0x%04x |", mem + i, i);
                    cur_print_column = print_column_start;
                }                   /* endif */
                if ((i & 3) == 0) { /* a blank each 4 bytes */
                    printf(" ");
                    --cur_print_column;
                } /* endif */
                printf("%02x", *memByte);
                cur_print_column -= 2;
            } /* endfor */
            /* print some blanks to align the characters with the other character columns */
            for (; cur_print_column > 0; --cur_print_column) {
                printf(" ");
            }
            printf(" |");
            while (printBytes < memByte) {
                if ((*printBytes > ' ') && (*printBytes < 'Z')) {
                    printf("%c", *printBytes);
                } else {
                    printf("%c", '.');
                } /* endif */
                ++printBytes;
            } /* endwhile */
            printf("|\n");
        }
    }
#ifdef __cplusplus
}
#endif

