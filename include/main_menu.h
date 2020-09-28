#ifndef GUARD_MAIN_MENU_H
#define GUARD_MAIN_MENU_H

void CB2_InitMainMenu(void);
void CreateYesNoMenuParameterized(u8 a, u8 b, u16 c, u16 d, u8 e, u8 f);
void Task_NewGameBirchSpeech_AndYouAre(u8);
void Task_NewGameBirchSpeech_StartBirchLotadPlatformFade(u8);
void Task_NewGameBirchSpeech_ChooseGender(u8 taskId);
void Task_NewGameBirchSpeech_WaitToShowGenderMenu(u8);
void Task_NewGameBirchSpeech_StartPlayerFadeIn(u8);
void InitTextWindow();

#endif // GUARD_MAIN_MENU_H
