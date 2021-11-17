#ifndef NS16550_H_
#define NS16550_H_

#include <stdint.h>

void ns16550_tx(uintptr_t addr, unsigned char c);

#endif /* NS16550_H_ */
