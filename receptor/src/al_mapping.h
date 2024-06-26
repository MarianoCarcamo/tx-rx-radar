/* Copyright 2024, Adan Lema <adanlema@hotmail.com> */

#ifndef AL_MAPPING_H
#define AL_MAPPING_H

/*==================[inclusions]=============================================*/
#include <stdint.h>
/*==================[macros]=================================================*/

#define FPGA_BUFF1         0x40000000
#define FPGA_BUFF2         0x40004000
#define FPGA_CTRL          0x40008000
#define FPGA_REG           4096
#define FPGA_MEM_CTRL_REG  6
#define BUFFER_SIZE       FPGA_REG * sizeof(int32_t)
#define FPGA_OFFSET_VALID  0

/*==================[typedef]================================================*/
typedef volatile uint32_t * addrs_t;
/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
addrs_t mappingInit(uint32_t addrs, uint32_t cant_reg);
void    mappingFinalize(addrs_t addr, uint32_t cant_reg);
/** @ doxygen end group definition */
/** @ doxygen end group definition */
/** @ doxygen end group definition */
/*==================[end of file]============================================*/
#endif
