package com.dreamingrobots.cameraaxe;

import android.content.Context;
import android.util.Log;

import java.nio.charset.Charset;

/**
 * Created by oe on 12/25/2016.
 */

/**
 * Base class that manages packet construction
 */
public class CAPacket {
    public static final short STATE_PACKER = 1;
    public static final short STATE_UNPACKER = 2;

    protected static final short PID_START_SENTINEL     = 0;  // Must be first
    protected static final short PID_MENU_HEADER        = 1;
    protected static final short PID_NEW_ROW            = 2;
    protected static final short PID_NEW_CELL           = 3;
    protected static final short PID_COND_START         = 4;
    protected static final short PID_COND_END           = 5;
    protected static final short PID_TEXT_STATIC        = 6;
    protected static final short PID_TEXT_DYNAMIC       = 7;
    protected static final short PID_BUTTON             = 8;
    protected static final short PID_CHECK_BOX          = 9;
    protected static final short PID_DROP_SELECT        = 10;
    protected static final short PID_EDIT_NUMBER        = 11;
    protected static final short PID_TIME_BOX           = 12;
    protected static final short PID_SCRIPT_END         = 13;
    protected static final short PID_ACTIVATE           = 14;
    protected static final short PID_LOGGER             = 15;
    protected static final short PID_CAM_STATE          = 16;
    protected static final short PID_CAM_SETTINGS       = 17;
    protected static final short PID_INTERVALOMETER     = 18;
    protected static final short PID_INTER_MODULE_LOGIC = 19;
    protected static final short PID_CONTROL_FLAGS      = 20;
    protected static final short PID_END_SENTINEL       = 21; // Must be last

    private Context mContext;
    private int mBitsUsed;
    private int mBitsVal;
    private int mBytesUsed;
    private int mState;
    private byte[] mBuf;
    private int mBufSize;

    public CAPacket(Context context, short state, byte[] buf, int bufSize) {
        mContext = context;
        mState = state;
        mBuf = buf;
        mBufSize = bufSize;
    }

    public void CA_ASSERT(boolean test, String str) {
        if (test == false) {
            Log.e("CA6_ASSERT", str);
        }
    }

