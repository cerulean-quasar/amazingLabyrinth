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

import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.TypedValue;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.ArrayList;

public class AboutActivity extends AppCompatActivity {
    private static final String path = "license/";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_about);

        setTitle("");

        TypedValue value = new TypedValue();
        getTheme().resolveAttribute(R.attr.app_name, value, true);

        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setBackgroundDrawable(getResources().getDrawable(value.resourceId));
        }

        Intent intent = getIntent();

        String text;
        try {
            PackageInfo info = getPackageManager().getPackageInfo(getPackageName(), 0);
            text = info.versionName;
        } catch (PackageManager.NameNotFoundException e) {
            text = getString(R.string.unknown);
        }
        TextView view = findViewById(R.id.gameVersion);
        view.setText(text);

        text = intent.getStringExtra(Constants.KeyGraphicsName);
        view = findViewById(R.id.graphicsAPI);
        view.setText(text);

        text = intent.getStringExtra(Constants.KeyVersionName);
        view = findViewById(R.id.graphicsAPIVersion);
        view.setText(text);

        text = intent.getStringExtra(Constants.KeyDeviceName);
        view = findViewById(R.id.graphicsDriverInfo);
        view.setText(text);

        ArrayList<String> infos = intent.getStringArrayListExtra(Constants.KeyBugInfo);
        if (infos == null) {
            LinearLayout infosLayout = findViewById(R.id.listOfIssues);
            view = new TextView(this);
            view.setText(R.string.none);
            infosLayout.addView(view);
        } else {
            LinearLayout infosLayout = findViewById(R.id.listOfIssues);
            for (String info : infos) {
                view = new TextView(this);
                view.setText(info);
                infosLayout.addView(view);
            }
        }

        boolean hasAccelerometer = intent.getBooleanExtra(Constants.KeyHasAccelerometer, false);
        view = findViewById(R.id.hasAccelerometer);
        if (hasAccelerometer) {
            view.setText(R.string.yes);
        } else {
            view.setText(R.string.no);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.about_menu, menu);
        return true;
    }

    public void onDone(MenuItem item) {
        setResult(RESULT_OK);
        finish();
    }

    private void loadLicense(String filename) {
        Intent intent = new Intent(this, LicenseViewerActivity.class);
        intent.putExtra(Constants.KeyLicenseFile, filename);
        startActivityForResult(intent, Constants.AMAZING_LABYRINTH_LICENSE_ACTIVITY);
    }

    public void onMitGlm(View v) {
        loadLicense(path + "mitGlm.txt");
    }

    public void onMitJsonForModernCpp(View v) {
        loadLicense(path + "mitJsonForModernCpp.txt");
    }

    public void onMitTinyObjLoader(View v) {
        loadLicense(path + "mitTinyObjLoader.txt");
    }

    public void onMitStb(View v) {
        loadLicense(path + "mitStb.txt");
    }

    public void onGnu3(View v) {
        loadLicense(path + "gpl3.txt");
    }

    public void onBoost(View v) {
        loadLicense(path + "boost.txt");
    }
}
