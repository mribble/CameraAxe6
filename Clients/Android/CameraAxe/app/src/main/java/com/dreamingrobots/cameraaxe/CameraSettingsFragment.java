package com.dreamingrobots.cameraaxe;


import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;

import static android.R.attr.duration;
import static android.R.attr.x;

/**
 * Fragments for camera settings
 */

// Instances of this class are fragments representing single camera setting menu
public class CameraSettingsFragment extends Fragment {
    public static final String ARG_OBJECT = "object";
    public static final String INTENT_BUNDLE = "intentBundle";
    public static final int MAX_SEQ = 8;
    private int mCameraNumber = 0;
    Spinner mCamMode;
    EditText mCamDelay;
    EditText mCamDuration;
    CheckBox mCamSeq[];
    CheckBox mApplyIntervalometer;
    EditText mCamSmartPreview;
    CheckBox mCamMirrorEnable;
    EditText mCamMirrorLockup;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.camera_settings, container, false);
        TextView textView = ((TextView) rootView.findViewById(R.id.camera_settings_text));
        Bundle args = getArguments();

        mCameraNumber = args.getInt(ARG_OBJECT);
        textView.setText(Integer.toString(mCameraNumber));

        mCamMode = (Spinner)rootView.findViewById(R.id.cam_mode);
        mCamDelay = (EditText)rootView.findViewById(R.id.cam_delay);
        mCamDuration = (EditText)rootView.findViewById(R.id.cam_duration);
        mCamSeq = new CheckBox[] {(CheckBox)rootView.findViewById(R.id.cam_seq0),
                (CheckBox)rootView.findViewById(R.id.cam_seq1),
                (CheckBox)rootView.findViewById(R.id.cam_seq2),
                (CheckBox)rootView.findViewById(R.id.cam_seq3),
                (CheckBox)rootView.findViewById(R.id.cam_seq4),
                (CheckBox)rootView.findViewById(R.id.cam_seq5),
                (CheckBox)rootView.findViewById(R.id.cam_seq6),
                (CheckBox)rootView.findViewById(R.id.cam_seq7)};
        mApplyIntervalometer = (CheckBox)rootView.findViewById(R.id.apply_intervalometer);
        mCamSmartPreview = (EditText)rootView.findViewById(R.id.cam_smart_preview);
        mCamMirrorEnable = (CheckBox)rootView.findViewById(R.id.cam_mirror_enable);
        mCamMirrorLockup = (EditText)rootView.findViewById(R.id.cam_mirror_lockup);
        return rootView;
    }

    private int getSequencerValue() {
        int val = 0;
        for (int i = 0; i<MAX_SEQ; i++) {
            if (mCamSeq[i].isChecked()) {
                val |= 1<<i;
            }
        }
        return val;
    }
    public byte[] getCameraSettings() {
        int portNumber = mCameraNumber;
        int mode = mCamMode.getSelectedItemPosition();
        int delayHours = 0; //todo get from mCamDelay
        int delayMinutes = 0; //todo get from mCamDelay
        int delaySeconds = 0; //todo get from mCamDelay
        int delayMilliseconds = 0; //todo get from mCamDelay
        int delayMicroseconds = 0; //todo get from mCamDelay
        int durationHours = 0; //todo get from mCamDuration
        int durationMinutes = 0; //todo get from mCamDuration
        int durationSeconds = 0; //todo get from mCamDuration
        int durationMilliseconds = 0; //todo get from mCamDuration
        int durationMicroseconds = 0; //todo get from mCamDuration
        int sequencer = getSequencerValue();
        int applyIntervalometer = (mApplyIntervalometer.isChecked()) ? 1 : 0;
        int smartPreview = Integer.parseInt(mCamSmartPreview.getText().toString());  //todo catch exception
        int mirrorEnable = (mCamMirrorEnable.isChecked()) ? 1 : 0;
        int mirrorMinutes = 0; // todo get from mCamMirrorLockup
        int mirrorSeconds = 0; // todo get from mCamMirrorLockup
        int mirrorMilliseconds = 0; // todo get from mCamMirrorLockup

        CAPacketHelper ph = new CAPacketHelper();
        int packetSize = ph.writePacketCamSettings(mCameraNumber, portNumber, mode, delayHours, delayMinutes,
                delayMilliseconds, delayMicroseconds, durationHours, durationMinutes, durationSeconds,
                durationMilliseconds, durationMicroseconds, sequencer, applyIntervalometer, smartPreview,
                mirrorEnable, mirrorMinutes, mirrorSeconds, mirrorMilliseconds);
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