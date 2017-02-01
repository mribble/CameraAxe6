package com.dreamingrobots.cameraaxe;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.Nullable;
import android.support.v4.app.FragmentManager;
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
    private static final String TAG_RETAINED_FRAGMENT = "RetainedFragment";
    EditText mIpAddress;
    Button mSendMessageButton;

    private RetainedNetworkFragment mRetainedNetworkFragment;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Setup all hooks back to the UI elements for easy access later
        mIpAddress = (EditText) findViewById(R.id.ip_address);
        mSendMessageButton = (Button) findViewById(R.id.send_message_button);

        // When this button is clicked we generate a network packet and spawn a thread
        mSendMessageButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final String ipAddress = mIpAddress.getText().toString();
                mRetainedNetworkFragment.startReceiveThread(ipAddress, mIpPort);
                mRetainedNetworkFragment.sendMessage(ipAddress, mIpPort);
            }
        });

        FragmentManager fm = getSupportFragmentManager();
        mRetainedNetworkFragment =
                (RetainedNetworkFragment)fm.findFragmentByTag(TAG_RETAINED_FRAGMENT);
        if (mRetainedNetworkFragment == null) {
            mRetainedNetworkFragment = new RetainedNetworkFragment();
            fm.beginTransaction().add(mRetainedNetworkFragment, TAG_RETAINED_FRAGMENT).commit();
        }

        // Run a packet tester - This code can be removed someday
        CAPacketHelper tester = new CAPacketHelper();
        tester.testPackets();
    }

    @Override
    protected void onPostCreate(@Nullable Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        // Create a new adapter and listView that displays the dynamic menu
        // Must be in postCreate because adapter is created during RetainedNetworkFragment.onCreate
        ListView listViewItems = (ListView)findViewById(R.id.dynamic_menu_list);
        listViewItems.setAdapter(mRetainedNetworkFragment.getAdapter());
    }
}
