/* Copyright 2024, Adan Lema <adanlema@hotmail.com> & Carcamo Mariano <mgcarcamo98@gmail.com> */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "al_mapping.h"
#include "al_server.h"
#include "log_manager.h"
/*==================[macros and definitions]=================================*/
#define EXT_ERR_CREATE_SERVER  1
#define EXT_ERR_CLIENT_CONNECT 2
#define EXT_ERR_LISTENING_SOCK 3
/*==================[internal data declaration]==============================*/
static int     server_sock, client_sock;

typedef struct BufferFpga {volatile uint32_t datos[FPGA_REG];} BufferFpga;

static BufferFpga *buff_1 = NULL;
static BufferFpga *buff_2 = NULL;

static struct RegistrosDeControl{
    volatile uint32_t phaseCarrier;
    volatile uint32_t addrReset;
    volatile union {
        uint32_t writeEn;
        struct{
            unsigned writeEn_1 : 1;
            unsigned writeEn_2 : 1;
            unsigned : 30;
        };
    };
    volatile uint32_t lastAddr;
    volatile uint32_t bufferToRead;
    volatile uint32_t lostData;
    volatile uint32_t start;
} *ctrl_regs;

static volatile uint32_t *data_sim;

/*==================[internal functions declaration]=========================*/
static void MySignalHandler(int sig);
/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
static void MySignalHandler(int sig) {
    log_add("[-]Cerrando el programa");
    // mappingFinalize(addr_buff_1, sizeof(BufferFpga)/sizeof(*(addrs_t)NULL));
    // mappingFinalize(addr_buff_2, sizeof(BufferFpga)/sizeof(*(addrs_t)NULL));
    // mappingFinalize((addrs_t) ctrl_regs, sizeof(*ctrl_regs)/sizeof(*((addrs_t)NULL)));
    close(client_sock);
    close(server_sock);
    log_add("[SUCCESS]Programa cerrado con exito");
    exit(EXIT_SUCCESS);
}

// static void sendData(int confd, addrs_t s_buff, addrs_t data){
//     // sprintf(s_buff,"Datos buffer uno:");
//     // send(confd, s_buff, BUFTCP_SIZE, 0);

//     // for (addrs_t p = mem; p < (mem + 200); ++p) {
//     // // for (addrs_t p = addr_buff_1; p < (addr_buff_1 + (addr_ctrl_regs[LAST_ADDR] == 0xFFFFFFFF)? 4095 : addr_ctrl_regs[LAST_ADDR]); ++p) {
//     //     // sprintf(s_buff, "\n%d", (int16_t)(0xFFFF & (*p)));
//     //     sprintf(s_buff,"\n%u",*p);
//     //     printf("%s",s_buff);
//     //     // send(confd, s_buff, BUFTCP_SIZE, 0);
//     // }
//     memset(s_buff, 0, BUFTCP_SIZE * 4);
//     memcpy(s_buff,data,BUFTCP_SIZE);
//     printf("\nDato:");
//     // printf("%u",mem[0]);
//     // printf("\n%u",s_buff[0]);
//     printf("%u",(uint16_t)(0xFFFF & (data[0])));
//     printf("\n%u",(uint16_t)(0xFFFF & (s_buff[0])));

//     // if (addr_ctrl_regs[LOST_DATA]) {
//     //     sprintf(s_buff,"\nSe Perdieron datos!!");
//     //     send(confd, s_buff, BUFTCP_SIZE, 0);
//     // }
// }

