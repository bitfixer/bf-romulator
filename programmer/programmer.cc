// ROMulator - RAM/ROM replacement and diagnostic for 8-bit CPUs
// Copyright (C) 2019  Michael Hill

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// programmer.cc - programmer to update flash contents on ROMulator FPGA board

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#include <vector>

#  define PI_ICE_MISO       24
#  define PI_ICE_CLK        27
#  define PI_ICE_CDONE      3

#  define PI_ICE_CRESET     29
#  define PI_ICE_MOSI       28
#  define PI_ICE_CS         25

using namespace std;
#include <wiringPi.h>

bool verbose = false;
bool ftdi_verbose = false;

bool send_zero = false;
bool recv_zero = false;
char current_send_recv_mode = 0;
int current_recv_ep = -1;

int last_recv_v = -1;
int last_recv_rep = 0;

void fpga_reset()
{
    pinMode(PI_ICE_CRESET,  OUTPUT);
    digitalWrite(PI_ICE_CRESET, LOW);
    delayMicroseconds(2000);
    digitalWrite(PI_ICE_CRESET, HIGH);
    delayMicroseconds(500000);
    if (digitalRead(PI_ICE_CDONE) != HIGH)
        fprintf(stderr, "Warning: cdone is low\n");
}

int get_time_ms()
{
    static struct timespec spec_start;
    static bool spec_start_initialized = false;
    
    struct timespec spec_now;
    clock_gettime(CLOCK_REALTIME, &spec_now);
    if (!spec_start_initialized) {
        spec_start = spec_now;
        spec_start_initialized = true;
    }
    
    int s = spec_now.tv_sec - spec_start.tv_sec;
    int ns = spec_now.tv_nsec - spec_start.tv_nsec;
    
    return s*1000 + ns/1000000;
}

void prog_bitstream(bool reset_only = false)
{
    pinMode(PI_ICE_CLK,     OUTPUT);
    pinMode(PI_ICE_MISO,    OUTPUT);
    pinMode(PI_ICE_CRESET,  OUTPUT);
    pinMode(PI_ICE_CS,      OUTPUT);
    
    fprintf(stderr, "reset..\n");
    
    // enable reset
    digitalWrite(PI_ICE_CRESET, LOW);
    
    // start clock high
    digitalWrite(PI_ICE_CLK, HIGH);
    
    // select SRAM programming mode
    digitalWrite(PI_ICE_CS, LOW);
    delayMicroseconds(100);
    
    // release reset
    digitalWrite(PI_ICE_CRESET, HIGH);
    delayMicroseconds(2000);
    
    //fprintf(stderr, "cdone: %s\n", digitalRead(PI_ICE_CDONE) == HIGH ? "high" : "low");
    
    if (reset_only)
        return;
    
    fprintf(stderr, "programming..\n");
    
    while (1)
    {
        int byte = getchar();
        if (byte < 0)
            break;
        for (int i = 7; i >= 0; i--) {
            digitalWrite(PI_ICE_MISO, ((byte >> i) & 1) ? HIGH : LOW);
            digitalWrite(PI_ICE_CLK, LOW);
            digitalWrite(PI_ICE_CLK, HIGH);
        }
    }
    
    for (int i = 0; i < 49; i++) {
        digitalWrite(PI_ICE_CLK, LOW);
        digitalWrite(PI_ICE_CLK, HIGH);
    }
    
    delayMicroseconds(2000);
    
    fprintf(stderr, "cdone: %s\n", digitalRead(PI_ICE_CDONE) == HIGH ? "high" : "low");
}

void spi_begin()
{
    digitalWrite(PI_ICE_CS, LOW);
}

void spi_end()
{
    digitalWrite(PI_ICE_CS, HIGH);
}

