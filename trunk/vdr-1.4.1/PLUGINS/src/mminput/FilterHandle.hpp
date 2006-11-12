/*
 *  $Id: device.h,v 1.3 2005/02/08 15:21:19 lordjaxom Exp $
 */
 
#ifndef VDR_FILTERHANDLE_H
#define VDR_FILTERHANDLE_H

#define MAXDEVICEFilter 16

#include <vdr/tools.h>
#include "TableBuilder.hpp"

class cFilterHandle {

private:

	struct FilterPids{
	int PidNum;
	int Rhandle;
	int Whandle;
	int length;
	int Tid;
	TableBuilder* tb;
	};
	
	FilterPids* FH;
	bool ClosePipe(int Pid, int tid);
	void sendTable();
public:
	int CreatePipe(int Pid, int tid);
	cFilterHandle();
	~cFilterHandle();
	int Process(uchar* buf);
};

#endif // VDR_FILTERHANDLE_H
