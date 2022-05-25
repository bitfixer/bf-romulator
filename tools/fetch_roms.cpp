// fetch_roms.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void usage()
{
    fprintf(stderr, "usage: fetch_roms <memory_set_csv_file> <baseurl>\n");
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        usage();
        exit(1);
    }

    char* csvfile = argv[1];
    char* baseurl = argv[2];
    char line[256];
    char rom_prefix[256];
    char romname[1024];
    char cmd[2048];

    FILE* fp_csv = fopen(csvfile, "rb");
    while (fgets(line, sizeof(line), fp_csv))
    {
        // skip empty lines
        if (strlen(line) < 2)
        {
            continue;
        }

        if (line[0] == '#' ||
            (line[1] == '/' && line[2] == '/'))
        {
            continue;
        }

        char* token = strtok(line, ",");
        
        // first field is set index
        int index = atoi(token);
        sprintf(romname, "%s%s", rom_prefix, strtok(NULL, ","));

        token = strtok(NULL, ",");
        uint32_t address;
        sscanf(token, "0x%X", &address);
        
        // create full rom url
        sprintf(cmd, "wget -nc %s%s", baseurl, romname);
        fprintf(stderr, "command: %s\n", cmd);
        system(cmd);
    }
}