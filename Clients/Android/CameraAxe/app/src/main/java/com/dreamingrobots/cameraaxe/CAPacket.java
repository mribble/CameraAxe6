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
    protected static final short PID_LOG                = 15;
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
            Log.e("CA6", "NewRow::set() never needs to be called");
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
            Log.e("CA6", "CondEnd::set() never needs to be called");
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
}
