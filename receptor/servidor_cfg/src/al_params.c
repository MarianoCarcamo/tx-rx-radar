/* Copyright 2024, Adan Lema <adanlema@hotmail.com> & Carcamo Mariano <mgcarcamo98@gmail.com> */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <json-c/json.h>

#include "al_params.h"
#include "al_mapping.h"
#include "log_manager.h"
/*==================[macros and definitions]=================================*/
#define OFFSETPHASE 0
#define OFFSETSTART 6

#define DEFAULT_FREQ  10000000
#define DEFAULT_START 0
#define FILE_TXT      "src/config.txt"
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/
static void paramsLoadConfig();
static void paramsCommand(const char * command);
/*==================[internal data definition]===============================*/
static struct params_s parametros = {0};
/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
static void paramsLoadConfig() {
    FILE * file = fopen(FILE_TXT, "r");
    if (file == NULL) {
        log_add("[ERROR]Error al abrir el archivo.");
        return;
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    if (fileSize == 0) {
        parametros.freq  = DEFAULT_FREQ;
        parametros.start = DEFAULT_START;

    } else {
        rewind(file);
        char texto_json[100];
        if (fgets(texto_json, sizeof(texto_json), file) != NULL) {
            paramsStrtoJson(texto_json, &parametros);
            parametros.start = DEFAULT_START;
        }
    }
    fclose(file);
}

static void paramsCommand(const char * command) {
    if (!strcmp(command, "default")) {
        parametros.freq  = DEFAULT_FREQ;
        parametros.start = DEFAULT_START;
    }
}

/*==================[external functions definition]==========================*/

int paramsStrtoJson(char * str, params_t params) {
    struct json_object * parsed_json;
    struct json_object * freq;
    struct json_object * start;

    parsed_json = json_tokener_parse(str);
    if (parsed_json != NULL) {
        json_object_object_get_ex(parsed_json, "freq", &freq);
        params->freq = freq != NULL ? json_object_get_int(freq) : params->freq;

        json_object_object_get_ex(parsed_json, "start", &start);
        params->start = start != NULL ? json_object_get_int(start) : params->start;
        return 0;
    } else {
        return -1;
    }
}

void paramsSetConfig(addrs_t mem_p, params_t config) {
    uint32_t phase_value = ceil((config->freq * 1e9) / 28610229);

    /* Escritura y liberacion del bloque */
    mem_p[OFFSETSTART] = 0;
    mem_p[OFFSETPHASE] = phase_value;
    mem_p[OFFSETSTART] = config->start;
}

void paramsSaveConfig(params_t params) {
    FILE * file = fopen(FILE_TXT, "w");
    if (file == NULL) {
        log_add("[ERROR]Error al abrir el archivo.");
        return;
    }
    fprintf(file, "{\"freq\":%d, \"start\":%d}\n", params->freq, params->start);
    fclose(file);
    if (ferror(file)) {
        log_add("[ERROR]Error al cerrar el archivo.");
    }
}

params_t paramsCreate() {
    paramsLoadConfig();
    return &parametros;
}

/** @ doxygen end group definition */
/** @ doxygen end group definition */
/** @ doxygen end group definition */