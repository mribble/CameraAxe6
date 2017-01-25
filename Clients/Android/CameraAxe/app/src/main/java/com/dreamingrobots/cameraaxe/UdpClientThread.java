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

    private void sendUiMessage(String state) {
        mHandler.sendMessage(Message.obtain(mHandler, MainActivity.UdpClientHandler.UPDATE_MESSAGE, state));
    }

    private void sendPacketMessage(CAPacket.PacketElement packet) {
        mHandler.sendMessage(Message.obtain(mHandler, MainActivity.UdpClientHandler.UPDATE_PACKET, packet));
    }

    @Override
    public void run() {
        sendUiMessage("...");

        try {
            mSocket = new DatagramSocket(mIpPort);
            mAddress = InetAddress.getByName(mIpAddr);

            // Build the packet

            if (mState == UdpClientThread.UdpClientState.SEND) {
                // send a packet
                CAPacketHelper ph = new CAPacketHelper();
                int packSize = ph.writePacketMenuSelect(1, 2);
                DatagramPacket packet = new DatagramPacket(ph.getData(), packSize, mAddress, mIpPort);
                mSocket.send(packet);
            }
            else if (mState == UdpClientThread.UdpClientState.RECEIVE) {
                byte[] data = new byte[256];
                CAPacketHelper ph = new CAPacketHelper();
                while (!Thread.currentThread().isInterrupted()) {
                    // receive packets
                    DatagramPacket packet = new DatagramPacket(data, data.length);
                    mSocket.receive(packet);
                    CAPacket.PacketElement receivedData = ph.processIncomingPacket(packet.getData(), packet.getLength());
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
                mHandler.sendEmptyMessage(MainActivity.UdpClientHandler.UPDATE_END);
            }
        }
    }
}
