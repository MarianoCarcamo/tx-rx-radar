/* Copyright 2024, Adan Lema <adanlema@hotmail.com> & Carcamo Mariano <mgcarcamo98@gmail.com> */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include <json-c/json.h>
#include "al_params.h"

/*==================[macros and definitions]=================================*/
#define CANT_PARAMS 10
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/
static void paramsCodeManager(params_t config, const char * code);
/*==================[internal data definition]===============================*/
static uint8_t         ocupacion_pr            = 0;
static struct params_s parametros[CANT_PARAMS] = {0};
/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
static void paramsCodeManager(params_t config, const char * code) {
    if (!strcmp("barker_7", code)) {
        config->code     = BARKER7_CODE;
        config->code_num = BARKER7_NUM;
        return;
    }
    if (!strcmp("barker_11", code)) {
        config->code     = BARKER11_CODE;
        config->code_num = BARKER11_NUM;
        return;
    }
    if (!strcmp("barker_13", code)) {
        config->code     = BARKER13_CODE;
        config->code_num = BARKER13_NUM;
        return;
    }
    printf("ERROR: Codigo invalido\n");
}
/*==================[external functions definition]==========================*/
params_t paramsCreate() {
    uint8_t posicion = 0;
    if (ocupacion_pr < CANT_PARAMS) {
        posicion = ocupacion_pr;
        ocupacion_pr++;
    } else {
        return NULL;
    }
    return &parametros[posicion];
}

int paramsStrtoJson(char * str, params_t params) {
    struct json_object * parsed_json;
    struct json_object * prf;
    struct json_object * freq;
    struct json_object * ab;
    struct json_object * code;
    struct json_object * code_num;
    struct json_object * start;

    parsed_json = json_tokener_parse(str);
    if (parsed_json != NULL) {
        json_object_object_get_ex(parsed_json, "prf", &prf);
        params->prf = prf != NULL ? json_object_get_int(prf) : params->prf;

        json_object_object_get_ex(parsed_json, "freq", &freq);
        params->freq = freq != NULL ? json_object_get_int(freq) : params->freq;

        json_object_object_get_ex(parsed_json, "ab", &ab);
        params->ab = ab != NULL ? json_object_get_int(ab) : params->ab;

        json_object_object_get_ex(parsed_json, "start", &start);
        params->start = start != NULL ? json_object_get_int(start) : params->start;

        json_object_object_get_ex(parsed_json, "code", &code);
        if (code != NULL)
            paramsCodeManager(params, json_object_get_string(code));
        return 0;
    } else {
        return -1;
    }
}

int paramsUpdate(char * str, params_t params) {
    struct json_object * parsed_json;
    struct json_object * prf;
    struct json_object * freq;
    struct json_object * ab;
    struct json_object * code;
    struct json_object * code_num;
    struct json_object * start;

    parsed_json = json_tokener_parse(str);
    if (parsed_json != NULL) {
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
/** @ doxygen end group definition */
/** @ doxygen end group definition */
/** @ doxygen end group definition */