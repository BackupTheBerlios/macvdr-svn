#ifndef BEFIELD_HPP
#define BEFIELD_HPP

#ifndef __LITTLE_ENDIAN__

#define BEFIELD_UINT32(X) X
#define BEFIELD_BEG
#define BEFIELD_END
#define BEFIELD( NAME, C ) uint32	NAME:C;

#else
// There are two BEField implementations. One uses simple macros but generates warnings,
// the other uses very complicated macros but generates no warnings. YMMY.

// this uses a mess of macros to construct the bitfield
#define MACROMESS_BEFIELD

// this is much nicer but gcc generates many warnings :(
//#define TYPEDEFWARNING_BEFIELD


template <int B, int C> struct BEField
{
	typedef BEField<B,C> This;

	inline operator uint32() const
	{
		return (uint32(rep) >> (32-(B+C))) & (uint32(-1) >> (32-C));
	}

	inline This & operator=( uint32 v )
	{
		uint32 msk = (uint32(-1) >> (32-C)) << (32-(B+C));
		rep = (rep & ~msk) | ((v << (32-(B+C))) & msk);
		return *this;
	}

	// don't need (and can't have) copy constructor since always in a union.
	// do need operator= for copying between fields in different structs,
	// but since it's not allowed user must remember to cast to uint32.
	//inline This & operator=( const This & oth )
	//	{ return this->operator=( uint32(oth) ); }

	ARITHMETIC_OPERATORS( inline This &, uint32,
		return this->operator=, uint32(*this), DISJOIN, val, ; )
	
	BEUInt32InUnion rep;
};

#endif



#ifdef MACROMESS_BEFIELD

// Trickiest bit is making sure commas/parentheses don't stuff up multiple expansion steps

// BEFIELD public interface
#define BEFIELD_UINT32(X) union { BEFIELD_EXZ(X) };
// need 2*(#fields+1) expansions

#define BEFIELD_BEG		BEFIELD_INT LPAREN 0, 0,
#define BEFIELD_END		0, end, BEFIELD_INT_END ), zero )

#define BEFIELD( NAME, C )	C, NAME, BEFIELD_INT_FLD ), (

// BEFIELD private internals
#define LPAREN (
// this is the magic one that allows us to leave trailing partial macro calls

#define BEFIELD_INT( DEPTH, S, C, NAME, OP )	OP( DEPTH, S, C, NAME )
#define BEFIELD_INT_FLD( DEPTH, S, C, NAME )	BEField<S,C> NAME; BEFIELD_PRE##_##DEPTH LPAREN S+C 

#define BEFIELD_INT_END( DEPTH, S, C, NAME ) BEFIELD_INT_CLEANUP LPAREN S+C
#define BEFIELD_INT_CLEANUP( TOTALBITS, ZERO )

#define BEFIELD_UNPACK( C, NAME, OP ) C, NAME, OP

#define BEFIELD_PRE_0( S, NEXT, ... )	BEFIELD_INT LPAREN 1, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_1( S, NEXT, ... )	BEFIELD_INT LPAREN 2, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_2( S, NEXT, ... )	BEFIELD_INT LPAREN 3, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_3( S, NEXT, ... )	BEFIELD_INT LPAREN 4, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_4( S, NEXT, ... )	BEFIELD_INT LPAREN 5, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_5( S, NEXT, ... )	BEFIELD_INT LPAREN 6, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_6( S, NEXT, ... )	BEFIELD_INT LPAREN 7, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_7( S, NEXT, ... )	BEFIELD_INT LPAREN 8, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_8( S, NEXT, ... )	BEFIELD_INT LPAREN 9, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_9( S, NEXT, ... )	BEFIELD_INT LPAREN 10, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_10( S, NEXT, ... )	BEFIELD_INT LPAREN 11, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_11( S, NEXT, ... )	BEFIELD_INT LPAREN 12, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_12( S, NEXT, ... )	BEFIELD_INT LPAREN 13, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_13( S, NEXT, ... )	BEFIELD_INT LPAREN 14, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_14( S, NEXT, ... )	BEFIELD_INT LPAREN 15, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_15( S, NEXT, ... )	BEFIELD_INT LPAREN 16, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_16( S, NEXT, ... )	BEFIELD_INT LPAREN 17, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_17( S, NEXT, ... )	BEFIELD_INT LPAREN 18, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_18( S, NEXT, ... )	BEFIELD_INT LPAREN 19, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_19( S, NEXT, ... )	BEFIELD_INT LPAREN 20, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_20( S, NEXT, ... )	BEFIELD_INT LPAREN 21, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_21( S, NEXT, ... )	BEFIELD_INT LPAREN 22, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_22( S, NEXT, ... )	BEFIELD_INT LPAREN 23, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_23( S, NEXT, ... )	BEFIELD_INT LPAREN 24, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_24( S, NEXT, ... )	BEFIELD_INT LPAREN 25, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_25( S, NEXT, ... )	BEFIELD_INT LPAREN 26, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_26( S, NEXT, ... )	BEFIELD_INT LPAREN 27, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_27( S, NEXT, ... )	BEFIELD_INT LPAREN 28, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_28( S, NEXT, ... )	BEFIELD_INT LPAREN 29, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_29( S, NEXT, ... )	BEFIELD_INT LPAREN 30, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_30( S, NEXT, ... )	BEFIELD_INT LPAREN 31, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )
#define BEFIELD_PRE_31( S, NEXT, ... )	BEFIELD_INT LPAREN 32, S, BEFIELD_UNPACK NEXT ), __VA_ARGS__ )

