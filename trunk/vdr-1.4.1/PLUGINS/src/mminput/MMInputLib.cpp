//#define __dead2
//#define __pure2

#include "MMInputLib.hpp"
#include "Drivers/MMInputFamily/MMInputUserCommon.hpp"

#import <IOKit/IOKitLib.h>


#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>
#import <Foundation/NSPropertyList.h>
#import <Foundation/NSData.h>
#include <string>



/**
 *	Constructor
 */
MMInputDevice::MMInputDevice( io_object_t ioobj, io_connect_t port ) :
	ioobj_( ioobj ),
	port_( port ),
	pond_( NULL )
{
}


/**
 *	Get a device interface to the index-th registered device
 */
MMInputDevice * MMInputDevice::withIndex( int index )
{
	kern_return_t res;
	
	mach_port_t masterPort;
	IOMasterPort( MACH_PORT_NULL, &masterPort );
	
	CFMutableDictionaryRef matchDict = IOServiceMatching( "MMInput" );
	
	io_iterator_t it;
	res = IOServiceGetMatchingServices( masterPort, matchDict, &it );
	if (res != KERN_SUCCESS)
	{
		fprintf( stderr, "No MMInput services\n" );
		return NULL;
	}

	io_object_t aninput;
	while (1)
	{
		aninput = IOIteratorNext( it );
		if (index-- <= 0) break;
		if (aninput == 0) break;
		IOObjectRelease( aninput );
	}

	IOObjectRelease( it );
	if (aninput == 0) return NULL;	

	return MMInputDevice::withIOService( aninput );
}

/**
 *	Get a device interface to the given device
 */
MMInputDevice * MMInputDevice::withIOService( io_object_t aninput )
{
	io_connect_t port = 0;
	kern_return_t res = IOServiceOpen( aninput, mach_task_self(), 0, &port );

	if (res != KERN_SUCCESS || port == 0)
	{
		fprintf( stderr, "Could not open MMInput service\n" );
		IOObjectRelease( aninput );
		
		return NULL;
	}
	
	return new MMInputDevice( aninput, port );
}

/**
 *	Destructor
 */
MMInputDevice::~MMInputDevice()
{
	this->deactivate();
	IOObjectRelease( ioobj_ );
	IOServiceClose( port_ );
}


/**
 *	Release method
 */
void MMInputDevice::release()
{
	delete this;
}





UInt32 MMInputDevice::identifier() const
{
	UInt32 ret = 0;
	kern_return_t res = IOConnectMethodScalarIScalarO(
		port_, kMMInputIdentifier, 0, 1, &ret );
	if (res != KERN_SUCCESS)
	{
		fprintf( stderr, "MMInputDevice::identifier: error 0x%08X\n", res );
		return 0;
	}
	return ret;
}

