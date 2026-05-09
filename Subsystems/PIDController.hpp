#ifndef _PIDController_Subsystem
#define _PIDController_Subsystem

#include "Constants.hpp"

class PIDController {
private:
    // Tuning Parameters
    float kp, ki, kd, kf;
        
    // Dynamics (State)
    float integrator = 0.0f;
    float prev_error = 0.0f;
    float prev_input = 0.0f;
    
    // Limits and Timing
    float out_min, out_max, T;
    
public:
    // Publicly accessible setpoint and input
    float setpoint = 0.0f;
    float input = 0.0f;

    // Constructor: Uses a Member Initializer List
    PIDController(float p, float i, float d, float f, float min, float max, float period)
        : kp(p), ki(i), kd(d), kf(f), out_min(min), out_max(max), T(period){
        // Body is empty because members are already initialized!
    }

    // The calculation function (Method)
    float compute();
    
    // Helper to reset the internal state if the robot stops
    void reset();
};

#endif