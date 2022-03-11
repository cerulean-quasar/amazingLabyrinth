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

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.util.ArrayList;
import java.util.Locale;

public class MySurfaceCallback implements SurfaceHolder.Callback {
    private MainActivity m_app;
    private Thread m_game;

    public MySurfaceCallback(MainActivity inApp) {
        m_app = inApp;
        m_game = null;
    }

    public void surfaceChanged(SurfaceHolder holder,
                               int format,
                               int width,
                               int height)
    {
        Draw.surfaceChanged(width, height, getRotation());
    }

    public void surfaceCreated(SurfaceHolder holder) {
        startDrawing(holder);
    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        joinDrawer();
    }

    public void startDrawing(SurfaceHolder holder) {
        Surface drawSurface = holder.getSurface();

        if (m_game == null) {
            m_app.keepAppAlive(true);
            Handler notify = new Handler(new GameErrorHandler());
            Settings settings = m_app.getSettings();
            m_game = new Thread(new Draw(notify, drawSurface, m_app.getAssets(),
                    m_app.getFilesDir().toString(), getRotation(), settings.getTryVulkan()));
            m_game.start();
        }
    }

    public void joinDrawer() {
        if (m_game != null) {
            Draw.stopDrawer();
            try {
                m_game.join();
            } catch (InterruptedException e) {
                // nothing to do
            }
            m_game = null;
        }
    }

    private float getRotation() {
        switch (m_app.getRotation()) {
            case Surface.ROTATION_90:
                return Draw.ROTATION_90;
            case Surface.ROTATION_180:
                return Draw.ROTATION_180;
            case Surface.ROTATION_270:
                return Draw.ROTATION_270;
            case Surface.ROTATION_0:
            default:
                return Draw.ROTATION_0;
        }
    }

    private class GameErrorHandler implements Handler.Callback {
        public boolean handleMessage(Message message) {
            Bundle data = message.getData();

            // is this an error message?
            String error = data.getString(Constants.KeyError);
            if (error != null) {
                MySurfaceCallback.this.m_app.publishError(error);
                return true;
            }

            if (data.containsKey(Constants.KeyEnableKeepAlive)) {
                boolean keepAlive = data.getBoolean(Constants.KeyEnableKeepAlive, false);
                MySurfaceCallback.this.m_app.keepAppAlive(keepAlive);
                return true;
            }

            // is this a message about with information about the hardware/drivers?
            String graphicsName = data.getString(Constants.KeyGraphicsName);
            if (graphicsName != null) {
                String version = data.getString(Constants.KeyVersionName, m_app.getString(R.string.unknown));
                String deviceName = data.getString(Constants.KeyDeviceName, m_app.getString(R.string.unknown));
                boolean hasAccelerometer = data.getBoolean(Constants.KeyHasAccelerometer, false);
                boolean isVulkanImplementation = data.getBoolean(Constants.keyIsVulkan, false);
                ArrayList<String> driverBugInfo = null;
                String str = null;
                int i = 0;
                do {
                    str = data.getString(Constants.KeyBugInfo + Integer.toString(i));
                    i++;
                    if (str != null) {
                        if (driverBugInfo == null) {
                            driverBugInfo = new ArrayList<>();
                        }
                        driverBugInfo.add(str);
                    }
                } while (str != null);

                MySurfaceCallback.this.m_app.setDeviceInfo(graphicsName, version, deviceName,
                        hasAccelerometer, isVulkanImplementation, driverBugInfo);
                return true;
            }

            // unknown message, ignore
            return true;
        }
    }
}
