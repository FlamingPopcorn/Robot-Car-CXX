#ifndef _Multicore
#define _Multicore

extern "C" {
    #include "pico/multicore.h"
    #include "hardware/timer.h"
}

#include "Constants.hpp"

#include "Subsystems\Drivetrain\Drivetrain.hpp"
#include "Subsystems\RemoteControl\RemoteControl.hpp"
#include "Subsystems\Screen\Screen.hpp"
#include "Subsystems\PIDController.hpp"

enum Core1Mode{
    Idle,
    Line_Following,
    Maze_Solving,
    Remote_Control,
    Skip
};

void core1_main_entry_point(void);

class Core1{
    private:
        enum Core1Mode Mode;
    public:
    std::atomic<bool> booted;
    bool enabled;
    Core1() : enabled(false), booted(false), Mode(Idle){
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
extern RemoteControl ibus;

void start_control_loop();

#endif