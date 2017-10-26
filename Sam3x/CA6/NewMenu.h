#ifndef NEW_MENUS_H
#define NEW_MENUS_H

#include <CASensorFilter.h>
#include "Context.h"
#include "PacketProcessor.h"

/*
 Here are the steps to add a new menu.
 1) Add a new menu to the webpage
   A) Start with an example at CameraAxe6/Esp8266/data/menus and modify it to be what you want
   B) The name of the file is the name that will show up in the menu
 2) Add menu logic to this file
   A) Take a menu from Menus.h, copy it here, and modify it to fit your needs
   B) A pretty good example to start with is Sound Menu
   C) The string returned by *_Name must match the file name used for the menu in part 1
   D) Update the names of the functions
   E) Update the logic in the menu
 3) Add the function names you added to CtxProcTable in Context.h
 4) Add the function names you added to MenuData.h
 5) Once the code is fully working and ready to check in move code from this file to Menus.h
 6) Create a pull request on github if you'd like this to be part of the official CA6 releases
*/

#endif //NEW_MENUS_H

