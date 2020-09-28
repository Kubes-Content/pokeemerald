#include "aa_kubes_main_menu.h"
#include "main_menu.h"
#include "strings.h"
#include "task.h"

void Kubes_Task_NewGame(u8 taskId) {
    InitTextWindow(gText_Birch_BoyOrGirl);
    gTasks[taskId].func = Task_NewGameBirchSpeech_WaitToShowGenderMenu  ;
}
