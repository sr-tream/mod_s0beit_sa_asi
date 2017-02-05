#ifndef CCALLHOOK_H
#define CCALLHOOK_H

/*#include "cshortasm.h"
#include "memsafe.h"*/

class CCallHook
{
public:
    CCallHook(void* addr, eSafeCall save, uint size = 5, eCodePos pos = cp_after);
    ~CCallHook();

    void enable(void(__stdcall*func)());
    void disable();

protected:
    byte* orig_bytes;
    uint _size;
    uint hook_offset;
    void* hook_addr;
    eCodePos hook_pos;

    template<typename T>
    bool checkFlag(T value, T flag );

private:
    CShortAsm *_inlineAsm;

    void setNops();
    void ModOriginalBytes(uint offset);
};

#endif // CCALLHOOK_H
