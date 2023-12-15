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

import android.content.Intent;
import android.graphics.PixelFormat;

import androidx.activity.result.ActivityResult;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.res.ResourcesCompat;

import android.os.Bundle;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;

public class MainActivity extends AppCompatActivity implements AdapterView.OnItemSelectedListener {
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private static final String m_settingsFile = "gameSettings";

    private String m_graphicsName = "";
    private String m_version = "";
    private String m_deviceName = "";
    private boolean m_hasAccelerometer = true;
    private boolean m_is64Bit = true;
    private ArrayList<String> m_driverBugInfo;
    private ActivityResultLauncher<Intent> m_selectLevelLauncher;
    private ActivityResultLauncher<Intent> m_settingsActivityLauncher;
    private Settings m_settings;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Don't sleep the app if inactive since to android, the app may seem inactive because the
        // user isn't doing anything.  In actuality the user is doing something.  They are tilting
        // the device to roll the ball.  Note: this will get turned off if the user does not tilt
        // the screen within a timeout period.  It will get turned back on if they start tilting the
        // screen again.
        keepAppAlive(true);

        setContentView(R.layout.activity_main);

        // setup activity launchers
        m_selectLevelLauncher = registerForActivityResult(new ActivityResultContracts.StartActivityForResult(),
                new ActivityResultCallback<ActivityResult>() {
                    @Override
                    public void onActivityResult(ActivityResult result) {
                        if (result.getResultCode() == RESULT_OK)
                        {
                            Intent data = result.getData();
                            if (data == null) {
                                return;
                            }

                            String level = data.getStringExtra(Constants.KeySelectedLevel);
                            if (level != null) {
                                Draw.switchLevel(level);
                            }
                        }
                    }
                });

        m_settingsActivityLauncher = registerForActivityResult(new ActivityResultContracts.StartActivityForResult(),
                new ActivityResultCallback<ActivityResult>() {
                    @Override
                    public void onActivityResult(ActivityResult result) {
                        if (result.getResultCode() == RESULT_OK) {
                            Intent intent = result.getData();
                            if (intent != null) {
                                m_settings.overrideFromIntent(intent);
                                saveSettingsToFile();
                            }
                        }
                    }
                });

        // retrieve settings
        m_settings = new Settings();
        try {
            FileInputStream in = new FileInputStream(getFilesDir() + "/" + m_settingsFile);
            m_settings.new File().open(in);
            in.close();
        } catch (IOException e) {
            // do nothing (means use default values).
        }

        // setup surface
        SurfaceView drawSurfaceView = findViewById(R.id.mainDrawingSurface);
        drawSurfaceView.setZOrderOnTop(true);
        SurfaceHolder drawSurfaceHolder = drawSurfaceView.getHolder();
        drawSurfaceHolder.setFormat(PixelFormat.TRANSPARENT);
        drawSurfaceHolder.addCallback(new MySurfaceCallback(this));

        setTitle("");

        TypedValue value = new TypedValue();
        getTheme().resolveAttribute(R.attr.app_name, value, true);

        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setBackgroundDrawable(ResourcesCompat.getDrawable(getResources(), value.resourceId, getTheme()));
        }

        m_graphicsName = getString(R.string.unknown);
        m_version = getString(R.string.unknown);
        m_deviceName = getString(R.string.unknown);
        m_hasAccelerometer = false;
        m_driverBugInfo = null;
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
        Intent intent = new Intent(this, ChooseLevelActivity.class);
        m_selectLevelLauncher.launch(intent);
    }

    public void onSettings(MenuItem item) {
        Intent intent = new Intent(this, SettingsActivity.class);
        m_settings.addToIntent(intent);
        m_settingsActivityLauncher.launch(intent);
    }
    public void onAbout(MenuItem item) {
        Intent intent = new Intent(this, AboutActivity.class);
        intent.putExtra(Constants.KeyHasAccelerometer, m_hasAccelerometer);
        intent.putExtra(Constants.KeyGraphicsName, m_graphicsName);
        intent.putExtra(Constants.KeyDeviceName, m_deviceName);
        intent.putExtra(Constants.KeyVersionName, m_version);
        intent.putExtra(Constants.KeyIs64Bit, m_is64Bit);
        if (m_driverBugInfo != null) {
            intent.putExtra(Constants.KeyBugInfo, m_driverBugInfo);
        }
        startActivity(intent);
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
                    setTitle(R.string.error).setView(msgLayout).show();
            okButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    dialog.dismiss();
                }
            });
        }
    }

    public void setDeviceInfo(
            String graphicsName,
            String version,
            String deviceName,
            boolean hasAccelerometer,
            boolean isVulkanImplementation,
            boolean is64Bit,
            ArrayList<String> driverBugInfo)
    {
        m_graphicsName = graphicsName;
        m_version = version;
        m_deviceName = deviceName;
        m_hasAccelerometer = hasAccelerometer;
        m_is64Bit = is64Bit;
        m_driverBugInfo = driverBugInfo;

        if (m_settings.getTryVulkan() && !isVulkanImplementation) {
            m_settings.setTryVulkan(false);
            saveSettingsToFile();
        }
    }

    public int getRotation() {
        SurfaceView drawSurfaceView = findViewById(R.id.mainDrawingSurface);
        return drawSurfaceView.getDisplay().getRotation();
    }

    public Settings getSettings() {
        return m_settings;
    }

    public void keepAppAlive(boolean keepAlive) {
        if (keepAlive) {
            /* Keep the screen on even though the user is not tapping it. */
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        } else {
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
    }

    private void saveSettingsToFile() {
        try {
            Settings.File file = m_settings.new File();
            FileOutputStream out = new FileOutputStream(getFilesDir() + "/" + m_settingsFile);
            file.save(out);
            out.close();
        } catch (IOException e) {
            // nothing to do
        }
    }
}
