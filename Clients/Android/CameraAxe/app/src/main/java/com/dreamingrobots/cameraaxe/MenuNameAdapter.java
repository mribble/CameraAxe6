package com.dreamingrobots.cameraaxe;

import android.app.Activity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import java.util.ArrayList;

/**
 * Adapter to list the different menus
 */

public class MenuNameAdapter extends ArrayAdapter<MenuName> {

    public MenuNameAdapter(Activity context, int textViewResourceId, ArrayList<MenuName> menuName) {
        super(context, textViewResourceId, menuName);
    }

    @Override
    public View getDropDownView(int position, View convertView, ViewGroup parent) {
        return getView(position, convertView, parent);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View spinnerItemView = convertView;
        if (spinnerItemView == null) {
            spinnerItemView = LayoutInflater.from(getContext()).inflate(R.layout.menu_name_item, parent, false);
        }

        TextView text = (TextView)spinnerItemView.findViewById(R.id.spinner_text);
        MenuName currentMenuName = getItem(position);
        text.setText(currentMenuName.getMenuName());
        return spinnerItemView;
    }
}
