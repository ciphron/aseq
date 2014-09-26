/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * Quick and dirty tool to print out a region of bytes after
 * a sequence of bytes is matched in a file.
 *
 */


#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define BUF_SIZE 1024

inline unsigned char hex_value(unsigned char c)
{
    return (isdigit(c)) ? c - '0' : ((islower(c)) ? c - 'a' : c - 'A') + 10;
}

int hex_to_bin(const char *str, unsigned char *bin)
{
    size_t len = strlen(str);
    int i;
    

    for (i = 0; i < len - 1; i += 2) {
        if (!isxdigit(str[i]) || !isxdigit(str[i + 1]))
            return -1;
        *bin++ = (hex_value(str[i]) << 4) | hex_value(str[i + 1]);
    }

    if (i < len) {
        if (isxdigit(str[i]))
            *bin++ = hex_value(str[i]);
        else
            return -1;
    }

    return 0;                
}

void usage()
{
    printf("usage: aseq file_name hex_sequence num_bytes\n"
           "file_name:\t name of the file\n"
           "hex_sequence:\t byte sequence encoded as a hex string e.g: 042B\n"
           "\t\t represents the two bytes 0x04 and 0x2B\n"
           "num_bytes: \t the number of bytes to print after the sequence\n"
           "\t\t is matched\n");
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "No file specified\n");
        usage();
        exit(1);
    }

    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file\n");
        usage();
        exit(1);
    }

    unsigned char buf[BUF_SIZE];

    int len = (strlen(argv[2]) + 1) / 2;
    unsigned char *bin = (unsigned char*)malloc(len);

    if (hex_to_bin(argv[2], bin) < 0 || atoi(argv[3]) <= 0) {
        free(bin);
        fprintf(stderr, "invalid input - non-hex string specified\n");
        usage();
        exit(1);
    }
    
    int after_len = atoi(argv[3]);
    
    
    int count;
    int match_pos = 0;
    int after_remaining = 0;
    int file_offset = 0;
    int num_matches = 0;
    count = fread(buf, 1, BUF_SIZE, file);
    while (count > 0) {
        int pos = 0;

        while (pos < count && after_remaining > 0) {
            printf("%02x|", buf[pos]);
            pos++;
            after_remaining--;
        }
        if (pos > 0 && after_remaining == 0)
            printf("\nAFTER MATCH END\n");

        while (pos < count && after_remaining == 0) {
            int i = 0;
            while ((match_pos + i) < len && (pos + i) < count && bin[match_pos + i] == buf[pos + i]) {
                i++;
            }
            if ((match_pos + i) == len) {
                after_remaining = after_len;
                printf("MATCH %d (position=%d)\n", ++num_matches, file_offset + pos);
                while ((pos + i) < count && after_remaining) {
                    printf("%02x|", buf[pos + i]);
                    i++;
                    after_remaining--;
                }
                if (after_remaining == 0)
                    printf("\n-----------------\n");

                pos += i;
                match_pos = 0;
            }
            else if ((match_pos + i) < len && (pos + i) < count) {
                match_pos = 0;
                pos ++;
            }
            else {
                match_pos += i;
                pos += i;
            }
        }
        file_offset += count;
        count = fread(buf, 1, BUF_SIZE, file);
        
    }
            
            
    fclose(file);
    free(bin);
        
    return 0;
}