UInt32 MMInputDevice::activate()
{
	if (pond_ != NULL) return 0;

	// activate it
	UserSpaceDataPond pond;
	IOByteCount structSize = sizeof(UserSpaceDataPond);
	kern_return_t res = IOConnectMethodScalarIStructureO(
		port_, kMMInputActivate, 0, &structSize, &pond );
	if (res != KERN_SUCCESS || structSize != sizeof(UserSpaceDataPond))
	{
		fprintf( stderr, "MMInputDevice::activate: error 0x%08X reason %d\n",
			res, int(pond.result()) );
		if (pond.result() == 0) return UInt32(-1);
		return pond.result();
	}
	fprintf( stderr, "MMInputDevice::activate: "
		"dataBegin 0x%08lX dataEnd 0x%08lX "
		"levelAddress 0x%08lX levelFormat 0x%08lX\n",
		pond.dataBegin_, pond.dataEnd_,
		pond.levelAddress_, pond.levelFormat_ );

	// map data memory
	dataBase_ = 0;
	res = IOConnectMapMemory( port_, 0, mach_task_self(),
		(uint*)&dataBase_, (uint*)&dataSize_, kIOMapAnywhere );
	if (res != KERN_SUCCESS || dataBase_ == 0)
	{
		fprintf( stderr, "MMInputDevice::activate: Could not get data "
			"memory mapping. res %d dataBase %08lX\n", res, dataBase_ );

		IOConnectMethodScalarIScalarO( port_, kMMInputDeactivate, 0, 0 );
		
		return UInt32(-1);
	}
	fprintf( stderr, "MMInputDevice::activate: Got data memory mapping. "
		"base %08lX size %ld\n", dataBase_, dataSize_ );
	
	// map level memory
	levelBase_ = 0;
	res = IOConnectMapMemory( port_, 1, mach_task_self(),
		(uint*)&levelBase_, (uint*)&levelSize_, kIOMapAnywhere );
	if (res != KERN_SUCCESS || levelBase_ == 0)
	{
		fprintf( stderr, "MMInputDevice::activate: Could not get level "
			"memory mapping. res %d levelBase %08lX\n", res, levelBase_ );

		IOConnectUnmapMemory( port_, 0, mach_task_self(), dataBase_ );
		IOConnectMethodScalarIScalarO( port_, kMMInputDeactivate, 0, 0 );
		
		return UInt32(-1);
	}
	fprintf( stderr, "MMInputDevice::activate: Got level memory mapping. "
		"base %08lX size %ld\n", levelBase_, levelSize_ );

	pond_ = new UserSpaceDataPond( pond );
	levelLast_ = UInt32(-2);

#if 0	// way to kernel panic! they should be fixed...
	// make sure we can read all the level memory byte-by-byte
	UInt32 levelPtr = levelBase_ + pond_->levelAddress_;
	for (UInt32 i = 0; i < levelSize_; i++)
	{
		if ((i&0xFFFF)==0)
		{
			fprintf( stderr, "reading at 0x%08X\n", i );
			sleep( 2 );
		}
		*(volatile uint8*)(levelBase_+i);
	}
#endif

	syncBuffer_ = new uint8[pond_->dataBlobSize_];

	return 0;
}

void MMInputDevice::deactivate()
{
	if (pond_ == NULL) return;
	
	// make sure we are not filling
	this->fill( false );

	// unmap level memory
	IOConnectUnmapMemory( port_, 1, mach_task_self(), levelBase_ );

	// unmap data memory
	IOConnectUnmapMemory( port_, 0, mach_task_self(), dataBase_ );

	// deactivate it
	kern_return_t res = IOConnectMethodScalarIScalarO(
		port_, kMMInputDeactivate, 0, 0 );
	if (res != KERN_SUCCESS)
	{
		fprintf( stderr, "MMInputDevice::deactivate: error 0x%08X\n", res );
		// continue on error nevertheless
	}
	
	delete pond_;
	pond_ = NULL;

	delete [] syncBuffer_;
	syncBuffer_ = NULL;
}

MMInputDevice::TuningParams MMInputDevice::tuningSpace() const
{
	UInt32 ret = 0;
	kern_return_t res = IOConnectMethodScalarIScalarO(
		port_, kMMInputTuningSpace, 0, 1, &ret );
	if (res != KERN_SUCCESS)
	{
		fprintf( stderr, "MMInputDevice::tuningSpace: error 0x%08X\n", res );
		return NONE;
	}
	return TuningParams(ret);
}

#include <string>
bool MMInputDevice::tune( NSDictionary * params )
{
	if (pond_ == NULL) return false;

	//NSString * paramsDesc = [params description];
	//std::string paramsXML = [paramsDesc lossyCString];
	NSData * paramsData = [NSPropertyListSerialization
		dataFromPropertyList:params
		format:NSPropertyListXMLFormat_v1_0
		errorDescription:NULL];
	std::string paramsXML( (const char*)[paramsData bytes], [paramsData length] );
	uint dictBeg = paramsXML.find( "<dict>" );
	uint dictEnd = paramsXML.rfind( "</dict>" ) + 7;
	paramsXML = paramsXML.substr( dictBeg, dictEnd-dictBeg );
	//fprintf( stderr, "XML looks like this:\n%s\nEnd of XML\n", paramsXML.c_str() );

	IOByteCount structOutSize = 0;
	kern_return_t res = IOConnectMethodStructureIStructureO(
		port_, kMMInputTune, paramsXML.length()+1, &structOutSize,
		(void*)paramsXML.c_str(), NULL );
	if (res != KERN_SUCCESS)
	{
		fprintf( stderr, "MMInputDevice::tune: error 0x%08X\n", res );
		return false;
	}
	return true;
}

