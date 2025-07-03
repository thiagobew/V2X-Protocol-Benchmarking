#ifndef EPOS_COMMON_H_
#define EPOS_COMMON_H_

#include <iostream>
#include <cassert>
#include <random>
#include "meta.h"
#include "traits.h"
#include <climits>

typedef unsigned int Interrupt_Id;

typedef void (* Interrupt_Handler)(const Interrupt_Id &);

typedef unsigned char Percent;

enum Debug_Error {ERR = 1};
enum Debug_Warning {WRN = 2};
enum Debug_Info {INF = 3};
enum Debug_Trace {TRC = 4};

class Null_Streambuf : public std::streambuf
{
protected:
    virtual int overflow(int c) {
        setp(_dummy_buffer, _dummy_buffer + sizeof(_dummy_buffer));
        return (c == EOF) ? '\0' : c;
    }
private:
    char _dummy_buffer[64];
};

class Null_Stream : public Null_Streambuf, public std::ostream
{
public:
    Null_Stream() : std::ostream(this) { }
};

template<typename T, typename L>
inline std::ostream & db(const L & l) {
    if(Traits<T>::debugged)
        return std::cout;
    else {
        static Null_Stream _null_stream;
        return _null_stream;
    }
}

typedef std::ostream OStream;
typedef std::ostream Debug;

class Random
{
public:
    static int random() { 
        std::random_device dev;
        std::mt19937 rng(dev());

        std::uniform_int_distribution<std::mt19937::result_type> dist(0,INT_MAX);

        return dist(rng);
    }
};

class CPU
{
public:
    static void int_enable() {}
    static void int_disable() {}
};

enum Power_Mode {
    PM_FULL,
    PM_LIGHT,
    PM_SLEEP,
    PM_OFF,
};

//============================================================================
// NAMESPACES AND DEFINITIONS
//============================================================================
namespace EPOS {
    namespace S {
        namespace U {}
        using namespace U;
    }
}

#define __BEGIN_API             namespace EPOS {
#define __END_API               }
#define _API                    ::EPOS

#define __BEGIN_UTIL            namespace EPOS { namespace S { namespace U {
#define __END_UTIL              }}}
#define __USING_UTIL            using namespace S::U;
#define _UTIL                   ::EPOS::S::U

#define __BEGIN_SYS             namespace EPOS { namespace S {
#define __END_SYS               }}
#define __USING_SYS             using namespace EPOS::S;
#define _SYS                    ::EPOS::S

__BEGIN_UTIL

inline void memcpy(void * dst, const void * src, unsigned int size) {
    assert(dst != 0);
    assert(src != 0);
    assert(size > 0);
    for (unsigned int i = 0; i < size; ++i)
        ((char *)dst)[i] = ((char *)src)[i];
}

__END_UTIL

#endif /* EPOS_COMMON_H_ */
