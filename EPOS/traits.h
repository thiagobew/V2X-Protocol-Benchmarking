#ifndef __traits_h
#define __traits_h

template <typename T>
struct Traits
{
    static const bool hysterically_debugged = false;
    static const bool debugged = true;
    static const bool error = false;
    static const bool warning = true;
    static const bool info = true;
    static const bool trace = true;
};

class Build;
template<> struct Traits<Build> : public Traits<void>
{
    static const unsigned int NODES = 10001;
};

#endif
