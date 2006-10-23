#ifndef PROGRAM_INFO_HPP
#define PROGRAM_INFO_HPP

#include <string>
#include <vector>

struct EventInfo
{
	EventInfo() : id_( ~0 ), ratedForAge_( 0 )  { }	// -1 = not set

	uint32	id_;
	uint32	begToD_;
	uint32	endToD_;
	uint64	startTime_;
	std::string	name_;
	std::string	desc_;
	struct ItemInfo
	{
		uint32 id_;
		std::string item_;
		std::string info_;
	};
	std::vector<ItemInfo> items_;
	int ratedForAge_;
};


struct ProgramInfo
{
	uint32		id_;
	std::string	pname_;
	std::string	cname_;
	
	std::vector<EventInfo>	events_;
	
	int			hasAudioVideo_;
};

#endif // PROGRAM_INFO_HPP
