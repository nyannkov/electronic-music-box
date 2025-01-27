/*
 * MIT License
 * (https://opensource.org/license/mit/)
 *
 * Copyright (c) 2025 nyannkov
 */
#include <FspTimer.h>
#include <Psgino.h>
#include "src/emu2149/emu2149.h"

#define ENDLESS_MOMOTARO_LOOP

const char mml[] =
#if defined(ENDLESS_MOMOTARO_LOOP)
    /*
     * Momotaro
     *
     * Music by Teiichi Okano
     *
     */
    // CH A
    "T107$B60" "V14L4O5" "$E1$A0$H40$D100$S65$F500"
    ">C8.C16<G4E8.E16A8.A16G8.G16E8.D16C4>C4<"
    "[0"
        "G4..A16G8.G16E4" "G8.G16E8.C16D2"
        "C8.C16D8.D16E8.E16D4" "E8.E16A8.A16G2"
        ">C8.C16<G4E8.E16A8.A16G8.G16E8.D16C2"
    "]"
    ","
    // CH B
    "T107$B60" "L4O3 S0M4000" "C8.C16G8.G16" "E8.E16F8.F16" "G8.G16E8.D16" "C8.<G16>C4"
    "[0"
        "C8.G16<G8.>G16" "C8.G16<G8.>G16" "C8.G16<G8.>G16" "<G8.D16G8.>D16"
        "C8.C16<G8.G16>" "C8.C16<G8.G16>" "C8.C16<F8.F16" "G8.G16A8.B16"
        ">C8.C16G8.G16" "E8.E16F8.F16" "G8.G16E8.D16" "C8.<G16>C4"
    "]"
    ","
    // CH C
    "T107$B60" "V14L4O5" "$E1$A0$H40$D100$S65$F500"
    "G8.G16E4C8.C16F8.F16E8.E16C8.<B16G4>G4"
    "[0"
        "E4..F16E8.E16C4" "E8.E16C8.<G16B8.G16B8.>G16"
        "<G8.G16B8.B16>C8.C16<B4" ">C8.C16F8.F16E8.>G16E8.D16<"
        "G8.G16E4C8.C16F8.F16E8.E16C8.<B16G2>"
    "]"
#else
    /*
     * Funiculì funiculà
     *
     * Music by Luigi Denza
     *
     */
    // CH A
    "$B30T136"
    "[2"
        "V15L4O5" "$M1$J12$L90$T4" "$E1$A0$H80$D100$S85$F0"
        "Q7"
        "[2 BA8R8BA8R8 >C8.<B16A8.>C16 | <B2] <G8."
        "[4 E16E8R16E16 | E8.] Q8>C2"
        "Q7D8.C16<B8.A16G8."
        "E16E8.F16G8.F16E8."
    "|" "D16Q8C2"
    "]" "D16C4$F500>C4"
    ","
    // CH B
    "$B30T136"
    "[2"
        "L4O3 S0M5000I0"
        "[7" "G8.G16M2000H8.M5000D16]"
        "C8.C16M2000H8.M5000C16<"
        "B8.B16M2000H8.M5000B16"
        "A8.A16M2000H8.M5000A16"
        "G#8.G#16M2000H8.M5000G#16"
        "A8.A16M2000H8.M5000A16"
        "F8.F16M2000H8.M5000F16"
        "G8.G16M2000H8.M5000G16"
        "G8.G16M2000H8.M5000G16"
    "|" "C8.C16M2000H8.M4000C16"
    "]" "C8.C16>C4"
    ","
    // CH C
    "$B30T136"
    "[2"
        "V15L4O5" "$M1$J12$L100$T4"
        "$E1$A0$H80$D100$S85$F0"
        "Q7"
        "GF8R8GF8R8 A8.G16F8.A16G2"
        "GF8R8GF8R8 A8.G16F8.A16E8."
        "<G16G8R16G16G#8."
        "G#16G#8R16G#16A8."
        "A16A8R16A16B8."
        "B16B8R16B16>E2"
        "Q7F8.E16D8.C16<B8."
        "G16G8.A16B8.A16G8."
    "|" "F16Q8E2"
    "]" "F16E4>$F500E4"
#endif
;

constexpr uint32_t PSG_EMU_CLOCK = 2000000; // 2 MHz
constexpr uint32_t PSG_RATE = 30000;        // 30 kHz
constexpr uint16_t MML_PROC_RATE = 400;     // 400 Hz
constexpr unsigned long EXEC_CYCLE_MML = (1000*1000uL)/MML_PROC_RATE; // unit: usec.

unsigned long time0_mml;
FspTimer fsp_timer;
PSG *psg;            // PSG emulator
Psgino psgino;       // MML decoder for PSGs

void cb_timer_overflow_irq(void) {

    if ( psg ) {
        int16_t output;
        output = PSG_calc(psg)>>4;
        analogWrite(DAC, output);
    }

    R_BSP_IrqStatusClear(fsp_timer.get_cfg()->cycle_end_irq);
}

void psg_write(uint8_t addr, uint8_t data) {

    __disable_irq();
    PSG_writeReg(psg, addr, data);
    __enable_irq();
}

void setup() {

    psg = PSG_new(PSG_EMU_CLOCK, PSG_RATE);
    PSG_reset(psg);

    psgino.Initialize(psg_write, PSG_EMU_CLOCK, MML_PROC_RATE);
    psgino.SetMML(mml);
    psgino.Play();

    fsp_timer.begin(TIMER_MODE_PERIODIC, GPT_TIMER, 0, static_cast<float>(PSG_RATE), 50.0F);
    fsp_timer.setup_overflow_irq(12, cb_timer_overflow_irq);
    fsp_timer.enable_overflow_irq();
    fsp_timer.open();
    fsp_timer.start();

    analogWriteResolution(12);

    // Stabilization wait
    delay(500);

    // Record the start time for measurement
    time0_mml = micros();
}

void loop() {

    unsigned long time_now = micros();

    if ( (time_now - time0_mml) >= EXEC_CYCLE_MML ) {
        time0_mml = time_now;
        psgino.Proc();
    }
}
