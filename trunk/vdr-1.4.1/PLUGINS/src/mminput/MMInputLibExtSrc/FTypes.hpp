#ifndef FTYPES_HPP
#define FTYPES_HPP

#ifndef NULL
#define NULL 0
#endif

typedef unsigned long long uint64;
typedef unsigned long uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef signed long long sint64;
typedef signed long sint32;
typedef signed short sint16;
typedef signed char sint8;

/// Helper macro to define most of the binary arithmetic operators
#define ARITHMETIC_OPERATORS( THIS, ARG, OTHA, OTHL, JOIN, OTHR, OTHZ )	\
	ARITHMETIC_OPERATOR_##JOIN( THIS, ARG, OTHA, OTHL, |, OTHR, OTHZ )	\
	ARITHMETIC_OPERATOR_##JOIN( THIS, ARG, OTHA, OTHL, &, OTHR, OTHZ )	\
	ARITHMETIC_OPERATOR_##JOIN( THIS, ARG, OTHA, OTHL, ^, OTHR, OTHZ )	\
	ARITHMETIC_OPERATOR_##JOIN( THIS, ARG, OTHA, OTHL, +, OTHR, OTHZ )	\
	ARITHMETIC_OPERATOR_##JOIN( THIS, ARG, OTHA, OTHL, -, OTHR, OTHZ )	\
	ARITHMETIC_OPERATOR_##JOIN( THIS, ARG, OTHA, OTHL, *, OTHR, OTHZ )	\
	ARITHMETIC_OPERATOR_##JOIN( THIS, ARG, OTHA, OTHL, /, OTHR, OTHZ )	\
	ARITHMETIC_OPERATOR_##JOIN( THIS, ARG, OTHA, OTHL, %, OTHR, OTHZ )	\

#define ARITHMETIC_OPERATOR_DISJOIN( THIS, ARG, OTHA, OTHL, OP, OTHR, OTHZ )	\
	THIS operator OP##=( ARG val )	{ OTHA (OTHL OP OTHR) OTHZ }
#define ARITHMETIC_OPERATOR_CONJOIN( THIS, ARG, OTHA, OTHL, OP, OTHR, OTHZ )	\
	THIS operator OP##=( ARG val )	{ OTHA (OTHL OP##OTHR) OTHZ }

/**
 *	A class for managing an alien endian long
 */
class AEUInt32Base
{
public:
	inline operator uint32() const	{ return this->out(); }
	
	inline void in( uint32 me )		{ _in( *this, me ); }
	inline uint32 out() const		{ return _out( *this ); }
	
private:
#ifndef __LITTLE_ENDIAN__
  #ifdef __METROWERKS__
	static inline void _in( register AEUInt32Base & self, register uint32 me )
		{ /*asm {stwbrx me, 0, self;}*/ __stwbrx( me, &self, 0 );  }

	static inline uint32 _out( register const AEUInt32Base & self )
		{ /*asm {lwbrx r3, 0, self;}*/ return __lwbrx( (void*)&self, 0 ); }
  #else
	static inline void _in( register AEUInt32Base & self, register uint32 me )
		{ __asm__( "stwbrx %0, 0, %1" : : "r"(me), "r"(&self) : "memory" ); }

	static inline uint32 _out( register const AEUInt32Base & self )
		{ uint32 ret; __asm__( "lwbrx %0, 0, %1" : "=r"(ret) : "r"(&self) ); return ret; }
  #endif
#else
	static inline void _in( AEUInt32Base & self, uint32 me )
//		{ __asm__( "bswapl %1\n\tmovl %1, %0" : "=g"(self) : "r"(me) : "%1" ); }
		{ __asm__( "bswapl %0" : "=r"(self) : "0"(me) : "%0" ); }
		// me is a local var even if already in a reg so ok to clobber it here

	static inline uint32 _out( const AEUInt32Base & self )
//		{ uint32 ret; __asm__( "movl %1, %0\n\tbswapl %0" : "=r"(ret) : "g"(self) ); return ret; }
		{ uint32 ret; __asm__( "bswapl %0" : "=r"(ret) : "0"(self) ); return ret; }
		// even 'tho it looks like we overwrite self here, gcc will always be forced
		// to make a copy before calling us since we ask for it in a register...
		// except if that's where self lives, in which case gcc will be forced to make
		// a copy if it ever wants to use self again (i.e. the level above won't want
		// ret in the same reg as self) - and if it doesn't ever want to use self again
		// then overwriting it is a most excellent thing to do!
#endif
public:
	uint32		ae_;
};
/// Normal usage of AEUInt32Base
class AEUInt32 : public AEUInt32Base
{
public:
	inline AEUInt32()				{ }
	inline AEUInt32( uint32 me )	{ this->in( me ); }

	inline AEUInt32 & operator=( const AEUInt32Base & le )
		{ *this = (const AEUInt32&)le; return *this; }

	ARITHMETIC_OPERATORS( inline AEUInt32 &, uint32,
		this->in, val, DISJOIN, *this, ; return *this; )
};
/// Union usage ... unions making trouble again :)
class AEUInt32InUnion : public AEUInt32Base
{
public:
	inline AEUInt32InUnion & operator=( uint32 me )
		{ this->in( me ); return *this; }

	inline AEUInt32InUnion & operator=( const AEUInt32Base & le )
		{ *this = (const AEUInt32InUnion&)le; return *this; }

	ARITHMETIC_OPERATORS( inline AEUInt32InUnion &, uint32,
		this->in, val, DISJOIN, *this, ; return *this; );
};


/**
 *	A class for managing an alien endian short
 */
class AEUInt16Base
{
public:
	inline operator uint16() const	{ return this->out(); }
	
