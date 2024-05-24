/* Copyright 2024, Adan Lema <adanlema@hotmail.com> */

#ifndef LOG_DESCRIPTOR_H
#define LOG_DESCRIPTOR_H

/*==================[inclusions]=============================================*/

#include <stdio.h>

/*==================[macros]=================================================*/

#define LOG_PATH "./src/rxsyslog.log"

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void log_add(char * str);
void log_delete();

/** @ doxygen end group definition */
/** @ doxygen end group definition */
/** @ doxygen end group definition */
/*==================[end of file]============================================*/
#endif
