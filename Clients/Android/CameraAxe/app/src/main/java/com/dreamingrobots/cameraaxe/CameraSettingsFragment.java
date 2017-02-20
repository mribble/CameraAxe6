package com.dreamingrobots.cameraaxe;


import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

/**
 * Fragments for camera settings
 */

// Instances of this class are fragments representing single camera setting menu
public class CameraSettingsFragment extends Fragment {
    public static final String ARG_OBJECT = "object";
    public static final String INTENT_BUNDLE = "intentBundle";
    private int mCameraNumber = 0;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.camera_settings, container, false);
        TextView textView = ((TextView) rootView.findViewById(R.id.camera_settings_text));
        Bundle args = getArguments();

        mCameraNumber = args.getInt(ARG_OBJECT);
        textView.setText(Integer.toString(mCameraNumber));
        return rootView;
    }

    public byte[] getCameraSettings() {
        CAPacketHelper ph = new CAPacketHelper();
        int packetSize = ph.writePacketCamSettings(mCameraNumber, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
        byte[] ret = new byte[packetSize];
        for(int i = 0; i < packetSize; i++) {
            ret[i] = ph.getData()[i];
        }
        return ret;
    }

    public int getCameraNumber() {
        return mCameraNumber;
    }

    @Override
    public void onPause() {
        ((CameraSettingsActivity) getActivity()).setCameraData(mCameraNumber, getCameraSettings());
        super.onPause();
    }
}