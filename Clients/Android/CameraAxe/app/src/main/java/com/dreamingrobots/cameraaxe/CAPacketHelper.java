package com.dreamingrobots.cameraaxe;

import android.util.Log;

/**
 * CAPacketHelper class makes processing incoming packets and sending them easier
 */

public class CAPacketHelper {
    private static final int mDataSize = 2048;
    private byte[] mData = new byte[mDataSize];
    private CAPacket mPacker = new CAPacket(CAPacket.STATE_PACKER, mData, mDataSize);

    public byte[] getData() {
        return mData;
    }

    public CAPacket.PacketElement processIncomingPacket(byte[] buf, int bufSize) {
        CAPacket unpacker = new CAPacket(CAPacket.STATE_UNPACKER, buf, bufSize);
        int packetSize = unpacker.unpackSize();
        short packetType = unpacker.unpackType();
        CAPacket.PacketElement ret;

        switch (packetType) {
            case CAPacket.PID_MENU_HEADER: {
                CAPacket.MenuHeader unpack = unpacker.new MenuHeader();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_TEXT_STATIC: {
                CAPacket.TextStatic unpack = unpacker.new TextStatic();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_TEXT_DYNAMIC: {
                CAPacket.TextDynamic unpack = unpacker.new TextDynamic();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_BUTTON: {
                CAPacket.Button unpack = unpacker.new Button();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_CHECK_BOX: {
                CAPacket.CheckBox unpack = unpacker.new CheckBox();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_DROP_SELECT: {
                CAPacket.DropSelect unpack = unpacker.new DropSelect();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_EDIT_NUMBER: {
                CAPacket.EditNumber unpack = unpacker.new EditNumber();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_TIME_BOX: {
                CAPacket.TimeBox unpack = unpacker.new TimeBox();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_SCRIPT_END: {
                CAPacket.ScriptEnd unpack = unpacker.new ScriptEnd();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_MENU_SELECT: {
                CAPacket.MenuSelect unpack = unpacker.new MenuSelect();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_MENU_LIST: {
                CAPacket.MenuList unpack = unpacker.new MenuList();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_MODULE_LIST: {
                CAPacket.ModuleList unpack = unpacker.new ModuleList();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_LOGGER: {
                CAPacket.Logger unpack = unpacker.new Logger();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_CAM_STATE: {
                CAPacket.CamState unpack = unpacker.new CamState();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_CAM_SETTINGS: {
                CAPacket.CamSettings unpack = unpacker.new CamSettings();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_INTERVALOMETER: {
                CAPacket.Intervalometer unpack = unpacker.new Intervalometer();
                unpack.unpack();
                ret = unpack;
            } break;
            case CAPacket.PID_CONTROL_FLAGS: {
                CAPacket.ControlFlags unpack = unpacker.new ControlFlags();
                unpack.unpack();
                ret = unpack;
            } break;
            default: {
                Log.e("CA6", "Invalid incoming packet");
                ret = null;
            } break;
        }

        return ret;
    }

    public int writePacketMenuSelect(int mode, int menuNumber) {
        CAPacket.MenuSelect pack0 = mPacker.new MenuSelect();
        pack0.set(mode, menuNumber);
        int packSize = pack0.pack();
        mPacker.resetBuffer();
        return packSize;
    }

    public int writePacketMenuList() {
        CAPacket.MenuList pack0 = mPacker.new MenuList();
        pack0.set(0,0,0,0,0,0,0,0,0,0,0,0,0,"0"); // Contents don't matter this just triggers a reply
        int packSize = pack0.pack();
        mPacker.resetBuffer();
        return packSize;
    }

    public void testPackets() {
        CAPacket pack0 = new CAPacket(CAPacket.STATE_PACKER, mData, mDataSize);
        CAPacket unpack0 = new CAPacket(CAPacket.STATE_UNPACKER, mData, mDataSize);

        {   // MenuHeader Packet Test
            CAPacket.MenuHeader pack1 = pack0.new MenuHeader();                 // Update per type
            CAPacket.MenuHeader unpack1 = unpack0.new MenuHeader();             // Update per type
            pack1.set(2, 3, "hello world");                                     // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_MENU_HEADER ||                     // Update per type
                    unpack1.getMajorVersion() != 2 ||
                    unpack1.getMinorVersion() != 3 ||
                    unpack1.getMenuName().equals("hello world") != true) {
                Log.e("CA6", "Packet Test Error - MENU_HEADER test failed");
            }
        }
        {   // TextStatic Packet Test
            CAPacket.TextStatic pack1 = pack0.new TextStatic();                 // Update per type
            CAPacket.TextStatic unpack1 = unpack0.new TextStatic();             // Update per type
            pack1.set("static");                                                // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_TEXT_STATIC ||                     // Update per type
                    unpack1.getText0().equals("static") != true) {
                Log.e("CA6", "Packet Test Error - TEXT_STATIC test failed");
            }
        }
        {   // TextDynamic Packet Test
            CAPacket.TextDynamic pack1 = pack0.new TextDynamic();               // Update per type
            CAPacket.TextDynamic unpack1 = unpack0.new TextDynamic();           // Update per type
            pack1.set(245, 1, "Dynamic", "123");                                // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_TEXT_DYNAMIC ||                    // Update per type
                    unpack1.getClientHostId() != 245 ||
                    unpack1.getModAttribute() != 1 ||
                    unpack1.getText0().equals("Dynamic") != true ||
                    unpack1.getText1().equals("123") != true) {
                Log.e("CA6", "Packet Test Error - TEXT_DYNAMIC test failed");
            }
        }
        {   // Button Packet Test
            CAPacket.Button pack1 = pack0.new Button();                         // Update per type
            CAPacket.Button unpack1 = unpack0.new Button();                     // Update per type
            pack1.set(23, 0, 1, 1, "Starter1", "Button1");                      // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_BUTTON ||                          // Update per type
                    unpack1.getClientHostId() != 23 ||
                    unpack1.getModAttribute() != 0 ||
                    unpack1.getType() != 1 ||
                    unpack1.getValue() != 1 ||
                    unpack1.getText0().equals("Starter1") != true ||
                    unpack1.getText1().equals("Button1") != true) {
                Log.e("CA6", "Packet Test Error - BUTTON test failed");
            }
        }
        {   // CheckBox Packet Test
            CAPacket.CheckBox pack1 = pack0.new CheckBox();                     // Update per type
            CAPacket.CheckBox unpack1 = unpack0.new CheckBox();                 // Update per type
            pack1.set(234, 2, 1, "text123");                                    // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_CHECK_BOX ||                       // Update per type
                    unpack1.getClientHostId() != 234 ||
                    unpack1.getModAttribute() != 2 ||
                    unpack1.getValue() != 1 ||
                    unpack1.getText0().equals("text123") != true) {
                Log.e("CA6", "Packet Test Error - CHECK_BOX test failed");
            }
        }
        {   // DropSelect Packet Test
            CAPacket.DropSelect pack1 = pack0.new DropSelect();                 // Update per type
            CAPacket.DropSelect unpack1 = unpack0.new DropSelect();             // Update per type
            pack1.set(7, 0, 1, "abc", "no|yes");                                // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_DROP_SELECT ||                     // Update per type
                    unpack1.getClientHostId() != 7 ||
                    unpack1.getModAttribute() != 0 ||
                    unpack1.getValue() != 1 ||
                    unpack1.getText0().equals("abc") != true ||
                    unpack1.getText1().equals("no|yes") != true) {
                Log.e("CA6", "Packet Test Error - DROP_SELECT test failed");
            }
        }
        {   // EditNumber Packet Test
            CAPacket.EditNumber pack1 = pack0.new EditNumber();                 // Update per type
            CAPacket.EditNumber unpack1 = unpack0.new EditNumber();             // Update per type
            pack1.set(32, 0, 2, 6, 0, 99999999, 12345678, "xyz");               // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_EDIT_NUMBER ||                     // Update per type
                    unpack1.getClientHostId() != 32 ||
                    unpack1.getModAttribute() != 0 ||
                    unpack1.getDigitsBeforeDecimal() != 2 ||
                    unpack1.getDigitsAfterDecimal() != 6 ||
                    unpack1.getMinValue() != 0 ||
                    unpack1.getMaxValue() != 99999999 ||
                    unpack1.getValue() != 12345678 ||
                    unpack1.getText0().equals("xyz") != true) {
                Log.e("CA6", "Packet Test Error - EDIT_NUMBER test failed");
            }
        }
        {   // TimeBox Packet Test
            CAPacket.TimeBox pack1 = pack0.new TimeBox();                       // Update per type
            CAPacket.TimeBox unpack1 = unpack0.new TimeBox();                   // Update per type
            pack1.set(17, 1, 0x3F, 999, 59, 58, 998, 997, 996, "blah");         // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_TIME_BOX ||                        // Update per type
                    unpack1.getClientHostId() != 17 ||
                    unpack1.getModAttribute() != 1 ||
                    unpack1.getEnableMask() != 0x3F ||
                    unpack1.getHours() != 999 ||
                    unpack1.getMinutes() != 59 ||
                    unpack1.getSeconds() != 58 ||
                    unpack1.getMilliseconds() != 998 ||
                    unpack1.getMicroseconds() != 997 ||
                    unpack1.getNanoseconds() != 996 ||
                    unpack1.getText0().equals("blah") != true) {
                Log.e("CA6", "Packet Test Error - TIME_BOX test failed");
            }
        }
        {   // ScriptEnd Packet Test
            CAPacket.ScriptEnd pack1 = pack0.new ScriptEnd();                   // Update per type
            CAPacket.ScriptEnd unpack1 = unpack0.new ScriptEnd();               // Update per type
            //pack1.set();                                                      // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            //unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_SCRIPT_END) {                     // Update per type
                Log.e("CA6", "Packet Test Error - SCRIPT_END test failed");
            }
        }
        {   // MenuSelect Packet Test
            CAPacket.MenuSelect pack1 = pack0.new MenuSelect();                 // Update per type
            CAPacket.MenuSelect unpack1 = unpack0.new MenuSelect();             // Update per type
            pack1.set(1, 44);                                                   // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_MENU_SELECT ||                     // Update per type
                    unpack1.getMode() != 1 ||
                    unpack1.getMenuNumber() != 44) {
                Log.e("CA6", "Packet Test Error - MENU_SELECT test failed");
            }
        }
        {   // MenuList Packet Test
            CAPacket.MenuList pack1 = pack0.new MenuList();                     // Update per type
            CAPacket.MenuList unpack1 = unpack0.new MenuList();                 // Update per type
            pack1.set(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, "mName");      // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_MENU_LIST ||                       // Update per type
                    unpack1.getMenuId() != 1 ||
                    unpack1.getModuleId0() != 2 ||
                    unpack1.getModuleMask0() != 3 ||
                    unpack1.getModuleId1() != 4 ||
                    unpack1.getModuleMask1() != 5 ||
                    unpack1.getModuleId2() != 6 ||
                    unpack1.getModuleMask2() != 7 ||
                    unpack1.getModuleId3() != 8 ||
                    unpack1.getModuleMask3() != 9 ||
                    unpack1.getModuleTypeId0() != 10 ||
                    unpack1.getModuleTypeMask0() != 11 ||
                    unpack1.getModuleTypeId1() != 12 ||
                    unpack1.getModuleTypeMask1() != 13 ||
                    unpack1.getMenuName().equals("mName") != true) {
                Log.e("CA6", "Packet Test Error - MENU_LIST test failed");
            }
        }
        {   // ModuleList Packet Test
            CAPacket.ModuleList pack1 = pack0.new ModuleList();                 // Update per type
            CAPacket.ModuleList unpack1 = unpack0.new ModuleList();             // Update per type
            pack1.set(244, 254, "module12");                                    // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_MODULE_LIST ||                     // Update per type
                    unpack1.getModuleId() != 244 ||
                    unpack1.getModuleTypeId() != 254 ||
                    unpack1.getModuleName().equals("module12") != true) {
                Log.e("CA6", "Packet Test Error - MODULE_LIST test failed");
            }
        }
        {   // Logger Packet Test
            CAPacket.Logger pack1 = pack0.new Logger();                         // Update per type
            CAPacket.Logger unpack1 = unpack0.new Logger();                     // Update per type
            pack1.set("Log123");                                                // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_LOGGER ||                          // Update per type
                    unpack1.getLog().equals("Log123") != true) {
                Log.e("CA6", "Packet Test Error - LOG test failed");
            }
        }
        {   // CamState Packet Test
            CAPacket.CamState pack1 = pack0.new CamState();                     // Update per type
            CAPacket.CamState unpack1 = unpack0.new CamState();                 // Update per type
            pack1.set(3, 0xbe, 0xef);                                           // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_CAM_STATE ||                       // Update per type
                    unpack1.getMultiplier() != 3 ||
                    unpack1.getFocus() != 0xbe ||
                    unpack1.getShutter() != 0xef) {
                Log.e("CA6", "Packet Test Error - CAM_STATE test failed");
            }
        }
        {   // CamSettings Packet Test
            CAPacket.CamSettings pack1 = pack0.new CamSettings();               // Update per type
            CAPacket.CamSettings unpack1 = unpack0.new CamSettings();           // Update per type
            pack1.set(50, 1, 999, 59, 58, 998, 997, 996, 57, 56, 995, 994,
                    0xbe, 1, 5, 1, 40, 41, 900);                            // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_CAM_SETTINGS ||                    // Update per type
                    unpack1.getCamPortNumber() != 50 ||
                    unpack1.getMode() != 1 ||
                    unpack1.getDelayHours() != 999 ||
                    unpack1.getDelayMinutes() != 59 ||
                    unpack1.getDelaySeconds() != 58 ||
                    unpack1.getDelayMilliseconds() != 998 ||
                    unpack1.getDelayMicroseconds() != 997 ||
                    unpack1.getDurationHours() != 996 ||
                    unpack1.getDurationMinutes() != 57 ||
                    unpack1.getDurationSeconds() != 56 ||
                    unpack1.getDurationMilliseconds() != 995 ||
                    unpack1.getDurationMicroseconds() != 994 ||
                    unpack1.getSequencer() != 0xbe ||
                    unpack1.getApplyIntervalometer() != 1 ||
                    unpack1.getSmartPreview() != 5 ||
                    unpack1.getMirrorLockupEnable() != 1 ||
                    unpack1.getMirrorLockupMinutes() != 40 ||
                    unpack1.getMirrorLockupSeconds() != 41 ||
                    unpack1.getMirrorLockupMilliseconds() != 900) {
                Log.e("CA6", "Packet Test Error - CAM_SETTINGS test failed");
            }
        }
        {   // Intervalometer Packet Test
            CAPacket.Intervalometer pack1 = pack0.new Intervalometer();     // Update per type
            CAPacket.Intervalometer unpack1 = unpack0.new Intervalometer(); // Update per type
            pack1.set(900, 50, 51, 901, 902, 903, 52, 53, 904, 905, 9999);  // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_INTERVALOMETER ||              // Update per type
                    unpack1.getStartHours() != 900 ||
                    unpack1.getStartMinutes() != 50 ||
                    unpack1.getStartSeconds() != 51 ||
                    unpack1.getStartMilliseconds() != 901 ||
                    unpack1.getStartMicroseconds() != 902 ||
                    unpack1.getIntervalHours() != 903 ||
                    unpack1.getIntervalMinutes() != 52 ||
                    unpack1.getIntervalSeconds() != 53 ||
                    unpack1.getIntervalMilliseconds() != 904 ||
                    unpack1.getIntervalMicroseconds() != 905 ||
                    unpack1.getRepeats() != 9999) {
                Log.e("CA6", "Packet Test Error - INTERVALOMETER test failed");
            }
        }
        {   // ControlFlags Packet Test
            CAPacket.ControlFlags pack1 = pack0.new ControlFlags();             // Update per type
            CAPacket.ControlFlags unpack1 = unpack0.new ControlFlags();         // Update per type
            pack1.set(1, 1);                                                    // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_CONTROL_FLAGS ||                   // Update per type
                    unpack1.getSlaveModeEnable() != 1 ||
                    unpack1.getExtraMessagesEnable() != 1) {
                Log.e("CA6", "Packet Test Error - CONTROL_FLAGS test failed");
            }
        }
    }
}
