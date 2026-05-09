extern "C" {
    #include "Sensor_Libs/ssd1306.h"
}

#include "Screen.hpp"

Screen* Screen::instance = nullptr;

void Screen::process_menuButton(int button){            
        switch(button){
            case BTN_UP:
                if (menu.cursorOption > 0 && menu.currentPageID < 10)
                    menu.cursorOption--;
                break;
            case BTN_DOWN:
                if (menu.cursorOption < MAX_PAGES - 1 && menu.currentPageID < 10)
                    menu.cursorOption++;

                break;
            case BTN_SELECT:
                if (menu.currentPageID == 0){
                    menu.currentPageID = menu.cursorOption + 1;
                    menu.cursorOption = 0;

                } else if (menu.currentPageID < 4 && menu.cursorOption == 0){ // Launch screen for specific mode
                    menu.currentPageID += 10;
                    // enum Core1Mode core1_mode = menu.currentPageID - 10;
                    // enum Core1Mode core1_mode = 3;
                    // change_core1_mode(core1_mode);
                    menu.cursorOption = -1;
                } else if (menu.currentPageID < 4 && menu.cursorOption == 1){ // Go back
                    menu.currentPageID = 0;
                    menu.cursorOption = 0;
                } else if (menu.currentPageID == 4 && menu.cursorOption == 0){ // Load Calibration
                    menu.currentPageID = 21;
                    menu.cursorOption = 0;
                    // bool loaded = loadCalibration(global_reflectiveSystem->calibration);
                    // global_reflectiveSystem->calibrationLoaded = loaded;
                    // menu.currentPageID = loaded ? 22 : 23;
                } else if (menu.currentPageID == 4 && menu.cursorOption == 1){ // Run Calibration
                    menu.currentPageID = 25;
                    menu.cursorOption = 0;
                    // runCalibration();
                } else if (menu.currentPageID == 4 && menu.cursorOption == 2){ // View Debug Values

                }
                break;
            case BTN_BACK:
                if (menu.currentPageID < 10){
                    menu.currentPageID = 0;
                    menu.cursorOption = 0;
                } else if (menu.currentPageID < 20){ // Exit screen for specific mode
                    // enum Core1Mode core1_mode = Idle;
                    // change_core1_mode(core1_mode);
                    menu.currentPageID -= 10;
                    menu.cursorOption = 0;
                } else if (menu.currentPageID == 22 || menu.currentPageID == 23 ||
                            menu.currentPageID == 28 || menu.currentPageID == 29){
                    menu.currentPageID = 4;
                    menu.cursorOption = 0;
                }
                break;
        }
}

void Screen::updateMenu(){
    ssd1306_clear(disp);
    
    if (menu.currentPageID < 10){
        // Draw Cursor
        ssd1306_draw_string(disp, 0, 15 + 10 * menu.cursorOption, 1, "->");
    }
    
    switch (menu.currentPageID) {
        // Main menu page
        case 0:
            ssd1306_draw_string(disp, 36, 0, 1, "Main Menu");
            ssd1306_draw_string(disp, 12, 15, 1,"Line Following");
            ssd1306_draw_string(disp, 12, 25, 1,"Maze Solving");
            ssd1306_draw_string(disp, 12, 35, 1,"Remote Control");
            ssd1306_draw_string(disp, 12, 45, 1,"Debug");
            break;
        
        // Line Following page
        case 1:
            ssd1306_draw_string(disp, 8, 0, 1, "Line Following Menu");
            ssd1306_draw_string(disp, 12, 15, 1,"Go");
            break;
        // Maze Solving page
        case 2:
            ssd1306_draw_string(disp, 8, 0, 1, "Maze Solving Menu");
            ssd1306_draw_string(disp, 12, 15, 1,"Go");
            break;
        // Radio Control page
        case 3:
            ssd1306_draw_string(disp, 8, 0, 1, "Radio Control Menu");
            ssd1306_draw_string(disp, 12, 15, 1,"Go");
            break;
        // Debug page
        case 4:
            ssd1306_draw_string(disp, 8, 0, 1, "Debug Menu");
            ssd1306_draw_string(disp, 12, 15, 1,"Load Calibration");
            ssd1306_draw_string(disp, 12, 25, 1,"Run Calibration");
            break;
        // Go Line Following
        case 11:
            ssd1306_draw_string(disp, 8, 0, 1, "Line Following :)");
            break;
        // Go Maze Solving
        case 12:
            ssd1306_draw_string(disp, 8, 0, 1, "Maze Solving :)");
            break;
        // Go Remote Control
        case 13:
            ssd1306_draw_string(disp, 8, 0, 1, "Remote Control :)");
            break;

        // Calibration Stuff
            // Go Load Calibration
            case 21:
                ssd1306_draw_string(disp, 8, 25, 1, "Loading Calibration...");
                break;
            // Successful Load
            case 22:
                ssd1306_draw_string(disp, 8, 15, 1, "Calibration Load");
                ssd1306_draw_string(disp, 13, 25, 1, "Succeded");
                break;
            // Failed Load
            case 23:
                ssd1306_draw_string(disp, 8, 15, 1, "Calibration Load");
                ssd1306_draw_string(disp, 13, 25, 1, "Failed");
                break;
            
            // Go Run Calibration
            case 25:
                ssd1306_draw_string(disp, 8, 25, 1, "Running Calibration...");
                break;
            // Calibrate White
            case 26:
                ssd1306_draw_string(disp, 8, 15, 1, "Place Sensors on");
                ssd1306_draw_string(disp, 13, 25, 1, "White");
                break;
            // Calibrate Black
            case 27:
                ssd1306_draw_string(disp, 8, 15, 1, "Place Sensors on");
                ssd1306_draw_string(disp, 13, 25, 1, "Black");
                break;
            // Successful Save
            case 28:
                ssd1306_draw_string(disp, 8, 15, 1, "Calibration Save");
                ssd1306_draw_string(disp, 13, 25, 1, "Succeded");
                break;
            // Failed Load
            case 29:
                ssd1306_draw_string(disp, 8, 15, 1, "Calibration Save");
                ssd1306_draw_string(disp, 13, 25, 1, "Failed");
                break;
    }

    // char buffer[20];
    // snprintf(buffer, sizeof(buffer), "%d, %d", menu->cursorOption, menu->currentPageID);
    // ssd1306_draw_string(disp, 12, 55, 1, buffer);

    ssd1306_show(disp);
}