MMInputDevice::DataKind MMInputDevice::dataKind() const
{
	UInt32 ret = 0;
	kern_return_t res = IOConnectMethodScalarIScalarO(
		port_, kMMInputDataKind, 0, 1, &ret );
	if (res != KERN_SUCCESS)
	{
		fprintf( stderr, "MMInputDevice::dataKind: error 0x%08X\n", res );
		return DataKind(-1);
	}
	return DataKind(ret);
}

bool MMInputDevice::fill( bool go )
{
	if (pond_ == NULL) return false;

	kern_return_t res = IOConnectMethodScalarIScalarO(
		port_, kMMInputFill, 1, 0, UInt32(go) );
	if (res != KERN_SUCCESS)
	{
		fprintf( stderr, "MMInputDevice::fill: error 0x%08X\n", res );
		return false;
	}

	levelLast_ = go ? UInt32(-1) : UInt32(-2);
	syncOffset_ = 0;
	syncAttained_ = false;
	syncCertainty_ = 0;
	return true;
}

UInt32 MMInputDevice::retrieveNew( void * & data, UInt32 & length ){
	data = NULL;
	length = 0;
	
	if (levelLast_ == UInt32(-2)) return 0;	// we don't think it's filling
	
	// read the level
	UInt32 levelNow = UInt32(-1);
	// read shared level value
	UInt32 levelPtr = levelBase_ + pond_->levelAddress_;
	switch (pond_->levelFormat_ & 15){
		case 0:	levelNow = *(BEUInt32*)levelPtr;	break;
		case 1:	levelNow = *(LEUInt32*)levelPtr;	break;
	}
	
	if (levelNow == UInt32(-1)) return 0;
	levelNow -= pond_->levelBegin_;
	if (SInt32(levelNow) < 0) return 0;
	if (!(pond_->levelFormat_ & (1<<15)))
		levelNow <<= (pond_->levelFormat_ >> 11) & 15;
	else
		levelNow >>= (pond_->levelFormat_ >> 11) & 15;
	levelNow *= (pond_->levelFormat_ >> 15) | 1;
	levelNow -= levelNow % pond_->dataBlobSize_;
	
	UInt32 dataWindow = pond_->dataEnd_ - pond_->dataBegin_;
	if (levelNow > dataWindow) return 0;
	
	// ok, we have valid data, let's get it out of there
	if (levelLast_ != UInt32(-1))
	{
		data = (void*)(dataBase_ + pond_->dataBegin_ + levelLast_);
		
		if (levelLast_ <= levelNow)
		{		// straightforward
			length = levelNow - levelLast_;
		}
		else
		{		// wrapped around
			length = dataWindow - levelLast_;
			levelNow = 0;
		}
		
		// find the sync byte if the driver is too lazy to sync the stream
		// (syncOffset_ is the number of bytes we hold back from the last blob)
//		if (syncRqrd_ && length > 0)
		if (length > 0)
		{
//			const uint8 syncByte = syncByte_;
			const uint8 syncByte = 0;
			uint8 *& dataBytes = (uint8*&)data;
			const uint32 blobSize = pond_->dataBlobSize_;
			
			// apply the sync offset and worry about the last blob case
			if (syncOffset_ != 0)
			{
				if (levelLast_ >= blobSize)
				{
					dataBytes -= syncOffset_;
				}
				else if (levelNow < blobSize)
				{
					// not far enough into next window
					return 0;	// try again next time
				}
				else
				{
					// blob from window wraparound
					memcpy( syncBuffer_,
							(void*)(dataBase_+pond_->dataEnd_ - syncOffset_),
							syncOffset_ );
					memcpy( syncBuffer_+syncOffset_,
							dataBytes,
							blobSize-syncOffset_ );
					data = syncBuffer_;
					length = blobSize;
					levelLast_ = blobSize;
					return 1;
				}
			}
			
			// is the sync marker present where we expect it?
			if (*dataBytes == syncByte)
			{
				if (syncCertainty_ >= 5)
					; /* do nothing - common case */
				else
				{
					// increase certainty, whether sync attained or not
					syncCertainty_++;
					if (syncCertainty_ >= 5 && !syncAttained_)
					{
						syncAttained_ = true;
						//IOLog( "QanuUSB::dataCompleted: attained certain sync\n" );
					}
				}
			}
			// nope, did we have sync before?
			else if (syncAttained_)
			{
				// yep, so reduce certainty from 5; at zero sync is lost
				syncCertainty_--;
				if (syncCertainty_ == 0)
				{
					syncAttained_ = false;
					//IOLog( "QanuUSB::dataCompleted: lost sync after attained\n" );
				}
			}
			// nope, go and resync then
			else
			{
				syncCertainty_ = 0;
				//IOLog( "QanuUSB::dataCompleted: scanning since unsynced\n" );
				uint8 * sbeg = dataBytes;
				uint8 * scur = sbeg;
				uint8 * send = dataBytes + length;
				while (scur != send)
				{
					if (*scur == syncByte) break;
					scur++;
				}
				// collapse the dodgy data until the sync marker if it was found
				dataBytes = scur;
				length -= scur-sbeg;
				if (length > 0)
				{
					// record this offset if we found the sync marker
					syncOffset_ = blobSize -
					uint32(dataBytes-(dataBase_+pond_->dataBegin_)) % blobSize;
					// note that if length < blobSize then no data will be
					// returned from this call, but since we remember the
					// sync marker it will be returned from the next one
				}
			}
		}
		
	}
	levelLast_ = levelNow;
	
	return length / pond_->dataBlobSize_;
}

