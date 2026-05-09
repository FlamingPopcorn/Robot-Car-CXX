#include "PIDController.hpp"

float PIDController::compute() {
    // Find the error
    float error = setpoint - input;

    // Proportional
    float proportional = kp * error;

    // Integral (Trapezoidal)
    integrator += ((prev_error + error) / 2.0f) * T;
    float integral = ki * integrator;

    // Anti-Windup Clamping
    if (integral > out_max) integrator = out_max / ki;
    else if (integral < out_min) integrator = out_min / ki;

    // Derivative (using input to avoid "derivative kick")
    float derivative = -kd * (input - prev_input) / T;

    // Feedforward
    float feedforward = kf * setpoint;

    // Save for next loop
    prev_error = error;
    prev_input = input;

    float total_output = proportional + (ki * integrator) + derivative + feedforward;

    // Final Output Clamping
    if (total_output > out_max) return out_max;
    if (total_output < out_min) return out_min;
    return total_output;
}

void PIDController::reset() {
    integrator = 0;
    prev_error = 0;
    prev_input = 0;
}