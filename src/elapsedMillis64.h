#ifndef DEF_ELLAPSEDMILLIS_64
#define DEF_ELLAPSEDMILLIS_64

#include "elapsedMillis.h"

extern uint64_t millis64();

class elapsedMillis64
{
  private:
    uint64_t ms;
  public:
    elapsedMillis64(void) { ms = millis64(); }
    elapsedMillis64(uint64_t val) { ms = millis64() - val; }
    elapsedMillis64(const elapsedMillis64 &orig) { ms = orig.ms; }
    operator uint64_t () const { return millis64() - ms; }
    elapsedMillis64 & operator = (const elapsedMillis64 &rhs) { ms = rhs.ms; return *this; }
    elapsedMillis64 & operator = (uint64_t val)       { ms = millis64() - val; return *this; }
    elapsedMillis64 & operator -= (uint64_t val)      { ms += val ; return *this; }
    elapsedMillis64 & operator += (uint64_t val)      { ms -= val ; return *this; }
    elapsedMillis64 operator - (int val) const            { elapsedMillis64 r(*this); r.ms += val; return r; }
    elapsedMillis64 operator - (unsigned int val) const   { elapsedMillis64 r(*this); r.ms += val; return r; }
    elapsedMillis64 operator - (long val) const           { elapsedMillis64 r(*this); r.ms += val; return r; }
    elapsedMillis64 operator - (unsigned long val) const  { elapsedMillis64 r(*this); r.ms += val; return r; }
    elapsedMillis64 operator - (int64_t val) const        { elapsedMillis64 r(*this); r.ms += val; return r; }
    elapsedMillis64 operator - (uint64_t val) const       { elapsedMillis64 r(*this); r.ms += val; return r; }
    elapsedMillis64 operator + (int val) const            { elapsedMillis64 r(*this); r.ms -= val; return r; }
    elapsedMillis64 operator + (unsigned int val) const   { elapsedMillis64 r(*this); r.ms -= val; return r; }
    elapsedMillis64 operator + (long val) const           { elapsedMillis64 r(*this); r.ms -= val; return r; }
    elapsedMillis64 operator + (unsigned long val) const  { elapsedMillis64 r(*this); r.ms -= val; return r; }
    elapsedMillis64 operator + (int64_t val) const        { elapsedMillis64 r(*this); r.ms -= val; return r; }
    elapsedMillis64 operator + (uint64_t val) const       { elapsedMillis64 r(*this); r.ms -= val; return r; }
};

#endif
