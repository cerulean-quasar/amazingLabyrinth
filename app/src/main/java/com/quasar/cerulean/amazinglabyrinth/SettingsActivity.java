/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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
import androidx.core.content.res.ResourcesCompat;
import androidx.databinding.DataBindingUtil;

import android.content.Intent;
import android.content.res.Resources;
import android.os.Bundle;
import android.util.TypedValue;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;

import com.quasar.cerulean.amazinglabyrinth.databinding.ActivitySettingsBinding;

public class SettingsActivity extends AppCompatActivity {
    private Settings m_settings;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // setup layout Data binding
        ActivitySettingsBinding binding =  DataBindingUtil.setContentView(this, R.layout.activity_settings);
        m_settings = new Settings(getIntent());
        binding.setSettings(m_settings);

        setTitle("");

        // Set the background to be the same as the main app.
        TypedValue value = new TypedValue();
        Resources.Theme theme = getTheme();
        theme.resolveAttribute(R.attr.app_name, value, true);

        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setBackgroundDrawable(ResourcesCompat.getDrawable(getResources(), value.resourceId, theme));
            // actionBar.setBackgroundDrawable(getResources().getDrawable(value.resourceId));
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.settings_menu, menu);
        return true;
    }

    public void onSaveSettings(MenuItem item) {
        if (m_settings.haveChangesBeenMade()) {
            Intent intent = new Intent();
            m_settings.addToIntent(intent);
            setResult(RESULT_OK, intent);
        } else {
            setResult(RESULT_CANCELED);
        }
        finish();
    }

    public void onCancelSettings(MenuItem item) {
        setResult(RESULT_CANCELED);
        finish();
    }
}