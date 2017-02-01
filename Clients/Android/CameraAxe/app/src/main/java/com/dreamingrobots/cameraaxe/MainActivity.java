package com.dreamingrobots.cameraaxe;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;

import static android.os.Build.VERSION_CODES.N;

/**
 * The main UI activity for the Android Udp application.
 * The major tasks for this activity are:
 * + Handle UI inputs/outputs
 * + Start threads to handle networking
 */
public class MainActivity extends AppCompatActivity {
    final static int mIpPort = 4045;
    EditText mIpAddress;
    Button mSendMessageButton;
    MenuAdapter mAdapter;

    // Setup threading for UDP network packets
    UdpClientHandler mUdpHandler;
    UdpClientThread mUdpSendThread;
    UdpClientThread mUdpReceiveThread;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Setup all hooks back to the UI elements for easy access later
        mIpAddress = (EditText) findViewById(R.id.ip_address);
        mSendMessageButton = (Button) findViewById(R.id.send_message_button);

        // Setup handler to update UI when network packets are received
        mUdpHandler = new UdpClientHandler(mAdapter);

        // When this button is clicked we generate a network packet and spawn a thread
        mSendMessageButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final String ipAddress = mIpAddress.getText().toString();
                if (mUdpReceiveThread == null) {
                    // The receive thread runs in a loop looking for new incoming data
                    mUdpReceiveThread = new UdpClientThread(UdpClientThread.UdpClientState.RECEIVE,
                            ipAddress, mIpPort + 1, mUdpHandler);
                    mUdpReceiveThread.start();
                }

                // The send thread sends a message and the thread ends
                mUdpSendThread = new UdpClientThread(UdpClientThread.UdpClientState.SEND, ipAddress,
                        mIpPort, mUdpHandler);
                mUdpSendThread.start();
            }
        });

        // Create a new adapter and listView that displays the dynamic menu
        mAdapter = new MenuAdapter(this);
        ListView listViewItems = (ListView)findViewById(R.id.dynamic_menu_list);
        listViewItems.setAdapter(mAdapter);

        // Run a packet tester - This code can be removed someday
        CAPacketHelper tester = new CAPacketHelper();
        tester.testPackets();
    }

    /**
     * Internal class to update UI based on messages from UdpClientThread thread.
     */
    public class UdpClientHandler extends Handler {
        static final int UPDATE_MESSAGE = 0;
        static final int UPDATE_PACKET = 1;
        //private MenuAdapter mAdapter;

        private void updateMessage(String msg) {
            Log.e("CA6", msg);
        }

        private void updatePacket(CAPacket.PacketElement packet) {
            mAdapter.addPacket(packet);
        }

        UdpClientHandler(MenuAdapter adapter) {
            super();
            //this.mAdapter = adapter;
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
