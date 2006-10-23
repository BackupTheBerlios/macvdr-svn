
#include <setupMMInputMenu.h>


cMenuSetupHello::cMenuSetupMMInput(void)
{
  newDeviceName = DeviceName;
  Add(new cMenuEditIntItem( tr("Device Name"),      &newDeviceName));
}