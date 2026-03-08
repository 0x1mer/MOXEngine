#pragma once

#include "BlockRegistry.h"

#define REGISTER_BLOCK(BLOCKTYPE)                       \
namespace {                                             \
    struct BLOCKTYPE##AutoRegister                      \
    {                                                   \
        BLOCKTYPE##AutoRegister()                       \
        {                                               \
            BlockRegistry::Register(BLOCKTYPE()());     \
        }                                               \
    };                                                  \
    static BLOCKTYPE##AutoRegister global_##BLOCKTYPE;  \
}