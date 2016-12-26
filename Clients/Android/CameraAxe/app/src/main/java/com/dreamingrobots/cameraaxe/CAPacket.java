package com.dreamingrobots.cameraaxe;

import android.content.Context;
import android.widget.Toast;

import java.nio.charset.Charset;

/**
 * Created by oe on 12/25/2016.
 */

public class CAPacket {
    static final short PID_START_SENTINEL = 0;
    static final short PID_MENU_HEADER = 1;
    static final short PID_END_SENTINEL = 24;

    static final short STATE_INIT = 0;
    static final short STATE_PACKER = 1;
    static final short STATE_UNPACKER = 2;

    Context mContext;
    protected int mBitsUsed;
    protected int mBitsVal;
    protected int mBytesUsed;
    protected int mState;
    protected byte[] mDstBuf;
    protected byte[] mSrcBuf;

    public CAPacket(Context context) {
        mContext = context;
    }

    public void startPacker(byte[] dst) {
        if (mBitsUsed != 0 || mBitsVal != 0 || mBytesUsed != 0 || mState != STATE_INIT) {
            Toast.makeText(mContext, "Error in startPacker", Toast.LENGTH_LONG);
        }
        mState = STATE_PACKER;
        mDstBuf = dst;
    }

    public void endPacker() {
        if (mBitsUsed != 0 || mBitsVal != 0 || mState != STATE_PACKER) {
            Toast.makeText(mContext, "Error in endPacker", Toast.LENGTH_LONG);
        }
        mState = STATE_INIT;
        mBytesUsed = 0;
    }

    public void startUnpacker(final byte[] src) {
        if (mBitsUsed != 0 || mBitsVal != 0 || mBytesUsed != 0 || mState != STATE_INIT) {
            Toast.makeText(mContext, "Error in startUnpacker", Toast.LENGTH_LONG);
        }
        mState = STATE_UNPACKER;
        mSrcBuf = src;
    }

    public void endUnpacker() {
        if (mBitsUsed != 0 || mBitsVal != 0 || mState != STATE_UNPACKER) {
            Toast.makeText(mContext, "Error in endUnpacker", Toast.LENGTH_LONG);
        }
        mState = STATE_INIT;
        mBytesUsed = 0;
    }

    protected long unpacker(byte unpackBits) {
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
        int len = 0;
        do {
            len++;
        } while (mSrcBuf[mBytesUsed+len] != 0); // This do while loop includes null terminator
        String str = new String(mSrcBuf, mBytesUsed, mBytesUsed+len);
        mBytesUsed += len;
        return str;
    }

    protected void packer( long val, byte packBits) {

    }

    protected void packerString(String src) {
        byte[] b = src.getBytes(Charset.forName("UTF-8"));

        for(int val=0; val<src.length(); val++) {
            mDstBuf[mBytesUsed++] = b[val];
        }
    }
}
