#ifndef MENU_DATA_H
#define MENU_DATA_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The Menu*Data structures are all the data a menu needs to track.  It is not possible to just put this data
// in a global because there can be multiple instances of each menu (for example you could have 2 sound modules
// connected and each of those would have a sound menu with seperate data.  So in the g_ctx an array of the
// of all the Menu*Data is stored.  The g_ctx stores data for each module connected.  To know how much memory
// needed to store the Menu*Data for each module there is union of all the Menu*Data to fine the worst case size.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Test Menu
extern const char* menuTest_Name();
extern void menuTest_MenuInit();
extern void menuTest_PhotoInit();
extern void menuTest_MenuRun();
extern void menuTest_PhotoRun();

// Sound Menu
extern const char* menuSound_Name();
extern void menuSound_MenuInit();
extern void menuSound_PhotoInit();
extern void menuSound_MenuRun();
extern void menuSound_PhotoRun();

// Vibration Menu
extern const char* menuVibration_Name();
extern void menuVibration_MenuInit();
extern void menuVibration_PhotoInit();
extern void menuVibration_MenuRun();
extern void menuVibration_PhotoRun();

// Lightning Menu
extern const char* menuLightning_Name();
extern void menuLightning_MenuInit();
extern void menuLightning_PhotoInit();
extern void menuLightning_MenuRun();
extern void menuLightning_PhotoRun();

// Add your new function declarations above 
//  Note - These declarations are needed because these go in the proc table and the Arduino IDE doesn't do it's 
//         declaration magic on header files.

#endif // MENU_DATA_H

