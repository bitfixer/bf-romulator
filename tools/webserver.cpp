#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>

#include <png.h>
#include <thread>
#include <mutex>

//#define IMAGETHREAD 1

#ifndef TEST
#include "libRomulatorDebug.h"
#endif

#include "libRomulatorVram.h"
#include "libbmp.h"
#include "../bf-shared/timer.hpp"

#define CONNMAX 5
#define BYTES 1024
#define MAX_SETTINGS 16
#define MAX_CHAR_ROM_NAME_LENGTH 256

char *ROOT;
int listenfd, clients[CONNMAX];
char characterRomName[1024];
uint8_t* characterRom;
uint8_t rgbbitmap[192000];
uint8_t bitmap[64000];
uint8_t* bmpImage;

uint8_t pngBuffer[2][192000];
int pngIndex;
int pngLen[2];

// character rom names
char* characterRoms[MAX_SETTINGS];

unsigned int lastMeasurementTime;
unsigned int measurementInterval;
unsigned int requestsInInterval;

png_bytep* row_pointers;

std::thread imageThread;
std::mutex imageMutex;

void startServer(char *);
void respond(int, int, int*);
void imageThreadRun();

void error(char* str) {
    fprintf(stderr, "%s\n", str);
}

long getSize(FILE* fp)
{
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return sz;
}

void png_init()
{
    int width = 320;
    int height = 200;
    // initialize row pointers, set them to the fixed rgb bitmap rows in memory
    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    png_bytep row = (png_bytep)rgbbitmap;
    for (int i = 0; i < height; i++)
    {
        row_pointers[i] = row;
        row += width * 3;
    }

    pngIndex = 0;
}

void trim(char* str)
{
    char* pos = str;
    while (*pos != 0)
    {
        if (isspace(*pos))
        {
            // move string one space left
            int len = strlen(pos);
            memmove(pos, pos+1, len);
        }
        else
        {
            pos++;
        }
    }
}

bool fileExists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "rb")))
    {
        fclose(file);
        return true;
    }
    return false;
}

void parseConfigFile()
{
    // initialize all character rom strings
    for (int i = 0; i < MAX_SETTINGS; i++)
    {
        characterRoms[i] = NULL;
    }

    fprintf(stderr, "parsing config file\n");
    FILE* fp = fopen("../config/enable_table_default.csv", "rb");

    // scan each line to check for "vram" string
    char line[256];

    while (fgets(line, sizeof(line), fp))
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

        if (strstr(line, "vram") != NULL)
        {
            fprintf(stderr, "vram line: %s\n", line);

            char* token;
            token = strtok(line, ",");

            int index;
            sscanf(token, "%d", &index);
            fprintf(stderr, "index %d\n", index);

            token = strtok(NULL, ",");

            // get start and end addresses
            uint32_t startAddr;
            sscanf(token, "0x%X", &startAddr);
            fprintf(stderr, "start %X\n", startAddr);

            token = strtok(NULL, ",");
            uint32_t endAddr;
            sscanf(token, "0x%X", &endAddr);
            fprintf(stderr, "end %X\n", endAddr);

            // skip vram string field
            token = strtok(NULL, ",");
            token = strtok(NULL, ",");

            if (token)
            {
                fprintf(stderr, "character rom name: %s\n", token);
                // copy character rom name
                characterRoms[index] = (char*)malloc(MAX_CHAR_ROM_NAME_LENGTH);
                sprintf(characterRoms[index], "../roms/%s", token);
                trim(characterRoms[index]);
            }
            else
            {
                fprintf(stderr, "no character rom found\n");
            }

        }
    }

    fclose(fp);

    fprintf(stderr, "character roms\n");
    for (int i = 0; i < MAX_SETTINGS; i++)
    {
        if (characterRoms[i])
        {
            fprintf(stderr, "%d: %s", i, characterRoms[i]);

            // verify this file exists
            if (fileExists(characterRoms[i]))
            {
                fprintf(stderr, " *\n");
            }
            else
            {
                fprintf(stderr, "\n");
                fprintf(stderr, "ERROR: could not find character rom %s\n", characterRoms[i]);
                fprintf(stderr, "please check config file ../config/enable_table_default.csv\n");
                exit(1);
            }

        }
    }
}

