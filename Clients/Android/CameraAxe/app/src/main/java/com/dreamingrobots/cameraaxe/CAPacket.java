package com.dreamingrobots.cameraaxe;

import android.util.Log;

import java.nio.charset.Charset;

import static android.R.attr.mode;

/**
 * CAPacket is the base class that manages packet construction
 * PacketElement is an interface for all the different packet types.  Each packet type has it's
 *  own class
 */
class CAPacket {
    static final int STATE_PACKER = 1;
    static final int STATE_UNPACKER = 2;

    private static final int PID_START_SENTINEL     = 0;    // Must be first
    static final int PID_MENU_HEADER                = 1;
    static final int PID_TEXT_STATIC                = 2;
    static final int PID_TEXT_DYNAMIC               = 3;
    static final int PID_BUTTON                     = 4;
    static final int PID_CHECK_BOX                  = 5;
    static final int PID_DROP_SELECT                = 6;
    static final int PID_EDIT_NUMBER                = 7;
    static final int PID_TIME_BOX                   = 8;
    static final int PID_SCRIPT_END                 = 9;   // Must be last of menu based packets
    static final int PID_MENU_SELECT                = 10;
    static final int PID_MENU_LIST                  = 11;
    static final int PID_MODULE_LIST                = 12;
    static final int PID_LOGGER                     = 13;
    static final int PID_CAM_STATE                  = 14;
    static final int PID_CAM_SETTINGS               = 15;
    static final int PID_INTERVALOMETER             = 16;
    static final int PID_CONTROL_FLAGS              = 17;
    private static final int PID_END_SENTINEL       = 18;   // Must be last

    private int mBitsUsed;  // Bits in use that didn't fill a full byte
    private int mBitsVal;   // Value of bits that didn't fill a full byte
    private int mBytesUsed; // Total full bytes used by the current packer
    private int mState;
    private byte[] mBuf;
    private int mBufSize;

    CAPacket(int state, byte[] buf, int bufSize) {
        mState = state;
        mBuf = buf;
        mBufSize = bufSize;
    }

    // This was just added to make code more similar between embedded C++ and java code
    private void CA_ASSERT(boolean test, String str) {
        if (!test) {
            Log.e("CA6_ASSERT", str);
        }
    }

    void resetBuffer() {
        flushPacket();  // Check for errors
        mBitsUsed = 0;
        mBitsVal = 0;
        mBytesUsed = 0;
    }

    int unpackSize() {return (int) unpacker(16);}

    public short unpackType() {
        short val = (short) unpacker(8);
        if ((val <= PID_START_SENTINEL) || (val >= PID_END_SENTINEL)) {
            Log.e("CA6", "Error: Invalid packet type");
        }
        return val;
    }

    protected long unpacker(int unpackBits) {
        if (mState != STATE_UNPACKER) {
            Log.e("CA6", "Error in unpacker");
            return -1;
        }
        int unpackBitsLeft = unpackBits;
        long ret = 0;
        int valShift = 0;
        // This loop shifts through the number of bytes you want to unpack in the src buffer
        // and puts them into an unsigned integer
        do {
            int bitsInCurSrcByte = 8 - mBitsUsed;
            int bitsToUnpack = Math.min(unpackBitsLeft, bitsInCurSrcByte);
            int unusedLeftBits = (unpackBitsLeft >= bitsInCurSrcByte) ?
                    0 : (bitsInCurSrcByte - unpackBitsLeft);
            int rightShift = mBitsUsed + unusedLeftBits;
            int val = (mBuf[mBytesUsed] << unusedLeftBits) & 0xff;  // Zero out left bits
            val = val >> rightShift;      // Shift bits to right most position for this byte
            ret |= ((long)val) << valShift;
            valShift += bitsToUnpack;
            if (mBitsUsed + bitsToUnpack == 8)
            {
                mBitsUsed = 0;
                mBytesUsed++;
            }
            else {
                mBitsUsed += bitsToUnpack;
            }
            unpackBitsLeft -= bitsToUnpack;
        } while (unpackBitsLeft != 0);
        return ret;
    }

    protected void unpackerString(StringBuilder str) {
        if (mState != STATE_UNPACKER) {
            Log.e("CA6", "Error in unpackerString");
            return;
        }
        int len = 0;
        do {
            len++;
        } while (mBuf[mBytesUsed+len] != 0); // This do while loop includes null terminator
        str.setLength(0);
        str.append(new String(mBuf, mBytesUsed, len));
        mBytesUsed += len+1;  // +1 for the null terminator
    }

    protected void packer( long val, int packBits) {
        if (mState != STATE_PACKER) {
            Log.e("CA6", "Error in packer");
            return;
        }
        int packBitsLeft = packBits;
        do {
            int bitsInCurDstByte = 8 - mBitsUsed;
            int bitsToPack = Math.min(bitsInCurDstByte, packBitsLeft);
            mBitsVal |= (val << mBitsUsed);
            mBitsUsed += bitsToPack;
            val = val >> bitsToPack;
            if (mBitsUsed == 8) // When byte is full write it's value
            {
                mBuf[mBytesUsed++] = (byte)(mBitsVal & 0xFF);
                mBitsVal = 0;
                mBitsUsed = 0;
            }
            packBitsLeft -= bitsToPack;
        } while (packBitsLeft != 0);
    }

    protected void packerString(String src) {
        if (mState != STATE_PACKER) {
            Log.e("CA6", "Error in packer");
            return;
        }
        byte[] b = src.getBytes(Charset.forName("UTF-8"));
        for(int val=0; val<src.length(); val++) {
            mBuf[mBytesUsed++] = b[val];
        }
        mBuf[mBytesUsed++] = 0; // Add null terminator
    }

