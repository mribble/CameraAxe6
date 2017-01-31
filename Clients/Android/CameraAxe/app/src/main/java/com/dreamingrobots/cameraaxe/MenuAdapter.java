package com.dreamingrobots.cameraaxe;

import android.app.Activity;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

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
        if (packet.getPacketType() >= CAPacket.PID_MENU_HEADER &&
                packet.getPacketType() <= CAPacket.PID_TIME_BOX) {
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
                menuHeader.setText(p.getMenuName()+" "+p.getMajorVersion()+"."+p.getMinorVersion());
                break;
            }
            case CAPacket.PID_TEXT_STATIC: {
                final ViewHolder holder;
                if (v == null) {
                    v = (LinearLayout) mInflater.inflate(R.layout.dm_text_static, parent, false);
                    holder = new ViewHolder();
                    holder.textView0 = (TextView) v.findViewById(R.id.text0);
                    v.setTag(holder);
                } else {
                    holder = (ViewHolder)v.getTag();
                }
                CAPacket.TextStatic p = (CAPacket.TextStatic)mData.get(position);
                holder.textView0.setText(p.getText0());
                break;
            }
            case CAPacket.PID_TEXT_DYNAMIC: {
                final ViewHolder holder;
                if (v == null) {
                    v = (LinearLayout) mInflater.inflate(R.layout.dm_text_dynamic, parent, false);
                    holder = new ViewHolder();
                    holder.textView0 = (TextView) v.findViewById(R.id.text0);
                    holder.textView1 = (TextView) v.findViewById(R.id.text1);
                    v.setTag(holder);
                } else {
                    holder = (ViewHolder)v.getTag();
                }
                CAPacket.TextDynamic p = (CAPacket.TextDynamic)mData.get(position);
                holder.textView0.setText(p.getText0());
                holder.textView1.setText(p.getText1());
                break;
            }
            case CAPacket.PID_BUTTON: {
                final ViewHolder holder;
                if (v == null) {
                    v = (LinearLayout) mInflater.inflate(R.layout.dm_button, parent, false);
                    holder = new ViewHolder();
                    holder.textView0 = (TextView) v.findViewById(R.id.text0);
                    holder.button = (Button) v.findViewById(R.id.button);
                    v.setTag(holder);
                } else {
                    holder = (ViewHolder)v.getTag();
                }
                CAPacket.Button p = (CAPacket.Button)mData.get(position);
                holder.textView0.setText(p.getText0());
                holder.button.setText(p.getText1());
                break;
            }
            case CAPacket.PID_CHECK_BOX: {
                final ViewHolder holder;
                if (v == null) {
                    v = (LinearLayout) mInflater.inflate(R.layout.dm_check_box, parent, false);
                    holder = new ViewHolder();
                    holder.textView0 = (TextView) v.findViewById(R.id.text0);
                    holder.checkBox = (CheckBox) v.findViewById(R.id.check_box);
                    v.setTag(holder);
                } else {
                    holder = (ViewHolder)v.getTag();
                }
                CAPacket.CheckBox p = (CAPacket.CheckBox)mData.get(position);
                holder.textView0.setText(p.getText0());
                holder.checkBox.setChecked(p.getValue() == 1);
                break;
        }
            case CAPacket.PID_DROP_SELECT: {
                final ViewHolder holder;
                if (v == null) {
                    v = (LinearLayout) mInflater.inflate(R.layout.dm_drop_select, parent, false);
                    holder = new ViewHolder();
                    holder.textView0 = (TextView) v.findViewById(R.id.text0);
                    holder.dropSelect = (Spinner) v.findViewById(R.id.drop_select);
                    v.setTag(holder);
                } else {
                    holder = (ViewHolder)v.getTag();
                }
                CAPacket.DropSelect p = (CAPacket.DropSelect)mData.get(position);
                holder.textView0.setText(p.getText0());
                String[] items = p.getText1().split("\\|", -1);
                ArrayAdapter<String> adapter = new ArrayAdapter<String>(mActivity, android.R.layout.simple_spinner_item, items);
                holder.dropSelect.setAdapter(adapter);
                break;
            }
            case CAPacket.PID_EDIT_NUMBER: {
                final ViewHolder holder;
                if (v == null) {
                    v = (LinearLayout) mInflater.inflate(R.layout.dm_edit_number, parent, false);
                    holder = new ViewHolder();
                    holder.textView0 = (TextView) v.findViewById(R.id.text0);
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
                holder.textView0.setText(p.getText0());
                holder.editText.setText(Long.toString(p.getValue()));
                holder.editText.setId(position);
                break;
            }
            case CAPacket.PID_TIME_BOX: {
                final ViewHolder holder;
                if (v == null) {
                    v = (LinearLayout) mInflater.inflate(R.layout.dm_time_box, parent, false);
                    holder = new ViewHolder();
                    holder.textView0 = (TextView) v.findViewById(R.id.text0);
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
                        String val;
                        CAPacket.TimeBox p = (CAPacket.TimeBox) mData.get(position);
                        /*p.set(p.getClientHostId(), p.getModAttribute(),
                                p.getEnableMask(), p.getHours(), p.getMinutes(),
                                p.getSeconds(), p.getMilliseconds(), p.getMicroseconds(),
                                p.getNanoseconds(), p.getText0());*/
                    }
                });

                CAPacket.TimeBox p = (CAPacket.TimeBox) mData.get(position);
                holder.textView0.setText(p.getText0());
                String str = Integer.toString(p.getHours()) + "." +
                        Integer.toString(p.getMinutes()) + "." +
                        Integer.toString(p.getSeconds()) + "." +
                        Integer.toString(p.getMilliseconds()) + "." +
                        Integer.toString(p.getMicroseconds()) + "." +
                        Integer.toString(p.getNanoseconds());
                holder.editText.setText(str);
                holder.editText.setId(position);
                break;
            }
            default:
                Log.e("CA6", "Invalid token type");
                break;
        }
        return v;
    }
    public static class ViewHolder {
        public TextView textView0;
        public TextView textView1;
        public EditText editText;
        public Button button;
        public CheckBox checkBox;
        public Spinner dropSelect;
        public int position;
    }

}
