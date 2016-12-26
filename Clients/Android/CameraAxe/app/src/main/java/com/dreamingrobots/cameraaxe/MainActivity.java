package com.dreamingrobots.cameraaxe;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
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
}
