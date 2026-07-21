#include <cmath>
#include <cstdio>

#include "hardware/watchdog.h"

#include "Multicore.hpp"

struct repeating_timer pid_timer;
struct repeating_timer odometry_timer;
struct repeating_timer control_timer;

static absolute_time_t last_time_pid;
static absolute_time_t last_time_odometry;

bool odometryUpdate_timer_callback(struct repeating_timer *t) {
    absolute_time_t now = get_absolute_time();
    // Calculate actual time passed in seconds
    float dt = absolute_time_diff_us(last_time_odometry, now) / 1000000.0f;
    last_time_odometry = now;

    // Get raw, absolute ticks
    auto [left_ticks, right_ticks] = drivetrain.getTicks();

    // Get calibrated IMU data
    IMUData motion = imu.getMotion();

    // Update Odometry
    odometry.update(motion, left_ticks, right_ticks, dt);

    return true;
}

bool pidUpdate_timer_callback(struct repeating_timer *t) {
    absolute_time_t now = get_absolute_time();
    // Calculate actual time passed in seconds
    float dt = absolute_time_diff_us(last_time_pid, now) / 1000000.0f;
    last_time_pid = now;

    // Ensure don't divide by zero if the loop runs too fast
    if (dt < 0.0001f) return true;

    auto [L, Lv, R, Rv] = drivetrain.readEncoders();
    drivetrain.pidL.input = Lv;
    drivetrain.pidR.input = Rv;

    switch(MultiCore1.getMode()){
        case Idle:
            drivetrain.pidL.setpoint = (0.0); 
            drivetrain.pidR.setpoint = (0.0);
            break;
        case Line_Following:
            // mode_LineFollowing(global_reflectiveSystem);
            break;
        case Maze_Solving:
            // mode_MazeSolving(global_distanceSystem);
            break;
        case Remote_Control:
            // mode_RemoteControl(global_ibus);
            break;
        case Remote_Control_BLE:

            break;
        case Skip: // Use for skip mode check
            break;
        default:
            MultiCore1.setMode(Idle);
            break;
    }

    drivetrain.pidL.setSetpoint(drivetrain.target_drive_mm_s + drivetrain.target_turn_mm_s);
    drivetrain.pidR.setSetpoint(drivetrain.target_drive_mm_s - drivetrain.target_turn_mm_s);

    return true; 
}

void core1_main_entry_point(){
    MultiCore1.core1_main_loop();
}