    public int unpackSize() {return (int) unpacker(8);}

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
        // and puts them into an uint32
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
        if (mBitsUsed != 0 || mBitsVal != 0 || mBytesUsed >= mBufSize) {
            Log.e("CA6", "Error detected during flush()");
        }
    }

    /***********************************************************************************************
     * MenuHeader Packet Class
     **********************************************************************************************/
    public class MenuHeader {

        private int mMajorVersion;
        private int mMinorVersion;
        private StringBuilder mMenuName;

        public MenuHeader() {
            mMenuName = new StringBuilder();
        }

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
            mMajorVersion = (int)unpacker(8);
            mMinorVersion = (int)unpacker(8);
            unpackerString(mMenuName);
            flushPacket();
        }

        public int pack() {
            int len = mMenuName.length() + 1;  // 1 for the null terminator
            int packetSize = 2 + 2 + len;
            packer(packetSize, 8);
            packer(PID_MENU_HEADER, 8);
            packer(mMajorVersion, 8);
            packer(mMinorVersion, 8);
            packerString(mMenuName.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * NewRow Packet Class
     **********************************************************************************************/
    public class NewRow {

        public NewRow() {}

        public void set() {
            Log.e("CA6", "NewRow::set() never needs to be called");
        }

        public void unpack() {
            Log.e("CA6", "NewRow::unpack() never needs to be called");
        }

        public int pack() {
            int packetSize = 2;
            packer(packetSize, 8);
            packer(PID_NEW_ROW, 8);
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * NewCell Packet Class
     **********************************************************************************************/
    public class NewCell {

        private int mColumnPercentage;
        private int mJustification;

        public NewCell() {}

        public int getColumnPercentage() {return mColumnPercentage;}
        public int getJustification() {return mJustification;}

        public void set(int columnPercentage, int justification) {
            mColumnPercentage = columnPercentage;
            mJustification = justification;
        }

        public void unpack() {
            mColumnPercentage = (int)unpacker(8);
            mJustification = (int)unpacker(8);
            flushPacket();
        }

        public int pack() {
            int packetSize = 2 + 2;
            packer(packetSize, 8);
            packer(PID_NEW_CELL, 8);
            packer(mColumnPercentage, 8);
            packer(mJustification, 8);
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * CondStart Packet Class
     **********************************************************************************************/
    public class CondStart {

        private int mClientHostId;
        private int mModAttribute;
        private int mValue;

        public CondStart() {}

        public int getClientHostId() {return mClientHostId;}
        public int getModAttribute() {return mModAttribute;}
        public int getValue() {return mValue;}

        public void set(int clientHostId, int modAttribute, int value) {
            mClientHostId = clientHostId;
            mModAttribute = modAttribute;
            mValue = value;
            CA_ASSERT((mModAttribute <= 1) && (mValue <= 1),
                    "Error in CAPacketCondStart::set()");
        }

        public void unpack() {
            mClientHostId = (int)unpacker(8);
            mModAttribute = (int)unpacker(4);
            mValue = (int)unpacker(4);
            flushPacket();
        }

        public int pack() {
            int packetSize = 2 + 2;
            packer(packetSize, 8);
            packer(PID_COND_START, 8);
            packer(mClientHostId, 8);
            packer(mModAttribute, 4);
            packer(mValue, 4);
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * CondEnd Packet Class
     **********************************************************************************************/
    public class CondEnd {

        public CondEnd() {}

        public void set() {
            Log.e("CA6", "CondEnd::set() never needs to be called");
        }

        public void unpack() {
            Log.e("CA6", "CondEnd::unpack() never needs to be called");
        }

        public int pack() {
            int packetSize = 2;
            packer(packetSize, 8);
            packer(PID_COND_END, 8);
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * TextStatic Packet Class
     **********************************************************************************************/
    public class TextStatic {

        private StringBuilder mText;

        public TextStatic() {
            mText = new StringBuilder();
        }

        public String getText() {return mText.toString();}

        public void set(String menuName) {
            mText.setLength(0);
            mText.append(menuName);
        }

        public void unpack() {
            unpackerString(mText);
            flushPacket();
        }

        public int pack() {
            int len = mText.length() + 1;  // 1 for the null terminator
            int packetSize = 2 + len;
            packer(packetSize, 8);
            packer(PID_TEXT_STATIC, 8);
            packerString(mText.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * TextDynamic Packet Class
     **********************************************************************************************/
    public class TextDynamic {

        private int mClientHostId;
        private StringBuilder mText;

        public TextDynamic() {
            mText = new StringBuilder();
        }

        public int getClientHostId() {return mClientHostId;}
        public String getText() {return mText.toString();}

        public void set(int clientHostId, String menuName) {
            mClientHostId = clientHostId;
            mText.setLength(0);
            mText.append(menuName);
        }

        public void unpack() {
            mClientHostId = (int)unpacker(8);
            unpackerString(mText);
            flushPacket();
        }

        public int pack() {
            int len = mText.length() + 1;  // 1 for the null terminator
            int packetSize = 2 + 1 + len;
            packer(packetSize, 8);
            packer(PID_TEXT_DYNAMIC, 8);
            packer(mClientHostId, 8);
            packerString(mText.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * Button Packet Class
     **********************************************************************************************/
    public class Button {

        private int mClientHostId;
        private int mType;
        private int mValue;
        private StringBuilder mText;

        public Button() {
            mText = new StringBuilder();
        }

        public int getClientHostId() {return mClientHostId;}
        public int getType() {return mType;}
        public int getValue() {return mValue;}
        public String getText() {return mText.toString();}

        public void set(int clientHostId, int type, int value, String menuName) {
            mClientHostId = clientHostId;
            mType = type;
            mValue = value;
            mText.setLength(0);
            mText.append(menuName);
            CA_ASSERT((mType <= 1) && (mValue <= 1),
                    "Error in CAPacketButton::set()");
        }

        public void unpack() {
            mClientHostId = (int)unpacker(8);
            mType = (int)unpacker(4);
            mValue = (int)unpacker(4);
            unpackerString(mText);
            flushPacket();
            CA_ASSERT((mType <= 1) && (mValue <= 1),
                    "Error in CAPacketButton::unpack()");

        }

        public int pack() {
            int len = mText.length() + 1;  // 1 for the null terminator
            int packetSize = 2 + 2 + len;
            packer(packetSize, 8);
            packer(PID_BUTTON, 8);
            packer(mClientHostId, 8);
            packer(mType, 4);
            packer(mValue, 4);
            packerString(mText.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * CheckBox Packet Class
     **********************************************************************************************/
    public class CheckBox {

        private int mClientHostId;
        private int mValue;

        public CheckBox() {}

        public int getClientHostId() {return mClientHostId;}
        public int getValue() {return mValue;}

        public void set(int clientHostId, int value) {
            mClientHostId = clientHostId;
            mValue = value;
            CA_ASSERT((mValue <= 1), "Error in CAPacketCheckBox::set()");
        }

        public void unpack() {
            mClientHostId = (int)unpacker(8);
            mValue = (int)unpacker(8);
            flushPacket();
            CA_ASSERT((mValue <= 1), "Error in CAPacketCheckBox::unpack()");
        }

        public int pack() {
            int packetSize = 2 + 2;
            packer(packetSize, 8);
            packer(PID_CHECK_BOX, 8);
            packer(mClientHostId, 8);
            packer(mValue, 8);
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * DropSelect Packet Class
     **********************************************************************************************/
    public class DropSelect {

        private int mClientHostId;
        private int mValue;
        private StringBuilder mText;

        public DropSelect() {
            mText = new StringBuilder();
        }

        public int getClientHostId() {return mClientHostId;}
        public int getValue() {return mValue;}
        public String getText() {return mText.toString();}

        public void set(int clientHostId, int value, String menuName) {
            mClientHostId = clientHostId;
            mValue = value;
            mText.setLength(0);
            mText.append(menuName);
        }

        public void unpack() {
            mClientHostId = (int)unpacker(8);
            mValue = (int)unpacker(8);
            unpackerString(mText);
            flushPacket();
        }

        public int pack() {
            int len = mText.length() + 1;  // 1 for the null terminator
            int packetSize = 2 + 2 + len;
            packer(packetSize, 8);
            packer(PID_DROP_SELECT, 8);
            packer(mClientHostId, 8);
            packer(mValue, 8);
            packerString(mText.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * EditNumber Packet Class
     **********************************************************************************************/
    public class EditNumber {

        private int mClientHostId;
        private int mDigitsBeforeDecimal;
        private int mDigitsAfterDecimal;
        private long mMinValue;
        private long mMaxValue;
        private long mValue;

        public EditNumber() {}

        public int getClientHostId() {return mClientHostId;}
        public int getDigitsBeforeDecimal() {return mDigitsBeforeDecimal;}
        public int getDigitsAfterDecimal() {return mDigitsAfterDecimal;}
        public long getMinValue() {return mMinValue;}
        public long getmMaxValue() {return mMaxValue;}
        public long getValue() {return mValue;}

        public void set(int clientHostId, int digitsBeforeDecimal, int digitsAfterDecimal,
                        long minValue, long maxValue, long value) {
            mClientHostId = clientHostId;
            mDigitsBeforeDecimal = digitsBeforeDecimal;
            mDigitsAfterDecimal = digitsAfterDecimal;
            mMinValue = minValue;
            mMaxValue = maxValue;
            mValue = value;
            CA_ASSERT((mDigitsBeforeDecimal <= 8) && (mDigitsAfterDecimal <= 8) &&
                            (mDigitsBeforeDecimal+mDigitsAfterDecimal <= 8) &&
                            (mMinValue <= 99999999) && (mMaxValue <= 99999999),
                    "Error in CAPacketEditNumber::set()");
        }

        public void unpack() {
            mClientHostId = (int)unpacker(8);
            mDigitsBeforeDecimal = (int)unpacker(4);
            mDigitsAfterDecimal = (int)unpacker(4);
            mMinValue = (long)unpacker(32);
            mMaxValue = (long)unpacker(32);
            mValue = (long)unpacker(32);
            flushPacket();
            CA_ASSERT((mDigitsBeforeDecimal <= 8) && (mDigitsAfterDecimal <= 8) &&
                            (mDigitsBeforeDecimal+mDigitsAfterDecimal <= 8) &&
                            (mMinValue <= 99999999) && (mMaxValue <= 99999999),
                    "Error in CAPacketEditNumber::unpack()");
        }

        public int pack() {
            int packetSize = 2 + 14;
            packer(packetSize, 8);
            packer(PID_EDIT_NUMBER, 8);
            packer(mClientHostId, 8);
            packer(mDigitsBeforeDecimal, 4);
            packer(mDigitsAfterDecimal, 4);
            packer(mMinValue, 32);
            packer(mMaxValue, 32);
            packer(mValue, 32);
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * TimeBox Packet Class
     **********************************************************************************************/
    public class TimeBox {

        private int mClientHostId;
        private int mEnableMask;
        private int mHours;
        private int mMinutes;
        private int mSeconds;
        private int mMilliseconds;
        private int mMicroseconds;
        private int mNanoseconds;

        public TimeBox() {}

        public int getClientHostId() {return mClientHostId;}
        public int getEnableMask() {return mEnableMask;}
        public int getHours() {return mHours;}
        public int getMinutes() {return mMinutes;}
        public int getSeconds() {return mSeconds;}
        public int getMilliseconds() {return mMilliseconds;}
        public int getMicroseconds() {return mMicroseconds;}
        public int getNanoseconds() {return mNanoseconds;}

        public void set(int clientHostId, int enableMask, int hours, int minutes, int seconds,
                        int milliseconds, int microseconds, int nanoseconds ) {
            mClientHostId = clientHostId;
            mEnableMask = enableMask;
            mHours = hours;
            mMinutes = minutes;
            mSeconds = seconds;
            mMilliseconds = milliseconds;
            mMicroseconds = microseconds;
            mNanoseconds = nanoseconds;
            CA_ASSERT((mEnableMask <= 0x3F) && (mHours <= 999) && (mMinutes <= 59) && (mSeconds <=59) &&
                            (mMilliseconds <= 999) && (mMicroseconds <= 999) && (mNanoseconds <= 999),
                            "Error in CAPacketTimeBox::set()");
        }

        public void unpack() {
            mClientHostId = (int)unpacker(8);
            mEnableMask = (int)unpacker(6);
            mHours = (int)unpacker(10);
            mMinutes = (int)unpacker(6);
            mSeconds = (int)unpacker(6);
            mMilliseconds = (int)unpacker(10);
            mMicroseconds = (int)unpacker(10);
            mNanoseconds = (int)unpacker(10);
            unpacker(6); // Unused
            flushPacket();
            CA_ASSERT((mEnableMask <= 0x3F) && (mHours <= 999) && (mMinutes <= 59) && (mSeconds <=59) &&
                            (mMilliseconds <= 999) && (mMicroseconds <= 999) && (mNanoseconds <= 999),
                            "Error in CAPacketTimeBox::set()");
        }

        public int pack() {
            int unused = 0;
            int packetSize = 2 + 9;
            packer(packetSize, 8);
            packer(PID_TIME_BOX, 8);
            packer(mClientHostId, 8);
            packer(mEnableMask, 6);
            packer(mHours, 10);
            packer(mMinutes, 6);
            packer(mSeconds, 6);
            packer(mMilliseconds, 10);
            packer(mMicroseconds, 10);
            packer(mNanoseconds, 10);
            packer(unused, 6);
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * ScriptEnd Packet Class
     **********************************************************************************************/
    public class ScriptEnd {

        public ScriptEnd() {}

        public void set() {
            Log.e("CA6", "ScriptEnd::set() never needs to be called");
        }

        public void unpack() {
            Log.e("CA6", "ScriptEnd::unpack() never needs to be called");
        }

        public int pack() {
            int packetSize = 2;
            packer(packetSize, 8);
            packer(PID_SCRIPT_END, 8);
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * Activate Packet Class
     **********************************************************************************************/
    public class Activate {

        private int mActivate;

        public Activate() {}

        public int getActivate() {return mActivate;}

        public void set(int activate) {
            mActivate = activate;
            CA_ASSERT((mActivate <= 1), "Error in CAPacketActivate::set()");
        }

        public void unpack() {
            mActivate = (int)unpacker(8);
            flushPacket();
            CA_ASSERT((mActivate <= 1), "Error in CAPacketActivate::unpack()");
        }

        public int pack() {
            int packetSize = 2 + 1;
            packer(packetSize, 8);
            packer(PID_ACTIVATE, 8);
            packer(mActivate, 8);
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * Log Packet Class
     **********************************************************************************************/
    public class Logger {

        private StringBuilder mLog;

        public Logger() {
            mLog = new StringBuilder();
        }

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
            int packetSize = 2 + len;
            packer(packetSize, 8);
            packer(PID_LOGGER, 8);
            packerString(mLog.toString());
            flushPacket();
            return packetSize;
        }
    }
    /***********************************************************************************************
     * CamState Packet Class
     **********************************************************************************************/
    public class CamState {
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

        public int getMultiplier() {return mMultiplier;}
        public int getFocus() {return mFocus;}
        public int getmShutter() {return mShutter;}

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
            int packetSize = 2 + 3;
            packer(packetSize, 8);
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
    public class CamSettings {
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
                        int mirrorLockupMinutes, int mirrorLockupSeconds, int mirrorLockupMilliseconds) {
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
                            (mDelaySeconds <= 59) && (mDelayMilliseconds <= 999) && (mDelayMicroseconds <= 999) &&
                            (mDurationHours <= 999) && (mDurationMinutes <= 59) && (mDurationSeconds <= 59) &&
                            (mDurationMilliseconds <= 999) && (mDurationMicroseconds <= 999) && (mApplyIntervalometer <= 1) &&
                            (mSmartPreview <= 59) && (mMirrorLockupEnable <= 1) && (mMirrorLockupMinutes <= 59) &&
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
                            (mDelaySeconds <= 59) && (mDelayMilliseconds <= 999) && (mDelayMicroseconds <= 999) &&
                            (mDurationHours <= 999) && (mDurationMinutes <= 59) && (mDurationSeconds <= 59) &&
                            (mDurationMilliseconds <= 999) && (mDurationMicroseconds <= 999) && (mApplyIntervalometer <= 1) &&
                            (mSmartPreview <= 59) && (mMirrorLockupEnable <= 1) && (mMirrorLockupMinutes <= 59) &&
                            (mMirrorLockupSeconds <= 59) && (mMirrorLockupMilliseconds <= 999),
                    "Error in CamSettings::unpack()");
        }

        public int pack() {
            int unused = 0;
            int packetSize = 2 + 17;
            packer(packetSize, 8);
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
    public class Intervalometer {

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
                    (mStartMilliseconds <= 999) && (mStartMicroseconds <= 999) && (mIntervalHours <= 999) &&
                    (mIntervalMinutes <= 59) && (mIntervalSeconds <= 59) && (mIntervalMilliseconds <= 999) &&
                    (mIntervalMicroseconds <= 999), "Error in CAPacketIntervalometer::set()");       }

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
                    (mStartMilliseconds <= 999) && (mStartMicroseconds <= 999) && (mIntervalHours <= 999) &&
                    (mIntervalMinutes <= 59) && (mIntervalSeconds <= 59) && (mIntervalMilliseconds <= 999) &&
                    (mIntervalMicroseconds <= 999), "Error in CAPacketIntervalometer::unpack()");
        }

        public int pack() {

            int unused=0;
            int packetSize = 2 + 13;
            packer(packetSize, 8);
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
}