UInt32 MMInputDevice::retrieve( void * & data, UInt32 & length )
{
	data = NULL;
	length = 0;
	if (pond_ == NULL) return 0;
	if (levelLast_ == UInt32(-2)) return 0;	// we don't think it's filling
	
	// read the level
	UInt32 levelNow = UInt32(-1);
	UInt32 levelPtr = levelBase_ + pond_->levelAddress_;
	switch (pond_->levelFormat_ & 15)
	{
		case 0:	levelNow = *(UInt32*)levelPtr;		break;
		case 1:	levelNow = *(LEUInt32*)levelPtr;	break;
	}
	
	if (levelNow == UInt32(-1)) return 0;
	levelNow -= pond_->levelBegin_;
	if (SInt32(levelNow) < 0) return 0;
	if (!(pond_->levelFormat_ & (1<<15)))
		levelNow <<= (pond_->levelFormat_ >> 11) & 15;
	else
		levelNow >>= (pond_->levelFormat_ >> 11) & 15;
	levelNow *= (pond_->levelFormat_ >> 15) | 1;
	levelNow -= levelNow % pond_->dataBlobSize_;
	
	UInt32 dataWindow = pond_->dataEnd_ - pond_->dataBegin_;
	if (levelNow > dataWindow) return 0;
	
	// ok, we have valid data, let's get it out of there
	if (levelLast_ != UInt32(-1))
	{
		data = (void*)(dataBase_ + pond_->dataBegin_ + levelLast_);
		
		if (levelLast_ <= levelNow)
		{		// straightforward
			length = levelNow - levelLast_;
		}
		else
		{		// wrapped around
			length = dataWindow - levelLast_;
			levelNow = 0;
 		}
		
		// find the sync byte if the driver is too lazy to sync the stream
		// (syncOffset_ is the number of bytes we hold back from the last blob)
		if ((pond_->levelFormat_ & 16) && length > 0)
		{
			const uint8 syncByte = 0x47; // only supported for DVB/ATSC at the mo'
			uint8 *& dataBytes = (uint8*&)data;
			const uint32 blobSize = pond_->dataBlobSize_;
			
			// apply the sync offset and worry about the last blob case
			if (syncOffset_ != 0)
			{
				if (levelLast_ >= blobSize)
				{
					dataBytes -= syncOffset_;
				}
				else if (levelNow < blobSize)
				{
					// not far enough into next window
					return 0;	// try again next time
				}
				else
				{
					// blob from window wraparound
					memcpy( syncBuffer_,
						(void*)(dataBase_+pond_->dataEnd_ - syncOffset_),
						syncOffset_ );
					memcpy( syncBuffer_+syncOffset_,
						dataBytes,
						blobSize-syncOffset_ );
					data = syncBuffer_;
					length = blobSize;
					levelLast_ = blobSize;
					return 1;
				}
			}

			// is the sync marker present where we expect it?
			if (*dataBytes == syncByte)
			{
				if (syncCertainty_ >= 5)
					; /* do nothing - common case */
				else
				{
					// increase certainty, whether sync attained or not
					syncCertainty_++;
					if (syncCertainty_ >= 5 && !syncAttained_)
					{
						syncAttained_ = true;
						//IOLog( "QanuUSB::dataCompleted: attained certain sync\n" );
					}
				}
			}
			// nope, did we have sync before?
			else if (syncAttained_)
			{
				// yep, so reduce certainty from 5; at zero sync is lost
				syncCertainty_--;
				if (syncCertainty_ == 0)
				{
					syncAttained_ = false;
					//IOLog( "QanuUSB::dataCompleted: lost sync after attained\n" );
				}
			}
			// nope, go and resync then
			else
			{
				syncCertainty_ = 0;
				//IOLog( "QanuUSB::dataCompleted: scanning since unsynced\n" );
				uint8 * sbeg = dataBytes;
				uint8 * scur = sbeg;
				uint8 * send = dataBytes + length;
				while (scur != send)
				{
					if (*scur == syncByte) break;
					scur++;
				}
				// collapse the dodgy data until the sync marker if it was found
				dataBytes = scur;
				length -= scur-sbeg;
				if (length > 0)
				{
					// record this offset if we found the sync marker
					syncOffset_ = blobSize -
						uint32(dataBytes-(dataBase_+pond_->dataBegin_)) % blobSize;
					// note that if length < blobSize then no data will be
					// returned from this call, but since we remember the
					// sync marker it will be returned from the next one
				}
			}
		}

	}
	levelLast_ = levelNow;

	return length / pond_->dataBlobSize_;
}


