#ifndef __SYMBOL_IMPL_H_
#define __SYMBOL_IMPL_H_

#include "express/symbol.h"

inline struct Symbol_ *SYMBOL_new() {
    return ALLOC_new(&SYMBOL_fl);
}

inline void SYMBOL_destroy(struct Symbol_ *x) {
    ALLOC_destroy(&SYMBOL_fl,(Freelist *)x);
}

#endif /* __SYMBOL_IMPL_H_ */
