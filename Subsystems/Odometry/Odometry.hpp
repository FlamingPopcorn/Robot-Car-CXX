#ifndef _ODOMETRY
#define _ODOMETRY

#include <cmath>

#include "Constants.hpp"

#include "Subsystems\IMU\IMU.hpp"

// struct Pose3D {
//     // Gyroscope Orientation (Degrees)
//     float roll;
//     float pitch;
//     float yaw;
//     // Accelerometer Posittion (mm)
//     float X;
//     float Y;
//     float Z;
//     // Accelerometer Velocity (mm/s)
//     float vX;
//     float vY;
//     float vZ;
// };

struct Pose2D {
    // Posittion (mm)
    float X;
    float Y;
    // Orientation (Degrees)
    float theta;
    float theta_rads;
};

class DifferentialOdometry {
    private:
        Pose2D currentPose = {0.0f, 0.0f, 0.0f, 0.0f};

        int32_t last_left_ticks = 0;
        int32_t last_right_ticks = 0;

        const float TICKS_TO_MM = MOTOR_OUTPUT_RATIO * WHEEL_CIRCUMFERENCE_MM;

    public:
        bool fieldOrientedControl = true;
        DifferentialOdometry() {}

        // Call this function in 100Hz Core 1 timer loop
        void update(IMUData imu, int32_t current_left_ticks, int32_t current_right_ticks, float dt_seconds) {
            
            // Calculate delta ticks independently of the PID loop
            int32_t delta_left_ticks = current_left_ticks - last_left_ticks;
            int32_t delta_right_ticks = current_right_ticks - last_right_ticks;
            
            // Save current ticks for the next loop
            last_left_ticks = current_left_ticks;
            last_right_ticks = current_right_ticks;

            // Convert ticks to millimeters
            float deltaLeft_mm = delta_left_ticks * TICKS_TO_MM;
            float deltaRight_mm = delta_right_ticks * TICKS_TO_MM;

            // Calculate the center distance traveled
            float deltaDistance = (deltaLeft_mm + deltaRight_mm) / 2.0f;

            // Convert IMU Gyro Z (degrees/sec) to radians/sec with (PI / 180.0f)
            float deltaTheta_gyroZ = imu.gyroZ * DEG2RAD * dt_seconds;

            // Wheel heading delta (Rads) based on track width mechanics
            float deltaTheta_wheels = (deltaLeft_mm - deltaRight_mm) / WHEEL_TRACK_WIDTH_MM;

            // Complementary Filter: Trust the Gyro for fast, dynamic updates (95%), 
            // but anchor it to the physical wheel changes (5%) to eliminate long-term drift.
            float deltaTheta = (0.95f * deltaTheta_gyroZ) + (0.05f * deltaTheta_wheels);

            // Save old orientation parameters for midpoint calculation
            float theta_old_rads = currentPose.theta_rads;
            
            // Update global orientation states
            currentPose.theta_rads += deltaTheta;

            // Keep internal values bound cleanly between -PI and +PI to protect downstream path solvers
            while (currentPose.theta_rads >  M_PI) {currentPose.theta_rads -= 2.0f * M_PI;}
            while (currentPose.theta_rads < -M_PI) {currentPose.theta_rads += 2.0f * M_PI;}

            // Integrate gyroscope data to update global heading degrees
            currentPose.theta = currentPose.theta_rads * RAD2DEG;

            // Runge-Kutta 2nd Order Integration (Midpoint Arc Approximation)
            // Instead of projecting using the old angle, project along the average direction of the arc 
            float theta_midpoint_rads = theta_old_rads + (deltaTheta / 2.0f);

            // Update global X and Y positions using trigonometry
            currentPose.X += deltaDistance * cos(theta_midpoint_rads);
            currentPose.Y += deltaDistance * sin(theta_midpoint_rads);
        }

        Pose2D getPose() {
            if (fieldOrientedControl){
                return currentPose;
            } else {
                return {0.0f};
            }
        }

        void resetPose() {
            currentPose = {0.0f, 0.0f, 0.0f, 0.0f};
        }
};

#endif