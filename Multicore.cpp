#include <cmath>

#include "Multicore.hpp"
#include <cstdio>

struct repeating_timer pid_timer;
struct repeating_timer control_timer;

static absolute_time_t last_time;

bool pidUpdate_timer_callback(struct repeating_timer *t) {
    absolute_time_t now = get_absolute_time();
    // Calculate actual time passed in seconds
    float dt = absolute_time_diff_us(last_time, now) / 1000000.0f;
    last_time = now;

    // Ensure we don't divide by zero if the loop runs too fast
    if (dt < 0.0001f) return true;

    auto [L, R] = drivetrain.readEncoders();
    drivetrain.pidL.input = L * MOTOR_OUTPUT_RATIO * WHEEL_CIRCUMFERENCE;
    drivetrain.pidR.input = R * MOTOR_OUTPUT_RATIO * WHEEL_CIRCUMFERENCE;

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

    while(!enabled){sleep_ms(50);}

    last_time = get_absolute_time();

    add_repeating_timer_ms(-1000*LEFT_PERIOD, pidUpdate_timer_callback, NULL, &pid_timer);

    printf("Hello! from Core1!\n");
    booted = true;
    while(true){
        enum Core1Mode Mode = MultiCore1.getMode();
        // printf("%d\n", Mode);
        switch (Mode){
            case Idle:
                // drive_pid->setpoint = (0.0); 
                // turn_pid->setpoint = (0.0);
                break;
            case Line_Following:
                // mode_LineFollowing(global_reflectiveSystem);
                break;
            case Maze_Solving:
                // mode_MazeSolving(global_distanceSystem);
                break;
            case Remote_Control:
                // mode_RemoteControl(global_ibus);
                // printf("Starting Remote Mode\n");
                ibus.readIBus();
                // printf("read Ibus\n");
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
            case Skip: // Use for skip mode check
                break;
            default:
                Mode = Idle;
            break;
        }
    }
}

bool control_motors_timer_callback(struct repeating_timer *t){
    // watchdog_update();

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
    add_repeating_timer_ms(-1000*LEFT_PERIOD, control_motors_timer_callback, NULL, &control_timer);
}