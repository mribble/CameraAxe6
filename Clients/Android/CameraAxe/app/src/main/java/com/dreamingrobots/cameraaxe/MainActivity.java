package com.dreamingrobots.cameraaxe;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.IBinder;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.content.ContextCompat;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import java.io.UnsupportedEncodingException;
import java.text.DateFormat;
import java.util.Date;

/**
 * The main UI activity for the Android Udp application.
 * The major tasks for this activity are:
 * + Handle UI inputs/outputs
 * + Start threads to handle networking
 */
public class MainActivity extends FragmentActivity {
    static final int REQUEST_CAMERA_SETTINGS = 3;
    static final boolean USE_BLE = false;
    static final int MAX_CAMERAS = 8;
    private static final String TAG_RETAINED_FRAGMENT = "RetainedFragment";
    Button mSendMessageButton;
    Spinner mSelectMenuSpinner;
    LinearLayout mDynamicMenuList;
    CheckBox mCheckboxPhotoMode;
    Button mCameraSettingsButton;
    private RetainedFragment mRetainedFragment;

    // Wifi settings
    static final String CAMERA_SETTING_HANDLE = "CAM_SET_HANDLE";
    static final int mIpPort = 4045;
    EditText mIpAddress;

    // Ble settings
    private static final int REQUEST_BLE_SELECT_DEVICE = 1;
    private static final int REQUEST_BLE_ENABLE = 2;
    private static final int BLE_UART_PROFILE_READY = 10;
    private static final int BLE_UART_PROFILE_CONNECTED = 20;
    private static final int BLE_UART_PROFILE_DISCONNECTED = 21;
    Button mBleConnectButton;
    private UartService mBleService = null;
    private BluetoothDevice mBleDevice = null;
    private BluetoothAdapter mBleAdapter = null;
    private int mState = BLE_UART_PROFILE_DISCONNECTED;

    // Coarse position is needed to scan BLE connections
    private void checkCoarsePositionPermission() {
        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.ACCESS_COARSE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, 3);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        if (USE_BLE) {
            checkCoarsePositionPermission();
            mBleAdapter = BluetoothAdapter.getDefaultAdapter();
            if (mBleAdapter == null) {
                showToastMessage("Bluetooth is not available");
                finish();
                return;
            }

            bleServiceInit();
        }


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

        if (USE_BLE) {
            mBleConnectButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (!mBleAdapter.isEnabled()) {
                        Log.i("CA6", "onClick - BT not enabled yet");
                        Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                        startActivityForResult(enableIntent, REQUEST_BLE_ENABLE);
                    } else {
                        if (mBleConnectButton.getText().equals("Connect")) {
                            //Connect button pressed, open DeviceListActivity class with popup windows that scan for devices
                            Intent newIntent = new Intent(MainActivity.this, DeviceListActivity.class);
                            startActivityForResult(newIntent, REQUEST_BLE_SELECT_DEVICE);
                        } else {
                            //Disconnect button pressed
                            if (mBleDevice != null) {
                                mBleService.disconnect();
                            }
                        }
                    }
                }
            });
        }

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
                    // Todo this needs to be made more like the wifi path
                    try {
                        //send data to service
                        String blah = "hello world";
                        byte[] value = blah.getBytes("UTF-8");
                        mBleService.writeRXCharacteristic(value);
                    } catch (UnsupportedEncodingException e) {
                        e.printStackTrace();
                    }
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
            case REQUEST_BLE_SELECT_DEVICE:
                // The DeviceListActivity returns with the selected device address
                if (resultCode == Activity.RESULT_OK && data != null) {
                    String deviceAddress = data.getStringExtra(BluetoothDevice.EXTRA_DEVICE);
                    mBleDevice = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(deviceAddress);
                    mBleService.connect(deviceAddress);
                }
                break;
            case REQUEST_BLE_ENABLE:
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
        if (mState == BLE_UART_PROFILE_CONNECTED) {
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






    private ServiceConnection mBleServiceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder rawBinder) {
            mBleService = ((UartService.LocalBinder) rawBinder).getService();
            Log.i("CA6", "onServiceConnected mService= " + mBleService);
            if (!mBleService.initialize()) {
                Log.e("CA6", "Unable to initialize Ble");
                finish();
            }
        }

        public void onServiceDisconnected(ComponentName classname) {
            mBleService.disconnect();
            mBleService = null;
        }
    };

    private final BroadcastReceiver UARTStatusChangeReceiver = new BroadcastReceiver() {

        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            final Intent mIntent = intent;

            if(action.equals(UartService.ACTION_GATT_CONNECTED)) {
                runOnUiThread(new Runnable() {
                    public void run() {
                        String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                        mBleConnectButton.setText("Disconnect");
                        mState = BLE_UART_PROFILE_CONNECTED;
                    }
                });
            }
            else if(action.equals(UartService.ACTION_GATT_DISCONNECTED)) {
                runOnUiThread(new Runnable() {
                    public void run() {
                        String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                        mBleConnectButton.setText("Connect");
                        mState = BLE_UART_PROFILE_DISCONNECTED;
                        mBleService.close();
                    }
                });
            }
            else if(action.equals(UartService.ACTION_GATT_SERVICES_DISCOVERED)) {
                mBleService.enableTXNotification();
            }
            else if(action.equals(UartService.ACTION_DATA_AVAILABLE)) {
                final byte[] txValue = intent.getByteArrayExtra(UartService.EXTRA_DATA);
                runOnUiThread(new Runnable() {
                    public void run() {
                        try {
                            String text = new String(txValue, "UTF-8");
                            Log.i("CA6", text);
                        } catch (Exception e) {
                            Log.e("CA6", e.toString());
                        }
                    }
                });
            }
            else if(action.equals(UartService.DEVICE_DOES_NOT_SUPPORT_UART)) {
                    showToastMessage("Device doesn't support UART. Disconnecting");
                    mBleService.disconnect();
            }
            else {
                Log.e("CA6", "Invalid statement");
            }
        }
    };

    private void bleServiceInit() {
        Intent bindIntent = new Intent(this, UartService.class);
        bindService(bindIntent, mBleServiceConnection, Context.BIND_AUTO_CREATE);

        LocalBroadcastManager.getInstance(this).registerReceiver(UARTStatusChangeReceiver,
                makeGattUpdateIntentFilter());
    }

    private static IntentFilter makeGattUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(UartService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(UartService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(UartService.ACTION_GATT_SERVICES_DISCOVERED);
        intentFilter.addAction(UartService.ACTION_DATA_AVAILABLE);
        intentFilter.addAction(UartService.DEVICE_DOES_NOT_SUPPORT_UART);
        return intentFilter;
    }

    private void showToastMessage(String msg) {
        Toast.makeText(this, msg, Toast.LENGTH_LONG).show();
    }
}
