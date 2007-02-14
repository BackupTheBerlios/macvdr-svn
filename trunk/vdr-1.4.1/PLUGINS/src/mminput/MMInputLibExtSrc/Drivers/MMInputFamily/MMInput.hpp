#ifndef MM_INPUT_HPP
#define MM_INPUT_HPP

#ifdef KERNEL
#include <IOKit/IOService.h>
#else
class IOService { };
class IOMemoryDescriptor;
#define OSDeclareAbstractStructors( X )
#include <Foundation/NSDictionary.h>
#define OSDictionary NSDictionary
#endif


/**
 *	This is the base class for drivers that provide multimedia input.
 *	If you have multiple input sources (e.g. two independent tuners) then
 *	you need to create multiple instances of this class to provide them all
 *	(easy in IOKit!). You can handle dependencies with the 'activate' call.
 *
 *	One of the ideas with this family is to eliminate repeated driver code,
 *	as well as doing anything in user space that possibly can be. For example,
 *	the default means of transferring data is by a shared block of memory between
 *	the device and the user. Notice how the word 'kernel' isn't in the previous
 *	sentence!
 *
 *	Another outcome of this policy is that drivers should not attempt to
 *	convert data between formats - unless of course the device does the
 *	format conversion in hardware. Otherwise user space code is perfectly
 *	capable of doing that kind of machination.
 *
 *	And note that this is an _input_ family, so if you are a hardware MPEG
 *	decoding board then this may not be the (only) family for you.
 */
class MMInput : public IOService
{
	OSDeclareAbstractStructors( MMInput )

public:
	
	virtual UInt32 identifier() const;

	
	/**
	 *	This is where new data gets put. It is the opposite of a data pump.
	 *	It is up to clients suck data out of the pond. If they don't keep up
	 *	with the rate that it is filled then it is usually just emptied.
	 */
	struct DataPond
	{
		IOMemoryDescriptor * dataDesc_;
		UInt32 dataBegin_;		// offset in dataDesc_
		UInt32 dataEnd_;		// offset in dataDesc_
		UInt32 dataBlobSize_;

		IOMemoryDescriptor * levelDesc_;
		UInt32 levelAddress_;	// offset in levelDesc_
		UInt32 levelFormat_;	// 0/1 = [BE/LE]UInt32, 0/16 = [/un]synced, *
		UInt32 levelBegin_;		// level val corresponding to dataBegin
	};	// * >>11&15 = shift, >>15&1 = shift [left/right], >>15|1 = multiply


	/**
	 *	Activate this input. Returns itself if successful. Returns NULL for
	 *	a miscellaneous error. Otherwise it may return a pointer to another
	 *	MMInput whose activity prevents this input from being activated.
	 *	It's up to the caller to organise to deactivate the other input first.
	 *	No other inputs should be disturbed by this call!
	 *
	 *	The argument is the data pond for the device which is filled in if
	 *	the activation is successful. On input, dataEnd_ minus dataBegin_
	 *	may be used to indicate the desired size of the pond (only a hint)
	 */
	virtual MMInput * activate( DataPond & pond ) = 0;

	/**
	 *	Deactivate this input. A nop if already inactive.
	 */
	virtual void deactivate() = 0;



	/**
	 *	What style(s) of parameters does this device support
	 */
	enum TuningParams
	{
		NONE = 0,
		FREQ = 1,
		OFDM = 2,
		QPSK = 3,
		VSB = 4,
		QAM = 5,
		
		PID_FILTER = 1<<7,
		
		// keep these, but explain them! figure out overlap with ones above
		TERRESTRIAL = 256,
		SATELLITE = 512,
		CABLE = 1024,
		
		VIDEO_FORMAT = 1<<16,
		AUDIO_FORMAT = 1<<17
	};
	/**
	 *	Get the tunable parameters for this driver
	 */
	virtual TuningParams tuningSpace() const = 0;

	/**
	 *	Tune to the given parameters.
	 *	If missing then interpreted as 'current'.
	 *	Often if 0 then interpreted as 'auto'.
	 *	Some drivers will not be able to tune various parameters when
	 *	they are filling. Usually frequency is OK 'tho.
	 */
	virtual bool tune( OSDictionary * params ) = 0;
	