#define BEFIELD_EX0(...) __VA_ARGS__
#define BEFIELD_EX1(...) BEFIELD_EX0(__VA_ARGS__)
#define BEFIELD_EX2(...) BEFIELD_EX1(__VA_ARGS__)
#define BEFIELD_EX3(...) BEFIELD_EX2(__VA_ARGS__)
#define BEFIELD_EX4(...) BEFIELD_EX3(__VA_ARGS__)
#define BEFIELD_EX5(...) BEFIELD_EX4(__VA_ARGS__)
#define BEFIELD_EX6(...) BEFIELD_EX5(__VA_ARGS__)
#define BEFIELD_EX7(...) BEFIELD_EX6(__VA_ARGS__)
#define BEFIELD_EX8(...) BEFIELD_EX7(__VA_ARGS__)
#define BEFIELD_EX9(...) BEFIELD_EX8(__VA_ARGS__)
#define BEFIELD_EX10(...) BEFIELD_EX9(__VA_ARGS__)
#define BEFIELD_EX11(...) BEFIELD_EX10(__VA_ARGS__)
#define BEFIELD_EX12(...) BEFIELD_EX11(__VA_ARGS__)
#define BEFIELD_EX13(...) BEFIELD_EX12(__VA_ARGS__)
#define BEFIELD_EX14(...) BEFIELD_EX13(__VA_ARGS__)
#define BEFIELD_EX15(...) BEFIELD_EX14(__VA_ARGS__)
#define BEFIELD_EX16(...) BEFIELD_EX15(__VA_ARGS__)
#define BEFIELD_EX17(...) BEFIELD_EX16(__VA_ARGS__)
#define BEFIELD_EX18(...) BEFIELD_EX17(__VA_ARGS__)
#define BEFIELD_EX19(...) BEFIELD_EX18(__VA_ARGS__)
#define BEFIELD_EX20(...) BEFIELD_EX19(__VA_ARGS__)
#define BEFIELD_EX21(...) BEFIELD_EX20(__VA_ARGS__)
#define BEFIELD_EX22(...) BEFIELD_EX21(__VA_ARGS__)
#define BEFIELD_EX23(...) BEFIELD_EX22(__VA_ARGS__)
#define BEFIELD_EX24(...) BEFIELD_EX23(__VA_ARGS__)
#define BEFIELD_EX25(...) BEFIELD_EX24(__VA_ARGS__)
#define BEFIELD_EX26(...) BEFIELD_EX25(__VA_ARGS__)
#define BEFIELD_EX27(...) BEFIELD_EX26(__VA_ARGS__)
#define BEFIELD_EX28(...) BEFIELD_EX27(__VA_ARGS__)
#define BEFIELD_EX29(...) BEFIELD_EX28(__VA_ARGS__)
#define BEFIELD_EX30(...) BEFIELD_EX29(__VA_ARGS__)
#define BEFIELD_EX31(...) BEFIELD_EX30(__VA_ARGS__)
#define BEFIELD_EX32(...) BEFIELD_EX31(__VA_ARGS__)
#define BEFIELD_EX33(...) BEFIELD_EX32(__VA_ARGS__)
#define BEFIELD_EX34(...) BEFIELD_EX33(__VA_ARGS__)
#define BEFIELD_EX35(...) BEFIELD_EX34(__VA_ARGS__)
#define BEFIELD_EX36(...) BEFIELD_EX35(__VA_ARGS__)
#define BEFIELD_EX37(...) BEFIELD_EX36(__VA_ARGS__)
#define BEFIELD_EX38(...) BEFIELD_EX37(__VA_ARGS__)
#define BEFIELD_EX39(...) BEFIELD_EX38(__VA_ARGS__)
#define BEFIELD_EX40(...) BEFIELD_EX39(__VA_ARGS__)
#define BEFIELD_EX41(...) BEFIELD_EX40(__VA_ARGS__)
#define BEFIELD_EX42(...) BEFIELD_EX41(__VA_ARGS__)
#define BEFIELD_EX43(...) BEFIELD_EX42(__VA_ARGS__)
#define BEFIELD_EX44(...) BEFIELD_EX43(__VA_ARGS__)
#define BEFIELD_EX45(...) BEFIELD_EX44(__VA_ARGS__)
#define BEFIELD_EX46(...) BEFIELD_EX45(__VA_ARGS__)
#define BEFIELD_EX47(...) BEFIELD_EX46(__VA_ARGS__)
#define BEFIELD_EX48(...) BEFIELD_EX47(__VA_ARGS__)
#define BEFIELD_EX49(...) BEFIELD_EX48(__VA_ARGS__)
#define BEFIELD_EX50(...) BEFIELD_EX49(__VA_ARGS__)
#define BEFIELD_EX51(...) BEFIELD_EX50(__VA_ARGS__)
#define BEFIELD_EX52(...) BEFIELD_EX51(__VA_ARGS__)
#define BEFIELD_EX53(...) BEFIELD_EX52(__VA_ARGS__)
#define BEFIELD_EX54(...) BEFIELD_EX53(__VA_ARGS__)
#define BEFIELD_EX55(...) BEFIELD_EX54(__VA_ARGS__)
#define BEFIELD_EX56(...) BEFIELD_EX55(__VA_ARGS__)
#define BEFIELD_EX57(...) BEFIELD_EX56(__VA_ARGS__)
#define BEFIELD_EX58(...) BEFIELD_EX57(__VA_ARGS__)
#define BEFIELD_EX59(...) BEFIELD_EX58(__VA_ARGS__)
#define BEFIELD_EX60(...) BEFIELD_EX59(__VA_ARGS__)
#define BEFIELD_EX61(...) BEFIELD_EX60(__VA_ARGS__)
#define BEFIELD_EX62(...) BEFIELD_EX61(__VA_ARGS__)
#define BEFIELD_EX63(...) BEFIELD_EX62(__VA_ARGS__)
#define BEFIELD_EXZ(...) BEFIELD_EX63(__VA_ARGS__)

