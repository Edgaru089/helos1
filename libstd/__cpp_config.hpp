#pragma once


#if __cplusplus >= 201103L
#define __CXX11
#endif
#if __cplusplus >= 201402L
#define __CXX14
#endif
#if __cplusplus >= 201703L
#define __CXX17
#endif
#if __cplusplus >= 202002L
#define __CXX20
#endif


#ifdef __CXX11
#define __NOTHROW noexcept
#define __NOTHROW_SINCECXX11 __NOTHROW
#else
#define __NOTHROW throw()
#define __NOTHROW_SINCECXX11
#endif