uint32_t spi_xfer(uint32_t data, int nbits = 8)
{
    uint32_t rdata = 0;
    
    for (int i = nbits-1; i >= 0; i--)
    {
        digitalWrite(PI_ICE_MOSI, (data & (1 << i)) ? HIGH : LOW);
        
        if (digitalRead(PI_ICE_MISO) == HIGH)
            rdata |= 1 << i;
        
        digitalWrite(PI_ICE_CLK, HIGH);
        digitalWrite(PI_ICE_CLK, LOW);
    }
    
    return rdata;
}

uint32_t spi_xfer_slow(uint32_t data, int nbits = 8)
{
    uint32_t rdata = 0;
    
    for (int i = nbits-1; i >= 0; i--)
    {
        digitalWrite(PI_ICE_MOSI, (data & (1 << i)) ? HIGH : LOW);
        
        delayMicroseconds(1);   
        if (digitalRead(PI_ICE_MISO) == HIGH)
            rdata |= 1 << i;
        
        delayMicroseconds(1);
        digitalWrite(PI_ICE_CLK, HIGH);
        delayMicroseconds(1);
        digitalWrite(PI_ICE_CLK, LOW);
        delayMicroseconds(1);
    }
    
    return rdata;
}

void flash_write_enable()
{
    spi_begin();
    spi_xfer(0x06);
    spi_end();
}

void flash_bulk_erase()
{
    spi_begin();
    spi_xfer(0xc7);
    spi_end();
}

void flash_erase_64kB(int addr)
{
    spi_begin();
    spi_xfer(0xd8);
    spi_xfer(addr >> 16);
    spi_xfer(addr >> 8);
    spi_xfer(addr);
    spi_end();
}

void flash_write(int addr, uint8_t *data, int n)
{
    spi_begin();
    spi_xfer(0x02);
    spi_xfer(addr >> 16);
    spi_xfer(addr >> 8);
    spi_xfer(addr);
    while (n--)
        spi_xfer(*(data++));
    spi_end();
}

void flash_read(int addr, uint8_t *data, int n)
{
    spi_begin();
    spi_xfer(0x03);
    spi_xfer(addr >> 16);
    spi_xfer(addr >> 8);
    spi_xfer(addr);
    while (n--)
        *(data++) = spi_xfer(0);
    spi_end();
}

void flash_read_slow(int addr, uint8_t *data, int n)
{
    spi_begin();
    spi_xfer_slow(0x03);
    spi_xfer_slow(addr >> 16);
    spi_xfer_slow(addr >> 8);
    spi_xfer_slow(addr);
    while (n--)
        *(data++) = spi_xfer_slow(0);
    spi_end();
}

int flash_wait()
{
    int ms_start = get_time_ms();
    
    while (1)
    {
        spi_begin();
        spi_xfer(0x05);
        int status = spi_xfer(0);
        spi_end();
        
        if ((status & 0x01) == 0)
            break;
        
        delayMicroseconds(1000);
    }
    
    return get_time_ms() - ms_start;
}

void prog_flasherase()
{
    pinMode(PI_ICE_CLK,     OUTPUT);
    pinMode(PI_ICE_MOSI,    OUTPUT);
    pinMode(PI_ICE_CS,      OUTPUT);
    
    // connect flash to Raspi
    digitalWrite(PI_ICE_CS, HIGH);
    digitalWrite(PI_ICE_CLK, LOW);
    delayMicroseconds(100);
    
    // power_up
    spi_begin();
    spi_xfer(0xab);
    spi_end();
    
    flash_write_enable();
    flash_bulk_erase();
    
    // power_down
    spi_begin();
    spi_xfer(0xb9);
    spi_end();
}

