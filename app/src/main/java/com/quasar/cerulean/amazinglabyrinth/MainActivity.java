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
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
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

    public static final String ERROR_STRING_KEY = "errorKey";
    private boolean surfaceReady = false;
    private Thread game = null;
    private AssetManager manager = null;
    private AlertDialog selectLevelDialog = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        manager = getAssets();
        SurfaceView drawSurfaceView = findViewById(R.id.mainDrawingSurface);
        drawSurfaceView.setZOrderOnTop(true);
        SurfaceHolder drawSurfaceHolder = drawSurfaceView.getHolder();
        drawSurfaceHolder.setFormat(PixelFormat.TRANSPARENT);
        drawSurfaceHolder.addCallback(new MySurfaceCallback(this));

    }

    @Override
    protected void onPause() {
        super.onPause();
        joinDrawer();
    }

    @Override
    protected void onStop() {
        super.onStop();
        joinDrawer();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        joinDrawer();
        destroySurface();
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
                android.R.layout.simple_spinner_item, getLevelList());
        // Specify the layout to use when the list of choices appears
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        // Apply the adapter to the spinner
        spinner.setAdapter(adapter);
        selectLevelDialog = new AlertDialog.Builder(this).setTitle(R.string.select_level).setView(selectLevelLayout).show();
    }

    public void onSelect(View view) {
        Spinner spinner = selectLevelDialog.findViewById(R.id.select_level_spinner);
        int pos = spinner.getSelectedItemPosition();
        if (pos == Spinner.INVALID_POSITION) {
            selectLevelDialog.dismiss();
            selectLevelDialog = null;
        } else {
            joinDrawer();
            destroySurface();
            SurfaceView surfaceView = findViewById(R.id.mainDrawingSurface);
            startDrawing(surfaceView.getHolder(), pos);
        }
        selectLevelDialog.dismiss();
        selectLevelDialog = null;
    }

    public void onCancel(View view) {
        selectLevelDialog.dismiss();
        selectLevelDialog = null;
    }

    public void destroySurface() {
        if (surfaceReady) {
            surfaceReady = false;
            destroyNativeSurface();
        }
    }

    public void joinDrawer() {
        boolean done = false;
        while (!done && game != null) {
            tellDrawerStop();
            try {
                game.join();
                done = true;
            } catch (InterruptedException e) {
            }
        }
        game = null;
    }

    public void startDrawing(SurfaceHolder holder, int level) {
        if (level < 0) {
            level = 0;
        }
        boolean usingVulkan = true;
        Surface drawSurface = holder.getSurface();
        String err = initPipeline(usingVulkan, drawSurface, manager, level);
        if (err != null && err.length() != 0) {
            usingVulkan = false;
            // Vulkan failed, try with OpenGL instead
            err = initPipeline(usingVulkan, drawSurface, manager, level);
            if (err != null && err.length() != 0) {
                publishError(err);
                return;
            }
        }

        surfaceReady = true;
        startGame();
    }

    private void startGame() {
        if (game == null && surfaceReady) {
            TextView errorView = findViewById(R.id.mainErrorResult);
            Handler notify = new Handler(new GameErrorHandler());
            game = new Thread(new Draw(notify));
            game.start();
        }
    }

    public void publishError(String err) {
        if (err != null && err.length() > 0) {
            TextView view = findViewById(R.id.mainErrorResult);
            view.setText(err);
        }
    }

    private class GameErrorHandler implements Handler.Callback {
        public boolean handleMessage(Message message) {
            Bundle data = message.getData();
            String error = data.getString(ERROR_STRING_KEY);
            publishError(error);
            return true;
        }
    }
    /**
     * Native methods.
     */
    public native String initPipeline(boolean usingVulkan, Surface drawingSurface, AssetManager manager, int level);
    public native void tellDrawerStop();
    public native String[] getLevelList();
    public native void destroyNativeSurface();
}
