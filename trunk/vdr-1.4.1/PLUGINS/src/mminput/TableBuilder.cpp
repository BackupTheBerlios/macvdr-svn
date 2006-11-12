// $Id: TableBuilder.cpp,v 1.3 2005/07/31 15:56:41 johnd Exp $
#include "TableBuilder.hpp"

#include <string>

#include "DPConnect/Streaming.hpp"

/**
 *	TableBuilder constructor
 */
TableBuilder::TableBuilder() :
	acc_( new uint8[4400] ),
	length_( 0 ),
	restart_( 0 ),
	retlen_( 0 )
{
}

/**
 *	TableBuilder destructor
 */
TableBuilder::~TableBuilder()
{
	delete [] acc_;
}

/**
 *	Clear out any state in this table
 */
void TableBuilder::clear()
{
	length_ = 0;
	restart_ = 0;
	retlen_ = 0;
}


/**
 *	Accumulate given data into current table
 */
void TableBuilder::accumulate( const TSHeader & tsh, uint8 * payload )
{
	uint32 payloadSize = 184;
	if (tsh.adaption & 0x02)
	{		// apparently these can occur here
		adaption_field & af = *(adaption_field*)payload;
		payload += af.size();
		payloadSize -= af.size();
	}
	
	if (int(payloadSize) <= 0)
	{
		printf( "TableBuilder: Packet has zero or negative payload %ld!\n",
			payloadSize );
		return;
	}

	if (!tsh.payloadStart)
	{
		if (length_ != 0)
		{
			memcpy( acc_+length_, payload, payloadSize );
			length_ += payloadSize;
		}
	}
	else
	{
		// this is where tables restart if they stop with TID_none
		restart_ = (length_ != 0) ? length_+payload[0] : 0;

		// if previous packet was part of a table then
		// accumulate the whole lot, otherwise use the pointer field
		uint32 payloadOff = (length_ != 0) ? 1 : 1 + payload[0];
		payload += payloadOff;
		payloadSize -= payloadOff;
		
		if (int(payloadSize) < 0)
		{
			printf( "TableBuilder: Packet negative table payload %ld!\n",
				payloadSize );
			return;
		}
		
		memcpy( acc_+length_, payload, payloadSize );
		length_ += payloadSize;
		
		// TODO: check when continuity does not match up 
	}
}

/**
 *	Return the next available table. This must be called until it
 *	returns NULL after each call to accumulate. (If we want to be
 *	able to delay calls then we'd have to use something different
 *	to restart when we get to the end of a table ... and there may be
 *	TID_none's either at the end of the prev packet or in the middle
 *	of the current packet ... so it's not totally straightforward)
 */
TableSection * TableBuilder::next()
{
	// see if we have any data
	if (length_ == 0) return 0;
	
	// eat up data returned from last call
	if (retlen_ > 0)
	{	// never sets retlen_ if length_ set to 0
		// never moves > 1 packet of data
		memmove( acc_, acc_+retlen_, length_ );
		retlen_ = 0;
	}
	
	// get a pointer to the table
	TableSection * table = (TableSection*)acc_;
	
	// see if it's a real table
	uint32 orestart = restart_;
	if (table->table_id == TID_none)
	{
		table = (TableSection*)(acc_ + restart_);
		restart_ = 0;
		if (table->table_id == TID_none)
		{
			length_ = 0;
			return 0;
		}
	}
	
	// check the data used by the table
	uint32 noff = ((uint8*)&table->next()) - acc_;
	if (noff > length_)	// if section_length is unread/garbage then always true
	{					// (since only 12 bits never negative (alloc'd > 4099 too))
		// not enough data yet
		restart_ = orestart;
		return 0;
	}
	else if (noff == length_)
	{
		// exactly the right amount of data
		length_ = 0;
	}
	else // (noff < length_)
	{
		// more data then we need
		retlen_ = noff;	// remember that this data has been used
		length_ -= noff;
		if (restart_ > 0)
		{
			restart_ -= noff;
			if (int(restart_) < 0)
			{
				printf( "ERROR: Table 0x%02x length %ld overruns restart point %ld\n",
					int(table->table_id), noff, restart_ + noff );
				restart_ = 0;
				// but still return the table...
			}
		}
	}
	
	return table;
}


/*
 * $Log: TableBuilder.cpp,v $
 * Revision 1.3  2005/07/31 15:56:41  johnd
 * Silenced some printf wrong-type warnings
 *
 * Revision 1.2  2004/11/27 07:36:36  johnd
 * Checking for negative sizes to prevent crashing from corrupted streams.
 *
 * Revision 1.1  2004/08/15 08:34:54  johnd
 * Separated out from TSHandler
 *
 */