void prog_flashmem(int pageoffset)
{
    pinMode(PI_ICE_CLK,     OUTPUT);
    pinMode(PI_ICE_MOSI,    OUTPUT);
    pinMode(PI_ICE_CS,      OUTPUT);
    
    // connect flash to Raspi
    digitalWrite(PI_ICE_CS, HIGH);
    digitalWrite(PI_ICE_CLK, LOW);
    delayMicroseconds(100);
    
    // power_up
    spi_begin();
    spi_xfer(0xab);
    spi_end();
    
    // read flash id
    spi_begin();
    spi_xfer(0x9f);
    fprintf(stderr, "flash id:");
    for (int i = 0; i < 20; i++)
        fprintf(stderr, " %02x", spi_xfer(0x00));
    fprintf(stderr, "\n");
    spi_end();
    
    // load prog data into buffer
    std::vector<uint8_t> prog_data;
    while (1) {
        int byte = getchar();
        if (byte < 0)
            break;
        prog_data.push_back(byte);
    }
    
    int ms_timer = 0;
    fprintf(stderr, "writing %.2fkB..", double(prog_data.size()) / 1024);
    
    for (int addr = 0; addr < int(prog_data.size()); addr += 256)
    {
        if (addr % (64*1024) == 0)
        {
            fprintf(stderr, "\n%3d%% @%06x ", 100*addr/int(prog_data.size()), addr);
            fprintf(stderr, "erasing 64kB sector..");
            
            flash_write_enable();
            flash_erase_64kB(addr + pageoffset * 0x10000);
            ms_timer += flash_wait();
        }
        
        if (addr % (32*256) == 0) {
            fprintf(stderr, "\n%3d%% @%06x writing: ", 100*addr/int(prog_data.size()), addr);
        }
        
        int n = std::min(256, int(prog_data.size()) - addr);
        uint8_t buffer[256];
        
        for (int retry_count = 0; retry_count < 100; retry_count++)
        {
            flash_write_enable();
            flash_write(addr + pageoffset * 0x10000, &prog_data[addr], n);
            ms_timer += flash_wait();
            
            flash_read(addr + pageoffset * 0x10000, buffer, n);
            
            if (!memcmp(buffer, &prog_data[addr], n)) {
                fprintf(stderr, "o");
                goto written_ok;
            }
            
            fprintf(stderr, "X");
        }
        
        // restart erasing and writing this 64kB sector
        addr -= addr % (64*1024);
        addr -= 256;
        
    written_ok:;
    }
    
    fprintf(stderr, "\n100%% total wait time: %d ms\n", ms_timer);
    
    // power_down
    spi_begin();
    spi_xfer(0xb9);
    spi_end();
}

void ice_reset()
{
    pinMode(PI_ICE_CRESET,      OUTPUT);
    digitalWrite(PI_ICE_CRESET, LOW);
}

void read_flashmem(int n)
{
    pinMode(PI_ICE_CLK,     OUTPUT);
    pinMode(PI_ICE_MOSI,    OUTPUT);
    pinMode(PI_ICE_CS,      OUTPUT);
    
    // connect flash to Raspi
    digitalWrite(PI_ICE_CS, HIGH);
    digitalWrite(PI_ICE_CLK, LOW);
    delayMicroseconds(100);
    
    // power_up
    spi_begin();
    spi_xfer(0xab);
    spi_end();
    
    // read flash id
    spi_begin();
    spi_xfer(0x9f);
    fprintf(stderr, "flash id:");
    for (int i = 0; i < 20; i++)
        fprintf(stderr, " %02x", spi_xfer(0x00));
    fprintf(stderr, "\n");
    spi_end();
    
    fprintf(stderr, "reading %.2fkB..\n", double(n) / 1024);
    for (int addr = 0; addr < n; addr += 256) {
        uint8_t buffer[256];
        flash_read_slow(addr, buffer, std::min(256, n - addr));
        fwrite(buffer, std::min(256, n - addr), 1, stdout);
    }
    
    // power_down
    spi_begin();
    spi_xfer(0xb9);
    spi_end();
}

void reset_inout()
{
    pinMode(PI_ICE_CLK,     INPUT);
    pinMode(PI_ICE_CDONE,   INPUT);
    pinMode(PI_ICE_MOSI,    INPUT);
    pinMode(PI_ICE_MISO,    INPUT);
    pinMode(PI_ICE_CRESET,  INPUT);
    pinMode(PI_ICE_CS,      INPUT);
    
    current_send_recv_mode = 0;
}

