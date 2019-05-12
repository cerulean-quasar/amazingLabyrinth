/**
 * Copyright 2018 Cerulean Quasar. All Rights Reserved.
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

import android.content.Intent;
import android.content.res.AssetManager;
import android.graphics.PixelFormat;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;

import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity implements AdapterView.OnItemSelectedListener {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        /* Keep the screen on even though the user is not tapping it. */
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.activity_main);

        SurfaceView drawSurfaceView = findViewById(R.id.mainDrawingSurface);
        drawSurfaceView.setZOrderOnTop(true);
        SurfaceHolder drawSurfaceHolder = drawSurfaceView.getHolder();
        drawSurfaceHolder.setFormat(PixelFormat.TRANSPARENT);
        drawSurfaceHolder.addCallback(new MySurfaceCallback(this, savedInstanceState));
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.main_menu, menu);
        return true;
    }

    public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
        /* do nothing */
    }

    public void onNothingSelected(AdapterView<?> parent) {
        /* do nothing */
    }

    public void onSelectLevel(MenuItem item) {
        LinearLayout layout = findViewById(R.id.mainLayout);
        LayoutInflater inflater = getLayoutInflater();

        LinearLayout selectLevelLayout = (LinearLayout) inflater.inflate(R.layout.select_level_dialog,
                layout, false);
        Spinner spinner = selectLevelLayout.findViewById(R.id.select_level_spinner);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, Draw.levelList());
        // Specify the layout to use when the list of choices appears
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        // Apply the adapter to the spinner
        spinner.setAdapter(adapter);
        Button okButton = selectLevelLayout.findViewById(R.id.select_level_ok);
        Button cancelButton = selectLevelLayout.findViewById(R.id.select_level_cancel);
        final AlertDialog selectLevelDialog = new AlertDialog.Builder(this).
                setTitle(R.string.select_level).setView(selectLevelLayout).show();
        okButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Spinner spinner = selectLevelDialog.findViewById(R.id.select_level_spinner);
                int pos = spinner.getSelectedItemPosition();
                if (pos == Spinner.INVALID_POSITION) {
                    selectLevelDialog.dismiss();
                } else {
                    Draw.switchLevel(pos);
                }
                selectLevelDialog.dismiss();
            }
        });

        cancelButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                selectLevelDialog.dismiss();
            }
        });
    }

    public void publishError(String err) {
        if (err != null && err.length() > 0) {
            LinearLayout layout = findViewById(R.id.mainLayout);
            LayoutInflater inflater = getLayoutInflater();

            LinearLayout msgLayout = (LinearLayout) inflater.inflate(R.layout.message_dialog,
                    layout, false);
            TextView errMsg = msgLayout.findViewById(R.id.message);
            errMsg.setText(err);
            Button okButton = msgLayout.findViewById(R.id.okButton);
            final AlertDialog dialog = new AlertDialog.Builder(this).
                    setTitle(R.string.select_level).setView(msgLayout).show();
            okButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    dialog.dismiss();
                }
            });
        }
    }
}
