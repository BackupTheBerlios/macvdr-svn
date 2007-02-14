#ifndef MM_INPUT_USER_HPP
#define MM_INPUT_USER_HPP

#include "MMInput.hpp"

/**
 *	Method number constants
 */
enum
{
	kMMInputIdentifier = 0,	// void in, UInt32 out

	kMMInputDataKind,		// void in, UInt32 out
	kMMInputTuningSpace,	// void in, UInt32 out

	kMMInputActivate,		// void in, DataPond out
	kMMInputDeactivate,		// void in, void out
	
	kMMInputTune,			// szXML dict in, void out [success in ret]
	kMMInputFill,			// bool in, void out [success in ret]

	kMMInputSituation,		// void in, UInt32 * 8 out
	kMMInputStatistics,		// void in, UInt32 * 8 out
	
	kMMInputMethodCount
};


/**
 *	This is the structure returned from a successful kMMInputActivate call.
 *	You should then get request the data and level memory maps as client
 *	memory types 0 and 1.
 */
struct UserSpaceDataPond : public MMInput::DataPond
{
	/**
	 *	Get the result of the activation operation.
	 *	0 = success, -1 = misc error, other = id of conflicting tuner
	 */
	UInt32	result()	{ return (UInt32)dataDesc_; }
	
	// dataBegin_ is the offset into client memory type 0 of the start of data
	// levelAddress_ is the offset into client memory type 1 of the level value
};



#endif // MM_INPUT_USER_HPP
