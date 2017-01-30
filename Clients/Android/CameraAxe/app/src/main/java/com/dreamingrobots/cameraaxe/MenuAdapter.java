package com.dreamingrobots.cameraaxe;

import android.app.Activity;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.ArrayList;

import static java.lang.Integer.parseInt;

/**
 * Adapter for dynamic menu generation
 */

public class MenuAdapter extends BaseAdapter{

    private ArrayList<CAPacket.PacketElement> mData = new ArrayList<CAPacket.PacketElement>();
    private Activity mActivity;
    private LayoutInflater mInflater;

    public MenuAdapter(Activity activity) {
        mActivity = activity;
        mInflater =  mActivity.getLayoutInflater();
    }

    public void addPacket(CAPacket.PacketElement packet) {
        if (packet.getPacketType() == CAPacket.PID_MENU_HEADER ||
                packet.getPacketType() == CAPacket.PID_TEXT_STATIC ||
                packet.getPacketType() == CAPacket.PID_EDIT_NUMBER) {
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
            case CAPacket.PID_MENU_HEADER: {
                if (v == null) {
                    v = (LinearLayout) mInflater.inflate(R.layout.dm_menu_header, parent, false);
                }
                TextView menuHeader = (TextView) v.findViewById(R.id.menu_header);
                CAPacket.MenuHeader p = (CAPacket.MenuHeader)mData.get(position);
                menuHeader.setText(p.getMenuName()+" "+p.getMajorVersion()+"."+p.getMajorVersion());
                break;
            }
            case CAPacket.PID_TEXT_STATIC: {
                if (v == null) {
                    v = (LinearLayout) mInflater.inflate(R.layout.dm_text_static, parent, false);
                }
                TextView text0 = (TextView) v.findViewById(R.id.text0);
                CAPacket.TextStatic p = (CAPacket.TextStatic)mData.get(position);
                text0.setText(p.getText0());
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
            case CAPacket.PID_EDIT_NUMBER: {
                final ViewHolder holder;
                if (v == null) {
                    v = (LinearLayout) mInflater.inflate(R.layout.dm_edit_number, null);
                    holder = new ViewHolder();
                    holder.textView = (TextView) v.findViewById(R.id.text0);
                    holder.editText = (EditText) v.findViewById(R.id.value);
                    v.setTag(holder);
                } else {
                    holder = (ViewHolder)v.getTag();
                }
                holder.position = position;

                //we need to update adapter once we finish with editing
                holder.editText.addTextChangedListener(new TextWatcher() {

                    @Override
                    public void onTextChanged(CharSequence arg0, int arg1, int arg2, int arg3) {
                    }

                    @Override
                    public void beforeTextChanged(CharSequence arg0, int arg1, int arg2,
                                                  int arg3) {
                    }

                    @Override
                    public void afterTextChanged(Editable arg0) {
                        final int position = holder.position;
                        int val;
                        try {
                            val = Integer.parseInt(arg0.toString());
                        } catch (NumberFormatException e) {
                            val = 0;
                        }
                        CAPacket.EditNumber p = (CAPacket.EditNumber) mData.get(position);
                        p.set(p.getClientHostId(), p.getModAttribute(),
                                p.getDigitsBeforeDecimal(), p.getDigitsAfterDecimal(),
                                p.getMinValue(), p.getMaxValue(),
                                val, p.getText0());
                    }
                });

                CAPacket.EditNumber p = (CAPacket.EditNumber) mData.get(position);
                holder.textView.setText(p.getText0());
                holder.editText.setText(Long.toString(p.getValue()));
                holder.editText.setId(position);
                break;
            }
            case CAPacket.PID_TIME_BOX:
                break;
            case CAPacket.PID_SCRIPT_END:
                break;
            default:
                break;
        }
        return v;
    }
    public static class ViewHolder {
        public TextView textView;
        public EditText editText;
        public int position;
    }

}
