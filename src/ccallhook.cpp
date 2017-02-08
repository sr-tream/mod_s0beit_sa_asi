#include "main.h"

CCallHook::CCallHook(void *addr, eSafeCall save, uint size, eCodePos pos)
{
    hook_addr = addr;
    _size = size;
    hook_pos = pos;
    _inlineAsm = new CShortAsm();

    orig_bytes = new byte[size + 1];
    memcpy_safe(orig_bytes, addr, size);

    if (pos == cp_before){
        if (orig_bytes[0] == 0xE9)
            memset_safe(orig_bytes, 0x90, 5);
        else ModOriginalBytes(reinterpret_cast<uint>(_inlineAsm->getAddr()) + _inlineAsm->getWriteOffset());
        _inlineAsm->insert(orig_bytes, size);
    }
    if (checkFlag(save, sc_registers))
        _inlineAsm->pushad();
    if (checkFlag(save, sc_flags))
        _inlineAsm->pushfd();

    hook_offset = _inlineAsm->getWriteOffset();
    if (pos == cp_skip)
        ModOriginalBytes(reinterpret_cast<uint>(_inlineAsm->getAddr()) + _inlineAsm->getWriteOffset());
    disable();

    if (checkFlag(save, sc_flags))
        _inlineAsm->popfd();
    if (checkFlag(save, sc_registers))
        _inlineAsm->popad();
    if (pos == cp_after){
        ModOriginalBytes(reinterpret_cast<uint>(_inlineAsm->getAddr()) + _inlineAsm->getWriteOffset());
        _inlineAsm->insert(orig_bytes, size);
    }

    _inlineAsm->jmp(reinterpret_cast<int>(addr) + static_cast<int>(size));

    memset_safe(addr, 0x90, size);
    byteValue<uint> v;
    v.value = reinterpret_cast<uint>(_inlineAsm->getAddr()) - (reinterpret_cast<uint>(addr) + 5);
    memset_safe(addr, 0xE9, 1);
    memset_safe(reinterpret_cast<void*>(reinterpret_cast<uint>(addr) + 1), v.bytes[0], 1);
    memset_safe(reinterpret_cast<void*>(reinterpret_cast<uint>(addr) + 2), v.bytes[1], 1);
    memset_safe(reinterpret_cast<void*>(reinterpret_cast<uint>(addr) + 3), v.bytes[2], 1);
    memset_safe(reinterpret_cast<void*>(reinterpret_cast<uint>(addr) + 4), v.bytes[3], 1);
}

CCallHook::~CCallHook()
{
    disable();
    //delete _inlineAsm;
}

void CCallHook::enable(void(__stdcall*func)())
{
    setNops();
    if (func == nullptr)
        return;
    _inlineAsm->setWriteOffset(hook_offset);
    _inlineAsm->call(reinterpret_cast<int>(func));
}

void CCallHook::disable()
{
    _inlineAsm->setWriteOffset(hook_offset);
    if (hook_pos == cp_skip)
        _inlineAsm->insert(orig_bytes, _size);
    else setNops();
}

template<typename T> inline
bool CCallHook::checkFlag(T value, T flag )
{
    return (value & flag);
}

void CCallHook::setNops()
{
    _inlineAsm->setWriteOffset(hook_offset);
    for (int i = 0; i < static_cast<int>(_size); ++i)
        _inlineAsm->nop();
}

void CCallHook::ModOriginalBytes(uint offset)
{
    // call and jmp (long)
    if (orig_bytes[0] == 0xE9 || orig_bytes[0] == 0xE8){
        byteValue<uint> v;
        v.bytes[0] = orig_bytes[1];
        v.bytes[1] = orig_bytes[2];
        v.bytes[2] = orig_bytes[3];
        v.bytes[3] = orig_bytes[4];
        uint o_addr = v.value + (reinterpret_cast<uint>(hook_addr) + 5);
        v.value = o_addr - (offset + 5);
        orig_bytes[1] = v.bytes[0];
        orig_bytes[2] = v.bytes[1];
        orig_bytes[3] = v.bytes[2];
        orig_bytes[4] = v.bytes[3];
    }
    // conditionals jmp's (long)
    else if (orig_bytes[0] == 0x0F) {
        if (orig_bytes[1] >= 0x81 && orig_bytes[1] <= 0x8F){
            byteValue<uint> v;
            v.bytes[0] = orig_bytes[2];
            v.bytes[1] = orig_bytes[3];
            v.bytes[2] = orig_bytes[4];
            v.bytes[3] = orig_bytes[5];
            uint o_addr = v.value + (reinterpret_cast<uint>(hook_addr) + 6);
            v.value = o_addr - (offset + 6);
            orig_bytes[2] = v.bytes[0];
            orig_bytes[3] = v.bytes[1];
            orig_bytes[4] = v.bytes[2];
            orig_bytes[5] = v.bytes[3];
        }
    }
}
