package com.dreamingrobots.cameraaxe;

import android.os.Message;

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
    private String mIpAddr;
    private int mIpPort;
    private MainActivity.UdpClientHandler mHandler;
    private DatagramSocket mSocket;
    private InetAddress mAddress;

    public UdpClientThread(UdpClientState state, String addr, int port, MainActivity.UdpClientHandler handler) {
        super();

        mState = state;
        mIpAddr = addr;
        mIpPort = port;
        mHandler = handler;
    }

    private void sendUiMessage(String state){
        mHandler.sendMessage(Message.obtain(mHandler, MainActivity.UdpClientHandler.UPDATE_MESSAGE, state));
    }

    @Override
    public void run() {
        sendUiMessage("...");

        try {
            mSocket = new DatagramSocket(mIpPort);
            mAddress = InetAddress.getByName(mIpAddr);

            // Build the packet
            byte[] data = new byte[256];

            if (mState == UdpClientThread.UdpClientState.SEND) {
                // send a packet
                CAPacket pack0 = new CAPacket(CAPacket.STATE_PACKER, data, 256);
                CAPacket.MenuSelect pack1 = pack0.new MenuSelect();
                pack1.set(1, 1);
                int packSize = pack1.pack();
                DatagramPacket packet = new DatagramPacket(data, packSize, mAddress, mIpPort);
                mSocket.send(packet);
            }
            else if (mState == UdpClientThread.UdpClientState.RECEIVE) {
                while (!Thread.currentThread().isInterrupted()) {
                    // receive packets
                    DatagramPacket packet = new DatagramPacket(data, data.length);
                    mSocket.receive(packet);
                    CAPacketHelper ph = new CAPacketHelper();
                    String receivedData = ph.processIncomingPackets(packet.getData(), packet.getLength());
                    sendUiMessage(receivedData);
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
                mHandler.sendEmptyMessage(MainActivity.UdpClientHandler.UPDATE_END);
            }
        }
    }
}
