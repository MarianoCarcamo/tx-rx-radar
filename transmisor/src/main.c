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

#define IP  "127.0.0.1"
#define PORT 2000
#define BUFF_SIZE 1024

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

void upload_config(addrs_t mem_p, params_s * config){
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

/*==================[external functions definition]==========================*/

int main() {
    struct sockaddr_in client, server;
    int lfd, confd, n;
    char *r_buff, *s_buff;

    params_s sent_params = {0};
    params_s *sent_params_p = &sent_params;

    r_buff = malloc(BUFF_SIZE);
    s_buff = malloc(BUFF_SIZE);

    memset(r_buff,0,BUFF_SIZE);
    memset(s_buff,0,BUFF_SIZE);

    // Realiza mapeo de memoria
    // addrs_t addr_fpga = mapping_initialize(FPGA_ADDRS, FPGA_REG);
    // if (addr_fpga == NULL) {
    //     return -1;
    // }
    // printf("\n\n\nMapeo de memoria realizo con exito...\n");

    //Crea y vincula el socket - Servidor
    lfd = socket(AF_INET, SOCK_STREAM, 0); 
    server.sin_family = AF_INET;
    server.sin_port = PORT;
    server.sin_addr.s_addr = inet_addr(IP);

    bind(lfd, (struct sockaddr *)&server, sizeof server);
    listen(lfd, 1);

    n = sizeof client;
    confd = accept(lfd, (struct sockaddr *)&client, &n);

    while(1){
        recv(confd, r_buff, BUFF_SIZE, 0);

        if(str_to_json(r_buff, sent_params_p) == -1){
            memset(s_buff,0,BUFF_SIZE);
            strcpy(s_buff,"{\"error\":\"Formato JSON no identificado\"}\n");
            send(confd, s_buff, BUFF_SIZE, 0);
        } else {
            // upload_config(addr_fpga, sent_params_p);
            memset(s_buff,0,BUFF_SIZE);
            strcpy(s_buff,"{\"info\":\"Configuracion cargada con exito\"}\n");
            send(confd, s_buff, BUFF_SIZE, 0);
        }

        memset(s_buff,0,BUFF_SIZE);
        sprintf(s_buff,"{\"prf\":%d, \"freq\":%d, \"ab\":%d, \"code\":%d, \"code-num\":%d, \"start\":%d}\n",sent_params_p->prf, sent_params_p->freq, sent_params_p->ab, sent_params_p->code, sent_params_p->code_num, sent_params_p->start);

        send(confd, s_buff, BUFF_SIZE, 0);
    }

    // mapping_finalize(addr_fpga, FPGA_REG);
    free(r_buff);
    free(s_buff);
    close(confd);
    close(lfd);
    return 0;
}

/** @ doxygen end group definition */
/** @ doxygen end group definition */
/** @ doxygen end group definition */
/*==================[end of file]============================================*/