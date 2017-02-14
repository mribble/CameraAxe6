package com.dreamingrobots.cameraaxe;


import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;

import java.util.ArrayList;
import java.util.List;

import static com.dreamingrobots.cameraaxe.MainActivity.mIpPort;


/**
 *  Handle networking in a retained fragment so threads aren't lost on a configuration change
 */
public class RetainedFragment extends Fragment {

    // Setup threading for UDP network packets
    private UdpClientThread.UdpClientHandler mUdpHandler;
    private UdpClientThread mUdpSendThread;
    private UdpClientThread mUdpReceiveThread;
    private MenuNameAdapter mMenuListAdapter;
    private DynamicMenuBuilder mDynamicMenuBuilder;
    private String mIpAddress;
    private int mIpPort;



    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRetainInstance(true);
    }

    public ArrayAdapter getMenuListAdapter() { return mMenuListAdapter; }

    public void setDynamicMenuBuilder(LinearLayout parentView) {
        if (mDynamicMenuBuilder == null) {
            mDynamicMenuBuilder = new DynamicMenuBuilder(getActivity(), parentView, this);
            mMenuListAdapter = new MenuNameAdapter(getActivity(), R.layout.menu_name_item, new ArrayList<MenuName>());
            mUdpHandler = new UdpClientThread.UdpClientHandler(mDynamicMenuBuilder, mMenuListAdapter);
        } else {
            mDynamicMenuBuilder.updateActivity(getActivity(), parentView);
        }
    }

    public void startReceiveThread(final String ipAddress, final int ipPort) {
        if (mUdpReceiveThread == null) {
            mUdpReceiveThread = new UdpClientThread(UdpClientThread.UdpClientState.RECEIVE,
                    ipAddress, ipPort + 1, mUdpHandler);
            mUdpReceiveThread.start();
        }
    }

    public void setNetwork(final String ipAddress, final int ipPort) {
        mIpAddress = ipAddress;
        mIpPort = ipPort;
    }

    public void sendMessage(CAPacketHelper ph, int packSize) {
        mUdpSendThread = new UdpClientThread(UdpClientThread.UdpClientState.SEND, mIpAddress,
                mIpPort, mUdpHandler, ph, packSize);
        mUdpSendThread.start();
    }

    public void reset() {
        mDynamicMenuBuilder.reset();
    }
}
