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

    public UdpClientThread(UdpClientState state, String address, int port,
                           UdpClientHandler handler) {
        super();

        mState = state;
        mIpAddress = address;
        mIpPort = port;
        mHandler = handler;
    }

    private void sendUiMessage(String state) {
        mHandler.sendMessage(Message.obtain(mHandler, UdpClientHandler.UPDATE_MESSAGE,
                state));
    }

    private void sendPacketMessage(CAPacket.PacketElement packet) {
        mHandler.sendMessage(Message.obtain(mHandler, UdpClientHandler.UPDATE_PACKET,
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
                CAPacketHelper ph = new CAPacketHelper();
                int packSize = ph.writePacketMenuSelect(1, 2);
                DatagramPacket packet = new DatagramPacket(ph.getData(), packSize, mAddress,
                        mIpPort);
                mSocket.send(packet);
            }
            else if (mState == UdpClientThread.UdpClientState.RECEIVE) {
                byte[] data = new byte[256];
                CAPacketHelper ph = new CAPacketHelper();
                while (!Thread.currentThread().isInterrupted()) {
                    // receive packets
                    DatagramPacket packet = new DatagramPacket(data, data.length);
                    mSocket.receive(packet);
                    CAPacket.PacketElement receivedData = ph.processIncomingPacket(packet.getData(),
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
        private MenuAdapter mAdapter;

        public UdpClientHandler(MenuAdapter adapter) {
            mAdapter = adapter;
        }

        public void updateMessage(String msg) {
            Log.e("CA6", msg);
        }

        public void updatePacket(CAPacket.PacketElement packet) {
            mAdapter.addPacket(packet);
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
