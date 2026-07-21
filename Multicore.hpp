#ifndef _Multicore
#define _Multicore

extern "C" {
    #include "pico/multicore.h"
    #include "hardware/timer.h"
}

#include "Constants.hpp"

#include "Subsystems\Drivetrain\Drivetrain.hpp"
#include "Subsystems\Odometry\Odometry.hpp"
#include "Subsystems\Odometry\Trajectory.hpp"
#include "Subsystems\IMU\IMU.hpp"
#include "Subsystems\RemoteControl\RemoteControl.hpp"
#include "Subsystems\Bluetooth\Bluetooth.hpp"
#include "Subsystems\Screen\Screen.hpp"
#include "Subsystems\PIDController.hpp"

enum Core1Mode{
    Idle,
    Line_Following,
    Maze_Solving,
    Remote_Control,
    Remote_Control_BLE,
    Skip
};

void core1_main_entry_point(void);

class Core1{
    private:
        enum Core1Mode Mode;
    public:
    std::atomic<bool> booted;

    Core1() : booted(false), Mode(Idle){}

    void init(){
        multicore_reset_core1();
        multicore_launch_core1(core1_main_entry_point);
    }

    enum Core1Mode getMode(){
        return Mode;
    }

    void setMode(enum Core1Mode Mode){
        this->Mode = Mode;
        return;
    }
    
    void core1_main_loop();
};

extern Core1 MultiCore1;
extern Drivetrain drivetrain;
extern DifferentialOdometry odometry;
extern PathTracker pathTracker;
extern BMI160 imu;
extern RemoteControl ibus;
extern BluetoothLE BLE;

void start_control_loop();

#endif