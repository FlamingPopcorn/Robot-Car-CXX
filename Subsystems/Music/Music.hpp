#ifndef _Music_Subsystem
#define _Music_Subsystem

#include <cstdint>

#include "hardware/pwm.h"

// Musical Notes for a standard Piezo/PWM driver
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139 // 139 hits resonance
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185 // 185 hits resonance
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262 //
#define NOTE_CS4 277 // 277 hits resonance
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523 // 523 hits resonance
#define NOTE_CS5 554 // 554 hits resonance
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define REST     0

class Music{
private:
    uint8_t bpm;
    const float time_signature;
    const uint16_t* notesL;
    const uint16_t* notesR;
    const uint8_t* notesDuration;
    const float note_percent;
    uint16_t length;

    uint getWholeNoteDuration(uint8_t bpm, float time_signature){
        return (uint) ((1000.0 * 60000.0 * time_signature / bpm));
    }

    // void playNote(uint16_t note, uint8_t level, struct Motor* motor){
    //     if (note > 25.0){
    //         uint16_t pwm_f = note;
    //         // const uint16_t pwm_wrap = 999;
    //         // float clkdiv = 150000000.0 / (float)((pwm_wrap + 1) * pwm_f);

    //         const float clkdiv = 100.0;
    //         uint16_t pwm_wrap = (150000000.0 / (float)(pwm_f * clkdiv)) - 1;
    //         pwm_set_wrap(motor->pwmSliceFWD, pwm_wrap);

    //         if (note < 90.0){
    //             pwm_set_both_levels(motor->pwmSliceFWD, pwm_wrap * level * 1.03 / 100, pwm_wrap);
    //             // pwm_set_gpio_level(motor->pinFWD, pwm_wrap * 16 / 16);
    //             // pwm_set_gpio_level(motor->pinRVS, pwm_wrap + 1);
    //         } else {
    //             pwm_set_both_levels(motor->pwmSliceFWD, pwm_wrap * level / 100, pwm_wrap);
    //             // pwm_set_gpio_level(motor->pinFWD, pwm_wrap * 6 / 6);
    //             // pwm_set_gpio_level(motor->pinRVS, pwm_wrap + 1);
    //         }
    //     } else {
    //         pwm_set_both_levels(motor->pwmSliceFWD, 0, 0);
    //     }
    // }

public:
    Music(uint8_t bpm, float time_sig, uint16_t* notesLeft, uint16_t* notesRight, uint8_t* notesDur, float notePer, uint16_t len) : bpm(bpm), time_signature(time_sig), notesL(notesLeft), notesR(notesRight), notesDuration(notesDur), note_percent(notePer), length(len){}
    ~Music();
};


// void playNote(uint16_t note, uint8_t level, struct Motor* motor);
void playMusic(uint8_t songNum);

#endif