#endif // MACROMESS_BEFIELD

#ifdef TYPEDEFWARNING_BEFIELD

// template isn't really necessary but easier on the eye
template <int I> struct BEFieldNextSize { enum { SIZE = I }; };

#define BEFIELD_UINT32(X)	union { X };
#define BEFIELD_BEG			typedef BEFieldNextSize<0>
#define BEFIELD_END			_last;
#define BEFIELD( NAME, C )	_prev_##NAME;								\
							BEField<_prev_##NAME::SIZE,C> NAME;			\
							typedef BEFieldNextSize<_prev_##NAME::SIZE + C>

#endif // TYPEDEFWARNING_BEFIELD


#ifdef BEFIELD_TEST
// to test: 'g++ -DBEFIELD_TEST -x c++ BEField.hpp -include FTypes.hpp ...'

#include <stdio.h>


struct S
{
	BEFIELD_UINT32
	( 
		BEFIELD_BEG

		BEFIELD( fa, 2 )
		BEFIELD( fb, 2 )
		BEFIELD( fc, 3 )

		BEFIELD_END
	)

	float other_stuff;
};


int main()
{
	S s;
	(BEUInt32&)s = 0x60000000;
	s.other_stuff = 4.f;

	printf( "fa is %d, fb is %d\n", int(s.fa), int(s.fb) );
}
#endif

#endif // BEFIELD_HPP
