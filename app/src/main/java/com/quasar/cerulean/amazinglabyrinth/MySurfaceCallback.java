/**
 * Copyright 2019 Cerulean Quasar. All Rights Reserved.
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

public class MySurfaceCallback implements SurfaceHolder.Callback {
    public static final String KeyError = "errorString";
    private static final String KeyGraphicsName = "graphicsName";
    private static final String KeyVersionName = "versionName";
    private static final String KeyDeviceName = "deviceName";
    private static final String KeyHasAccelerometer = "hasAccelerometer";

    private MainActivity m_app;
    private Thread m_game;
    private Bundle m_saveData;

    public MySurfaceCallback(MainActivity inApp, Bundle saveData) {
        m_app = inApp;
        m_game = null;

        if (saveData != null) {
            m_saveData = saveData.getBundle(Draw.SAVE_GAME_DATA);
        } else {
            m_saveData = null;
        }
    }

    public void surfaceChanged(SurfaceHolder holder,
                               int format,
                               int width,
                               int height)
    {
        Draw.surfaceChanged(width, height);
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
            Handler notify = new Handler(new GameErrorHandler());
            m_game = new Thread(new Draw(notify, drawSurface, m_app.getAssets(), m_app.getFilesDir().toString()));
            m_game.start();
        }
    }

    public void joinDrawer() {
        if (m_game != null) {
            Draw.stopDrawer();
            try {
                m_game.join();
            } catch (InterruptedException e) {
            }
            m_game = null;
        }
    }

    private class GameErrorHandler implements Handler.Callback {
        public boolean handleMessage(Message message) {
            Bundle data = message.getData();
            String error = data.getString(Draw.ERROR_STRING_KEY);
            if (error != null) {
                MySurfaceCallback.this.m_app.publishError(error);
            }
            return true;
        }
    }
}
