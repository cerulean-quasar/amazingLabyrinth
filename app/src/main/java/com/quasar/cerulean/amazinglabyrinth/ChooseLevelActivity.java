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

import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.os.Bundle;
import android.util.TypedValue;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

import java.util.ArrayList;

public class ChooseLevelActivity extends AppCompatActivity {
    RecyclerView m_recycler;
    RecyclerView.LayoutManager m_layoutManager;
    ChooseLevelAdaptor m_adapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_choose_level);
        ArrayList<String> levels = getIntent().getStringArrayListExtra(Constants.KeyLevels);
        if (levels == null) {
            setResult(RESULT_CANCELED);
            finish();
            return;
        }

        TypedValue value = new TypedValue();
        getTheme().resolveAttribute(R.attr.app_name, value, true);

        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setBackgroundDrawable(getResources().getDrawable(value.resourceId));
        }

        m_recycler = findViewById(R.id.recycler);

        m_layoutManager = new LinearLayoutManager(this);

        m_adapter = new ChooseLevelAdaptor(this, levels);

        m_recycler.setHasFixedSize(false);
        m_recycler.setLayoutManager(m_layoutManager);
        m_recycler.setAdapter(m_adapter);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.choose_level_menu, menu);
        return true;
    }

    public void onCancel(MenuItem item) {
        setResult(RESULT_CANCELED);
        finish();
    }
}
