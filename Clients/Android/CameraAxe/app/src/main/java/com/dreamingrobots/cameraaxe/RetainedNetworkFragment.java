package com.dreamingrobots.cameraaxe;


import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.widget.ArrayAdapter;

import java.util.ArrayList;
import java.util.List;


/**
 *  Handle networking in a retained fragment so threads aren't lost on a configuration change
 */
public class RetainedNetworkFragment extends Fragment {

    // Setup threading for UDP network packets
    UdpClientThread.UdpClientHandler mUdpHandler;
    UdpClientThread mUdpSendThread;
    UdpClientThread mUdpReceiveThread;
    DynamicMenuAdapter mDynamicMenuAdapter;
    MenuNameAdapter mMenuListAdapter;



    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRetainInstance(true);
        mDynamicMenuAdapter = new DynamicMenuAdapter(getActivity());
        mMenuListAdapter = new MenuNameAdapter(getActivity(), R.layout.menu_name_item, new ArrayList<MenuName>());

        mUdpHandler = new UdpClientThread.UdpClientHandler(mDynamicMenuAdapter, mMenuListAdapter);
    }

    public DynamicMenuAdapter getDynamicMenuAdapter() {
        return mDynamicMenuAdapter;
    }
    public ArrayAdapter getMenuListAdapter() { return mMenuListAdapter; }

    public void startReceiveThread(final String ipAddress, final int ipPort) {
        if (mUdpReceiveThread == null) {
            mUdpReceiveThread = new UdpClientThread(UdpClientThread.UdpClientState.RECEIVE,
                    ipAddress, ipPort + 1, mUdpHandler);
            mUdpReceiveThread.start();
        }
    }

    public void sendMessage(final String ipAddress, final int ipPort, CAPacketHelper ph, int packSize) {
        mUdpSendThread = new UdpClientThread(UdpClientThread.UdpClientState.SEND, ipAddress,
                ipPort, mUdpHandler, ph, packSize);
        mUdpSendThread.start();
    }

    public void reset() {
        getDynamicMenuAdapter().reset();
    }
}
