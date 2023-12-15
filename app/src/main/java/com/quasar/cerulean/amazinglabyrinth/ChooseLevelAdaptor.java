/**
 * Copyright 2023 Cerulean Quasar. All Rights Reserved.
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
import android.util.JsonReader;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import androidx.annotation.NonNull;
import androidx.core.content.res.ResourcesCompat;
import androidx.recyclerview.widget.RecyclerView;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;

public class ChooseLevelAdaptor extends RecyclerView.Adapter<RecyclerView.ViewHolder> {
    private static final int BUTTON_TYPE = 1;
    private static final int m_nbrToLoadAtOnce = 1;

    private class LevelEntry {
        String name;
        String filename;
        String description;

        LevelEntry() {
            name = null;
            filename = null;
            description = null;
        }
    }

    private ArrayList<LevelEntry> m_levels;
    private Activity m_parentActivity;
    private JsonReader m_reader;

    private LevelEntry readEntry() throws IOException {
        LevelEntry le = new LevelEntry();
        m_reader.beginObject();
        while (m_reader.hasNext()) {
            String key = m_reader.nextName();
            if (key.equals("Name")) {
                le.name = m_reader.nextString();
            } else if (key.equals("File")) {
                le.filename = m_reader.nextString();
            } else if (key.equals("Description")) {
                le.description = m_reader.nextString();
            }
        }
        m_reader.endObject();
        return le;
    }

    private class ChooseLevelViewHolder extends RecyclerView.ViewHolder {
        public Button m_button;
        public ChooseLevelViewHolder(Button button) {
            super(button);
            m_button = button;
        }
    }

    public ChooseLevelAdaptor(Activity activity, InputStream inputStream) {
        m_parentActivity = activity;
        m_levels = new ArrayList<>();

        m_reader = new JsonReader(new InputStreamReader(inputStream, StandardCharsets.UTF_8));
        try {
            m_reader.beginObject();
            String key = m_reader.nextName();
            while (!key.equals("Levels")) {
                m_reader.skipValue();
                key = m_reader.nextName();
            }
            m_reader.beginArray();
            while (m_reader.hasNext()) {
                LevelEntry le = readEntry();
                m_levels.add(le);
            }
            m_reader.close();
        } catch (IOException e) {
            try {
                m_reader.close();
            } catch (IOException e2) {
            }
            activity.setResult(Activity.RESULT_CANCELED);
            activity.finish();
        }
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

        button.setBackground(ResourcesCompat.getDrawable(m_parentActivity.getResources(), value.resourceId, m_parentActivity.getTheme()));
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
        int nbrItems = m_levels.size();
        if (vh.getBindingAdapterPosition() < m_levels.size()) {
            clvh.m_button.setText(m_levels.get(position).description);
            clvh.m_button.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Intent intent = new Intent();
                    intent.putExtra(Constants.KeySelectedLevel, m_levels.get(vh.getBindingAdapterPosition()).name);
                    m_parentActivity.setResult(Activity.RESULT_OK, intent);
                    m_parentActivity.finish();
                }
            });
        }
    }
}
