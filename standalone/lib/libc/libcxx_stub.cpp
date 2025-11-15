/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * FilePath: libcpp_stub.cpp
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 09:25:20
 * Description:  This file is for C++ standard library stub function implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhugengyu   2023-05-19   first release
 */

#include <cstdlib>
#include <iostream>
#include <new>
#include <functional>

#include "system_call.h"
#include "fdebug.h"

#ifdef LOG_LOCAL_LEVEL
#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL FT_LOG_ERROR
#endif

#define FCPP_DEBUG_TAG "CPP"
#define FCPP_ERROR(format, ...) FT_DEBUG_PRINT_E(FCPP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCPP_WARN(format, ...) FT_DEBUG_PRINT_W(FCPP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCPP_INFO(format, ...) FT_DEBUG_PRINT_I(FCPP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCPP_DEBUG(format, ...) FT_DEBUG_PRINT_D(FCPP_DEBUG_TAG, format, ##__VA_ARGS__)

/*
 * override all the global C++ operators responsible for memory allocation/deallocation
 * */
void* operator new(size_t size) noexcept
{
    FCPP_DEBUG("%s", __func__);
    return malloc(size);
}

void operator delete(void *p) noexcept
{
	FCPP_DEBUG("%s %p", __func__, p);
    free(p);
}

void operator delete(void *p, size_t size) noexcept
{
	FCPP_DEBUG("%s %p", __func__, p);
    free(p);
}

void* operator new[](size_t size) noexcept
{
    FCPP_DEBUG("%s", __func__);
    return operator new(size); /*  Same as regular new */
}

void operator delete[](void *p) noexcept
{
	FCPP_DEBUG("%s %p", __func__, p);
    operator delete(p); /*  Same as regular delete  */
}

void operator delete[](void *p, size_t size) noexcept
{
	FCPP_DEBUG("%s %p", __func__, p);
    operator delete(p); /*  Same as regular delete  */
}

void* operator new(size_t size, std::nothrow_t) noexcept
{
    FCPP_DEBUG("%s", __func__);
    return operator new(size); /*  Same as regular new  */
}

void operator delete(void *p,  std::nothrow_t) noexcept
{
	FCPP_DEBUG("%s %p", __func__, p);
    operator delete(p); /*  Same as regular delete  */
}

void* operator new[](size_t size, std::nothrow_t) noexcept
{
    FCPP_DEBUG("%s %p", __func__);
    return operator new(size); /*  Same as regular new  */
}

void operator delete[](void *p,  std::nothrow_t) noexcept
{
	FCPP_DEBUG("%s %p", __func__, p);
    operator delete(p); /*  Same as regular delete  */
}

/*  Virtual destructors require also operator delete, in case they are  */
/*  invoked using delete operator.  */

/*  Required when there is pure virtual function  */
extern "C" void __cxa_pure_virtual()
{
	FCPP_DEBUG("%s: Illegal to call a pure virtual function.", __func__);
    while (true) {}
}

/*  Compiler requires this function when there are static objects that  */
/*  require custom destructors  */
extern "C" int __aeabi_atexit(
    void *object,
    void (*destructor)(void *),
    void *dso_handle)
{
    static_cast<void>(object);
    static_cast<void>(destructor);
    static_cast<void>(dso_handle);
    return 0;
}

namespace std
{

namespace placeholders
{

decltype(std::placeholders::_1) _1;
decltype(std::placeholders::_2) _2;
decltype(std::placeholders::_3) _3;
decltype(std::placeholders::_4) _4;

}  /*  namespace placeholders  */

}  /*  namespace std  */

__attribute__((weak)) void *__dso_handle = 0;

int sigprocmask (int a, const sigset_t *b, sigset_t *c)
{
   return 1;
}