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
#include <pthread.h>
/*==================[macros and definitions]=================================*/
#define IP  "127.0.0.1"
#define PORT 2000

#define BUFF_SIZE 1024
#define SIZE_CODE 100

#define BARKER7_CODE  0x72
#define BARKER7_NUM   0x7
#define BARKER11_CODE 0x0712
#define BARKER11_NUM  0xB
#define BARKER13_CODE 0x1F35
#define BARKER13_NUM  0xD
/*==================[internal data declaration]==============================*/
typedef struct {
    uint32_t prf;
    uint32_t ab; 
    uint32_t freq;
    uint32_t code;
    uint32_t code_num;
    uint32_t start;
} params_s;

typedef struct {
    int * soc;
    params_s * params;
    char    * r_buff;
    char    * s_buff;
} thread_args;

/*==================[internal functions declaration]=========================*/
void code_manager(params_s * config, const char * code);
/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
int update_params(char *str, params_s * params){
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

int str_to_json(char *str, params_s * params){
    struct json_object *parsed_json;
    struct json_object *prf;
    struct json_object *freq;
    struct json_object *ab;
    struct json_object *code;
    struct json_object *start;

    parsed_json = json_tokener_parse(str);
    if(parsed_json != NULL){
        json_object_object_get_ex(parsed_json, "prf", &prf);
        params->prf = prf != NULL ? json_object_get_int(prf) : params->prf;

        json_object_object_get_ex(parsed_json, "freq", &freq);
        params->freq = freq != NULL ? json_object_get_int(freq) : params->freq;

        json_object_object_get_ex(parsed_json, "ab", &ab);
        params->ab = ab != NULL ? json_object_get_int(ab) : params->ab;

        json_object_object_get_ex(parsed_json, "start", &start);
        params->start = start != NULL ? json_object_get_int(start) : params->start;

        json_object_object_get_ex(parsed_json, "code", &code);
        if(code != NULL)
            code_manager(params, json_object_get_string(code));

        return 0;
    } else {
        return -1;
    }
}

void code_manager(params_s * config, const char * code){
    if(!strcmp("barker_7",code)){
        config->code = BARKER7_CODE;
        config->code_num = BARKER7_NUM;
        return;
    }
    if(!strcmp("barker_11",code)){
        config->code = BARKER11_CODE;
        config->code_num = BARKER11_NUM;
        return;
    }
    if(!strcmp("barker_13",code)){
        config->code = BARKER13_CODE;
        config->code_num = BARKER13_NUM;
        return;
    }
    printf("ERROR: Codigo invalido\n");
}

void * recive_msg(void * args){
    thread_args * arg = args;
    char *buff = arg->r_buff;
    int soc_d = *(arg->soc);
    params_s * params = arg->params;

    while(1){
        memset(buff,0,BUFF_SIZE);
        recv(soc_d, buff, BUFF_SIZE, 0);
        update_params(buff,params);
        printf("\n[server] %s", buff);
    }
}

void * send_msg(void * args){
    thread_args * arg = args;
    char *buff = arg->s_buff;
    params_s * params = arg->params;
    int soc_d = *(arg->soc);

    while(1){
        memset(buff,0,BUFF_SIZE);
        scanf("%s",buff);
        if(!strcmp(buff,"EXIT")){
            break;
        }
        if(str_to_json(buff, params) == -1) {
            printf("\nERROR: Se requiere una estructura JSON para enviar los datos\n");
        } else {
            memset(buff,0,BUFF_SIZE);
            sprintf(buff,"{\"prf\":%d, \"freq\":%d, \"ab\":%d, \"code\":%d, \"code-num\":%d, \"start\":%d}\n",params->prf, params->freq, params->ab, params->code, params->code_num, params->start);
            send(soc_d, buff, BUFF_SIZE, 0);
        }
    }
}

/*==================[external functions definition]==========================*/

int main() {
    int lfd;
    struct sockaddr_in server;
    char *r_buff, *s_buff;

    r_buff = malloc(BUFF_SIZE);
    s_buff = malloc(BUFF_SIZE);

    memset(r_buff,0,BUFF_SIZE);
    memset(s_buff,0,BUFF_SIZE);

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);

    connect(lfd, (struct sockaddr *)&server, sizeof server);

    params_s usr_params = {0};
    params_s *usr_params_p = &usr_params;

    pthread_t thread_recv;
    pthread_t thread_send;
    thread_args * thread_args_p = malloc(sizeof *thread_args_p);
    int err;

    thread_args_p->soc = &lfd;
    thread_args_p->r_buff = r_buff;
    thread_args_p->s_buff = s_buff;
    thread_args_p->params = usr_params_p;

    err = pthread_create(&thread_recv, NULL, recive_msg, thread_args_p);
    if (err) {
        printf("An error occured: %d", err);
        return 1;
    }

    err = pthread_create(&thread_send, NULL, send_msg, thread_args_p);
    if (err) {
        printf("An error occured: %d", err);
        return 1;
    }

    while (1) {
        if(!strcmp(s_buff,"EXIT")){
            printf("Cerrando conexion...\n");
            close(lfd);
            free(r_buff);
            free(s_buff);
            free(thread_args_p);
            pthread_cancel(thread_recv);
            pthread_cancel(thread_send);
            printf("Conexion cerrada con exito\n");
            break;
        }
    }

    return 0;
}

/** @ doxygen end group definition */
/** @ doxygen end group definition */
/** @ doxygen end group definition */
/*==================[end of file]============================================*/