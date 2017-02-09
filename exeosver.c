/*
 * Copyright 2017 Cameron Hall
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define OFFSET_PE_HEADER 0x3C
#define OFFSET_OSVERMAJOR 0x40
#define OFFSET_OSVERMINOR 0x42
#define OFFSET_SUBSYSVERMAJOR 0x48
#define OFFSET_SUBSYSVERMINOR 0x4A

static int versionMajor = 4;
static int versionMinor = 0;

static bool parse_version_number(char *str)
{
    versionMajor = strtol(str, &str, 10);
    if (str == NULL)  /* Failed to parse number */
        return false;
    if (*str == '.')
    {
        str++;
        versionMinor = strtol(str, &str, 10);
        if (str == NULL)  /* Failed to parse number */
            return false;
    }
    else  /* Bad character after major version number */
        return false;
    
    return true;
}

static void serialize16(uint8_t *ptr, uint16_t value)
{
    ptr[0] = value & 0xFF;
    ptr[1] = (value >> 8) & 0xFF;
}

static uint32_t deserialize32(const uint8_t *ptr)
{
    return (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | (ptr[0]);
}

int main(int argc, char **argv)
{
    const char *filename;
    FILE *file;
    uint8_t buffer[0x50];
    uint32_t peHeaderOffset;
    
    /* Get parameters */
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s 'file' 'os_version'\nwhere 'file' is a "
          "Windows .exe file you wish to patch and 'os_version' is the version "
          "of Windows you wish to target. If not specifed, 'os_version' "
          "defaults to 4.0.\n", argv[0]);
        return 1;
    }
    filename = argv[1];
    if (argc >= 3)
    {
       if (!parse_version_number(argv[2]))
       {
           fputs("Invalid OS version specified.\n", stderr);
           return 1;
       }
    }
    printf("Patching required OS version to %i.%i.\n", versionMajor, versionMinor);
    
    /* Open file */
    file = fopen(filename, "rb+");
    if (file == NULL)
    {
        fprintf(stderr, "Failed to open file %s: %s\n", filename, strerror(errno));
        return 1;
    }
    
    /* Read DOS header and verify file signature */
    fseek(file, 0, SEEK_SET);
    fread(buffer, 1, sizeof(buffer), file);
    if (buffer[0] != 'M' || buffer[1] != 'Z')
    {
        fputs("This is not a valid Windows executable (invalid MZ signature).\n", stderr);
        fclose(file);
        return 1;
    }
    
    /* Read Windows header and verify signature */
    peHeaderOffset = deserialize32(buffer + 0x3C);
    fseek(file, peHeaderOffset, SEEK_SET);
    fread(buffer, 1, sizeof(buffer), file);
    if (deserialize32(buffer) != 0x00004550)
    {
        fputs("This is not a valid Windows executable (invalid PE signature).\n", stderr);
        fclose(file);
        return 1;
    }
    
    /* Write new OS and subsystem version fields */
    serialize16(buffer + OFFSET_OSVERMAJOR, versionMajor);
    serialize16(buffer + OFFSET_OSVERMINOR, versionMinor);
    serialize16(buffer + OFFSET_SUBSYSVERMAJOR, versionMajor);
    serialize16(buffer + OFFSET_SUBSYSVERMINOR, versionMinor);
    fseek(file, peHeaderOffset, SEEK_SET);
    fwrite(buffer, 1, sizeof(buffer), file);
    fclose(file);
    puts("File successfully patched.");
    return 0;
}
