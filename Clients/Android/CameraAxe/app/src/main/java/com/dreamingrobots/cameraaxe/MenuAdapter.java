package com.dreamingrobots.cameraaxe;

import android.app.Activity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.ArrayList;

/**
 * Created by oe on 1/23/2017.
 */

public class MenuAdapter extends BaseAdapter{

    private ArrayList<CAPacket.PacketElement> mData = new ArrayList<CAPacket.PacketElement>();

    private Activity mActivity;
    private LayoutInflater mInflater;

    public MenuAdapter(Activity activity) {
        mActivity = activity;
        mInflater = mActivity.getLayoutInflater();
    }

    public void addPacket(CAPacket.PacketElement packet) {
        if (packet.getPacketType() == CAPacket.PID_TEXT_STATIC) { // Only supported packet right now
            mData.add(packet);
            notifyDataSetChanged();
        }
    }

    @Override
    public int getItemViewType(int position) {
        return mData.get(position).getPacketType();
    }

    @Override
    public int getViewTypeCount() {
        return CAPacket.PID_SCRIPT_END+1;
    }

    @Override
    public int getCount() {
        return mData.size();
    }

    @Override
    public Object getItem(int position) {
        return mData.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        int type = getItemViewType(position);
        LinearLayout v = (LinearLayout) convertView;

        switch(type) {
            case CAPacket.PID_MENU_HEADER:
                break;
            case CAPacket.PID_TEXT_STATIC: {
                if (v == null) {
                    v = (LinearLayout) mInflater.inflate(R.layout.dm_text_static, null);
                }
                TextView tView = (TextView) v.findViewById(R.id.dml_text_static);
                CAPacket.TextStatic p = (CAPacket.TextStatic)mData.get(position);
                tView.setText(p.getText());
                break;
            }
            case CAPacket.PID_TEXT_DYNAMIC:
                break;
            case CAPacket.PID_BUTTON:
                break;
            case CAPacket.PID_CHECK_BOX:
                break;
            case CAPacket.PID_DROP_SELECT:
                break;
            case CAPacket.PID_EDIT_NUMBER:
                v = (LinearLayout) mInflater.inflate(R.layout.dm_edit_number, null);
                break;
            case CAPacket.PID_TIME_BOX:
                break;
            case CAPacket.PID_SCRIPT_END:
                break;
            default:
                break;
        }
        return v;
    }
}