static void sendDataHandler(int confd) {
    // addrs_t s_buff = NULL;
    // uint32_t buff_leido = -1;
    int j = 0;
    ssize_t trama_size;
    // log_add("inicio de server client manager");

    // s_buff = malloc(BUFTCP_SIZE * 4);
    // if (s_buff == NULL) {
    //     log_add("[ERROR]Error al reservar memoria");
    //     exit(EXIT_FAILURE);
    // }

    log_add("Enviando datos");
    while(j<10){
        trama_size = (j+1) * sizeof(uint32_t);
        data_sim = (uint32_t*)malloc(trama_size);
        for ( int i = 0 ; i < j+1 ; i++ ) {
            data_sim[i] = i;
        }
        send(confd, &trama_size, sizeof(trama_size), 0);
        send(confd, (const uint32_t *) data_sim, ((j+1) * sizeof(uint32_t)), 0);
        j++;
        free((uint32_t*)data_sim);
        // if(buff_leido != ctrl_regs->bufferToRead){
        //     printf("\nLeer buffer %d\n",buff_leido);
        //     switch (ctrl_regs->bufferToRead) {
        //         case 1:
        //             buff_leido = 1;
        //             ctrl_regs->writeEn_1 = 0; 
        //             // sprintf(s_buff,"\nDatos buffer uno:");
        //             // send(confd, s_buff, BUFTCP_SIZE, 0);
        //             sendData(s_buff, buff_1->datos, confd);
        //             ctrl_regs->writeEn_1 = 1;
        //             i++;
        //             break;
        //         case 2:
        //             buff_leido = 2;
        //             ctrl_regs->writeEn_2=0;
        //             // sprintf(s_buff,"\nDatos buffer dos:");
        //             // send(confd, s_buff, BUFTCP_SIZE, 0);
        //             sendData(s_buff,buff_2->datos,confd);
        //             ctrl_regs->writeEn_2=1;
        //             i++;
        //             break;
        //         default:
        //             buff_leido = -1;
        //             break;
        //     };
        // }
    }
    // log_add("Datos enviados");
    // // free((void *)s_buff);
    // log_add("Buffer liberado");
}

/*==================[external functions definition]==========================*/
int main() {
    int n;
    struct sockaddr_in client;

    log_delete();

    // // Mapeo de memoria...
    // buff_1 = (BufferFpga*)mappingInit(FPGA_BUFF1, sizeof(BufferFpga)/sizeof(*((addrs_t)NULL)));
    // if (buff_1 == NULL) {
    //     return -1;
    // }
    // buff_2 = (BufferFpga*)mappingInit(FPGA_BUFF2, sizeof(BufferFpga)/sizeof(*((addrs_t)NULL)));
    // if (buff_2 == NULL) {
    //     return -1;
    // }
    // ctrl_regs = (struct RegistrosDeControl *) mappingInit(FPGA_CTRL, sizeof(*ctrl_regs)/sizeof(*((addrs_t)NULL)));
    // if (ctrl_regs == NULL) {
    //     return -1;
    // }
    // log_add("[SUCCESS]Mapeo de memoria realizo con exito");

    // Creacion del server...
    server_sock = serverInit();
    if (server_sock < 0) {
        log_add("[ERROR]Error al crear el server");
        exit(EXT_ERR_CREATE_SERVER);
    }
    log_add("server creado");
    if (listen(server_sock, 1) == -1) {
        log_add("[ERROR]Error al poner en escucha el socket");
        exit(EXT_ERR_LISTENING_SOCK);
    }
    log_add("server escuchando");

    // Manejo de señales de linux...
    signal(SIGABRT, MySignalHandler);
    signal(SIGINT,  MySignalHandler);
    signal(SIGTERM, MySignalHandler);
    signal(SIGKILL, MySignalHandler);
    log_add("Manejo de seniales linux inicializado");

    // //Valores iniciales
    // ctrl_regs->phaseCarrier =  0x14d5555c; // Frecuencia de portadora en 10 MHz - No deberia tocar esto desde aqui!!!
    // ctrl_regs->addrReset = 1;
    // ctrl_regs->writeEn   = 3;
    // ctrl_regs->start     = 1;

    // log_add("Valores inicializados");

    while (1) {
        // Conexion con el cliente...
        n = sizeof(client);
        client_sock = accept(server_sock, (struct sockaddr *)&client, &n);
        if (client_sock < 0) {
            log_add("[ERROR]Error al conectar el cliente");
            continue;
        }
        log_add("Cliente conectado");
        sendDataHandler(client_sock);
        close(client_sock);
    }

    return 0;
}
/** @ doxygen end group definition */
/** @ doxygen end group definition */
/** @ doxygen end group definition */
/*==================[end of file]============================================*/