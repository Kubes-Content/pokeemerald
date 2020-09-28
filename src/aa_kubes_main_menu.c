#include "aa_kubes_main_menu.h"
#include "main_menu.h"
#include "strings.h"
#include "task.h"
#include "gpu_regs.h"

void Kubes_Task_NewGame(u8 taskId) {
    InitTextWindow();
    gTasks[taskId].data[5] = TRUE;  // required for Task_NewGameBirchSpeech_StartPlayerFadeIn to run
    gTasks[taskId].data[7] = FALSE; // zero timer
    SetGpuReg(0x14, -60); // reposition paltform/spotlight
    gTasks[taskId].func = Task_NewGameBirchSpeech_StartPlayerFadeIn;
}
