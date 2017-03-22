package com.dreamingrobots.cameraaxe;


import android.Manifest;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.IBinder;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.util.Arrays;

import static android.R.attr.data;

/**
 * Help manage different types of connections (BLE, wifi)
 */

public class ConnectionManager {
    MainActivity mActivity;
    boolean mUseBle;
    private DynamicMenuBuilder mDynamicMenuBuilder;
    private CAPacketHelper mPacketHelper;
    private ByteArrayOutputStream mData;
    private MenuNameAdapter mMenuListAdapter;

    // Ble settings
    public static final int REQUEST_BLE_SELECT_DEVICE = 1;
    public static final int REQUEST_BLE_ENABLE = 2;
    private static final int BLE_UART_PROFILE_READY = 10;
    private static final int BLE_UART_PROFILE_CONNECTED = 20;
    private static final int BLE_UART_PROFILE_DISCONNECTED = 21;
    private UartService mBleService = null;
    private BluetoothDevice mBleDevice = null;
    private BluetoothAdapter mBleAdapter = null;
    private int mState = BLE_UART_PROFILE_DISCONNECTED;

    ConnectionManager(MainActivity activity, boolean useBle, DynamicMenuBuilder dynamicMenuBuilder, MenuNameAdapter menuListAdapter) {
        mActivity = activity;
        mUseBle = useBle;
        mDynamicMenuBuilder = dynamicMenuBuilder;
        mMenuListAdapter = menuListAdapter;

        mPacketHelper = new CAPacketHelper();
        mData = new ByteArrayOutputStream();

        if (mUseBle) {
            checkRuntimePermissions();
            mBleAdapter = BluetoothAdapter.getDefaultAdapter();
            if (mBleAdapter == null) {
                Log.e("CA6", "Bluetooth is not available");
                mActivity.finish();
                return;
            }

            bleServiceInit();
        }
    }

    public void toggleConnection() {

        if (mUseBle) {
            if (!mBleAdapter.isEnabled()) {
                Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                mActivity.startActivityForResult(enableIntent, REQUEST_BLE_ENABLE);
            } else {
                if (mState ==BLE_UART_PROFILE_DISCONNECTED) {
                    //Connect button pressed, open DeviceListActivity class with popup windows that scan for devices
                    Intent newIntent = new Intent(mActivity, DeviceListActivity.class);
                    mActivity.startActivityForResult(newIntent, REQUEST_BLE_SELECT_DEVICE);
                } else {
                    //Disconnect button pressed
                    if (mBleDevice != null) {
                        mBleService.disconnect();
                    }
                }
            }
        }

    }

    public boolean isConnected() {
        return (mState == BLE_UART_PROFILE_CONNECTED);
    }
    public void requestSelectDevice(int resultCode, Intent data) {
        // The DeviceListActivity returns with the selected device address
        if (resultCode == Activity.RESULT_OK && data != null) {
            String deviceAddress = data.getStringExtra(BluetoothDevice.EXTRA_DEVICE);
            mBleDevice = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(deviceAddress);
            mBleService.connect(deviceAddress);
        }

    }

    public void sendData(CAPacketHelper ph, int size) {
        byte[] subset = Arrays.copyOfRange(ph.getData(), 0, size);
        mBleService.writeRXCharacteristic(subset);
    }

    private ServiceConnection mBleServiceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder rawBinder) {
            mBleService = ((UartService.LocalBinder) rawBinder).getService();
            Log.i("CA6", "onServiceConnected mService= " + mBleService);
            if (!mBleService.initialize()) {
                Log.e("CA6", "Unable to initialize Ble");
                mActivity.finish();
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

            if (action.equals(UartService.ACTION_GATT_CONNECTED)) {
                mActivity.runOnUiThread(new Runnable() {
                    public void run() {
                        mActivity.setConnectionUi(true);
                        mState = BLE_UART_PROFILE_CONNECTED;
                    }
                });
            } else if (action.equals(UartService.ACTION_GATT_DISCONNECTED)) {
                mActivity.runOnUiThread(new Runnable() {
                    public void run() {
                        mActivity.setConnectionUi(false);
                        mState = BLE_UART_PROFILE_DISCONNECTED;
                        mBleService.close();
                    }
                });
            } else if (action.equals(UartService.ACTION_GATT_SERVICES_DISCOVERED)) {
                mBleService.enableTXNotification();
            } else if (action.equals(UartService.ACTION_DATA_AVAILABLE)) {
                final byte[] txValue = intent.getByteArrayExtra(UartService.EXTRA_DATA);
                mActivity.runOnUiThread(new Runnable() {
                    public void run() {
                        try {
                            mData.write(txValue);
                            CAPacket.PacketElement packet = mPacketHelper.processIncomingPacket(mData.toByteArray(), mData.size());
                            if (packet != null) {
                                if (packet.getPacketType() == CAPacket.PID_MENU_LIST) {
                                    CAPacket.MenuList p = (CAPacket.MenuList) packet;
                                    MenuName mn = new MenuName(p.getMenuId(), p.getModuleId0(), p.getModuleMask0(),
                                            p.getModuleId1(), p.getModuleMask1(),
                                            p.getModuleId2(), p.getModuleMask2(),
                                            p.getModuleId3(), p.getModuleMask3(),
                                            p.getModuleTypeId0(), p.getModuleTypeMask0(),
                                            p.getModuleTypeId1(), p.getModuleTypeMask1(), p.getMenuName());
                                    mMenuListAdapter.add(mn);
                                } else {
                                    mDynamicMenuBuilder.addPacket(packet);
                                }

                                mData.reset();
                            }
                        } catch (Exception e) {
                            Log.e("CA6", e.toString());
                        }
                    }
                });
            } else if (action.equals(UartService.DEVICE_DOES_NOT_SUPPORT_UART)) {
                Log.e("CA6", "Device doesn't support UART. Disconnecting");
                mBleService.disconnect();
            } else {
                Log.e("CA6", "Invalid statement");
            }
        }
    };

    private void bleServiceInit() {
        Intent bindIntent = new Intent(mActivity, UartService.class);
        mActivity.bindService(bindIntent, mBleServiceConnection, Context.BIND_AUTO_CREATE);

        LocalBroadcastManager.getInstance(mActivity).registerReceiver(UARTStatusChangeReceiver,
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

    // Coarse position is needed to scan BLE connections
    private void checkRuntimePermissions() {
        if (ContextCompat.checkSelfPermission(mActivity, Manifest.permission.ACCESS_COARSE_LOCATION) !=
                PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(mActivity, new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, 3);
        }
    }
}
