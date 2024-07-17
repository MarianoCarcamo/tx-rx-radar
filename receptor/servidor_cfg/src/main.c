/* Copyright 2024, Adan Lema <adanlema@hotmail.com> & Carcamo Mariano <mgcarcamo98@gmail.com> */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

#include "al_mapping.h"
#include "al_server.h"
#include "al_params.h"
#include "log_manager.h"
#include "fpga_redpitaya.h"
/*==================[macros and definitions]=================================*/
#define EXT_ERR_CREATE_SERVER  1
#define EXT_ERR_CLIENT_CONNECT 2
#define EXT_ERR_LISTENING_SOCK 3
#define EXT_ERR_GETTING_ID      4

// #define ID          "rx_rpf098f4"
#define DEFAULT_PORT_SERVER 2027
#define IP_SERVER   "0.0.0.0"
#define BUFTCP_SIZE 1024
/*==================[internal data declaration]==============================*/
static char id [20];
static int port_server;
/*==================[internal functions declaration]=========================*/
static void MySignalHandler(int sig);
static void cmdArguments(int argc, char ** argv);
static void initConfigRx(fpgarx_t mem, params_t config);
static void setConfigRx(fpgarx_t mem, params_t config);

static void dataManagement(server_t sv, params_t params, fpgarx_t mem);
// static void dataManagement(server_t sv, params_t params);
/*==================[internal data definition]===============================*/
static fpgarx_t fpgarx = NULL;
static server_t server = NULL;
/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
static void initConfigRx(fpgarx_t mem, params_t config) {
    uint32_t phase_value = ceil((config->freq * 1e9) / 28610229);
    mem->start           = 0;
    mem->phaseCarrier    = phase_value;
    mem->addrReset       = 0;
    mem->writeEn         = 3;
    mem->start           = config->start;
}
static void cmdArguments(int argc, char ** argv) {
    int c;
    int digit_optind = 0;
    int port_flag = 0, id_flag = 0; 

    while ((c = getopt(argc, argv, "p:i:")) != -1) {
        int this_option_optind = optind ? optind : 1;
        switch (c) {
            case 'p':
                port_flag = 1;
                port_server = atoi(optarg);
                break;
            case 'i':
                id_flag = 1;
                strcpy(id,optarg);
                break;
            case '?':
                printf("The only valid options are:\n-p: To indicate the port number\n-i: To indicate the Board ID\n");
                break;
            default:
                break;
        }
    }

    if (optind < argc) {
        printf ("non-option ARGV-elements: ");
        printf ("\n");
    }

    if(!id_flag){
        printf("ID (-i BoardID) argument is required\n");
        exit(EXT_ERR_GETTING_ID);
    }

    if(!port_flag){
        port_server = DEFAULT_PORT_SERVER;
    }
}
static void setConfigRx(fpgarx_t mem, params_t config) {
    uint32_t phase_value = ceil((config->freq * 1e9) / 28610229);

    mem->start        = 0;
    mem->phaseCarrier = phase_value;
    mem->start        = config->start;
}
// static void dataManagement(server_t sv, params_t params) {
static void dataManagement(server_t sv, params_t params, fpgarx_t mem) {
    char *r_buff, *s_buff;
    r_buff = malloc(BUFTCP_SIZE);
    s_buff = malloc(BUFTCP_SIZE);
    if (r_buff == NULL || s_buff == NULL) {
        log_add("[ERROR]Error al reservar memoria.");
        exit(EXIT_FAILURE);
    }
    memset(r_buff, 0, BUFTCP_SIZE);
    memset(s_buff, 0, BUFTCP_SIZE);

    sprintf(s_buff, "{\"%s\"}", id);
    serverSend(sv, s_buff, BUFTCP_SIZE);

    while (1) {
        memset(r_buff, 0, BUFTCP_SIZE);
        if (!(serverRecive(server, r_buff, BUFTCP_SIZE) == 0)) {
            if (paramsStrtoJson(r_buff, params) == -1) {
                memset(s_buff, 0, BUFTCP_SIZE);
                strcpy(s_buff, "{\"error\":\"Formato JSON no identificado\"}\n");
                serverSend(server, s_buff, BUFTCP_SIZE);
            } else {
                setConfigRx(mem, params);
                paramsSaveConfig(params);
                memset(s_buff, 0, BUFTCP_SIZE);
                strcpy(s_buff, "{\"info\":\"Configuracion cargada con exito\"}");
                serverSend(server, s_buff, BUFTCP_SIZE);
                memset(s_buff, 0, BUFTCP_SIZE);
                sprintf(s_buff, "{\"freq\":%d, \"start\":%s}\n", params->freq,
                        params->start ? "true" : "false");
                serverSend(server, s_buff, BUFTCP_SIZE);
            }
        } else {
            break;
        }
    }
    free(r_buff);
    free(s_buff);
}

static void MySignalHandler(int sig) {
    log_add("[-]Cerrando el programa");
    mappingFinalize((addrs_t)fpgarx);
    serverCloseClient(server);
    serverDisconnect(server);
    log_add("[SUCCESS]Programa cerrado con exito");
    exit(EXIT_SUCCESS);
}

/*==================[external functions definition]==========================*/
int main(int argc, char **argv) {
    log_delete();
    cmdArguments(argc,argv);
    // Mapeo de memoria...
    fpgarx = (fpgarx_t)mappingInit(FPGARX_ADDR, FPGARX_REGS);
    if (fpgarx == NULL) {
        return -1;
    }
    params_t params = paramsCreate();
    initConfigRx(fpgarx, params);

    // Creacion del server...
    server = serverCreate(port_server, IP_SERVER);
    if (server == NULL) {
        log_add("[ERROR]Error al crear el server...");
        exit(EXT_ERR_CREATE_SERVER);
    }
    if (serverConnect(server) != 0) {
        log_add("[ERROR]Error al conectar el server...");
        exit(EXT_ERR_CREATE_SERVER);
    }
    if (serverListen(server, 2) == -1) {
        log_add("[ERROR]Error al poner en escucha el socket.");
        exit(EXT_ERR_LISTENING_SOCK);
    }

    // Manejo de señales de linux...
    signal(SIGABRT, MySignalHandler);
    signal(SIGINT, MySignalHandler);
    signal(SIGTERM, MySignalHandler);
    signal(SIGKILL, MySignalHandler);

    while (1) {
        if (serverAccept(server) != 0) {
            continue;
        }
        // dataManagement(server, params);
        dataManagement(server, params, fpgarx);
        serverCloseClient(server);
    }

    mappingFinalize((addrs_t)fpgarx);
    serverDisconnect(server);
    return 0;
}

/** @ doxygen end group definition */
/** @ doxygen end group definition */
/** @ doxygen end group definition */
/*==================[end of file]============================================*/