int main(int argc, char** argv)
{
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    int c;
    
    //Default Values PATH = ~/ and PORT=10000
    char PORT[6];
    ROOT = getenv("PWD");
    strcpy(PORT,"10000");
    int slot=0;
    memset(characterRomName, 0, 1024);
    characterRom = NULL;
    bmpImage = NULL;

    //Parsing the command line arguments
    while ((c = getopt(argc, argv, "p:r:c:")) != -1)
    {
        switch (c)
        {
            case 'r':
                ROOT = (char*)malloc(strlen(optarg));
                strcpy(ROOT,optarg);
                break;
            case 'p':
                strcpy(PORT,optarg);
                break;
            case '?':
                fprintf(stderr,"Wrong arguments given!!!\n");
                exit(1);
            case 'c':
                // character rom
                strcpy(characterRomName, optarg);
                break;
            default:
                fprintf(stderr, "default\n");
                exit(1);
        }
    }

    fprintf(stderr, "Server started at port no. %s with root directory as %s\n",PORT,ROOT);
    // Setting all elements to -1: signifies there is no client connected
    int i;
    for (i=0; i < CONNMAX; i++)
    {
        clients[i] = -1;
    }


    parseConfigFile();
    startServer(PORT);

    fprintf(stderr, "server started\n");
    png_init();

    #ifdef IMAGETHREAD
    std::thread imageThread(imageThreadRun);
    #endif
    Tools::Timer::startProgramTimer();

    lastMeasurementTime = Tools::Timer::millis();
    measurementInterval = 1000;
    requestsInInterval = 0;

    // initialize romulator connection
    #ifndef TEST
    romulatorInit();
    #endif

    signal(SIGPIPE, SIG_IGN);

    // ACCEPT connections
    int ii = 0;
    while (1)
    {
        //printf("start request slot %d\n", slot);
        addrlen = sizeof(clientaddr);

        clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

        if (clients[slot] < 0)
            error ("accept() error");
        else
        {
            // single threaded handler
            respond(slot, ii, &clients[slot]);
            ii++;

            unsigned int currTime = Tools::Timer::millis();
            if (currTime - lastMeasurementTime >= measurementInterval)
            {
                if (requestsInInterval > 0)
                {
                    float intervalSeconds = (float)(currTime - lastMeasurementTime) / 1000.0;
                    float fps = (float)requestsInInterval / intervalSeconds;
                    requestsInInterval = 0;

                    fprintf(stderr, "FPS %0.2f\n", fps);
                }

                lastMeasurementTime = currTime;
            }
        }

        while (clients[slot]!=-1) slot = (slot+1)%CONNMAX;
    }
    return 0;
}

//start server
void startServer(char *port)
{
    struct addrinfo hints, *res, *p;
    
    // getaddrinfo for host
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo( NULL, port, &hints, &res) != 0)
    {
        perror ("getaddrinfo() error");
        exit(1);
    }
    // socket and bind
    for (p = res; p!=NULL; p=p->ai_next)
    {
        listenfd = socket (p->ai_family, p->ai_socktype, 0);
        if (listenfd == -1) continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
    }
    if (p==NULL)
    {
        perror ("socket() or bind()");
        exit(1);
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if ( listen (listenfd, 1000000) != 0 )
    {
        perror("listen() error");
        exit(1);
    }
}

void write_ppm_header(int width, int height, FILE* fp)
{
    fprintf(fp, "P6\n");
    fprintf(fp, "%d %d\n", width, height);
    fprintf(fp, "255\n");
}

void write_pixel(uint8_t r, uint8_t g, uint8_t b, FILE* fp)
{
    //printf("%d %d %d ", r, g, b);
    fwrite(&r, 1, 1, fp);
    fwrite(&g, 1, 1, fp);
    fwrite(&b, 1, 1, fp);
}

