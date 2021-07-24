//
// Created by PGZXB on 2021/4/21.
//
#ifndef PGDEBUG_H
#define PGDEBUG_H

#define PGZXB_PASS (void(0))
#define PGZXB_UNUSED(val) (void(val))

#if defined(PGZXB_DEBUG)

#include <bitset>
#include <iostream>
#define PGZXB_DEBUG_INFO_HEADER "[DEBUG]In " << __FILE__ << " in " << __func__ << " at " << __LINE__ << " : "
#define PGZXB_DEBUG_PrintVar(val) std::cout << PGZXB_DEBUG_INFO_HEADER << #val" : " << val << "\n"
#define PGZBX_DEBUG_PrintBin(val) std::cout << PGZXB_DEBUG_INFO_HEADER << #val" : " << std::bitset<sizeof(val) * 8>(val) << "\n"
#define PGZXB_DEBUG_CallFunc(func) PGZXB_DEBUG_PrintVar(func)

#else

#define PGZXB_DEBUG_INFO_HEADER ""
#define PGZXB_DEBUG_PrintVar(val) PGZXB_UNUSED(val)
#define PGZBX_DEBUG_PrintBin(val) PGZXB_UNUSED(val)
#define PGZXB_DEBUG_CallFunc(func) PGZXB_UNUSED(val)

#endif

#endif // PGDEBUG_H
