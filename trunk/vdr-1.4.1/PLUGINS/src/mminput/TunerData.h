#ifndef TUNER_DATA_H
#define TUNER_DATA_H

#include <vector>
#include <string>

#include "FTypes.hpp"
#include "MMInputLib.hpp"
#include "device.hpp"

using namespace std;

struct TunerRec{
	MMInputDevice* pMM;
	cMMInputDevice* pDevice;
	bool	activated;
	int		activationErr;
	int		mmindex;
	string deviceName;
};

#endif TUNER_DATA_H
