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

#define CONNMAX 1000
#define BYTES 1024

char *ROOT;
int listenfd, clients[CONNMAX];
void startServer(char *);
void respond(int);

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

    fprintf(stderr, "here\n");

    //Parsing the command line arguments
    while ((c = getopt(argc, argv, "p:r:")) != -1)
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

    // ACCEPT connections
    while (1)
    {
        addrlen = sizeof(clientaddr);
        clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

        if (clients[slot]<0)
            error ("accept() error");
        else
        {
            if ( fork()==0 )
            {
                respond(slot);
                exit(0);
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

int get_screen_image()
{
    // this is a request for a pet screen image.
    // first - dummy request for fixed image

    // use console to get memory dump
    system("bin/console -r > memory.bin");
    system("bin/make_screen_image -r ../roms/characters-2.901447-10.bin -c 40 < memory.bin > out.ppm");
    system("convert out.ppm out.png");
    return open("out.png", O_RDONLY);
}

void sendStringToClient(int client, char* string)
{
    send(client, string, strlen(string), 0);
}


//client connection
void respond(int n)
{
    char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999];
    int rcvd, fd, bytes_read;

    memset( (void*)mesg, (int)'\0', 99999 );

    rcvd=recv(clients[n], mesg, 99999, 0);

    if (rcvd<0)    // receive error
        fprintf(stderr,("recv() error\n"));
    else if (rcvd==0)    // receive socket closed
        fprintf(stderr,"Client disconnected upexpectedly.\n");
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
                if (strstr(path, "romulator")) {
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
                    else
                    {
                        write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
                    }
                }
                else
                {
                    if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
                    {
                        send(clients[n], "HTTP/1.0 200 OK\n\n", 17, 0);
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