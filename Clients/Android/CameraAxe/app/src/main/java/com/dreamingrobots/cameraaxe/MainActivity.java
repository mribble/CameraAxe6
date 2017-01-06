package com.dreamingrobots.cameraaxe;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

/**
 * The main UI activity for the Android Udp application.
 * The major tasks for this activity are:
 * + Handle UI inputs/outputs
 * + Start threads to handle networking
 */
public class MainActivity extends AppCompatActivity {
    private EditText mIpAddress;
    private EditText mIpPort;
    private Button mSendMessageButton;
    private TextView mReturnedMessage;

    // Setup threading for UDP network packets
    UdpClientHandler mUdpClientHandler;
    UdpClientThread mUdpClientThread;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Setup all hooks back to the UI elements for easy access later
        mIpAddress = (EditText) findViewById(R.id.ip_address);
        mIpPort = (EditText) findViewById(R.id.ip_port);
        mSendMessageButton = (Button) findViewById(R.id.send_message_button);
        mReturnedMessage = (TextView) findViewById(R.id.returned_message);

        // When this button is clicked we generate a network packet and spawn a thread
        mSendMessageButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String ipAddress = mIpAddress.getText().toString();
                int ipPort = Integer.parseInt(mIpPort.getText().toString());
                mUdpClientThread = new UdpClientThread(ipAddress, ipPort, mUdpClientHandler);
                mUdpClientThread.start();
            }
        });

        // Setup handler to update UI when network packets are received
        mUdpClientHandler = new UdpClientHandler(this);

        testPackets();
    }

    /**
     * Internal class to update UI based on messages from UdpClientThread thread.
     */
    public static class UdpClientHandler extends Handler {
        public static final int UPDATE_MESSAGE = 0;
        public static final int UPDATE_END = 1;
        private MainActivity mParent;

        private void updateMessage(String msg) {
            mParent.mReturnedMessage.setText(msg);
        }
        private void clientEnd() {
            mParent.mUdpClientThread = null;
        }

        public UdpClientHandler(MainActivity parent) {
            super();
            this.mParent = parent;
        }

        @Override
        public void handleMessage(Message msg) {

            switch (msg.what) {
                case UPDATE_MESSAGE:
                    updateMessage((String) msg.obj);
                    break;
                case UPDATE_END:
                    clientEnd();
                    break;
                default:
                    super.handleMessage(msg);
            }
        }
    }

    protected void testPackets() {
        byte[] dataA = new byte[256];
        CAPacket pack0 = new CAPacket(this, CAPacket.STATE_PACKER, dataA, 256);
        CAPacket unpack0 = new CAPacket(this, CAPacket.STATE_UNPACKER, dataA, 256);

        {   // MenuHeader Packet Test
            CAPacket.MenuHeader pack1 = pack0.new MenuHeader();                 // Update per type
            CAPacket.MenuHeader unpack1 = unpack0.new MenuHeader();             // Update per type
            pack1.set(2, 3, "hello world");                                     // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_MENU_HEADER ||                     // Update per type
                    unpack1.getMajorVersion() != 2 ||
                    unpack1.getMinorVersion() != 3 ||
                    unpack1.getMenuName().equals("hello world") != true) {
                Log.e("CA6", "Packet Test Error - MENU_HEADER test failed");
            }
        }
        {   // NewRow Packet Test
            CAPacket.NewRow pack1 = pack0.new NewRow();                         // Update per type
            CAPacket.NewRow unpack1 = unpack0.new NewRow();                     // Update per type
            //pack1.set();                                                      // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            //unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_NEW_ROW ) {                        // Update per type
                Log.e("CA6", "Packet Test Error - NEW_ROW test failed");
            }
        }
        {   // NewCell Packet Test
            CAPacket.NewCell pack1 = pack0.new NewCell();                       // Update per type
            CAPacket.NewCell unpack1 = unpack0.new NewCell();                   // Update per type
            pack1.set(100, 1);                                                  // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_NEW_CELL ||                        // Update per type
                    unpack1.getColumnPercentage() != 100 ||
                    unpack1.getJustification() != 1 ) {
                Log.e("CA6", "Packet Test Error - NEW_CELL test failed");
            }
        }
        {   // CondStart Packet Test
            CAPacket.CondStart pack1 = pack0.new CondStart();                   // Update per type
            CAPacket.CondStart unpack1 = unpack0.new CondStart();               // Update per type
            pack1.set(2, 1, 1);                                                 // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_COND_START ||                      // Update per type
                    unpack1.getClientHostId() != 2 ||
                    unpack1.getModAttribute() != 1 ||
                    unpack1.getValue() != 1 ) {
                Log.e("CA6", "Packet Test Error - COND_START test failed");
            }
        }
        {   // CondEnd Packet Test
            CAPacket.CondEnd pack1 = pack0.new CondEnd();                       // Update per type
            CAPacket.CondEnd unpack1 = unpack0.new CondEnd();                   // Update per type
            //pack1.set();                                                      // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            //unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_COND_END ) {                       // Update per type
                Log.e("CA6", "Packet Test Error - COND_END test failed");
            }
        }
        {   // TextStatic Packet Test
            CAPacket.TextStatic pack1 = pack0.new TextStatic();                 // Update per type
            CAPacket.TextStatic unpack1 = unpack0.new TextStatic();             // Update per type
            pack1.set("static");                                                // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_TEXT_STATIC ||                     // Update per type
                    unpack1.getText().equals("static") != true) {
                Log.e("CA6", "Packet Test Error - TEXT_STATIC test failed");
            }
        }
        {   // TextDynamic Packet Test
            CAPacket.TextDynamic pack1 = pack0.new TextDynamic();               // Update per type
            CAPacket.TextDynamic unpack1 = unpack0.new TextDynamic();           // Update per type
            pack1.set(245, "Dynamic");                                          // Update per type
            int packSize = pack1.pack();
            int unpackSize = unpack0.unpackSize();
            short packType = unpack0.unpackType();
            unpack1.unpack();
            if (packSize != unpackSize ||
                    packType != CAPacket.PID_TEXT_DYNAMIC ||                    // Update per type
                    unpack1.getClientHostId() != 245 ||
                    unpack1.getText().equals("Dynamic") != true) {
                Log.e("CA6", "Packet Test Error - TEXT_DYNAMIC test failed");
            }
        }

    }
}
