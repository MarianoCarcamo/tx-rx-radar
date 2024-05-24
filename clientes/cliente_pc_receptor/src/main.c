/* Copyright 2024, Adan Lema <adanlema@hotmail.com> & Carcamo Mariano <mgcarcamo98@gmail.com> */

/*==================[inclusions]=============================================*/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
/*==================[macros and definitions]=================================*/
#define IP  "127.0.0.1"
#define PORT 8080

#define BUFF_SIZE 4096*4

#define FILE_PATH "src/rxdata.bin"
/*==================[internal data declaration]==============================*/
static void *r_buff;
static int lfd;
static FILE *data_file;
/*==================[internal functions declaration]=========================*/
static void signalHandler(int sig);
/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
static void signalHandler(int sig){
    // fclose(data_file);
    close(lfd);
    free(r_buff);
    printf("\nConexion cerrada con exito\n");
    exit(EXIT_SUCCESS);
}
/*==================[external functions definition]==========================*/

int main() {
    struct sockaddr_in server;
    size_t recv_size;
    data_file = fopen(FILE_PATH,"w");

    r_buff = malloc(BUFF_SIZE);

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);

    connect(lfd, (struct sockaddr *)&server, sizeof server);

    // Manejo de señales de linux...
    signal(SIGABRT, signalHandler);
    signal(SIGINT,  signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGKILL, signalHandler);

    while (1) {
        memset(r_buff,0,BUFF_SIZE);
        recv_size = recv(lfd, r_buff, BUFF_SIZE, 0);
        if (!(recv_size == 0 || recv_size == -1)) {
            fwrite(r_buff,1,recv_size,data_file);
        } else {
            FILE * bin_data;
            FILE * txt_data;
            uint32_t data[4096];
            size_t bin_data_size;

            fclose(data_file);

            txt_data = fopen("src/rxdata.txt", "w");
            bin_data = fopen("src/rxdata.bin","r");

            fseek(bin_data,0,SEEK_END);
            bin_data_size = ftell(bin_data);
            rewind(bin_data);

            while ( ftell(bin_data) < bin_data_size ) {

                fread(&recv_size,sizeof(recv_size),1,bin_data);
                fprintf(txt_data,"\nTRAMA: %lu\n",recv_size/4);

                fread(data,1,recv_size,bin_data);

                for ( int i = 0 ; i < recv_size/sizeof(uint32_t) ; i++ ) {
                    fprintf(txt_data,"%u\n",data[i]);
                }
            }
            signalHandler(6);
        }
    }

    return 0;
}

/** @ doxygen end group definition */
/** @ doxygen end group definition */
/** @ doxygen end group definition */
/*==================[end of file]============================================*/