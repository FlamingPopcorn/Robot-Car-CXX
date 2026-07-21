#ifndef PATH_TRACKER
#define PATH_TRACKER

#include <algorithm>

#include "Constants.hpp"

#include "Subsystems\Odometry\Odometry.hpp"

// Hold Drivetrain Commands
struct DrivetrainCommands {
    float drive_power;
    float turn_power;
};

class PathTracker {
    private:
        // Tuning Parameters (Adjust these based on your chassis physics)
        const float PATH_CRUISE_SPEED_MM_S = 180.0f;
        const float KP_STEERING = 2.8f;  // Proportional steering steering adjustment dial feedback gain.
        

        float clampFloat(float value, float min, float max) {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }

    public:
        bool navigating = false;
        float targetX  = 0.0f, targetY  = 0.0f, targetTheta = 0.0f;
        /// Computes the exact wheel speeds your low-level velocity PIDs should target
        DrivetrainCommands computeTrackingCommands(Pose2D current, float targetX_mm, float targetY_mm, float targetTheta_deg) {
            // Calculate relative physical distance offsets
            float deltaX = targetX_mm - current.X;
            float deltaY = targetY_mm - current.Y;
            float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);

            // Safety check to prevent division by zero errors if exactly on top of a node marker
            if (distance < 1.0f) {
                return {0.0f, 0.0f};
            }

            // Compute the angle from your current position looking directly at the target waypoint
            float targetHeading_rads = std::atan2(deltaY, deltaX);
            
            // Compute heading error (difference between where we face vs where the waypoint is)
            float headingError = targetHeading_rads - current.theta_rads;

            // Normalize heading error between -PI and +PI so the robot always takes the shortest turn path
            while (headingError >  PI) headingError -= 2.0f * PI;
            while (headingError < -PI) headingError += 2.0f * PI;

            // Calculate overall target velocities
             // Drive at a stable cruise velocity, but scale it down proportionally 
            // using a cosine dot-product mapping if facing an aggressive corner turn adjustment angle.
            float cruiseVel = PATH_CRUISE_SPEED_MM_S * std::cos(headingError);

            // If heading error is greater than 90 degrees, prevent forward movement completely 
            // so the chassis rotates cleanly in place first
            if (cruiseVel < 0.0f) cruiseVel = 0.0f; 

            // Use a standard proportional control law loop. We multiply headingError by our gain parameter.
            float targetTurnPower = (headingError / PI) * -KP_STEERING;

            // Clamp turning parameters to safe structural limit envelopes (-1.0 to 1.0 power ratio limits)
            targetTurnPower = clampFloat(targetTurnPower, -1.0f, 1.0f);

            // Kinematic Unmixing Matrix (Differential Drive Forward Math mapping to individual wheel velocities)
            // float halfTrack = WHEEL_TRACK_WIDTH_MM / 2.0f;
            
            DrivetrainCommands commands;
            // Normalize linear velocity to a fractional engine ratio matching your drive parameters
            commands.drive_power = cruiseVel / MAX_MM_S;
            
            // Because targetTurnPower is calculated directly as a scaled error vector ratio,
            // map it directly as normalized target turn power command
            commands.turn_power = targetTurnPower;
            return commands;
        }
};

#endif