#ifndef __HASH_IMPL_H
#define __HASH_IMPL_H

#include "express/hash.h"

inline size_t HASHhash(unsigned char *);
inline void HASHexpand_table(Hash_Table);

inline Hash_Table HASH_Table_new();
inline void HASH_Table_destroy(Hash_Table x);

#endif /* __HASH_IMPL_H */
