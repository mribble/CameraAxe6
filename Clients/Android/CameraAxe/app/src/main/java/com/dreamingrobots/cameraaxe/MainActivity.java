package com.dreamingrobots.cameraaxe;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;

/**
 * The main UI activity for the Android Udp application.
 * The major tasks for this activity are:
 * + Handle UI inputs/outputs
 * + Start threads to handle networking
 */
public class MainActivity extends FragmentActivity {
    static final int MAX_CAMERAS = 8;
    static final int CAMERA_SETTINGS_REQUEST = 2;
    static final String CAMERA_SETTING_HANDLE = "CAM_SET_HANDLE";
    static final int mIpPort = 4045;
    private static final String TAG_RETAINED_FRAGMENT = "RetainedFragment";
    EditText mIpAddress;
    Button mSendMessageButton;
    Spinner mSpinner;
    LinearLayout mDynamicMenuList;
    CheckBox mCheckboxPhotoMode;
    Button mCameraSettingsButton;

    private RetainedFragment mRetainedFragment;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Setup all hooks back to the UI elements for easy access later
        mIpAddress = (EditText) findViewById(R.id.ip_address);
        mSendMessageButton = (Button) findViewById(R.id.send_message_button);
        mSpinner = (Spinner)findViewById(R.id.spinner_menu_list);
        mDynamicMenuList = (LinearLayout)findViewById(R.id.dynamic_menu_list);
        mCheckboxPhotoMode = (CheckBox)findViewById(R.id.checkbox_photo_mode);
        mCameraSettingsButton = (Button)findViewById(R.id.camera_settings_button);

        // When this button is clicked we generate a network packet and spawn a thread
        mSendMessageButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                int packSize;
                int menuMode;
                mRetainedFragment.reset();
                final String ipAddress = mIpAddress.getText().toString();
                mRetainedFragment.startReceiveThread(ipAddress, mIpPort);

                CAPacketHelper ph0 = new CAPacketHelper();

                MenuName menuName = (MenuName)mSpinner.getSelectedItem();
                if (mCheckboxPhotoMode.isChecked()) {
                    menuMode = 1;
                } else {
                    menuMode = 0;
                }

                // Only load the menu names the first time.  Then reload the menu every time after that
                if (menuName == null) {
                    packSize = ph0.writePacketMenuList();
                } else {
                    packSize = ph0.writePacketMenuSelect(menuMode, menuName.getMenuId());
                }
                mRetainedFragment.setNetwork(ipAddress, mIpPort);
                mRetainedFragment.sendMessage(ph0, packSize);

            }
        });

        mCameraSettingsButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, CameraSettingsActivity.class);
                startActivityForResult(intent, CAMERA_SETTINGS_REQUEST);
            }
        });

        FragmentManager fm = getSupportFragmentManager();
        mRetainedFragment = (RetainedFragment)fm.findFragmentByTag(TAG_RETAINED_FRAGMENT);
        if (mRetainedFragment== null) {
            mRetainedFragment = new RetainedFragment();
            fm.beginTransaction().add(mRetainedFragment, TAG_RETAINED_FRAGMENT)
                    .disallowAddToBackStack()
                    .commit();
        }

        // Run a packet tester - This code can be removed someday
        CAPacketHelper tester = new CAPacketHelper();
        tester.testPackets();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == CAMERA_SETTINGS_REQUEST && data != null) {
            String m0 = data.getStringExtra(CAMERA_SETTING_HANDLE+0);
            String m1 = data.getStringExtra(CAMERA_SETTING_HANDLE+1);
            String m2 = data.getStringExtra(CAMERA_SETTING_HANDLE+2);
            String m3 = data.getStringExtra(CAMERA_SETTING_HANDLE+3);
            String m4 = data.getStringExtra(CAMERA_SETTING_HANDLE+4);
            String m5 = data.getStringExtra(CAMERA_SETTING_HANDLE+5);
            String m6 = data.getStringExtra(CAMERA_SETTING_HANDLE+6);
            String m7 = data.getStringExtra(CAMERA_SETTING_HANDLE+7);

            Log.e("CA6", m0+m1+m2+m3+m4+m5+m6+m7);
        }
    }

    @Override
    public void onBackPressed() {
        Fragment f = getSupportFragmentManager().findFragmentByTag(TAG_RETAINED_FRAGMENT);

        if (f instanceof RetainedFragment) {
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

        mRetainedFragment.setDynamicMenuBuilder(mDynamicMenuList);
        mSpinner.setAdapter(mRetainedFragment.getMenuListAdapter());
    }
}
