/* Copyright 2024, Adan Lema <adanlema@hotmail.com> & Carcamo Mariano <mgcarcamo98@gmail.com> */

/*==================[inclusions]=============================================*/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <json-c/json.h>
#include <unistd.h>
#include "al_mapping.h"

/*==================[macros and definitions]=================================*/

#define IP          "127.0.0.1"
#define PORT        2000
#define BUFF_SIZE   1024

#define DEFAULT_AB          5000  
#define DEFAULT_CODE        1810  
#define DEFAULT_CODE_NUM    11   
#define DEFAULT_FREQ        10000000   
#define DEFAULT_PRF         50     
#define DEFAULT_START       0   

/*==================[internal data declaration]==============================*/

typedef struct {
    uint32_t prf;
    uint32_t ab; 
    uint32_t freq;
    uint32_t code;
    uint32_t code_num;
    uint32_t start;
} params_s;

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

int str_to_json(char *str, params_s * params){
    struct json_object *parsed_json;
    struct json_object *prf;
    struct json_object *freq;
    struct json_object *ab;
    struct json_object *code;
    struct json_object *code_num;
    struct json_object *start;

    parsed_json = json_tokener_parse(str);
    if(parsed_json != NULL){
        json_object_object_get_ex(parsed_json, "prf", &prf);
        params->prf = prf != NULL ? json_object_get_int(prf) : params->prf;

        json_object_object_get_ex(parsed_json, "freq", &freq);
        params->freq = freq != NULL ? json_object_get_int(freq) : params->freq;

        json_object_object_get_ex(parsed_json, "ab", &ab);
        params->ab = ab != NULL ? json_object_get_int(ab) : params->ab;

        json_object_object_get_ex(parsed_json, "code", &code);
        params->code = code != NULL ? json_object_get_int(code) : params->code;

        json_object_object_get_ex(parsed_json, "code-num", &code_num);
        params->code_num = code_num != NULL ? json_object_get_int(code_num) : params->code_num;

        json_object_object_get_ex(parsed_json, "start", &start);
        params->start = start != NULL ? json_object_get_int(start) : params->start;

        return 0;

    } else {
        return -1;
    }
}

void set_config(addrs_t mem_p, params_s * config){
    /*  Variables por defecto */
    uint32_t prt_value   = ceil(122880000 / config->prf);
    uint32_t phase_value = ceil((config->freq * 1e9) / 28610229);
    uint32_t tb          = ceil(122880000 / config->ab);
    uint32_t t_value     = ceil(config->code_num * tb);

    /* Escritura y liberacion del bloque */
    mem_p[OFFSET_START]  = 0;
    mem_p[OFFSET_PHASE]  = phase_value;
    mem_p[OFFSET_PERIOD] = t_value;
    mem_p[OFFSET_PRT]    = prt_value;
    mem_p[OFFSET_CODE]   = config->code;
    mem_p[OFFSET_NUMDIG] = config->code_num;
    mem_p[OFFSET_TB]     = tb;
    mem_p[OFFSET_START]  = config->start;
}

// void client_server_management(int confd, params_s * params, addrs_t addr_fpga){
void client_server_management(int confd, params_s * params){
    char *r_buff, *s_buff;

    r_buff = malloc(BUFF_SIZE);
    s_buff = malloc(BUFF_SIZE);
    
    if (r_buff == NULL || s_buff == NULL) {
        perror("Error al reservar memoria");
        exit(EXIT_FAILURE);
    }

    memset(r_buff,0,BUFF_SIZE);
    memset(s_buff,0,BUFF_SIZE);

    sprintf(s_buff,"{\"info\":\"Configuracion actual\"}\n");
    send(confd, s_buff, BUFF_SIZE, 0);

    memset(s_buff,0,BUFF_SIZE);
    sprintf(s_buff,"{\"prf\":%d, \"freq\":%d, \"ab\":%d, \"code\":%d, \"code-num\":%d, \"start\":%s}\n",params->prf, params->freq, params->ab, params->code, params->code_num, params->start? "true" : "false");
    send(confd, s_buff, BUFF_SIZE, 0);

    while(1){
        if(!(recv(confd, r_buff, BUFF_SIZE, 0) == 0)){
            if(str_to_json(r_buff, params) == -1){
                memset(s_buff,0,BUFF_SIZE);
                strcpy(s_buff,"{\"error\":\"Formato JSON no identificado\"}\n");
                send(confd, s_buff, BUFF_SIZE, 0);
            } else {
                // set_config(addr_fpga, params);
                memset(s_buff,0,BUFF_SIZE);
                strcpy(s_buff,"{\"info\":\"Configuracion cargada con exito\"}\n");
                send(confd, s_buff, BUFF_SIZE, 0);
            }
            memset(s_buff,0,BUFF_SIZE);
            sprintf(s_buff,"{\"prf\":%d, \"freq\":%d, \"ab\":%d, \"code\":%d, \"code-num\":%d, \"start\":%s}\n",params->prf, params->freq, params->ab, params->code, params->code_num, params->start? "true" : "false");
            send(confd, s_buff, BUFF_SIZE, 0);
        } else {
            break;
        }
    }

    free(r_buff);
    free(s_buff);
}

/*==================[external functions definition]==========================*/

int main() {
    struct sockaddr_in client, server;
    int lfd, confd, n;
    params_s *params_p = malloc(sizeof(*params_p));

    memset(params_p, 0, sizeof(*params_p));

    //Inicializo valores por defecto
    params_p->ab = DEFAULT_AB;
    params_p->code = DEFAULT_CODE;
    params_p->code_num = DEFAULT_CODE_NUM;
    params_p->freq = DEFAULT_FREQ;
    params_p->prf = DEFAULT_PRF;
    params_p->start = DEFAULT_START;

    memset(&server, 0, sizeof(server));
    memset(&client, 0, sizeof(client));

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Realiza mapeo de memoria
    // addrs_t addr_fpga = mapping_initialize(FPGA_ADDRS, FPGA_REG);
    // if (addr_fpga == NULL) {
    //     return -1;
    // }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    //Seteo valores por defecto
    // set_config(addr_fpga,params_p);

    //Crea y vincula el socket - Servidor
    lfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (lfd == -1) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);

    if (bind(lfd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("Error al vincular el socket");
        exit(EXIT_FAILURE);
    }

    if (listen(lfd, 2) == -1) {
        perror("Error al poner en escucha el socket");
        exit(EXIT_FAILURE);
    }

    while(1){
        n = sizeof(client);
        confd = accept(lfd, (struct sockaddr *)&client, &n);
        if (confd < 0) { 
            exit(0); 
        }
        client_server_management(confd, params_p);
        close(confd);
    }

    // mapping_finalize(addr_fpga, FPGA_REG);
    free(params_p);
    close(confd);
    close(lfd);
    return 0;
}

/** @ doxygen end group definition */
/** @ doxygen end group definition */
/** @ doxygen end group definition */
/*==================[end of file]============================================*/
