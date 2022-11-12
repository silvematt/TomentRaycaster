#pragma once
#include <stdlib.h>
/*
    NOTES:

    - types, symbols and names are WIN32
*/

#ifdef _WIN32
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#endif

#ifdef  _MSC_VER
#define DBJ_FUNCSIG __FUNCDNAME__
#else
#define DBJ_FUNCSIG __FUNCSIG__
#endif

#define DBJ_STR_(X) #X
#define DBJ_STR(X) DBJ_STR_(X)

#define DBJ_CCAT_(A,B) A ## B
#define DBJ_CCAT(A,B) DBJ_CCAT_(A,B)

#define DBJ_PRINT( ... ) \
     fprintf( stdout, __VA_ARGS__ );

#define DBJ_TERROR( ... ) \
{ \
     fprintf( stderr, __VA_ARGS__ );\
     fprintf(stderr,"\n\n%s (%-6d)\n\nfunction: %s\n\n terror exit now!\n\n", __FILE__, __LINE__, DBJ_FUNCSIG );\
     system("@pause"); \
     exit(EXIT_FAILURE); \
}

#define DBJ_TERROR_NULL(X) \
if ( NULL == (X)) { \
     fprintf(stderr,"\n\n%s (%-6d)\n\nfunction: %s , expression : %s\n\nrezult is NULL, terror exit now!\n\n", __FILE__, __LINE__, DBJ_FUNCSIG, (#X));\
     system("@pause"); \
     exit(EXIT_FAILURE); \
}

#define DBJ_TERROR_FALSE(X) \
if ( FALSE == (X)) { \
     fprintf(stderr,"\n\n%s (%-6d)\n\nfunction: %s , expression : %s\n\nrezult is FALSE, terror exit now!\n\n", __FILE__, __LINE__, DBJ_FUNCSIG, (#X));\
     system("@pause"); \
     exit(EXIT_FAILURE); \
}

// #define DBJ_MALLOC( SZE_) (void * DBJ_CCAT(vp_, __COUNT__) = calloc(1, SZE_ ), DBJ_TERROR_NULL(DBJ_CCAT(vp_, __COUNT__)), DBJ_CCAT(vp_, __COUNT__))

__inline void* DBJ_MALLOC( size_t size_ )
{ 
    void* vp_ = calloc(1, size_);  DBJ_TERROR_NULL(vp_); return vp_;
}

/*
 WIN32 utils
*/

#include <Windows.h>

// use the result immediately
__inline const char* const dbj_current_directory()
{
    #define buflen_ 0xFF
    static char lpBuffer[buflen_] = {0};
    ZeroMemory( lpBuffer, buflen_ );
    DBJ_TERROR_FALSE( GetCurrentDirectoryA(
        /*[in]  DWORD  nBufferLength*/ buflen_ - 1,
        /*[out] LPTSTR */ lpBuffer 
    ));
    return lpBuffer;
#undef buflen_
}
