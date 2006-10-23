#ifndef <string>

string newDeviceName;

class cMenuSetupMMInput : public cMenuSetupPage {
private:
  int newDeviceName;
protected:
  virtual void Store(void);
public:
  cMenuSetupMMInput(void);
  };
