package com.dreamingrobots.cameraaxe;

/**
 *  Container class for MENU_LIST packet data
 */

public class MenuName {
    private int mMenuId;
    private int[] mModuleId = new int[4];
    private int[] mModuleMask = new int[4];
    private int[] mModuleTypeId = new int[2];
    private int[] mModuleTypeMask = new int[2];
    String mMenuName;

    public MenuName (int menuId, int moduleId0, int moduleMask0, int moduleId1, int moduleMask1,
                     int moduleId2, int moduleMask2, int moduleId3, int moduleMask3,
                     int moduleTypeId0, int moduleTypeMask0, int moduleTypeId1, int moduleTypeMask1, String menuName) {
        mMenuId = menuId;
        mModuleId[0] = moduleId0;
        mModuleMask[0] = moduleMask0;
        mModuleId[1] = moduleId1;
        mModuleMask[1] = moduleMask1;
        mModuleId[2] = moduleId2;
        mModuleMask[2] = moduleMask2;
        mModuleId[3] = moduleId3;
        mModuleMask[3] = moduleMask3;
        mModuleTypeId[0] = moduleTypeId0;
        mModuleTypeMask[0] = moduleTypeMask0;
        mModuleTypeId[1] = moduleTypeId1;
        mModuleTypeMask[1] = moduleTypeMask1;
        mMenuName = menuName;
    }

    public int getMenuId() { return mMenuId; }
    public int getModuleId(int index) { return mModuleId[index]; }
    public int getModuleMask(int index) { return mModuleMask[index]; }
    public int getModuleTypeId(int index) { return mModuleTypeId[index]; }
    public int getModuleTypeMask(int index) { return mModuleTypeMask[index]; }
    public String getMenuName() { return mMenuName; }
}

