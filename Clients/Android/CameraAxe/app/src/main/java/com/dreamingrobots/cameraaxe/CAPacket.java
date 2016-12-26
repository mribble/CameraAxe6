package com.dreamingrobots.cameraaxe;

import android.content.Context;
import android.widget.Toast;

import java.nio.charset.Charset;

/**
 * Created by oe on 12/25/2016.
 */

/**
 * Base class that manages packet construction
 */
public class CAPacket {
    static final short PID_START_SENTINEL = 0;
    static final short PID_MENU_HEADER = 1;
    static final short PID_END_SENTINEL = 24;

    static final short STATE_INIT = 0;
    static final short STATE_PACKER = 1;
    static final short STATE_UNPACKER = 2;

    protected Context mContext;
    protected int mBitsUsed;
    protected int mBitsVal;
    protected int mBytesUsed;
    protected int mState;
    protected byte[] mDstBuf;
    protected byte[] mSrcBuf;


    public class MenuHeader {

        public int mMajorVersion;
        public int mMinorVersion;
        public String mMenuName;

        public MenuHeader() {
        }

        public void unpack() {
            startUnpacker();
            mMajorVersion = (int)unpacker(8);
            mMinorVersion = (int)unpacker(8);
            mMenuName = unpackerString();
            endUnpacker();
        }

        public void pack() {
            int len = mMenuName.length();
            int packetSize = 2 + len + 1;  // 1 for the null terminator
            startPacker();
            packer(packetSize, 8);
            packer(PID_MENU_HEADER, 8);
            packer(mMajorVersion, 8);
            packer(mMinorVersion, 8);
            packerString(mMenuName);
            endPacker();
        }

        public void load(int majorVersion, int minorVersion, String menuName) {
            mMajorVersion = majorVersion;
            mMinorVersion = minorVersion;
            mMenuName = menuName;
        }
    }

    public CAPacket(Context context, byte[] dstBuf, byte[] srcBuf) {
        mContext = context;
        mDstBuf = dstBuf;
        mSrcBuf = srcBuf;
    }

    public void startPacker() {
        if (mBitsUsed != 0 || mBitsVal != 0 || mBytesUsed != 0 || mState != STATE_INIT) {
            Toast.makeText(mContext, "Error in startPacker", Toast.LENGTH_LONG);
        }
        mState = STATE_PACKER;
    }

    public void endPacker() {
        if (mBitsUsed != 0 || mBitsVal != 0 || mState != STATE_PACKER) {
            Toast.makeText(mContext, "Error in endPacker", Toast.LENGTH_LONG);
        }
        mState = STATE_INIT;
        mBytesUsed = 0;
    }

    public void startUnpacker() {
        if (mBitsUsed != 0 || mBitsVal != 0 || mBytesUsed != 0 || mState != STATE_INIT) {
            Toast.makeText(mContext, "Error in startUnpacker", Toast.LENGTH_LONG);
        }
        mState = STATE_UNPACKER;
    }

    public void endUnpacker() {
        if (mBitsUsed != 0 || mBitsVal != 0 || mState != STATE_UNPACKER) {
            Toast.makeText(mContext, "Error in endUnpacker", Toast.LENGTH_LONG);
        }
        mState = STATE_INIT;
        mBytesUsed = 0;
    }

    protected long unpacker(int unpackBits) {
        if (mState != STATE_UNPACKER) {
            Toast.makeText(mContext, "Error in unpacker", Toast.LENGTH_LONG);
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
            int val = mSrcBuf[mBytesUsed] << unusedLeftBits;  // Zero out left bits
            val = val >> rightShift;      // Shift bits to right most position for this byte
            ret |= (((long)val) << valShift);
            valShift += bitsToUnpack;
            if (mBitsUsed + bitsToUnpack == 8)
            {
                mBitsUsed = 0;
                mBytesUsed++;
            }
            else
            {
                mBitsUsed += bitsToUnpack;
            }
            unpackBitsLeft -= bitsToUnpack;
        } while (unpackBitsLeft != 0);
        return ret;
    }

    protected String unpackerString() {
        if (mState != STATE_UNPACKER) {
            Toast.makeText(mContext, "Error in unpackerString", Toast.LENGTH_LONG);
            return null;
        }
        int len = 0;
        do {
            len++;
        } while (mSrcBuf[mBytesUsed+len] != 0); // This do while loop includes null terminator
        String str = new String(mSrcBuf, mBytesUsed, mBytesUsed+len);
        mBytesUsed += len;
        return str;
    }

    protected void packer( long val, int packBits) {
        if (mState != STATE_PACKER) {
            Toast.makeText(mContext, "Error in packer", Toast.LENGTH_LONG);
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
                mDstBuf[mBytesUsed++] = (byte)(mBitsVal & 0xFF);
                mBitsVal = 0;
                mBitsUsed = 0;
            }
            packBitsLeft -= bitsToPack;
        } while (packBitsLeft != 0);
    }

    protected void packerString(String src) {
        if (mState != STATE_PACKER) {
            Toast.makeText(mContext, "Error in packer", Toast.LENGTH_LONG);
            return;
        }
        byte[] b = src.getBytes(Charset.forName("UTF-8"));
        for(int val=0; val<src.length(); val++) {
            mDstBuf[mBytesUsed++] = b[val];
        }
    }
}