    protected void flushPacket() {
        if (mBitsUsed != 0 || mBitsVal != 0 || mBytesUsed > mBufSize) {
            Log.e("CA6", "Error detected during flush()");
        }
    }

    /***********************************************************************************************
    * The PacketElement interface allows all the different packet types allows for a visitor
     * design pattern
    **********************************************************************************************/
    public interface PacketElement {
        public int getPacketType();
        public int getClientHostId();
        public void unpack();
        public int pack();
    }

    /***********************************************************************************************
     * MenuHeader Packet Class
     **********************************************************************************************/
    public class MenuHeader implements PacketElement {

        private int mMajorVersion;
        private int mMinorVersion;
        private StringBuilder mMenuName;

        public MenuHeader() {
            mMenuName = new StringBuilder();
        }

        public int getPacketType() {return PID_MENU_HEADER;}
        public int getClientHostId() {return -1;}
        public int getMajorVersion() {return mMajorVersion;}
        public int getMinorVersion() {return mMinorVersion;}
        public String getMenuName() {return mMenuName.toString();}

        public void set(int majorVersion, int minorVersion, String menuName) {
            mMajorVersion = majorVersion;
            mMinorVersion = minorVersion;
            mMenuName.setLength(0);
            mMenuName.append(menuName);
        }

        public void unpack() {
            mMajorVersion = (int)unpacker(16);
            mMinorVersion = (int)unpacker(16);
            unpackerString(mMenuName);
            flushPacket();
        }

