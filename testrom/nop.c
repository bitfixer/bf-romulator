#include <stdio.h>

int main(int argc, char** argv)
{
    // generate a binary file containing only NOP instructions
    // 0xEA
    int i;
    unsigned char nopinstr;
    nopinstr = 0xEA;
    for (i = 0; i < 65536; i++)
    {
        fwrite(&nopinstr, 1, 1, stdout);
    }
}