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

#define TEST 1

#ifndef TEST
#include "libRomulatorDebug.h"
#endif

#include "libRomulatorVram.h"
#include "libbmp.h"

#define CONNMAX 1000
#define BYTES 1024

char *ROOT;
int listenfd, clients[CONNMAX];
char characterRomName[1024];
uint8_t* characterRom;
uint8_t rgbbitmap[192000];
uint8_t bitmap[64000];

void startServer(char *);
void respond(int, int);

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
    for (i=0; i<CONNMAX; i++)
    {
        clients[i]=-1;
    }
    startServer(PORT);

    fprintf(stderr, "server started\n");

    // initialize romulator connection
    #ifndef TEST
    romulatorInit();
    #endif

    // ACCEPT connections
    int ii = 0;
    while (1)
    {
        addrlen = sizeof(clientaddr);
        clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

        if (clients[slot]<0)
            error ("accept() error");
        else
        {
            /*
            if ( fork()==0 )
            {
                respond(slot);
                exit(0);
            }
            */

            respond(slot, ii++);
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

int get_screen_image()
{
    // this is a request for a pet screen image.
    // first - dummy request for fixed image

    // use console to get memory dump
    //system("bin/console -r > memory.bin");
    /*
    system("bin/console -s > screen.bin");
    system("bin/make_screen_image -r roms/characters-2.901447-10.bin -o 0 -m 1024 -c 40 < screen.bin > out.ppm");
    system("convert out.ppm out.png");
    return open("out.png", O_RDONLY);
    */

    return -1;
}

void getBmpImage(uint8_t* bmpBuffer, int pos)
{
    int width = 320;
    int height = 200;

    /*
    uint8_t val = 0;
    int index = 0;

    int ww = pos % width;
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            if (w >= ww && w < ww + 10)
            {
                bitmap[index] = 255;
            }
            else
            {
                bitmap[index] = 0;
            }
            index++;
        }
    }
    */

    uint8_t vram[1024];

    #ifdef TEST
    // fill screen with characters
    uint8_t v = 0;
    for (int i = 0; i < 1024; i++)
    {
        vram[i] = v++;
    }
    #else
    // get vram from romulator
    romulatorReadVram(vram, 1024, 1000, 5);
    #endif
    // convert vram into a bitmap

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

    for (int i = 0; i < 10; i++)
    {
        fprintf(stderr, "vram %d %02X\n", i, vram[i]);
    }

    for (int i = 0; i < 10; i++)
    {
        fprintf(stderr, "crom %d %02X\n", i, characterRom[i]);
    }

    romulatorVramToBitmap(vram, characterRom, 40, 25, 8, 8, bitmap);

    for (int i = 0; i < 10; i++)
    {
        fprintf(stderr, "bmp %d %02X\n", i, bitmap[i]);
    }
    
    /*
    int rgbindex = 0;
    for (int i = 0; i < 64000; i++)
    {
        uint8_t v = bitmap[i];
        if (v != 0) v = 255;

        rgbbitmap[rgbindex] = v;
        rgbbitmap[rgbindex+1] = v;
        rgbbitmap[rgbindex+2] = v;
        rgbindex += 3;
    }
    */

    int rgbindex = 0;
    for (int h = 0; h < height; h++)
    {
        int bmp_h = height - 1 - h;
        for (int w = 0; w < width; w++)
        {
            int bmp_index = (bmp_h * width) + w;

            uint8_t v = bitmap[bmp_index];
            if (v != 0) v = 255;

            rgbbitmap[rgbindex] = v;
            rgbbitmap[rgbindex+1] = v;
            rgbbitmap[rgbindex+2] = v;
            rgbindex += 3;
        }
    }

    // now create bitmap image
    generateBitmapImageToMemory(rgbbitmap, height, width, bmpBuffer);
}

void sendStringToClient(int client, char* string)
{
    send(client, string, strlen(string), 0);
}


//client connection
void respond(int n, int tmp)
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
        printf("%s", mesg);
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

                fprintf(stderr, "requested path: %s\n", reqline[1]);

                strcpy(path, ROOT);
                strcpy(&path[strlen(ROOT)], reqline[1]);
                printf("file: %s\n", path);

                // handle specific paths
                if (strstr(path, "romulator.")) {
                    fprintf(stderr, "romulator path\n");

                    if (strstr(path, ".png"))
                    {
                        fd = get_screen_image();

                        if (fd != -1)
                        {
                            //send(clients[n], "HTTP/1.0 200 OK\n", 17, 0);
                            sendStringToClient(clients[n], "HTTP/1.0 200 OK\n");
                            sendStringToClient(clients[n], "Content-Type: image/png\n\n");
                            //send(clients[n], "Content-Type: image/ppm\n\n");
                            while ( (bytes_read=read(fd, data_to_send, BYTES))>0 )
                            {
                                write (clients[n], data_to_send, bytes_read);
                            }
                        }
                        else
                        {
                            write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
                        }
                    }
                    else if (strstr(path, ".bmp"))
                    {
                        int bmpSize = bmpGetFileSize(200, 320);
                        uint8_t* bmp = (uint8_t*)malloc(bmpSize);

                        getBmpImage(bmp, tmp);

                        FILE* fp = fopen("test.bmp", "wb");
                        fwrite(bmp, 1, bmpSize, fp);
                        fclose(fp);

                        sendStringToClient(clients[n], "HTTP/1.0 200 OK\n");
                        sendStringToClient(clients[n], "Content-Type: image/bmp\n\n");

                        int bytesToWrite = bmpSize;
                        uint8_t* bmpPtr = bmp;
                        while (bytesToWrite > 0)
                        {
                            ssize_t res = write(clients[n], bmpPtr, bytesToWrite >= 1024 ? 1024 : bytesToWrite);
                            bmpPtr += res;
                            bytesToWrite -= res;
                        }

                        free(bmp);
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
                        send(clients[n], "HTTP/1.0 200 OK\n", 17, 0);
                        sendStringToClient(clients[n], "Content-Type: text/html\n\n");
                        while ( (bytes_read=read(fd, data_to_send, BYTES))>0 )
                        {
                            write (clients[n], data_to_send, bytes_read);
                        }
                    }
                    else
                    {
                        write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
                    }
                }
            }
        }
    }

    //Closing SOCKET
    shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
    close(clients[n]);
    clients[n]=-1;
}