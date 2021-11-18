#ifndef QUARD_STAR_H_
#define QUARD_STAR_H_

#include "riscv-reg.h"

#ifdef __ASSEMBLER__
#define CONS(NUM, TYPE)NUM
#else
#define CONS(NUM, TYPE)NUM##TYPE
#endif /* __ASSEMBLER__ */

#define PRIM_HART			0

#define SYSCON_ADDR		    CONS(0x00100000, UL)
#define NS16550_ADDR		CONS(0x10000000, UL)
#define SIFIVE_SPI_ADDR		CONS(0x10007000, UL)

#ifndef __ASSEMBLER__

#endif /* __ASSEMBLER__ */

#endif /* QUARD_STAR_H_ */
