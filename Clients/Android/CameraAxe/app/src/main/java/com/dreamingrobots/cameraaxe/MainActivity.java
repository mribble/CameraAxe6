package com.dreamingrobots.cameraaxe;


import android.app.Activity;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v7.app.AlertDialog;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

/**
 * The main UI activity for the Android Udp application.
 * The major tasks for this activity are:
 * + Handle UI inputs/outputs
 * + Start threads to handle networking
 */
public class MainActivity extends FragmentActivity {
    static final int REQUEST_CAMERA_SETTINGS = 3;
    static final boolean USE_BLE = true;
    static final int MAX_CAMERAS = 8;
    private static final String TAG_RETAINED_FRAGMENT = "RetainedFragment";
    Button mSendMessageButton;
    Spinner mSelectMenuSpinner;
    LinearLayout mDynamicMenuList;
    CheckBox mCheckboxPhotoMode;
    Button mCameraSettingsButton;
    private RetainedFragment mRetainedFragment;
    private ConnectionManager mConnectionManager;

    Button mBleConnectButton;

    // Wifi settings
    static final String CAMERA_SETTING_HANDLE = "CAM_SET_HANDLE";
    static final int mIpPort = 4045;
    EditText mIpAddress;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Setup all hooks back to the UI elements for easy access later
        mBleConnectButton = (Button) findViewById(R.id.ble_connect_button);
        mIpAddress = (EditText) findViewById(R.id.ip_address);
        mSendMessageButton = (Button) findViewById(R.id.send_message_button);
        mSelectMenuSpinner = (Spinner) findViewById(R.id.spinner_menu_list);
        mDynamicMenuList = (LinearLayout) findViewById(R.id.dynamic_menu_list);
        mCheckboxPhotoMode = (CheckBox) findViewById(R.id.checkbox_photo_mode);
        mCameraSettingsButton = (Button) findViewById(R.id.camera_settings_button);
        // Hide the UI elements not needed for the current connection mode
        if (USE_BLE) {
            TextView textView = (TextView) findViewById(R.id.ip_address_text);
            textView.setVisibility(View.GONE);
            mIpAddress.setVisibility(View.GONE);
        } else {
            TextView textView = (TextView) findViewById(R.id.ble_connect_button_text);
            textView.setVisibility(View.GONE);
            mBleConnectButton.setVisibility(View.GONE);
        }

        mConnectionManager = new ConnectionManager(this, USE_BLE, mBleConnectButton);

        // When this button is clicked we generate a network packet
        mSendMessageButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                int packSize;
                int menuMode;
                mRetainedFragment.reset();

                MenuName menuName = (MenuName) mSelectMenuSpinner.getSelectedItem();
                if (mCheckboxPhotoMode.isChecked()) {
                    menuMode = 1;
                } else {
                    menuMode = 0;
                }

                // Only load the menu names the first time.  Then reload the menu every time after that
                CAPacketHelper ph = new CAPacketHelper();
                if (menuName == null) {
                    packSize = ph.writePacketMenuList();
                } else {
                    packSize = ph.writePacketMenuSelect(menuMode, menuName.getMenuId());
                }

                if (USE_BLE) {
                    mConnectionManager.sendData();
                } else {
                    final String ipAddress = mIpAddress.getText().toString();
                    mRetainedFragment.startReceiveThread(ipAddress, mIpPort);
                    mRetainedFragment.setNetwork(ipAddress, mIpPort);
                }
                mRetainedFragment.sendMessage(ph, packSize);
            }
        });

        mCameraSettingsButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, CameraSettingsActivity.class);
                startActivityForResult(intent, REQUEST_CAMERA_SETTINGS);
            }
        });

        // Reuse the retained fragment if it exists
        FragmentManager fm = getSupportFragmentManager();
        mRetainedFragment = (RetainedFragment) fm.findFragmentByTag(TAG_RETAINED_FRAGMENT);
        if (mRetainedFragment == null) {
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

        switch (requestCode) {
            case ConnectionManager.REQUEST_BLE_SELECT_DEVICE:
                mConnectionManager.requestSelectDevice(resultCode, data);

                break;
            case ConnectionManager.REQUEST_BLE_ENABLE:
                //  The request to enable Ble returns
                if (resultCode == Activity.RESULT_OK) {
                    showToastMessage("Bluetooth has turned on ");

                } else {
                    // User did not enable Bluetooth or an error occurred
                    Log.e("CA6", "BT not enabled");
                    showToastMessage("Problem in BT Turning ON ");
                    finish();
                }
                break;
            case REQUEST_CAMERA_SETTINGS:
                // The CameraSettingActivity returns camera settings
                if (resultCode == Activity.RESULT_OK && data != null) {
                    CAPacketHelper ph = new CAPacketHelper();
                    for (int i = 0; i < MainActivity.MAX_CAMERAS; i++) {
                        byte[] m = data.getByteArrayExtra(CAMERA_SETTING_HANDLE + i);
                        if (m != null) {
                            ph.setData(m);
                            mRetainedFragment.sendMessage(ph, m.length);
                        }
                    }
                }
                break;
            default:
                Log.e("CA6", "Wrong request code");
                break;
        }
    }

    @Override
    public void onBackPressed() {
        if (mConnectionManager.isConnected()) {
            Intent startMain = new Intent(Intent.ACTION_MAIN);
            startMain.addCategory(Intent.CATEGORY_HOME);
            startMain.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(startMain);
            showToastMessage("CA6's running in background.\n             Disconnect to exit");
        }
        else {
            new AlertDialog.Builder(this)
                    .setIcon(android.R.drawable.ic_dialog_alert)
                    .setTitle(R.string.popup_title)
                    .setMessage(R.string.popup_message)
                    .setPositiveButton(R.string.popup_yes, new DialogInterface.OnClickListener()
                    {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    })
                    .setNegativeButton(R.string.popup_no, null)
                    .show();
        }
    }

    @Override
    protected void onPostCreate(@Nullable Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        // Create populate the new LinearLayout containing the dynamic menu from retained packet data
        // and reattach a retained adapter to the new listView for the menu name spinner.
        // Must be in postCreate because adapter is created during RetainedNetworkFragment.onCreate

        mRetainedFragment.setDynamicMenuBuilder(mDynamicMenuList);
        mSelectMenuSpinner.setAdapter(mRetainedFragment.getMenuListAdapter());
    }

    private void showToastMessage(String msg) {
        Toast.makeText(this, msg, Toast.LENGTH_LONG).show();
    }
}
