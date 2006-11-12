#ifndef TABLE_BUILDER_HPP
#define TABLE_BUILDER_HPP

#include "FTypes.hpp"


struct TableSection;
struct TSHeader;

class TableBuilder
{
public:
	TableBuilder();
	~TableBuilder();
	void clear();

	void accumulate( const TSHeader & tsh, uint8 * payload );
	TableSection * next();
	uint32 length(){return length_;}
	uint8* getTable(){return acc_;}
private:
	uint8	* acc_;
	uint32	length_;
	uint32	restart_;
	uint32	retlen_;
};


#endif // TABLE_BUILDER_HPP
