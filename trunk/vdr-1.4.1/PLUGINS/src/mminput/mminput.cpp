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
        
        TunerRec* myFirstTuner = new TunerRec;
        TunerRec* mySecondTuner = new TunerRec;
        
        myFirstTuner->pDevice = cMMInputDevice::Init();
        myFirstTuner->pMM = 0x0;
        myFirstTuner->activated = false;
        myFirstTuner->activationErr = 0;
        myFirstTuner->mmindex = -1;
        myFirstTuner->deviceName = "dummyDevice";
        
        mySecondTuner->pDevice = cMMInputDevice::Init();
        mySecondTuner->pMM = 0x0;
        mySecondTuner->activated = false;
        mySecondTuner->activationErr = 0;
        mySecondTuner->mmindex = -1;
        mySecondTuner->deviceName = "dummyDevice2";
        
        _TunerData->push_back(myFirstTuner);	
        _TunerData->push_back(mySecondTuner);	
        
        _TimerMMInput = new cTimerMMInput(1, _TunerData);
        _TimerMMInput->Start();
        
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
        
        bool found = false;
        MMInputDevice * pMM;
        
        while(Running()){
                
                if ( found == true ) Cancel(1);
                
                for(int i = 0; i < (int)_TunerData->size(); i++){
                        printf("cTimerMMInput check index %d\n",i);
                        if( (*_TunerData)[i]->mmindex != -1) continue;
                        
                        printf("cTimerMMInput try to fetch device with index %d\n",i);
                        pMM = 0x0;
                        pMM = MMInputDevice::withIndex( i );
                        
                        if(pMM == 0x0){
                                printf("cTimerMMInput found no device with index %d\n",i);
                        }
                        else{
                                printf("cTimerMMInput found device with index %d, array size: %d\n",i,(int)_TunerData->size());
                                
                                TunerRec* tuner;
                                
                                printf("set device at position %d\n",i);
                                tuner =(* _TunerData)[i];
                                
                                tuner->mmindex = i;
                                tuner->pMM = pMM;
                                
                                stringstream mynum;
                                mynum.str("");
                                mynum << i;
                                
                                tuner->deviceName = "MMInputDevice " + mynum.str();
                                printf("cTimerMMInput new device name: %s\n",tuner->deviceName.c_str());
                                
                                tuner->pDevice->SetMMInputDevice(pMM, i);
                                found = true;
                        }                        
                }
                
                printf("TimerMMInput running with sleep time %d seconds. number of device(s) %d\n",
                       _sleepTime,(int)_TunerData->size());
                cCondWait::SleepMs(_sleepTime*1000);
        }
}

VDRPLUGINCREATOR(cPluginMMInput); // Don't touch this!
