/*
 * mminput.cpp: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: mminput.cpp,v 0.0.1 2007/02/08 Stefan Rieke 
 */
#include <sstream>

#include "mminput.hpp"
#include "device.hpp"
#include "setup.hpp"

#include "i18n.h"
#define MAXDVBDEVICES 2
#define MAXMMINPUTDEVICEINDEX 5
#define SLEEPTIME 10
const char *cPluginMMInput::DESCRIPTION = "MMInput Driver";

cPluginMMInput::cPluginMMInput(void) {
        _TunerData = NULL;
        _TunerData = new std::vector< TunerRec* >;
}

cPluginMMInput::~cPluginMMInput() {
        if(_TunerData != 0x0){
                _TunerData->clear();
        }
        delete _TunerData;
        delete _TimerMMInput;
}

const char *cPluginMMInput::Description(void) {
        return tr(DESCRIPTION);
}

bool cPluginMMInput::Start(void) {
        //	i18n_name = Name();
        RegisterI18n(Phrases);
                
        _TimerMMInput = new cTimerMMInput(SLEEPTIME, _TunerData);
        _TimerMMInput->Start();
        
        return true;
}

bool cPluginMMInput::Initialize(void){
        for (int i = 0; i < MAXDVBDEVICES; i++) {
                TunerRec* myTuner = new TunerRec;
                myTuner->pDevice = cMMInputDevice::Init();
                myTuner->pMM = 0x0;
                myTuner->activated = false;
                myTuner->activationErr = 0;
                myTuner->mmindex = -1;
                myTuner->deviceName = "dummyDevice";
                if(myTuner->pDevice != NULL){
                        _TunerData->push_back(myTuner);
                }
                else{
                        delete myTuner;
                }
        }
        return true;
}

void cPluginMMInput::Housekeeping(void) {
}

const char *cPluginMMInput::MainMenuEntry(void) {
        return NULL;
}

cOsdObject *cPluginMMInput::MainMenuAction(void) {
        return NULL;
}

cMenuSetupPage *cPluginMMInput::SetupMenu(void) {
        return new cMMInputMenuSetupPage;
}

bool cPluginMMInput::SetupParse(const char *Name, const char *Value) {
        return MMInputSetup.SetupParse(Name, Value);
}

cTimerMMInput::cTimerMMInput(int sleepTime, std::vector< TunerRec* >* TunerData):cThread("TimerMMInput"){
        _sleepTime = sleepTime;
        _TunerData = TunerData;
        if(_sleepTime < 1) _sleepTime = 1; // make sure, that the sleep has an minimum of 1 second.
        
}

cTimerMMInput::~cTimerMMInput(){
}

/*if at least on device is found, the trhead will be destroyed */
void cTimerMMInput::Action(void){
        
        MMInputDevice * pMM;
        
        while(Running()){
                
                // check if there si an empty slot in the vector
                // if not delete timer thread.
                
                // first: count registerd devices
                uint8_t numDev = 0;
                for(int i = 0; i < (int)_TunerData->size(); i++){

                        TunerRec* tuner = (* _TunerData)[i];
                        if( tuner->mmindex != -1) numDev++;
                }
                
                // now check, if there is an empty slot too register a new device
                if(numDev == (uint8_t)_TunerData->size()){
                        printf("cTimerMMInput destreoy timer thread\n");                        
                        Cancel(1);
                        break;
                }
                
                for(int i = 0; i < MAXMMINPUTDEVICEINDEX; i++){
                        bool full = true; // see is there enough space to handle more devices
                        pMM = 0x0;
                        pMM = MMInputDevice::withIndex( i );
                        
                        printf("cTimerMMInput test index %d\n",i);
                        if(pMM == 0x0) break; // stop if no devices is present
                        
                        for(int j = 0; j < (int)_TunerData->size(); j++){

                                printf("cTimerMMInput check index %d\n",i);
                                TunerRec* tuner = (* _TunerData)[j];
                                if( tuner->mmindex == i){
                                        full = false;
                                        break;                                        
                                }
                                
                                if( tuner->mmindex != -1) continue;
                                full = false;
                                
                                printf("cTimerMMInput try to fetch device with index %d\n",i);
                        
                                printf("set device with index %d at position %d\n",i,j);
                                
                                tuner->mmindex = i;
                                tuner->pMM = pMM;
                                
                                stringstream mynum;
                                mynum.str("");
                                mynum << i;
                                
                                tuner->deviceName = "MMInputDevice " + mynum.str();
                                printf("cTimerMMInput new device name: %s\n",tuner->deviceName.c_str());
                                
                                tuner->pDevice->SetMMInputDevice(pMM, i);
                                tuner->activated = true;
                                numDev++;
                                break;
                        }                        
                        if(full == true) break; // because we could not handle addition devices
                }
                
                printf("TimerMMInput running with sleep time %d seconds. number of device(s) %d\n",
                       _sleepTime,numDev);
                cCondWait::SleepMs(_sleepTime*1000);
        }
}

VDRPLUGINCREATOR(cPluginMMInput); // Don't touch this!
