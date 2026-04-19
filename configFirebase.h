#ifndef __CONFIGFIREBASE__H
#define __CONFIGFIREBASE__H
// Master code final
#include <Arduino.h>

class CONFIGFIREBASE {
  public:
    CONFIGFIREBASE();
    bool initFirebase();
    void sendFirebaseData(int mq135Val, int mq7Val, int dust, float temperature, bool isPersonFall, bool warningStatus, bool emergencyStatus, float humidity);

  private:
    const char * error = NULL;
    void initTime();
};

#endif