void help(const char *progname)
{
    fprintf(stderr, "\n");
    fprintf(stderr, "Resetting FPGA:\n");
    fprintf(stderr, "    %s -R\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "Resetting FPGA (reload from flash):\n");
    fprintf(stderr, "    %s -b\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "Bulk erase entire flash:\n");
    fprintf(stderr, "    %s -E\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "Programming FPGA bit stream:\n");
    fprintf(stderr, "    %s -p < data.bin\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "Programming serial flash:\n");
    fprintf(stderr, "    %s -f < data.bin\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "Reading serial flash (first N bytes):\n");
    fprintf(stderr, "    %s -F N > data.bin\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "Testing bit-parallel link (using ep0):\n");
    fprintf(stderr, "    %s -T\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "Testing link bandwidth (using ep0):\n");
    fprintf(stderr, "    %s -B\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "Writing a file to ep N:\n");
    fprintf(stderr, "    %s -w N < data.bin\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "Reading a file from ep N:\n");
    fprintf(stderr, "    %s -r N > data.bin\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "Console at ep N:\n");
    fprintf(stderr, "    %s -c N\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "Dumping a VCD file (from ep1, using trig1)\n");
    fprintf(stderr, "  with a debug core with N bits width:\n");
    fprintf(stderr, "    %s -V N > dbg_trace.vcd\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "Additional options:\n");
    fprintf(stderr, "    -v      verbose output\n");
    fprintf(stderr, "    -O N    offset (in 64 kB pages) for -f\n");
    fprintf(stderr, "    -z      send a terminating zero byte with -w/-c\n");
    fprintf(stderr, "    -Z      wait for terminating zero byte in -r/-c\n");
    fprintf(stderr, "    -t N    send trigger N before -w/-r/-c\n");
    fprintf(stderr, "\n");
    exit(1);
}

int main(int argc, char **argv)
{
    int opt, n = -1, t = -1;
    int pageoffset = 0;
    char mode = 0;
    
    while ((opt = getopt(argc, argv, "RbEpfF:TBw:r:c:vzZt:O:V:")) != -1)
    {
        switch (opt)
        {
            case 'w':
            case 'r':
            case 'c':
            case 'V':
            case 'F':
                n = atoi(optarg);
                // fall through
                
            case 'E':
            case 'R':
            case 'b':
            case 'p':
            case 'f':
            case 'T':
            case 'B':
                if (mode)
                    help(argv[0]);
                mode = opt;
                break;
                
            case 'v':
                if (verbose)
                    ftdi_verbose = true;
                verbose = true;
                break;
                
            case 'z':
                send_zero = true;
                break;
                
            case 'Z':
                recv_zero = true;
                break;
                
            case 't':
                t = atoi(optarg);
                break;
                
            case 'O':
                pageoffset = atoi(optarg);
                break;
                
            default:
                help(argv[0]);
        }
    }
    
    if (optind != argc || !mode)
        help(argv[0]);
    
    if (mode == 'R') {
        wiringPiSetup();
        reset_inout();
        prog_bitstream(true);
        reset_inout();
    }
    
    if (mode == 'b') {
        wiringPiSetup();
        reset_inout();
        fpga_reset();
        reset_inout();
    }
    
    if (mode == 'E') {
        wiringPiSetup();
        reset_inout();
        prog_flasherase();
        reset_inout();
    }
    
    if (mode == 'p') {
        wiringPiSetup();
        reset_inout();
        prog_bitstream();
        reset_inout();
    }
    
    if (mode == 'f') {
        wiringPiSetup();
        reset_inout();
        ice_reset();
        prog_flashmem(pageoffset);
        reset_inout();
    }
    
    if (mode == 'F') {
        wiringPiSetup();
        reset_inout();
        ice_reset();
        read_flashmem(n);
        reset_inout();
    }
    
    if (verbose)
        fprintf(stderr, "\n");
    
    return 0;
}