	/**
	 *	What kind of data are we talking about
	 */
	enum DataKind
	{
		DVB = 0,	// in MPEG2 transport stream - see http://www.dvb.org/
		ATSC = 1,	// in MPEG2 transport stream - see http://www.atsc.org/
		// below here are still theory
		PAL = 2,	// video, audio and teletext
		NSTC = 3	// video, audio and teletext
	};
	/**
	 *	Get the kind of data coming out of this driver.
	 */
	virtual DataKind dataKind() const = 0;

	/**
	 *	Enable or disable filling of the pond. The input must be active for
	 *	this call to succeed. Setting it to its current state is a nop and
	 *	should always succeed. Stopping and restarting filling is not
	 *	guaranteed to cleanly 'pause' the input, i.e. the data pool may
	 *	be emptied at any time when not filling. But then again it may not.
	 *	You pretty much have to check the level to get a starting position
	 *	after enabling filling.
	 */
	virtual bool fill( bool go ) = 0;

	
	/**
	 *	Determine the current situation of the driver wrt tuning.
	 *	Some drivers may only be able to get accurate answers when they are filling.
	 *
	 *	progress is integer progress of tuning operation
	 *	strength is the strength of the signal
	 *	quality is the quality of the data provided (equals strength
	 *	 if there is no algorithmic error recovery)
	 */
	virtual void situation( UInt32 & progress, UInt32 & strength, UInt32 & quality ) = 0;

	/**
	 *	Get various statistics about historical performance.
	 *	Will probably change this to a struct with more counters in the future.
	 *	Some drivers may only be able to get accurate answers when they are filling.
	 *
	 *	A blob is whatever unit of transfer the driver works with,
	 *	which could be anything from single bytes to whole video frames.
	 *	If a driver does not track one of these then it should set it to zero.
	 */
	virtual void statistics( UInt32 & blobsTotal, UInt32 & blobsErrors ) = 0;
};




// Expected keys for tune parameters:
// FREQ:
//	FrequencyHz as Number
// ODFM:
//  FrequencyHz as Number
//	Inversion as Boolean
//	BandwidthHz as Number
//	CodeRateHP as Number, i.e. x / (x+1)
//	CodeRateLP as Number, i.e. x / (x+1)
//  Constellation as Number, 2 = BPSK, 4 = QPSK, 16 = QAM16, etc
//  TransmissionMode as Number, i.e. around 2000 = 2k, 8000 = 8k
//  GuardInterval as Number, i.e. 1/x
//  Hierarchy as Number, -1 = NONE
// QPSK:
//	FrequencyHz as Number
//	Inversion as Boolean
//	SymbolRate as Number
//	CodeRate as Number, i.e. x / (x+1)
//	LNBPower as Boolean
//	LNBVoltage as Number
//	LNBTone as Boolean
//	DiSEqCMini as Boolean, i.e. false = A, true = B
//	DiSEqCSend as String
//	DiSEqCRecv as TBD (ptr and len of recv buffer)
// QAM:
//	FrequencyHz as Number
//	Inversion as Boolean
//	SymbolRate as Number
//  Constellation as Number, 2 = BPSK, 4 = QPSK, 16 = QAM16, etc
// PID_FILTER:
//  PacketIDs as Array of Number, others may still be provided

// Known string keys in registry properties (prefixed with 'MMInput_'):
//  name
//  manufacturer
//  model
//  revision (of hardware)
//  address (for recognition across reboots)

// Advice on address:
// Users should be able to match it character for character, such that
// partial matches are acceptable, and longer is better. So put more
// easily variable information at the end. If a serial number is available,
// it should be put such that a replacement box would still be recognised.
// Variability should be decided from the point of view of a human user.
// Serial numbers thus equate to labels on a box. Some examples:
// 'Ter_PCI_TechnoTrend_Budget_Bus0_Slot3'
// 'Ter_USB_TwinHan_VisionDTV_Ser8712349_Bus3_Addr2'
// i.e. users are more likely to keep the same physical box with whatever
// it's connected to, rather then remember which USB Bus it is plugged in to.
// Obviously this is more important for satellite devices which may have
// complicated antenna networks. Anyway, try to get this right but don't
// worry about it too much - you can never anticipate every user's actions :)


#endif // MM_INPUT_HPP
