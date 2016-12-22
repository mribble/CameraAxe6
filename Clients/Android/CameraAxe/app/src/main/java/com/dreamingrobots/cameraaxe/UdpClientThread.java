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

            // send request
            String str = "Message from Android.";
            byte[] buf = str.getBytes("UTF-8");
            DatagramPacket packet = new DatagramPacket(buf, buf.length, mAddress, mIpPort);
            mSocket.send(packet);

            packet = new DatagramPacket(buf, buf.length);

            mSocket.receive(packet);
            String receivedData = new String(packet.getData(), 0, packet.getLength());
            sendUiMessage(receivedData);
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
