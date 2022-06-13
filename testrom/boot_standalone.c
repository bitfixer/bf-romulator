
void main()
{
    unsigned char* a = (unsigned char*)0x8000;
    unsigned char* tmp = (unsigned char*)0x7FFF;
    unsigned char v = 0;

    for (a = (unsigned char*)0x8000; a < (unsigned char*)0x8800; a += 0x01)
    {
        *a = v;
        v++;
    }

    *tmp = 0xdd;
    while (1) {}
}