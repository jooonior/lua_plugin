#include "engine.hpp"

#include "platform.hpp"


bool ConnectEnginePrintFunctions()
{
    auto *tier0 = GetModuleHandle(DYNAMIC_LIBRARY("tier0"));

    if (tier0 == nullptr)
        return false;

    Print = GetSymbolAddress<PrintFn_t>(tier0, "Msg");
    if (Print == nullptr)
        return false;

    Warn = GetSymbolAddress<PrintFn_t>(tier0, "Warning");
    if (Warn == nullptr)
        return false;

    return true;
}