void Core1::core1_main_loop(){
    last_time_pid = last_time_odometry = get_absolute_time();

    add_repeating_timer_ms(-1000*ODOMETRY_PERIOD, odometryUpdate_timer_callback, NULL, &odometry_timer);
    sleep_ms(5);
    add_repeating_timer_ms(-1000*PID_PERIOD, pidUpdate_timer_callback, NULL, &pid_timer);

    Pose2D currentPose;

    printf("Hello! from Core1!\n");
    booted = true;
    while(true){
        enum Core1Mode Mode = MultiCore1.getMode();
        // printf("%d\n", Mode);
        switch (Mode){
            case Idle:
                printf("Cheeto is Idle\n");
                sleep_ms(250);
                break;
            case Line_Following:
                // mode_LineFollowing(global_reflectiveSystem);
                break;
            case Maze_Solving:
                // mode_MazeSolving(global_distanceSystem);
                break;
            case Remote_Control:
                // mode_RemoteControl(global_ibus);
                ibus.readIBus();

                drivetrain.target_drive_power = (ibus.getChannel(CHANNEL_LSTICK_Y).value - 1500) / 500.0f;
                drivetrain.target_turn_power = (ibus.getChannel(CHANNEL_RSTICK_X).value - 1500) / 250.0f;
                // printf("Powers: %f | %f\n", drivetrain.target_drive_power.load(), drivetrain.target_turn_power.load());

                // Deadzones
                if (fabsf(drivetrain.target_drive_power) < 0.05f) drivetrain.target_drive_power = 0.0f;
                if (fabsf(drivetrain.target_turn_power) < 0.05f) drivetrain.target_turn_power = 0.0f;

                drivetrain.target_drive_mm_s = drivetrain.target_drive_power * MAX_MM_S;
                drivetrain.target_turn_mm_s = drivetrain.target_turn_power * MAX_MM_S;

                // Reset integrators if stopped to prevent "jump" when starting
                if (drivetrain.target_drive_mm_s == 0.0f && drivetrain.target_turn_mm_s == 0.0f) {
                    drivetrain.pidL.reset();
                    drivetrain.pidR.reset();
                }
                sleep_ms(1);

                break;
            case Remote_Control_BLE:
                BLE.poll();
                BLE.getState();

                switch (BLE.state.controller){
                    case FLUTTER_MAGIC_BYTE:
                        if (BLE.state.btn1){odometry.resetPose();}
                        odometry.fieldOrientedControl = BLE.state.sw1;

                        currentPose = odometry.getPose();

                        drivetrain.target_drive_power = BLE.state.joystick_radius * sinf(((BLE.state.joystick_angle / 255.0f * 360.0) - currentPose.theta) * DEG2RAD) / 255.0f;
                        drivetrain.target_turn_power = BLE.state.joystick_radius * cosf(((BLE.state.joystick_angle / 255.0f * 360.0) - currentPose.theta) * DEG2RAD) / 255.0f;
                        break;
                    case FLUTTER_PATH_MAGIC_BYTE:
                        odometry.fieldOrientedControl = true;
                        currentPose = odometry.getPose();

                        if (!pathTracker.navigating && BLE.hasPathPoints()) {
                            if (BLE.popNextWaypoint(pathTracker.targetX, pathTracker.targetY, pathTracker.targetTheta)) {
                                pathTracker.navigating = true;
                                odometry.resetPose();
                                printf("[NAV] Tracking started. Heading toward first node: (%f, %f)\n", pathTracker.targetX, pathTracker.targetY);
                            }
                        }
                        
                        if (pathTracker.navigating){
                            float dX = pathTracker.targetX - currentPose.X;
                            float dY = pathTracker.targetY - currentPose.Y;
                            float distance_remaining = std::sqrt(dX * dX + dY * dY);

                            if (distance_remaining < 15.0f) { // Arrived within 15mm threshold zone
                                printf("[NAV] Heading towards: %f, %f, with %f final angle | from: %f, %f, with %f - %f start angle\n", pathTracker.targetX, pathTracker.targetY, pathTracker.targetTheta, currentPose.X, currentPose.Y, currentPose.theta_rads, currentPose.theta);
                                if (!BLE.popNextWaypoint(pathTracker.targetX, pathTracker.targetY, pathTracker.targetTheta)) {
                                    pathTracker.navigating = false;
                                    drivetrain.target_drive_power = 0.0f;
                                    drivetrain.target_turn_power = 0.0f;
                                    printf("[NAV] Trajectory complete.\n");
                                }
                            }

                            if (pathTracker.navigating) {
                                // Compute commands based on your current odometry positions
                                DrivetrainCommands outputs = pathTracker.computeTrackingCommands(currentPose, pathTracker.targetX, pathTracker.targetY, pathTracker.targetTheta);

                                // Update Drivetrain
                                drivetrain.target_drive_power = outputs.drive_power;
                                drivetrain.target_turn_power = outputs.turn_power;
                            }
                        }

                        break;
                    case DABBLE_MAGIC_BYTE:
                        if (BLE.state.cross){odometry.resetPose();}
                        if (BLE.state.square){odometry.fieldOrientedControl = !odometry.fieldOrientedControl;}

                        currentPose = odometry.getPose();

                        drivetrain.target_drive_power = BLE.state.joystick_radius * sinf(((BLE.state.joystick_angle * 15.0) - currentPose.theta) * DEG2RAD) / 7.0f;
                        drivetrain.target_turn_power = BLE.state.joystick_radius * cosf(((BLE.state.joystick_angle * 15.0) - currentPose.theta) * DEG2RAD) / 7.0f;
                        break;
                    defualt:
                        drivetrain.target_drive_power = 0.0;
                        drivetrain.target_turn_power = 0.0;
                }

                if (BLE.state.controller != FLUTTER_PATH_MAGIC_BYTE){
                    while (BLE.hasPathPoints()){
                        if (!BLE.popNextWaypoint(pathTracker.targetX, pathTracker.targetY, pathTracker.targetTheta)) pathTracker.navigating = false;
                    }

                }
                // printf("Powers: %f | %f\n", drivetrain.target_drive_power.load(), drivetrain.target_turn_power.load());
                // printf("Joystick: %d | %d Theta: %f\n", BLE.state.joystick_radius, BLE.state.joystick_angle, current_pos.theta);

                drivetrain.target_drive_mm_s = drivetrain.target_drive_power * MAX_MM_S;
                drivetrain.target_turn_mm_s = drivetrain.target_turn_power * MAX_MM_S;

                // Reset integrators if stopped to prevent "jump" when starting
                if (drivetrain.target_drive_mm_s == 0.0f && drivetrain.target_turn_mm_s == 0.0f) {
                    drivetrain.pidL.reset();
                    drivetrain.pidR.reset();
                }
                sleep_ms(1);

                break;
            case Skip: // Use for skip mode check
                break;
            default:
                Mode = Idle;
            break;
        }
    }
}

bool control_motors_timer_callback(struct repeating_timer *t){
    watchdog_update();

    float left_out = 0.0;
    float right_out = 0.0;

    // Run Independent PIDs
    if (MultiCore1.getMode() != Idle){
        left_out = drivetrain.pidL.compute();
        right_out = drivetrain.pidR.compute();
    }

    // Call the updated drive function
    drivetrain.setVelocities(left_out, right_out);
    return true;
}

void start_control_loop() {
    // Negative delay (-1000) means "start to start" timing
    // This ensures exactly 1kHz even if the PID math takes 100us
    add_repeating_timer_ms(-1000*PID_PERIOD, control_motors_timer_callback, NULL, &control_timer);
}