void MMInputDevice::situation( UInt32 & progress, UInt32 & strength, UInt32 & quality )
{
	progress = 0;
	strength = 0;
	quality = 0;
	if (pond_ == NULL) return;

	UInt32	sits[8];
	IOByteCount structSize = sizeof(sits);
	kern_return_t res = IOConnectMethodScalarIStructureO(
		port_, kMMInputSituation, 0, &structSize, sits );
	if (res != KERN_SUCCESS || structSize < 3*sizeof(UInt32))
	{
		fprintf( stderr, "MMInputDevice::situation: error 0x%08X\n", res );
		return;
	}
	progress = sits[0];
	strength = sits[1];
	quality = sits[2];
}

void MMInputDevice::statistics( UInt32 & blobsTotal, UInt32 & blobsErrors )
{
	blobsTotal = 0;
	blobsErrors = 0;
	if (pond_ == NULL) return;

	UInt32	stats[8];
	IOByteCount structSize = sizeof(stats);
	kern_return_t res = IOConnectMethodScalarIStructureO(
		port_, kMMInputStatistics, 0, &structSize, stats );
	if (res != KERN_SUCCESS || structSize < 2*sizeof(UInt32))
	{
		fprintf( stderr, "MMInputDevice::statistics: error 0x%08X\n", res );
		return;
	}
	blobsTotal = stats[0];
	blobsErrors = stats[1];
}



/**
 *	Get the given property
 */
NSString * MMInputDevice::prop( NSString * key ) const
{
	/*
	NSMutableDictionary * props;
	IORegistryEntryCreateCFProperties( aninput, (CFMutableDictionaryRef*)&props, NULL, 0 );
	fprintf( stderr, "props: %s\n", [[props description] lossyCString] );
	[props release];
	*/

	NSString * query = [@"MMInput_" stringByAppendingString:key];
	NSObject * res = (NSObject*)IORegistryEntryCreateCFProperty( ioobj_, (CFStringRef)query, NULL, 0 );
	if (res != NULL)
	{
		[res autorelease];
		// TODO: check type of res
	}
	return (NSString*)res;
}

/**
 *	Get the name property, which is a bit special
 */
NSString * MMInputDevice::propName() const
{
	// try the name from its properties
	NSString * nameObj = this->prop( @"name" );
	if (nameObj != NULL) return nameObj;

	// fall back to the class name then
	io_name_t aname;	// character array
	IOObjectGetClass( ioobj_, aname );
	return [NSString stringWithCString:aname];
}


/**
 *  Accessor to internals
 */
UserSpaceDataPond * MMInputDevice::internals( UInt32 & dataBase, UInt32 & dataSize,
	UInt32 & levelBase, UInt32 & levelSize ) const
{
	dataBase = dataBase_;
	dataSize = dataSize_;
	levelBase = levelBase_;
	levelSize = levelSize_;
	return pond_;
}


/**
 *	this is to stop the linker complaining
 */
UInt32 MMInput::identifier() const
{
	return 0;
}










