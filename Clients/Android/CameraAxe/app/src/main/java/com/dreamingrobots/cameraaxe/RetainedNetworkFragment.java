package com.dreamingrobots.cameraaxe;


import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

/**
 *  Handle networking in a retained fragment so threads aren't lost on a configuration change
 */
public class RetainedNetworkFragment extends Fragment {

    // Setup threading for UDP network packets
    UdpClientThread.UdpClientHandler mUdpHandler;
    UdpClientThread mUdpSendThread;
    UdpClientThread mUdpReceiveThread;
    MenuAdapter mAdapter;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRetainInstance(true);
        mAdapter = new MenuAdapter(getActivity());
        mUdpHandler = new UdpClientThread.UdpClientHandler(mAdapter);
    }

    public MenuAdapter getAdapter() {
        return mAdapter;
    }

    public void startReceiveThread(final String ipAddress, final int ipPort) {
        if (mUdpReceiveThread == null) {
            mUdpReceiveThread = new UdpClientThread(UdpClientThread.UdpClientState.RECEIVE,
                    ipAddress, ipPort + 1, mUdpHandler);
            mUdpReceiveThread.start();
        }
    }

    public void sendMessage(final String ipAddress, final int ipPort) {
        mUdpSendThread = new UdpClientThread(UdpClientThread.UdpClientState.SEND, ipAddress,
                ipPort, mUdpHandler);
        mUdpSendThread.start();
    }
}
