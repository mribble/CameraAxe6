package com.dreamingrobots.cameraaxe;

import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.List;

import static com.dreamingrobots.cameraaxe.UdpClientThread.UdpClientHandler.UPDATE_MESSAGE;
import static com.dreamingrobots.cameraaxe.UdpClientThread.UdpClientHandler.UPDATE_PACKET;


/**
 * Thread that handles reading and writing UDP packets
 */
public class UdpClientThread extends Thread {

    public enum UdpClientState {SEND, RECEIVE}

    private UdpClientState mState;
    private String mIpAddress;
    private int mIpPort;
    private UdpClientHandler mHandler;
    private DatagramSocket mSocket;
    private InetAddress mAddress;
    private CAPacketHelper mPacketHelper;
    private int mPacketHelperSize;

    public UdpClientThread(UdpClientState state, String address, int port,
                           UdpClientHandler handler) {
        super();

        mState = state;
        mIpAddress = address;
        mIpPort = port;
        mHandler = handler;
        mPacketHelper = new CAPacketHelper();
        mPacketHelperSize = 0;
    }

    public UdpClientThread(UdpClientState state, String address, int port,
                           UdpClientHandler handler, CAPacketHelper packetHelper, int packetSize) {
        super();
        // This constructor is just for sending packets
        mState = state;
        mIpAddress = address;
        mIpPort = port;
        mHandler = handler;
        mPacketHelper = packetHelper;
        mPacketHelperSize = packetSize;
    }

    private void sendUiMessage(String state) {
        mHandler.sendMessage(Message.obtain(mHandler, UPDATE_MESSAGE,
                state));
    }

    private void sendPacketMessage(CAPacket.PacketElement packet) {
        mHandler.sendMessage(Message.obtain(mHandler, UPDATE_PACKET,
                packet));
    }

    @Override
    public void run() {

        if (mState == UdpClientState.RECEIVE) {
            Log.i("CA6", "Receive thread started: " + mIpAddress + " : " + mIpPort);
        }

        try {
            mSocket = new DatagramSocket(mIpPort);
            mAddress = InetAddress.getByName(mIpAddress);

            // Build the packet

            if (mState == UdpClientThread.UdpClientState.SEND) {
                // send a packet
                DatagramPacket packet = new DatagramPacket(mPacketHelper.getData(), mPacketHelperSize, mAddress,
                        mIpPort);
                mSocket.send(packet);
            }
            else if (mState == UdpClientThread.UdpClientState.RECEIVE) {
                byte[] data = new byte[256];
                while (!Thread.currentThread().isInterrupted()) {
                    // receive packets
                    DatagramPacket packet = new DatagramPacket(data, data.length);
                    mSocket.receive(packet);
                    CAPacket.PacketElement receivedData = mPacketHelper.processIncomingPacket(packet.getData(),
                            packet.getLength());
                    sendPacketMessage(receivedData);
                }
            }
        } catch (SocketException e) {
            e.printStackTrace();
        } catch (UnknownHostException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if(mSocket != null){
                mSocket.close();
            }
        }

        if (mState == UdpClientState.RECEIVE) {
            Log.i("CA6", "Receive thread ended");
        }
    }

    /**
     * Internal class to update UI based on messages from UdpClientThread thread.
     */
    static public class UdpClientHandler extends Handler {
        static final int UPDATE_MESSAGE = 0;
        static final int UPDATE_PACKET = 1;
        private DynamicMenuAdapter mDynamicMenuAdapter;
        private MenuNameAdapter mMenuListAdapter;

        public UdpClientHandler(DynamicMenuAdapter dynamicMenuAdapter, MenuNameAdapter menuListAdapter) {
            mDynamicMenuAdapter = dynamicMenuAdapter;
            mMenuListAdapter = menuListAdapter;
        }

        public void updateMessage(String msg) {
            Log.e("CA6", msg);
        }

        public void updatePacket(CAPacket.PacketElement packet) {
            if (packet.getPacketType() == CAPacket.PID_MENU_LIST) {
                CAPacket.MenuList p = (CAPacket.MenuList) packet;
                MenuName mn = new MenuName(p.getMenuId(), p.getModuleId0(), p.getModuleMask0(),
                        p.getModuleId1(), p.getModuleMask1(),
                        p.getModuleId2(), p.getModuleMask2(),
                        p.getModuleId3(), p.getModuleMask3(),
                        p.getModuleTypeId0(), p.getModuleTypeMask0(),
                        p.getModuleTypeId1(), p.getModuleTypeMask1(), p.getMenuName());
                mMenuListAdapter.add(mn);
            } else {
                mDynamicMenuAdapter.addPacket(packet);
            }
        }

        @Override
        public void handleMessage(Message msg) {

            switch (msg.what) {
                case UPDATE_MESSAGE:
                    updateMessage((String) msg.obj);
                    break;
                case UPDATE_PACKET:
                    updatePacket((CAPacket.PacketElement)msg.obj);
                    break;
                default:
                    super.handleMessage(msg);
            }
        }
    }
}
