package com.dreamingrobots.cameraaxe;


import android.support.v4.app.FragmentManager;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentStatePagerAdapter;
import android.support.v4.view.ViewPager;

import java.util.List;

public class CameraSettingsActivity extends FragmentActivity {
    CameraSettingsPagerAdapter mCameraSettingsPagerAdapter;
    ViewPager mViewPager;
    String[] mData = new String[MainActivity.MAX_CAMERAS];

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera_settings);

        // Create the adapter that will return a fragment for each of the three primary sections of the app.
        mCameraSettingsPagerAdapter = new CameraSettingsPagerAdapter(getSupportFragmentManager());
        mViewPager = (ViewPager) findViewById(R.id.camera_settings_pager);
        mViewPager.setAdapter(mCameraSettingsPagerAdapter);
    }

    public void setCameraData(int index, String data) {
        mData[index] = data;
    }

    @Override
    public void onBackPressed() {
        // Save any fragments that didn't get saved during onPause
        List<Fragment> fragments = getSupportFragmentManager().getFragments();
        for (int j = 0; j < fragments.size(); j++) {
            if (fragments.get(j) != null) {
                int val = ((CameraSettingsFragment) fragments.get(j)).getCameraNumber();
                mData[val] = "Robin" + val;
            }
        }

        // Send all the data back to the parent activity
        Intent intent = new Intent();
        for (int i = 0; i < MainActivity.MAX_CAMERAS; i++) {
            intent.putExtra(MainActivity.CAMERA_SETTING_HANDLE + i, mData[i]);
        }
        setResult(MainActivity.CAMERA_SETTINGS_REQUEST, intent);
        super.onBackPressed();
    }

    // Since this is an object collection, use a FragmentStatePagerAdapter, and not a FragmentPagerAdapter.
    public class CameraSettingsPagerAdapter extends FragmentStatePagerAdapter {
        public CameraSettingsPagerAdapter(FragmentManager fm) {
            super(fm);
        }

        @Override
        public Fragment getItem(int i) {
            Fragment fragment = new CameraSettingsFragment();
            Bundle args = new Bundle();
            args.putInt(CameraSettingsFragment.ARG_OBJECT, i);
            fragment.setArguments(args);
            return fragment;
        }

        @Override
        public int getCount() {
            return MainActivity.MAX_CAMERAS;
        }
    }
}
