#ifndef MENU_DATA_H
#define MENU_DATA_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The Menu*Data structures are all the data a menu needs to track.  It is not possible to just put this data
// in a global because there can be multiple instances of each menu (for example you could have 2 sound modules
// connected and each of those would have a sound menu with seperate data.  So in the g_ctx an array of the
// of all the Menu*Data is stored.  The g_ctx stores data for each module connected.  To know how much memory
// needed to store the Menu*Data for each module there is union of all the Menu*Data to fine the worst case size.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Sound Menu
extern void MenuSound_Info();
extern void MenuSound_MenuInit();
extern void MenuSound_PhotoInit();
extern void MenuSound_MenuRun();
extern void MenuSound_PhotoRun();

// Test Menu
extern void MenuTest_Info();
extern void MenuTest_MenuInit();
extern void MenuTest_PhotoInit();
extern void MenuTest_MenuRun();
extern void MenuTest_PhotoRun();

// Add your new function declarations above 
//  Note - These declarations are needed because these go in the proc table and the Arduino IDE doesn't do it's 
//         declaration magic on header files.

#define MENU_INIT_MODE  0
#define MENU_RUN_MODE   1
#define PHOTO_INIT_MODE 2
#define PHOTO_RUN_MODE  3


#endif // MENU_DATA_H

