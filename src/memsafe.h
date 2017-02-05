#ifndef MEMSAFE_H
#define MEMSAFE_H

/*#include <stdint.h>
#include <windows.h>*/

enum eCodePos{
    cp_skip = 0,
    cp_before,
    cp_after
};

enum eSafeCall{
    sc_not = 0,
    sc_registers,
    sc_flags
};

namespace memsafe
{
void *copy(void *dest, const void *src, size_t stLen);
int compare(const void *_s1, const void *_s2, uint32_t len);
int set(void *_dest, int c, DWORD len);
};

#endif // MEMSAFE_H
