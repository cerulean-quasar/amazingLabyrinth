/**
 * Copyright 2020 Cerulean Quasar. All Rights Reserved.
 *
 *  This file is part of AmazingLabyrinth.
 *
 *  AmazingLabyrinth is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  AmazingLabyrinth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AmazingLabyrinth.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
package com.quasar.cerulean.amazinglabyrinth;

import android.app.Activity;
import android.content.Intent;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;

public class ChooseLevelAdaptor extends RecyclerView.Adapter<RecyclerView.ViewHolder> {
    private static final int BUTTON_TYPE = 1;

    private ArrayList<String> m_levels;
    private Activity m_parentActivity;

    private class ChooseLevelViewHolder extends RecyclerView.ViewHolder {
        public Button m_button;
        public ChooseLevelViewHolder(Button button) {
            super(button);
            m_button = button;
        }
    }

    public ChooseLevelAdaptor(Activity activity, ArrayList<String> levels) {
        m_parentActivity = activity;
        m_levels = levels;
    }

    @Override
    public int getItemViewType(int position) {
        return BUTTON_TYPE;
    }

    @Override @NonNull
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        Button button = new Button(parent.getContext());
        TypedValue value = new TypedValue();
        m_parentActivity.getTheme().resolveAttribute(R.attr.button_background, value, true);

        button.setBackground(m_parentActivity.getResources().getDrawable(value.resourceId));
        button.setPadding(20,20,20,20);
        button.setGravity(Gravity.CENTER_HORIZONTAL);
        return new ChooseLevelViewHolder(button);
    }

    @Override
    public int getItemCount() {
        return m_levels.size();
    }

    @Override
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder vh, final int position) {
        ChooseLevelViewHolder clvh = (ChooseLevelViewHolder) vh;
        clvh.m_button.setText(m_levels.get(position));
        clvh.m_button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent();
                intent.putExtra(Constants.KeySelectedLevel, position);
                m_parentActivity.setResult(Activity.RESULT_OK, intent);
                m_parentActivity.finish();
            }
        });
    }

}
