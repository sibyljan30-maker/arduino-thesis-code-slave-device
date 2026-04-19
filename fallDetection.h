#ifndef __FALLDETECTION__H
#define __FALLDETECTION__H
//Master code final
#include <MPU6050.h>

class FALLDETECTION {
  public:
    FALLDETECTION();
    void initMPU();
    bool isPersonFainted();

  private:
    MPU6050 _mpu;

    // Thresholds
    static constexpr float FALL_ACCEL_THRESHOLD  = 1.5f;   // g — impact spike
    static constexpr float FAINT_ACCEL_THRESHOLD = 0.2f;   // g — near stillness
    static constexpr float FAINT_GYRO_THRESHOLD  = 5.0f;  // °/s — no rotation
    static constexpr unsigned long FAINT_DURATION_MS = 3000; // 10 seconds

    enum class State { MONITORING, FALL_DETECTED, CHECKING_FAINT };
    State _state;
    unsigned long _faintStartTime;

    float getAccelMagnitude();
    float getGyroMagnitude();
};

#endif