        public int pack() {
            int len = mMenuName.length() + 1;  // 1 for the null terminator
            int packetSize = 3 + 2 + len;
            packer(packetSize, 16);
            packer(PID_MENU_HEADER, 8);
            packer(mMajorVersion, 16);
            packer(mMinorVersion, 16);
            packerString(mMenuName.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * TextStatic Packet Class
     **********************************************************************************************/
    public class TextStatic implements PacketElement {

        private StringBuilder mText0;

        public TextStatic() {
            mText0 = new StringBuilder();
        }

        public int getPacketType() {return PID_TEXT_STATIC;}
        public int getClientHostId() {return -1;}
        public String getText0() {return mText0.toString();}

        public void set(String text0) {
            mText0.setLength(0);
            mText0.append(text0);
        }

        public void unpack() {
            unpackerString(mText0);
            flushPacket();
        }

        public int pack() {
            int len = mText0.length() + 1;  // 1 for the null terminator
            int packetSize = 3 + len;
            packer(packetSize, 16);
            packer(PID_TEXT_STATIC, 8);
            packerString(mText0.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * TextDynamic Packet Class
     **********************************************************************************************/
    public class TextDynamic implements PacketElement {

        private int mClientHostId;
        private int mModAttribute;
        private StringBuilder mText0;
        private StringBuilder mText1;

        public TextDynamic(){
            mText0 = new StringBuilder();
            mText1 = new StringBuilder();
        }

        public int getPacketType() {return PID_TEXT_DYNAMIC;}
        public int getClientHostId() {return mClientHostId;}
        public int getModAttribute() {return mModAttribute;}
        public String getText0() {return mText0.toString();}
        public String getText1() {return mText1.toString();}

        public void set(int clientHostId, int modAttribute, String text0, String text1) {
            mClientHostId = clientHostId;
            mModAttribute = modAttribute;
            mText0.setLength(0);
            mText0.append(text0);
            mText1.setLength(0);
            mText1.append(text1);
            CA_ASSERT(mModAttribute <= 2,
                    "Error in CAPacketTextDynamic::set()");
        }

        public void unpack() {
            mClientHostId = (int)unpacker(8);
            mModAttribute = (int)unpacker(8);
            unpackerString(mText0);
            unpackerString(mText1);
            flushPacket();
            CA_ASSERT(mModAttribute <= 2,
                    "Error in CAPacketTextDynamic::unpack()");
        }

        public int pack() {
            int len = mText0.length()+1+mText1.length()+1;  // 1 for the null terminator
            int packetSize = 3 + 2 + len;
            packer(packetSize, 16);
            packer(PID_TEXT_DYNAMIC, 8);
            packer(mClientHostId, 8);
            packer(mModAttribute, 8);
            packerString(mText0.toString());
            packerString(mText1.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * Button Packet Class
     **********************************************************************************************/
    public class Button implements PacketElement {

        private int mClientHostId;
        private int mModAttribute;
        private int mType;
        private int mValue;
        private StringBuilder mText0;
        private StringBuilder mText1;

        public Button() {
            mText0 = new StringBuilder();
            mText1 = new StringBuilder();
        }

        public int getPacketType() {return PID_BUTTON;}
        public int getClientHostId() {return mClientHostId;}
        public int getModAttribute() {return mModAttribute;}
        public int getType() {return mType;}
        public int getValue() {return mValue;}
        public String getText0() {return mText0.toString();}
        public String getText1() {return mText1.toString();}

        public void set(int clientHostId, int modAttribute, int type, int value, String text0,
                        String text1) {
            mClientHostId = clientHostId;
            mModAttribute = modAttribute;
            mType = type;
            mValue = value;
            mText0.setLength(0);
            mText1.setLength(0);
            mText0.append(text0);
            mText1.append(text1);
            CA_ASSERT((mType <= 1) && (mValue <= 1) && (mModAttribute <= 2),
                    "Error in CAPacketButton::set()");
        }

        public void unpack() {
            mClientHostId = (int)unpacker(8);
            mModAttribute = (int)unpacker(8);
            mType = (int)unpacker(4);
            mValue = (int)unpacker(4);
            unpackerString(mText0);
            unpackerString(mText1);
            flushPacket();
            CA_ASSERT((mType <= 1) && (mValue <= 1) && (mModAttribute <= 2),
                    "Error in CAPacketButton::unpack()");
        }

        public int pack() {
            int len = mText0.length()+1+mText1.length()+1;  // 1 for the null terminator
            int packetSize = 3 + 3 + len;
            packer(packetSize, 16);
            packer(PID_BUTTON, 8);
            packer(mClientHostId, 8);
            packer(mModAttribute, 8);
            packer(mType, 4);
            packer(mValue, 4);
            packerString(mText0.toString());
            packerString(mText1.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * CheckBox Packet Class
     **********************************************************************************************/
    public class CheckBox implements PacketElement {

        private int mClientHostId;
        private int mModAttribute;
        private int mValue;
        private StringBuilder mText0;

        public CheckBox() {mText0 = new StringBuilder();}

        public int getPacketType() {return PID_CHECK_BOX;}
        public int getClientHostId() {return mClientHostId;}
        public int getModAttribute() {return mModAttribute;}
        public int getValue() {return mValue;}
        public String getText0() {return mText0.toString();}

        public void set(int clientHostId, int modAttribute, int value, String text0) {
            mClientHostId = clientHostId;
            mModAttribute = modAttribute;
            mValue = value;
            mText0.setLength(0);
            mText0.append(text0);
            CA_ASSERT((mValue <= 1) && (mModAttribute <= 2), "Error in CAPacketCheckBox::set()");
        }

        public void unpack() {
            mClientHostId = (int)unpacker(8);
            mModAttribute = (int)unpacker(8);
            mValue = (int)unpacker(8);
            unpackerString(mText0);
            flushPacket();
            CA_ASSERT((mValue <= 1) && (mModAttribute <= 2), "Error in CAPacketCheckBox::unpack()");
        }

        public int pack() {
            int len = mText0.length() + 1;  // 1 for the null terminator
            int packetSize = 3 + 3 + len;
            packer(packetSize, 16);
            packer(PID_CHECK_BOX, 8);
            packer(mClientHostId, 8);
            packer(mModAttribute, 8);
            packer(mValue, 8);
            packerString(mText0.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * DropSelect Packet Class
     **********************************************************************************************/
    public class DropSelect implements PacketElement {

        private int mClientHostId;
        private int mModAttribute;
        private int mValue;
        private StringBuilder mText0;
        private StringBuilder mText1;

        public DropSelect() {
            mText0 = new StringBuilder();
            mText1 = new StringBuilder();
        }

        public int getPacketType() {return PID_DROP_SELECT;}
        public int getClientHostId() {return mClientHostId;}
        public int getModAttribute() {return mModAttribute;}
        public int getValue() {return mValue;}
        public String getText0() {return mText0.toString();}
        public String getText1() {return mText1.toString();}

        public void set(int clientHostId, int modAttribute, int value, String text0, String text1) {
            mClientHostId = clientHostId;
            mModAttribute = modAttribute;
            mValue = value;
            mText0.setLength(0);
            mText1.setLength(0);
            mText0.append(text0);
            mText1.append(text1);
            CA_ASSERT(mModAttribute <= 2,
                    "Error in CAPacketDropSelect::set()");

        }

        public void unpack() {
            mClientHostId = (int)unpacker(8);
            mModAttribute = (int)unpacker(8);
            mValue = (int)unpacker(8);
            unpackerString(mText0);
            unpackerString(mText1);
            flushPacket();
            CA_ASSERT(mModAttribute <= 2,
                    "Error in CAPacketDropSelect::unpack()");
        }

        public int pack() {
            int len = mText0.length()+1+mText1.length()+1;  // 1 for the null terminator
            int packetSize = 3 + 3 + len;
            packer(packetSize, 16);
            packer(PID_DROP_SELECT, 8);
            packer(mClientHostId, 8);
            packer(mModAttribute, 8);
            packer(mValue, 8);
            packerString(mText0.toString());
            packerString(mText1.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * EditNumber Packet Class
     **********************************************************************************************/
    public class EditNumber implements PacketElement {

        private int mClientHostId;
        private int mModAttribute;
        private int mDigitsBeforeDecimal;
        private int mDigitsAfterDecimal;
        private long mMinValue;
        private long mMaxValue;
        private long mValue;
        private StringBuilder mText0;

        public EditNumber() {mText0 = new StringBuilder();}

        public int getPacketType() {return PID_EDIT_NUMBER;}
        public int getClientHostId() {return mClientHostId;}
        public int getModAttribute() {return mModAttribute;}
        public int getDigitsBeforeDecimal() {return mDigitsBeforeDecimal;}
        public int getDigitsAfterDecimal() {return mDigitsAfterDecimal;}
        public long getMinValue() {return mMinValue;}
        public long getMaxValue() {return mMaxValue;}
        public long getValue() {return mValue;}
        public String getText0() {return mText0.toString();}

        public void set(int clientHostId, int modAttribute, int digitsBeforeDecimal,
                        int digitsAfterDecimal, long minValue, long maxValue, long value,
                        String text0) {
            mClientHostId = clientHostId;
            mModAttribute = modAttribute;
            mDigitsBeforeDecimal = digitsBeforeDecimal;
            mDigitsAfterDecimal = digitsAfterDecimal;
            mMinValue = minValue;
            mMaxValue = maxValue;
            mValue = value;
            mText0.setLength(0);
            mText0.append(text0);
            CA_ASSERT((mDigitsBeforeDecimal <= 8) && (mDigitsAfterDecimal <= 8) &&
                            (mDigitsBeforeDecimal+mDigitsAfterDecimal <= 8) &&
                            (mMinValue <= 99999999) && (mMaxValue <= 99999999) &&
                            (mModAttribute <= 2),
                    "Error in CAPacketEditNumber::set()");
        }

        public void unpack() {
            mClientHostId = (int)unpacker(8);
            mModAttribute = (int)unpacker(8);
            mDigitsBeforeDecimal = (int)unpacker(4);
            mDigitsAfterDecimal = (int)unpacker(4);
            mMinValue = (long)unpacker(32);
            mMaxValue = (long)unpacker(32);
            mValue = (long)unpacker(32);
            unpackerString(mText0);
            flushPacket();
            CA_ASSERT((mDigitsBeforeDecimal <= 8) && (mDigitsAfterDecimal <= 8) &&
                            (mDigitsBeforeDecimal+mDigitsAfterDecimal <= 8) &&
                            (mMinValue <= 99999999) && (mMaxValue <= 99999999) &&
                            (mModAttribute <= 2),
                    "Error in CAPacketEditNumber::unpack()");
        }

        public int pack() {
            int len = mText0.length() + 1;  // 1 for the null terminator
            int packetSize = 3 + 15 + len;
            packer(packetSize, 16);
            packer(PID_EDIT_NUMBER, 8);
            packer(mClientHostId, 8);
            packer(mModAttribute, 8);
            packer(mDigitsBeforeDecimal, 4);
            packer(mDigitsAfterDecimal, 4);
            packer(mMinValue, 32);
            packer(mMaxValue, 32);
            packer(mValue, 32);
            packerString(mText0.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * TimeBox Packet Class
     **********************************************************************************************/
    public class TimeBox implements PacketElement {

        private int mClientHostId;
        private int mModAttribute;
        private int mEnableMask;
        private int mHours;
        private int mMinutes;
        private int mSeconds;
        private int mMilliseconds;
        private int mMicroseconds;
        private int mNanoseconds;
        private StringBuilder mText0;

        public TimeBox() {mText0 = new StringBuilder();}

        public int getPacketType() {return PID_TIME_BOX;}
        public int getClientHostId() {return mClientHostId;}
        public int getModAttribute() {return mModAttribute;}
        public int getEnableMask() {return mEnableMask;}
        public int getHours() {return mHours;}
        public int getMinutes() {return mMinutes;}
        public int getSeconds() {return mSeconds;}
        public int getMilliseconds() {return mMilliseconds;}
        public int getMicroseconds() {return mMicroseconds;}
        public int getNanoseconds() {return mNanoseconds;}
        public String getText0() {return mText0.toString();}

        public void set(int clientHostId, int modAttribute, int enableMask, int hours, int minutes,
                        int seconds, int milliseconds, int microseconds, int nanoseconds,
                        String text0) {
            mClientHostId = clientHostId;
            mModAttribute = modAttribute;
            mEnableMask = enableMask;
            mHours = hours;
            mMinutes = minutes;
            mSeconds = seconds;
            mMilliseconds = milliseconds;
            mMicroseconds = microseconds;
            mNanoseconds = nanoseconds;
            mText0.setLength(0);
            mText0.append(text0);
            CA_ASSERT((mEnableMask <= 0x3F) && (mHours <= 999) && (mMinutes <= 59) &&
                    (mSeconds <=59) && (mMilliseconds <= 999) && (mMicroseconds <= 999) &&
                    (mNanoseconds <= 999) && (mModAttribute <= 2),
                    "Error in CAPacketTimeBox::set()");
        }

        public void unpack() {
            mClientHostId = (int)unpacker(8);
            mModAttribute = (int)unpacker(8);
            mEnableMask = (int)unpacker(6);
            mHours = (int)unpacker(10);
            mMinutes = (int)unpacker(6);
            mSeconds = (int)unpacker(6);
            mMilliseconds = (int)unpacker(10);
            mMicroseconds = (int)unpacker(10);
            mNanoseconds = (int)unpacker(10);
            unpacker(6); // Unused
            unpackerString(mText0);
            flushPacket();
            CA_ASSERT((mEnableMask <= 0x3F) && (mHours <= 999) && (mMinutes <= 59) &&
                    (mSeconds <=59) && (mMilliseconds <= 999) && (mMicroseconds <= 999) &&
                    (mNanoseconds <= 999) && (mModAttribute <= 2),
                    "Error in CAPacketTimeBox::set()");
        }

        public int pack() {
            int unused = 0;
            int len = mText0.length() + 1;  // 1 for the null terminator
            int packetSize = 3 + 10 + len;
            packer(packetSize, 16);
            packer(PID_TIME_BOX, 8);
            packer(mClientHostId, 8);
            packer(mModAttribute, 8);
            packer(mEnableMask, 6);
            packer(mHours, 10);
            packer(mMinutes, 6);
            packer(mSeconds, 6);
            packer(mMilliseconds, 10);
            packer(mMicroseconds, 10);
            packer(mNanoseconds, 10);
            packer(unused, 6);
            packerString(mText0.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * ScriptEnd Packet Class
     **********************************************************************************************/
    public class ScriptEnd implements PacketElement {

        public ScriptEnd() {}

        public int getPacketType() {return PID_SCRIPT_END;}
        public int getClientHostId() {return -1;}

        public void set() {}

        public void unpack() {}

        public int pack() {
            int packetSize = 3;
            packer(packetSize, 16);
            packer(PID_SCRIPT_END, 8);
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * MenuSelect Packet Class
     **********************************************************************************************/
    public class MenuSelect implements PacketElement {

        private int mMode;
        private int mMenuNumber;

        public MenuSelect() {}

        public int getPacketType() {return PID_MENU_SELECT;}
        public int getClientHostId() {return -1;}
        public int getMode() {return mMode;}
        public int getMenuNumber() {return mMenuNumber;}

        public void set(int mode, int menuNumber) {
            mMode = mode;
            mMenuNumber = menuNumber;
            CA_ASSERT((mMode <= 1), "Error in MenuSelect::set()");
        }

        public void unpack() {
            mMode = (int)unpacker(8);
            mMenuNumber = (int)unpacker(8);
            flushPacket();
            CA_ASSERT((mMode <= 1), "Error in MenuSelect::unpack()");
        }

        public int pack() {
            int packetSize = 3 + 2;
            packer(packetSize, 16);
            packer(PID_MENU_SELECT, 8);
            packer(mMode, 8);
            packer(mMenuNumber, 8);
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * MenuList Packet Class
     **********************************************************************************************/
    public class MenuList implements PacketElement {

        private int mMenuId;
        private int mModuleId0;
        private int mModuleMask0;
        private int mModuleId1;
        private int mModuleMask1;
        private int mModuleId2;
        private int mModuleMask2;
        private int mModuleId3;
        private int mModuleMask3;
        private int mModuleTypeId0;
        private int mModuleTypeMask0;
        private int mModuleTypeId1;
        private int mModuleTypeMask1;
        private StringBuilder mMenuName;

        public MenuList() {mMenuName = new StringBuilder();}

        public int getPacketType() {return PID_MENU_LIST;}
        public int getClientHostId() {return -1;}
        public int getMenuId() {return mMenuId;}
        public int getModuleId0() {return mModuleId0;}
        public int getModuleMask0() {return mModuleMask0;}
        public int getModuleId1() {return mModuleId1;}
        public int getModuleMask1() {return mModuleMask1;}
        public int getModuleId2() {return mModuleId2;}
        public int getModuleMask2() {return mModuleMask2;}
        public int getModuleId3() {return mModuleId3;}
        public int getModuleMask3() {return mModuleMask3;}
        public int getModuleTypeId0() {return mModuleTypeId0;}
        public int getModuleTypeMask0() {return mModuleTypeMask0;}
        public int getModuleTypeId1() {return mModuleTypeId1;}
        public int getModuleTypeMask1() {return mModuleTypeMask1;}
        public String getMenuName() {return mMenuName.toString();}

        public void set(int menuId, int moduleId0, int moduleMask0,  int moduleId1, int moduleMask1,
                        int moduleId2, int moduleMask2, int moduleId3, int moduleMask3,
                        int moduleTypeId0, int moduleTypeMask0, int moduleTypeId1, int moduleTypeMask1,
                        String menuName) {
            mMenuId = menuId;
            mModuleId0 = moduleId0;
            mModuleMask0 = moduleMask0;
            mModuleId1 = moduleId1;
            mModuleMask1 = moduleMask1;
            mModuleId2 = moduleId2;
            mModuleMask2 = moduleMask2;
            mModuleId3 = moduleId3;
            mModuleMask3 = moduleMask3;
            mModuleTypeId0 = moduleTypeId0;
            mModuleTypeMask0 = moduleTypeMask0;
            mModuleTypeId1 = moduleTypeId1;
            mModuleTypeMask1 = moduleTypeMask1;
            mMenuName.setLength(0);
            mMenuName.append(menuName);
            flushPacket();
            CA_ASSERT(mModuleMask0 <= 0xf && mModuleMask1 <= 0xf &&
                    mModuleMask2 <= 0xf && mModuleMask3 <= 0xf &&
                    mModuleTypeMask0 <= 0xf && mModuleTypeMask1 <= 0xf , "Error in MenuList::set()");
        }

        public void unpack() {
            mMenuId = (int)unpacker(8);
            mModuleId0 = (int)unpacker(8);
            mModuleMask0 = (int)unpacker(4);
            mModuleId1 = (int)unpacker(8);
            mModuleMask1 = (int)unpacker(4);
            mModuleId2 = (int)unpacker(8);
            mModuleMask2 = (int)unpacker(4);
            mModuleId3 = (int)unpacker(8);
            mModuleMask3 = (int)unpacker(4);
            mModuleTypeId0 = (int)unpacker(8);
            mModuleTypeMask0 = (int)unpacker(4);
            mModuleTypeId1 = (int)unpacker(8);
            mModuleTypeMask1 = (int)unpacker(4);
            unpackerString(mMenuName);
            CA_ASSERT(mModuleMask0 <= 0xf && mModuleMask1 <= 0xf &&
                    mModuleMask2 <= 0xf && mModuleMask3 <= 0xf &&
                    mModuleTypeMask0 <= 0xf && mModuleTypeMask1 <= 0xf , "Error in MenuList::unpack()");
        }

        public int pack() {
            int len = mMenuName.length() + 1;  // 1 for the null terminator
            int packetSize = 3 + 10 + len;
            packer(packetSize, 16);
            packer(PID_MENU_LIST, 8);
            packer(mMenuId, 8);
            packer(mModuleId0, 8);
            packer(mModuleMask0, 4);
            packer(mModuleId1, 8);
            packer(mModuleMask1, 4);
            packer(mModuleId2, 8);
            packer(mModuleMask2, 4);
            packer(mModuleId3, 8);
            packer(mModuleMask3, 4);
            packer(mModuleTypeId0, 8);
            packer(mModuleTypeMask0, 4);
            packer(mModuleTypeId1, 8);
            packer(mModuleTypeMask1, 4);
            packerString(mMenuName.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * ModuleList Packet Class
     **********************************************************************************************/
    public class ModuleList implements PacketElement {

        private int mModuleId;
        private int mModuleTypeId;
        private StringBuilder mModuleName;

        public ModuleList() {mModuleName = new StringBuilder();}

        public int getPacketType() {return PID_MODULE_LIST;}
        public int getClientHostId() {return -1;}
        public int getModuleId() {return mModuleId;}
        public int getModuleTypeId() {return mModuleTypeId;}
        public String getModuleName() {return mModuleName.toString();}

        public void set(int moduleId, int moduleTypeId, String moduleName) {
            mModuleId = moduleId;
            mModuleTypeId = moduleTypeId;
            mModuleName.setLength(0);
            mModuleName.append(moduleName);
        }

        public void unpack() {
            mModuleId = (int)unpacker(8);
            mModuleTypeId = (int)unpacker(8);
            unpackerString(mModuleName);
            flushPacket();
        }

        public int pack() {
            int len = mModuleName.length() + 1;  // 1 for the null terminator
            int packetSize = 3 + 2 + len;
            packer(packetSize, 16);
            packer(PID_MODULE_LIST, 8);
            packer(mModuleId, 8);
            packer(mModuleTypeId, 8);
            packerString(mModuleName.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * Log Packet Class
     **********************************************************************************************/
    public class Logger implements PacketElement {

        private StringBuilder mLog;

        public Logger() {mLog = new StringBuilder();}

        public int getPacketType() {return PID_LOGGER;}
        public int getClientHostId() {return -1;}
        public String getLog() {return mLog.toString();}

        public void set(String log) {
            mLog.setLength(0);
            mLog.append(log);
        }

        public void unpack() {
            unpackerString(mLog);
            flushPacket();
        }

        public int pack() {
            int len = mLog.length() + 1;  // 1 for the null terminator
            int packetSize = 3 + len;
            packer(packetSize, 16);
            packer(PID_LOGGER, 8);
            packerString(mLog.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * CamState Packet Class
     **********************************************************************************************/
    public class CamState implements PacketElement {
        public static final short CAM0  = 0x01;
        public static final short CAM1  = 0x02;
        public static final short CAM2  = 0x04;
        public static final short CAM3  = 0x08;
        public static final short CAM4  = 0x10;
        public static final short CAM5  = 0x20;
        public static final short CAM6  = 0x40;
        public static final short CAM7  = 0x80;

        private int mMultiplier;
        private int mFocus;
        private int mShutter;

        public CamState() {}

        public int getPacketType() {return PID_CAM_STATE;}
        public int getClientHostId() {return -1;}
        public int getMultiplier() {return mMultiplier;}
        public int getFocus() {return mFocus;}
        public int getShutter() {return mShutter;}

        public void set(int multiplier, int focus, int shutter) {
            mMultiplier = multiplier;
            mFocus = focus;
            mShutter = shutter;
        }

        public void unpack() {
            mMultiplier = (int)unpacker(8);
            mFocus =  (int)unpacker(8);
            mShutter = (int)unpacker(8);
            flushPacket();
        }

        public int pack() {
            int packetSize = 3 + 3;
            packer(packetSize, 16);
            packer(PID_CAM_STATE, 8);
            packer(mMultiplier, 8);
            packer(mFocus, 8);
            packer(mShutter, 8);
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * CamSettings Packet Class
     **********************************************************************************************/
    public class CamSettings implements PacketElement {
        public static final short SEQ0  = 0x01;
        public static final short SEQ1  = 0x02;
        public static final short SEQ2  = 0x04;
        public static final short SEQ3  = 0x08;
        public static final short SEQ4  = 0x10;
        public static final short SEQ5  = 0x20;
        public static final short SEQ6  = 0x40;
        public static final short SEQ7  = 0x80;

        private int mCamPortNumber;
        private int mMode;
        private int mDelayHours;
        private int mDelayMinutes;
        private int mDelaySeconds;
        private int mDelayMilliseconds;
        private int mDelayMicroseconds;
        private int mDurationHours;
        private int mDurationMinutes;
        private int mDurationSeconds;
        private int mDurationMilliseconds;
        private int mDurationMicroseconds;
        private int mSequencer;
        private int mApplyIntervalometer;
        private int mSmartPreview;
        private int mMirrorLockupEnable;
        private int mMirrorLockupMinutes;
        private int mMirrorLockupSeconds;
        private int mMirrorLockupMilliseconds;

        public CamSettings() {}

        public int getPacketType() {return PID_CAM_SETTINGS;}
        public int getClientHostId() {return -1;}
        public int  getCamPortNumber() {return mCamPortNumber;}
        public int  getMode() {return mMode;}
        public int  getDelayHours() {return mDelayHours;}
        public int  getDelayMinutes() {return mDelayMinutes;}
        public int  getDelaySeconds() {return mDelaySeconds;}
        public int  getDelayMilliseconds() {return mDelayMilliseconds;}
        public int  getDelayMicroseconds() {return mDelayMicroseconds;}
        public int  getDurationHours() {return mDurationHours;}
        public int  getDurationMinutes() {return mDurationMinutes;}
        public int  getDurationSeconds() {return mDurationSeconds;}
        public int  getDurationMilliseconds() {return mDurationMilliseconds;}
        public int  getDurationMicroseconds() {return mDurationMicroseconds;}
        public int  getSequencer() {return mSequencer;}
        public int  getApplyIntervalometer() {return mApplyIntervalometer;}
        public int  getSmartPreview() {return mSmartPreview;}
        public int  getMirrorLockupEnable() {return mMirrorLockupEnable;}
        public int  getMirrorLockupMinutes() {return mMirrorLockupMinutes;}
        public int  getMirrorLockupSeconds() {return mMirrorLockupSeconds;}
        public int  getMirrorLockupMilliseconds() {return mMirrorLockupMilliseconds;}

        public void set(int camPortNumber, int mode, int delayHours, int delayMinutes,
                        int delaySeconds, int delayMilliseconds, int delayMicroseconds,
                        int durationHours, int durationMinutes, int durationSeconds,
                        int durationMilliseconds, int durationMicroseconds, int sequencer,
                        int applyIntervalometer, int smartPreview, int mirrorLockupEnable,
                        int mirrorLockupMinutes, int mirrorLockupSeconds,
                        int mirrorLockupMilliseconds) {
            mCamPortNumber = camPortNumber;
            mMode = mode;
            mDelayHours = delayHours;
            mDelayMinutes = delayMinutes;
            mDelaySeconds = delaySeconds;
            mDelayMilliseconds = delayMilliseconds;
            mDelayMicroseconds = delayMicroseconds;
            mDurationHours = durationHours;
            mDurationMinutes = durationMinutes;
            mDurationSeconds = durationSeconds;
            mDurationMilliseconds = durationMilliseconds;
            mDurationMicroseconds = durationMicroseconds;
            mSequencer = sequencer;
            mApplyIntervalometer = applyIntervalometer;
            mSmartPreview = smartPreview;
            mMirrorLockupEnable = mirrorLockupEnable;
            mMirrorLockupMinutes = mirrorLockupMinutes;
            mMirrorLockupSeconds = mirrorLockupSeconds;
            mMirrorLockupMilliseconds = mirrorLockupMilliseconds;
            CA_ASSERT((mMode <= 2) && (mDelayHours <= 999) && (mDelayMinutes <=59) &&
                    (mDelaySeconds <= 59) && (mDelayMilliseconds <= 999) &&
                    (mDelayMicroseconds <= 999) && (mDurationHours <= 999) &&
                    (mDurationMinutes <= 59) && (mDurationSeconds <= 59) &&
                    (mDurationMilliseconds <= 999) && (mDurationMicroseconds <= 999) &&
                    (mApplyIntervalometer <= 1) && (mSmartPreview <= 59) &&
                    (mMirrorLockupEnable <= 1) && (mMirrorLockupMinutes <= 59) &&
                    (mMirrorLockupSeconds <= 59) && (mMirrorLockupMilliseconds <= 999),
                    "Error in CamSettings::set()");
        }

        public void unpack() {
            mCamPortNumber = (int)unpacker(8);
            mMode = (int)unpacker(2);
            mDelayHours = (int)unpacker(10);
            mDelayMinutes = (int)unpacker(6);
            mDelaySeconds = (int)unpacker(6);
            mDelayMilliseconds = (int)unpacker(10);
            mDelayMicroseconds = (int)unpacker(10);
            mDurationHours = (int)unpacker(10);
            mDurationMinutes = (int)unpacker(6);
            mDurationSeconds = (int)unpacker(6);
            mDurationMilliseconds = (int)unpacker(10);
            mDurationMicroseconds = (int)unpacker(10);
            mSequencer = (int)unpacker(8);
            mApplyIntervalometer = (int)unpacker(1);
            mSmartPreview = (int)unpacker(6);
            mMirrorLockupEnable = (int)unpacker(1);
            mMirrorLockupMinutes = (int)unpacker(6);
            mMirrorLockupSeconds = (int)unpacker(6);
            mMirrorLockupMilliseconds = (int)unpacker(10);
            unpacker(4); // Unused
            flushPacket();
            CA_ASSERT((mMode <= 2) && (mDelayHours <= 999) && (mDelayMinutes <=59) &&
                    (mDelaySeconds <= 59) && (mDelayMilliseconds <= 999) &&
                    (mDelayMicroseconds <= 999) && (mDurationHours <= 999) &&
                    (mDurationMinutes <= 59) && (mDurationSeconds <= 59) &&
                    (mDurationMilliseconds <= 999) && (mDurationMicroseconds <= 999) &&
                    (mApplyIntervalometer <= 1) && (mSmartPreview <= 59) &&
                    (mMirrorLockupEnable <= 1) && (mMirrorLockupMinutes <= 59) &&
                    (mMirrorLockupSeconds <= 59) && (mMirrorLockupMilliseconds <= 999),
                    "Error in CamSettings::unpack()");
        }

        public int pack() {
            int unused = 0;
            int packetSize = 3 + 17;
            packer(packetSize, 16);
            packer(PID_CAM_SETTINGS, 8);
            packer(mCamPortNumber, 8);
            packer(mMode, 2);
            packer(mDelayHours, 10);
            packer(mDelayMinutes, 6);
            packer(mDelaySeconds, 6);
            packer(mDelayMilliseconds, 10);
            packer(mDelayMicroseconds, 10);
            packer(mDurationHours, 10);
            packer(mDurationMinutes, 6);
            packer(mDurationSeconds, 6);
            packer(mDurationMilliseconds, 10);
            packer(mDurationMicroseconds, 10);
            packer(mSequencer, 8);
            packer(mApplyIntervalometer, 1);
            packer(mSmartPreview, 6);
            packer(mMirrorLockupEnable, 1);
            packer(mMirrorLockupMinutes, 6);
            packer(mMirrorLockupSeconds, 6);
            packer(mMirrorLockupMilliseconds, 10);
            packer(unused, 4);
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * Intervalometer Packet Class
     **********************************************************************************************/
    public class Intervalometer implements PacketElement {

        private int mStartHours;
        private int mStartMinutes;
        private int mStartSeconds;
        private int mStartMilliseconds;
        private int mStartMicroseconds;
        private int mIntervalHours;
        private int mIntervalMinutes;
        private int mIntervalSeconds;
        private int mIntervalMilliseconds;
        private int mIntervalMicroseconds;
        private int mRepeats;

        public Intervalometer() {}

        public int getPacketType() {return PID_INTERVALOMETER;}
        public int getClientHostId() {return -1;}
        public int getStartHours() {return mStartHours;}
        public int getStartMinutes() {return mStartMinutes;}
        public int getStartSeconds() {return mStartSeconds;}
        public int getStartMilliseconds() {return mStartMilliseconds;}
        public int getStartMicroseconds() {return mStartMicroseconds;}
        public int getIntervalHours() {return mIntervalHours;}
        public int getIntervalMinutes() {return mIntervalMinutes;}
        public int getIntervalSeconds() {return mIntervalSeconds;}
        public int getIntervalMilliseconds() {return mIntervalMilliseconds;}
        public int getIntervalMicroseconds() {return mIntervalMicroseconds;}
        public int getRepeats() {return mRepeats;}

        public void set(int startHours, int startMinutes, int startSeconds, int startMilliseconds,
                        int startMicroseconds, int intervalHours, int intervalMinutes,
                        int intervalSeconds, int intervalMilliseconds, int intervalMicroseconds,
                        int repeats) {
            mStartHours = startHours;
            mStartMinutes = startMinutes;
            mStartSeconds = startSeconds;
            mStartMilliseconds = startMilliseconds;
            mStartMicroseconds = startMicroseconds;
            mIntervalHours = intervalHours;
            mIntervalMinutes = intervalMinutes;
            mIntervalSeconds = intervalSeconds;
            mIntervalMilliseconds = intervalMilliseconds;
            mIntervalMicroseconds = intervalMicroseconds;
            mRepeats = repeats;
            CA_ASSERT((mStartHours <= 999) && (mStartMinutes <= 59) && (mStartSeconds <=59) &&
                    (mStartMilliseconds <= 999) && (mStartMicroseconds <= 999) &&
                    (mIntervalHours <= 999) && (mIntervalMinutes <= 59) &&
                    (mIntervalSeconds <= 59) && (mIntervalMilliseconds <= 999) &&
                    (mIntervalMicroseconds <= 999), "Error in CAPacketIntervalometer::set()");
        }

        public void unpack() {
            mStartHours = (int)unpacker(10);
            mStartMinutes = (int)unpacker(6);
            mStartSeconds = (int)unpacker(6);
            mStartMilliseconds = (int)unpacker(10);
            mStartMicroseconds = (int)unpacker(10);
            mIntervalHours = (int)unpacker(10);
            mIntervalMinutes = (int)unpacker(6);
            mIntervalSeconds = (int)unpacker(6);
            mIntervalMilliseconds = (int)unpacker(10);
            mIntervalMicroseconds = (int)unpacker(10);
            mRepeats = (int)unpacker(16);
            unpacker(4);  // Unused
            flushPacket();
            CA_ASSERT((mStartHours <= 999) && (mStartMinutes <= 59) && (mStartSeconds <=59) &&
                    (mStartMilliseconds <= 999) && (mStartMicroseconds <= 999) &&
                    (mIntervalHours <= 999) && (mIntervalMinutes <= 59) &&
                    (mIntervalSeconds <= 59) && (mIntervalMilliseconds <= 999) &&
                    (mIntervalMicroseconds <= 999), "Error in CAPacketIntervalometer::unpack()");
        }

        public int pack() {

            int unused=0;
            int packetSize = 3 + 13;
            packer(packetSize, 16);
            packer(PID_INTERVALOMETER, 8);
            packer(mStartHours, 10);
            packer(mStartMinutes, 6);
            packer(mStartSeconds, 6);
            packer(mStartMilliseconds, 10);
            packer(mStartMicroseconds, 10);
            packer(mIntervalHours, 10);
            packer(mIntervalMinutes, 6);
            packer(mIntervalSeconds, 6);
            packer(mIntervalMilliseconds, 10);
            packer(mIntervalMicroseconds, 10);
            packer(mRepeats, 16);
            packer(unused, 4);
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * ControlFlags Packet Class
     **********************************************************************************************/
    public class ControlFlags implements PacketElement {

        private int mSlaveModeEnable;
        private int mExtraMessagesEnable;

        public ControlFlags() {}

        public int getPacketType() {return PID_CONTROL_FLAGS;}
        public int getClientHostId() {return -1;}
        public int getSlaveModeEnable() {return mSlaveModeEnable;}
        public int getExtraMessagesEnable() {return mExtraMessagesEnable;}

        public void set(int slaveModeEnable, int extraMessagesEnable) {
            mSlaveModeEnable = slaveModeEnable;
            mExtraMessagesEnable = extraMessagesEnable;
            CA_ASSERT((mSlaveModeEnable <= 1) && (mExtraMessagesEnable <= 1),
                    "Error in CAPacketControlFlags::set()");
        }

        public void unpack() {
            mSlaveModeEnable = (int)unpacker(1);
            mExtraMessagesEnable =  (int)unpacker(1);
            unpacker(6); // Unused
            flushPacket();
            CA_ASSERT((mSlaveModeEnable <= 1) && (mExtraMessagesEnable <= 1),
                    "Error in CAPacketControlFlags::unpack()");
        }

        public int pack() {
            int unused = 0;
            int packetSize = 3 + 1;
            packer(packetSize, 16);
            packer(PID_CONTROL_FLAGS, 8);
            packer(mSlaveModeEnable, 1);
            packer(mExtraMessagesEnable, 1);
            packer(unused, 6);
            flushPacket();
            return packetSize;
        }
    }
}
