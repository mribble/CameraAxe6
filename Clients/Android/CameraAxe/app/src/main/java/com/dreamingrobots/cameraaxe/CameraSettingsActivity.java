package com.dreamingrobots.cameraaxe;

import android.app.ActionBar;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentStatePagerAdapter;
import android.support.v4.view.ViewPager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.app.FragmentTransaction;

public class CameraSettingsActivity extends FragmentActivity {
    CameraSettingsPagerAdapter mCameraSettingsPagerAdapter;
    ViewPager mViewPager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera_settings);

        // Create the adapter that will return a fragment for each of the three primary sections
        // of the app.
        mCameraSettingsPagerAdapter = new CameraSettingsPagerAdapter(getSupportFragmentManager());
        mViewPager = (ViewPager)findViewById(R.id.camera_settings_pager);
        mViewPager.setAdapter(mCameraSettingsPagerAdapter);
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
            args.putInt(CameraSettingsFragment.ARG_OBJECT, i + 1);
            fragment.setArguments(args);
            return fragment;
        }

        @Override
        public int getCount() {
            return 8;
        }

        @Override
        public CharSequence getPageTitle(int position) {
            return "OBJECT " + (position + 1);
        }
    }

    // Instances of this class are fragments representing single camera setting menu
    public static class CameraSettingsFragment extends Fragment {
        public static final String ARG_OBJECT = "object";

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.camera_settings, container, false);
            Bundle args = getArguments();

            ((TextView) rootView.findViewById(R.id.camera_settings_text))
                    .setText(Integer.toString(args.getInt(ARG_OBJECT)));
            return rootView;
        }
    }
}