void draw_bitmap(uint8_t* bitmap, int image_width, int image_height, FILE* fp)
{
    write_ppm_header(image_width, image_height, fp);
    for (int y = 0; y < image_height; y++)
    {
        for (int x = 0; x < image_width; x++)
        {
            int pixel_index = (y * image_width) + x;
            if (bitmap[pixel_index] == 0)
            {
                write_pixel(0, 0, 0, fp);
            }
            else
            {
                write_pixel(255, 255, 255, fp);
            }
        }
    }
}

void convertMonoToRGBBitmap(uint8_t* monoBitmap, uint8_t* rgbBitmap, int width, int height)
{
    int rgbindex = 0;
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            int bmp_index = (h * width) + w;

            uint8_t v = monoBitmap[bmp_index];
            if (v != 0) v = 255;

            rgbBitmap[rgbindex] = v;
            rgbBitmap[rgbindex+1] = v;
            rgbBitmap[rgbindex+2] = v;
            rgbindex += 3;
        }
    }
}

void getVram(uint8_t* vram, int len, int pos)
{
    #ifdef TEST
    // fill screen with characters
    uint8_t v = pos % 256;
    for (int i = 0; i < len; i++)
    {
        vram[i] = v++;
    }
    #else
    // get vram from romulator
    romulatorReadVram(vram, len, len, 5);
    #endif
}

void getMonoBitmap(int width, int height, int pos)
{
    uint8_t vram[1024];

    unsigned int startTime = Tools::Timer::millis();
    getVram(vram, 1024, pos);
    unsigned int readVramTime = Tools::Timer::millis();

    if (characterRom == NULL)
    {
        // read character rom from file
        FILE* fp = fopen(characterRomName, "rb");
        fseek(fp, 0, SEEK_END);
        int size = (int)ftell(fp);
        fseek(fp, 0, SEEK_SET);

        characterRom = (uint8_t*)malloc(size);
        fread(characterRom, 1, size, fp);
        fclose(fp);
    }

    romulatorVramToBitmap(vram, characterRom, 25, 40, 8, 8, bitmap);
    unsigned int convertBitmap = Tools::Timer::millis();

    fprintf(stderr, "mono total %d read %d convert %d\n",
        convertBitmap - startTime,
        readVramTime - startTime,
        convertBitmap - readVramTime);
}

void getRGBBitmap(int width, int height, int pos)
{
    getMonoBitmap(width, height, pos);
    convertMonoToRGBBitmap(bitmap, rgbbitmap, width, height);
}

void getBmpImage(uint8_t* bmpBuffer, int pos)
{
    int width = 320;
    int height = 200;
    
    unsigned int startBitmap = Tools::Timer::millis();
    getRGBBitmap(width, height, pos);
    unsigned int endBitmap = Tools::Timer::millis();
    // now create bitmap image
    generateBitmapImageToMemory(rgbbitmap, height, width, bmpBuffer);
    unsigned int doneBmp = Tools::Timer::millis();

    fprintf(stderr, "bmp image bitmap %d bmp %d\n",
        endBitmap - startBitmap,
        doneBmp - endBitmap);
}

void getPngImage(int pos, uint8_t* buffer, int* len)
{
    int width = 320;
    int height = 200;

    png_structp png_ptr;
    png_infop info_ptr;
    png_byte color_type;
    
    unsigned int startBitmap = Tools::Timer::millis();
    getRGBBitmap(width, height, pos);
    unsigned int endBitmap = Tools::Timer::millis();

    color_type = PNG_COLOR_TYPE_RGB;
    FILE* fp = fmemopen(buffer, 192000, "wb");
    
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);


    //setjmp(png_jmpbuf(png_ptr));
    png_init_io(png_ptr, fp);

    //setjmp(png_jmpbuf(png_ptr));
    
    png_set_IHDR(png_ptr, info_ptr, width, height,
                     8, color_type, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    unsigned int startPngWrite = Tools::Timer::millis();
    
    //if (setjmp(png_jmpbuf(png_ptr)))
    //            fprintf(stderr,"[write_png_file] Error during writing bytes");
    png_write_image(png_ptr, row_pointers);

    unsigned int endPngWrite = Tools::Timer::millis();

    /* end write */
    //    if (setjmp(png_jmpbuf(png_ptr)))
    //            fprintf(stderr,"[write_png_file] Error during end of write");

    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    *len = ftell(fp);
    fclose(fp);
    unsigned int donePngWrite = Tools::Timer::millis();

    /*
    fprintf(stderr, "png image bitmap %d header %d encode %d close %d\n",
        endBitmap - startBitmap,
        startPngWrite - endBitmap,
        endPngWrite - startPngWrite,
        donePngWrite - endPngWrite);
    */
}

