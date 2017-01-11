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
    private String mIpAddr;
    private int mIpPort;
    MainActivity.UdpClientHandler mHandler;
    DatagramSocket mSocket;
    InetAddress mAddress;

    public UdpClientThread(String addr, int port, MainActivity.UdpClientHandler handler) {
        super();
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
            mSocket = new DatagramSocket();
            mAddress = InetAddress.getByName(mIpAddr);

            // Build the packet
            byte[] data = new byte[256];
            CAPacket pack0 = new CAPacket(CAPacket.STATE_PACKER, data, 256);
            CAPacket.Logger pack1 = pack0.new Logger();
            pack1.set("Start");
            int packSize = pack1.pack();

            // send request
            DatagramPacket packet = new DatagramPacket(data, packSize, mAddress, mIpPort);
            mSocket.send(packet);

            //packet = new DatagramPacket(data, data.length);
            //mSocket.receive(packet);
            //String receivedData = new String(packet.getData(), 0, packet.getLength());
            //sendUiMessage(receivedData);
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
