package com.dreamingrobots.cameraaxe;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.Spinner;

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
    Spinner mSpinner;
    LinearLayout mDynamicMenuList;

    private RetainedNetworkFragment mRetainedNetworkFragment;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Setup all hooks back to the UI elements for easy access later
        mIpAddress = (EditText) findViewById(R.id.ip_address);
        mSendMessageButton = (Button) findViewById(R.id.send_message_button);
        mSpinner = (Spinner)findViewById(R.id.spinner_menu_list);
        mDynamicMenuList = (LinearLayout)findViewById(R.id.dynamic_menu_list);

        // When this button is clicked we generate a network packet and spawn a thread
        mSendMessageButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mRetainedNetworkFragment.reset();
                final String ipAddress = mIpAddress.getText().toString();
                mRetainedNetworkFragment.startReceiveThread(ipAddress, mIpPort);

                int packSize;
                CAPacketHelper ph0 = new CAPacketHelper();

                MenuName menuName = (MenuName)mSpinner.getSelectedItem();

                // Only load the menu names the first time.  Then reload the menu every time after that
                if (menuName == null) {
                    packSize = ph0.writePacketMenuList();
                    mRetainedNetworkFragment.sendMessage(ipAddress, mIpPort, ph0, packSize);
                } else {
                    packSize = ph0.writePacketMenuSelect(0, menuName.getMenuId());
                    mRetainedNetworkFragment.sendMessage(ipAddress, mIpPort, ph0, packSize);
                }
            }
        });

        FragmentManager fm = getSupportFragmentManager();
        mRetainedNetworkFragment =
                (RetainedNetworkFragment)fm.findFragmentByTag(TAG_RETAINED_FRAGMENT);
        if (mRetainedNetworkFragment == null) {
            mRetainedNetworkFragment = new RetainedNetworkFragment();
            fm.beginTransaction().add(mRetainedNetworkFragment, TAG_RETAINED_FRAGMENT)
                    .disallowAddToBackStack()
                    .commit();
        }

        // Run a packet tester - This code can be removed someday
        CAPacketHelper tester = new CAPacketHelper();
        tester.testPackets();
    }

    @Override
    public void onBackPressed() {
        Fragment f = getSupportFragmentManager().findFragmentByTag(TAG_RETAINED_FRAGMENT);

        if (f instanceof RetainedNetworkFragment) {
            // do nothing to prevent a bug
        } else {
            super.onBackPressed();
        }
    }

    @Override
    protected void onPostCreate(@Nullable Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        // Create populate the new LinearLayout containing the dynamic menu from retained packet data
        // and reattach a retained adapter to the new listView for the menu name spinner.
        // Must be in postCreate because adapter is created during RetainedNetworkFragment.onCreate

        mRetainedNetworkFragment.setDynamicMenuBuilder(mDynamicMenuList);
        mSpinner.setAdapter(mRetainedNetworkFragment.getMenuListAdapter());
    }
}