void sendStringToClient(int client, char* string)
{
    send(client, string, strlen(string), 0);
}

bool sendBufferToClient(uint8_t* buffer, int length, int sendTo)
{
    int bytesToWrite = length;
    uint8_t* bufPtr = buffer;
    while (bytesToWrite > 0)
    {
        ssize_t res = write(sendTo, bufPtr, bytesToWrite >= 1024 ? 1024 : bytesToWrite);
        if (res == -1)
        {
            printf("**** broken pipe\n");
            break;
        }
        
        bufPtr += res;
        bytesToWrite -= res;
    }

    if (bytesToWrite == 0)
    {
        return true;
    }

    return false;
}

//client connection
void respond(int n, int tmp, int* cc)
{
    char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999];
    int rcvd, fd, bytes_read;

    memset( (void*)mesg, (int)'\0', 99999 );

    rcvd=recv(clients[n], mesg, 99999, 0);

    if (rcvd<0)    // receive error
        fprintf(stderr,("recv() error\n"));
    else if (rcvd==0)    // receive socket closed
        fprintf(stderr,"Client disconnected unexpectedly.\n");
    else    // message received
    {
        //printf("%s\n", mesg);
        reqline[0] = strtok (mesg, " \t\n");
        if ( strncmp(reqline[0], "GET\0", 4)==0 )
        {
            reqline[1] = strtok (NULL, " \t");
            reqline[2] = strtok (NULL, " \t\n");
            if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
            {
                write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
            }
            else
            {
                if ( strncmp(reqline[1], "/\0", 2)==0 ) {
                    sprintf(reqline[1], "/index.html");
                    //reqline[1] = "/index.html";
                }

                //fprintf(stderr, "requested path: %s\n", reqline[1]);

                strcpy(path, ROOT);
                strcpy(&path[strlen(ROOT)], reqline[1]);
                //printf("file: %s\n", path);

                // handle specific paths
                if (strstr(path, "romulator.")) {
                    //fprintf(stderr, "romulator path\n");

                    // mono bitmap, unpacked
                    if (strstr(path, ".bin"))
                    {
                        unsigned int startTime = Tools::Timer::millis();
                        getMonoBitmap(320, 200, tmp);
                        unsigned int gotBitmap = Tools::Timer::millis();
                        sendStringToClient(clients[n], "HTTP/1.0 200 OK\n");
                        sendStringToClient(clients[n], "Content-Type: application/octet-stream\n\n");
                        sendBufferToClient(bitmap, 64000, clients[n]);
                        unsigned int sentBitmap = Tools::Timer::millis();

                        fprintf(stderr, "bin total %d img %d send %d\n",
                            sentBitmap - startTime,
                            gotBitmap - startTime,
                            sentBitmap - gotBitmap);

                    }
                    else if (strstr(path, ".rom"))
                    {
                        // look up the character rom for this configuration
                        #ifdef TEST
                        uint8_t configByte = romulatorReadConfig();
                        #else
                        uint8_t configByte = 1;
                        #endif
                        
                        fprintf(stderr, "got rom request, config byte %d\n", configByte);
                        char* charRomName = characterRoms[configByte];
                        if (charRomName)
                        {
                            fd = open(charRomName, O_RDONLY);
                            sendStringToClient(clients[n], "HTTP/1.0 200 OK\n");
                            sendStringToClient(clients[n], "Content-Type: application/octet-stream\n\n");
                            while ( (bytes_read=read(fd, data_to_send, BYTES))>0 )
                            {
                                write (clients[n], data_to_send, bytes_read);
                            }
                        }
                        else
                        {
                            fprintf(stderr, "character rom for setting %d not found\n", configByte);
                            write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
                        }
                    }
                    else if (strstr(path, ".vram")) // retrieve contents of vram section
                    {
                        uint8_t vram[1024];
                        getVram(vram, 1024, tmp);

                        sendStringToClient(clients[n], "HTTP/1.0 200 OK\n");
                        sendStringToClient(clients[n], "Content-Type: application/octet-stream\n\n");
                        sendBufferToClient(vram, 1024, clients[n]);

                        // record request for FPS measurement
                        requestsInInterval++;
                    }
                    else if (strstr(path, ".bmp"))
                    {
                        unsigned int startTime = Tools::Timer::millis();
                        int bmpSize = bmpGetFileSize(200, 320);

                        if (bmpImage == NULL)
                        {
                            bmpImage = (uint8_t*)malloc(bmpSize);
                        }

                        getBmpImage(bmpImage, tmp);
                        unsigned int bmpDone = Tools::Timer::millis();
                        
                        //char tmp[1024];
                        sendStringToClient(clients[n], "HTTP/1.0 200 OK\n");
                        sendStringToClient(clients[n], "Content-Type: image/bmp\n\n");
                        
                        sendBufferToClient(bmpImage, bmpSize, clients[n]);
                        unsigned int bmpSent = Tools::Timer::millis();

                        fprintf(stderr, "bmp total %d image %d send %d\n",
                            bmpSent - startTime,
                            bmpDone - startTime,
                            bmpSent - bmpDone);
                    }
                    else if (strstr(path, ".png"))
                    {
                        unsigned int startMillis = Tools::Timer::millis();
                        #ifndef IMAGETHREAD
                        getPngImage(tmp, pngBuffer[pngIndex], &pngLen[pngIndex]);
                        #endif
                        unsigned int readDataMillis = Tools::Timer::millis();

                        sendStringToClient(clients[n], "HTTP/1.0 200 OK\n");
                        sendStringToClient(clients[n], "Content-Type: image/png\n\n");

                        {
                            #ifdef IMAGETHREAD
                            std::lock_guard<std::mutex> guard(imageMutex);
                            #endif
                            sendBufferToClient(pngBuffer[pngIndex], pngLen[pngIndex], clients[n]);
                        }

                        unsigned int sentDataMillis = Tools::Timer::millis();
                        unsigned int dataReadTime = readDataMillis - startMillis;
                        unsigned int sendTime = sentDataMillis - readDataMillis;
                        unsigned int totalTime = sentDataMillis - startMillis;

                        fprintf(stderr, "png time %d, read %d send %d\n", totalTime, dataReadTime, sendTime);
                    }
                    else
                    {
                        write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
                    }
                }
                else
                {
                    if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
                    {
                        sendStringToClient(clients[n], "HTTP/1.0 200 OK\n");
                        sendStringToClient(clients[n], "Content-Type: text/html\n\n");
                        while ( (bytes_read=read(fd, data_to_send, BYTES))>0 )
                        {
                            write (clients[n], data_to_send, bytes_read);
                        }
                    }
                    else
                    {
                        fprintf(stderr, "not found\n");
                        write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
                    }
                }
            }
        }
    }

    //Closing SOCKET
    shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
    close(clients[n]);
    clients[n] = -1;
}

void imageThreadRun()
{
    while (1)
    {
        unsigned int startTime = Tools::Timer::millis();
        int writeIndex = pngIndex == 0 ? 1 : 0;
        getPngImage(0, pngBuffer[writeIndex], &pngLen[writeIndex]);

        unsigned int endTime = Tools::Timer::millis();
        
        // switch to other buffer
        std::lock_guard<std::mutex> guard(imageMutex);
        pngIndex = writeIndex;
    }
}