	inline void in( uint16 me )		{ _in( *this, me ); }
	inline uint16 out() const		{ return _out( *this ); }
	
private:
#ifndef __LITTLE_ENDIAN__
  #ifdef __METROWERKS__
	static inline void _in( register AEUInt16Base & self, register uint16 me )
		{ /*asm { sthbrx me, r0, self; }*/ __sthbrx( me, &self, 0 ); }
		// rA is r0 not 0 to work around an assembler parsing bug

	static inline uint16 _out( register const AEUInt16Base & self )
		{ /*asm { lhbrx r3, 0, self; }*/ return (uint16)__lhbrx( (void*)&self, 0 ); }
  #else
	static inline void _in( register AEUInt16Base & self, register uint16 me )
		{ __asm__( "sthbrx %0, 0, %1" : : "r"(me), "r"(&self) : "memory" ); }

	static inline uint16 _out( register const AEUInt16Base & self )
		{ uint16 ret; __asm__( "lhbrx %0, 0, %1" : "=r"(ret) : "r"(&self) ); return ret; }
  #endif
#else
	static inline void _in( AEUInt16Base & self, uint16 me )
//		{ __asm__( "xchgb %b1, %h1\n\tmovw %1, %0" : "=g"(self) : "q"(me) : "%1" ); }
		{ __asm__( "xchgb %b1, %h1" : "=q"(self) : "0"(me) : "%0" ); }

	static inline uint16 _out( const AEUInt16Base & self )
//		{ uint16 ret; __asm__( "movw %1, %0\n\txchgb %b0, %h0" : "=q"(ret) : "g"(self) ); return ret; }
		{ uint16 ret; __asm__( "xchgb %b0, %h0" : "=q"(ret) : "0"(self) ); return ret; }
#endif


	uint16		ae_;
};
/// Normal usage of AEUInt16Base
class AEUInt16 : public AEUInt16Base
{
public:
	inline AEUInt16()				{ }
	inline AEUInt16( uint16 me )	{ this->in( me ); }

	inline AEUInt16 & operator=( const AEUInt16Base & le )
		{ *this = (const AEUInt16&)le; return *this; }

	ARITHMETIC_OPERATORS( inline AEUInt16 &, uint16,
		this->in, val, DISJOIN, *this, ; return *this; )
};
/// Union usage ... unions making trouble again :)
class AEUInt16InUnion : public AEUInt16Base
{
public:
	inline AEUInt16InUnion & operator=( uint16 me )
		{ this->in( me ); return *this; }

	inline AEUInt16InUnion & operator=( const AEUInt16Base & le )
		{ *this = (const AEUInt16InUnion&)le; return *this; }

	ARITHMETIC_OPERATORS( inline AEUInt16InUnion &, uint16,
		this->in, val, DISJOIN, *this, ; return *this; )
};


#ifndef __LITTLE_ENDIAN__
typedef AEUInt32 LEUInt32;
typedef AEUInt32InUnion LEUInt32InUnion;
typedef AEUInt16 LEUInt16;
typedef AEUInt16InUnion LEUInt16InUnion;
typedef uint32 BEUInt32;
typedef uint32 BEUInt32InUnion;
typedef uint16 BEUInt16;
typedef uint16 BEUInt16InUnion;
#else
typedef uint32 LEUInt32;
typedef uint32 LEUInt32InUnion;
typedef uint16 LEUInt16;
typedef uint16 LEUInt16InUnion;
typedef AEUInt32 BEUInt32;
typedef AEUInt32InUnion BEUInt32InUnion;
typedef AEUInt16 BEUInt16;
typedef AEUInt16InUnion BEUInt16InUnion;
#endif


/**
 *	A struct for a big-endian 24 bit unsigned integer
 */
struct BEUInt24
{
	inline BEUInt24()			{ }
	inline BEUInt24( uint32 v )	{ (*this) = v; }
#ifndef __LITTLE_ENDIAN__
	inline operator uint32() const
		{ return ((*(uint16*)v_) << 8) | v_[2]; }
	inline BEUInt24 & operator=( uint32 v )
		{ *(uint16*)v_ = (v>>8); v_[2] = uint8(v); return *this; }
#else
	inline operator uint32() const
		{ return (v_[0] << 16) | (v_[1] << 8) | v_[2]; }
	inline BEUInt24 & operator=( uint32 v )
		{ v_[0] = uint8(v>>16); v_[1] = uint8(v>>8); v_[2] = uint8(v); return *this; }
#endif

	ARITHMETIC_OPERATORS( inline BEUInt24 &, uint32,
		return, (*this), CONJOIN, = val, ; )
	
	uint8 v_[3];
};

#ifndef __LITTLE_ENDIAN__
  #ifndef __MWERKS__
inline int __cntlzw( int arg )
{
	__asm__ volatile("cntlzw %0, %1" : "=r" (arg) : "r" (arg)); 
	return arg;
}
  #endif
#else
inline int __cntlzw( int arg )
{
	int ret, tmp;
	__asm__ (
		"bsr %2, %0\n\t"
		"setz %b1\n\t"
		"ror $1, %1\n\t"
		"sar $31, %1\n\t"
		"or %1, %0\n\t"
		:"=r"(ret), "=q"(tmp) : "r"(arg) );
	return 31 - ret;	// ret is -1 if all zeros
}
#endif

inline uint32 cntlzd( const uint64 & val )
{
	uint32 * length32s = (uint32*)&val;
#ifndef __LITTLE_ENDIAN__
	return length32s[0] ?
		__cntlzw(length32s[0]) : 32+__cntlzw(length32s[1]);
#else
	return length32s[1] ?
		__cntlzw(length32s[1]) : 32+__cntlzw(length32s[0]);
#endif
}




#include "BEField.hpp"



#endif // FTYPES_HPP
