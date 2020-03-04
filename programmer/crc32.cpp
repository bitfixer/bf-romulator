#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

uint32_t crc32_for_byte(uint32_t r) {
  for(int j = 0; j < 8; ++j)
    r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
  return r ^ (uint32_t)0xFF000000L;
}

void crc32(const void *data, size_t n_bytes, uint32_t* crc) {
    static uint32_t table[0x100];
    if(!*table)
    {
        fprintf(stderr, "generating table\n");
        for(size_t i = 0; i < 0x100; ++i) 
        {
            table[i] = crc32_for_byte(i);
        }
    }

    for(size_t i = 0; i < n_bytes; ++i)
    {
        *crc = table[(uint8_t)*crc ^ ((uint8_t*)data)[i]] ^ *crc >> 8;
    }
}

uint32_t* make_crc32_table() {
    uint32_t* table = new uint32_t[256];
    for (int i = 0; i < 256; i++)
    {
        table[i] = crc32_for_byte(i);
    }
    return table;
}

uint32_t crc32_byte(uint8_t byte, uint32_t* table, uint32_t crc) 
{
    uint32_t crc_new = table[(uint8_t)crc ^ byte] ^ (crc >> 8);
    return crc_new;
}

void write_byte(uint8_t byte, bool text_mode)
{
    if (text_mode) 
    {
        printf("%02X", byte);
    } 
    else 
    {
        fwrite(&byte, 1, 1, stdout);
    }
}

int main(int argc, char** argv) 
{
    int opt;
    bool write_table = false;
    bool text_mode = false;
    bool read_table = false;
    char table_fname[256];

    while ((opt = getopt(argc, argv, "tr:x")) != -1)
    {
        switch(opt)
        {
            case 't':
                write_table = true;
                break;
            case 'r':
                read_table = true;
                strcpy(table_fname, optarg);
                break;
            case 'x':
                text_mode = true;
                break;
        }
    }

    uint32_t* table;
    if (read_table)
    {
        table = new uint32_t[256];
        uint8_t byte;
        uint32_t entry;
        fprintf(stderr, "reading crc32 table from %s\n", table_fname);
        FILE* table_fp = fopen(table_fname, "rb");
        if (!table_fp)
        {
            fprintf(stderr, "could not open table file %s\n", table_fname);
            return 1;
        }

        for (int i = 0; i < 256; i++)
        {
            entry = 0;
            for (int b = 0; b < 4; b++)
            {
                fread(&byte, 1, 1, table_fp);
                entry <<= 8;
                entry += (uint32_t)byte;
            }
            table[i] = entry;
        }

        fclose(table_fp);
    }
    else
    {
        table = make_crc32_table();
    }

    for (int j = 0; j < 10; j++)
    {
        fprintf(stderr, "entry %d: %X\n", j, table[j]);
    }

    if (write_table)
    {
        for (int i = 0; i < 256; i++)
        {
            uint32_t entry = table[i];
            uint8_t byte;
            // output big endian
            byte = (uint8_t)((entry & 0xFF000000) >> 24);
            write_byte(byte, text_mode);

            byte = (uint8_t)((entry & 0x00FF0000) >> 16);
            write_byte(byte, text_mode);

            byte = (uint8_t)((entry & 0x0000FF00) >> 8);
            write_byte(byte, text_mode);

            byte = (uint8_t)(entry & 0x000000FF);
            write_byte(byte, text_mode);

            if (text_mode)
            {
                printf("\n");
            }
        }

        delete table;
        return 0;
    }

    FILE* fp = stdin;

    uint8_t byte;
    uint32_t crc = 0;
    while (!feof(fp) && !ferror(fp))
    {
        if (fread(&byte, 1, 1, fp))
        {
            crc = crc32_byte(byte, table, crc);
        }
    }

    fprintf(stderr, "%08x\n", crc);
    delete table;

/*
  FILE *fp;
  char buf[1L << 15];
  for(int i = ac > 1; i < ac; ++i)
    if((fp = i? fopen(av[i], "rb"): stdin)) { 
      uint32_t crc = 0;
      while(!feof(fp) && !ferror(fp))
        crc32(buf, fread(buf, 1, sizeof(buf), fp), &crc);
      if(!ferror(fp))
        printf("%08x%s%s\n", crc, ac > 2? "\t": "", ac > 2? av[i]: "");
      if(i)
        fclose(fp);
    }
    */
    
  return 0;
}