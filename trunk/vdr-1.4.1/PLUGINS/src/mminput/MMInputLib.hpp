#ifndef MM_INPUT_LIB_HPP
#define MM_INPUT_LIB_HPP

#include <IOKit/IOTypes.h>
#include "FTypes.hpp"

#include "Drivers/MMInputFamily/MMInput.hpp"
struct UserSpaceDataPond;


/**
 *	This class provides glue for applications to talk to MultiMedia Input devices.
 *
 *	See MMInput.hpp for documentation.
 */
class MMInputDevice : public MMInput
{
public:
	static MMInputDevice * withIndex( int index = 0 );
	static MMInputDevice * withIOService( io_object_t service );
	void release();
	
	virtual UInt32 identifier() const;
	
	virtual UInt32 activate();
	virtual void deactivate();
	
	virtual TuningParams tuningSpace() const;
	virtual bool tune( NSDictionary * params );
	
	virtual DataKind dataKind() const;
	virtual bool fill( bool go );

	UInt32 retrieve( void * & data, UInt32 & length );
	UInt32 retrieveNew( void * & data, UInt32 & length );

	virtual void situation( UInt32 & progress, UInt32 & strength, UInt32 & quality );
	virtual void statistics( UInt32 & blobsTotal, UInt32 & blobsErrors );

	
	NSString * prop( NSString * keyName ) const;
	NSString * propName() const;
	
	UserSpaceDataPond * internals( UInt32 & dataBase, UInt32 & dataSize,
		UInt32 & levelBase, UInt32 & levelSize ) const;

private:
	MMInputDevice( io_object_t ioobj, io_connect_t port );
	~MMInputDevice();
	
	virtual MMInput * activate( DataPond & pond )
		{ this->activate(); return NULL; } // different in userspace

	io_object_t			ioobj_;
	io_connect_t		port_;
	UserSpaceDataPond	* pond_;
	
	UInt32	dataBase_;
	UInt32	dataSize_;
	UInt32	levelBase_;
	UInt32	levelSize_;

	UInt32	levelLast_;

	UInt32	syncOffset_;
	bool	syncAttained_;
	int		syncCertainty_;
	uint8 * syncBuffer_;

};


#endif // MM_INPUT_LIB_HPP
