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

import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.Surface;

public class Draw implements Runnable {
    public static final float ROTATION_0 = 0;
    public static final float ROTATION_90 = 90;
    public static final float ROTATION_180 = 180;
    public static final float ROTATION_270 = 270;

    private Handler m_notify;
    private Surface m_drawingSurface;
    private AssetManager m_assetManager;
    private String m_saveGameDir;
    private float m_rotaation;

    public Draw(Handler inNotify, Surface inDrawingSurface, AssetManager inAssetManager, String inSaveGameDir, float inRotation) {
        m_notify = inNotify;
        m_drawingSurface = inDrawingSurface;
        m_assetManager = inAssetManager;
        m_saveGameDir = inSaveGameDir;
        m_rotaation = inRotation;
    }

    public void run() {
        startGame(m_drawingSurface, m_assetManager, m_saveGameDir, new GameReturnChannel(m_notify), m_rotaation);
    }

    public static void switchLevel(String level) {
        tellDrawerSwitchLevel(level);
    }

    public static void stopDrawer() {
        tellDrawerStop();
    }

    public static void surfaceChanged(int width, int height, float rotationAngle) {
        tellDrawerSurfaceChanged(width, height, rotationAngle);
    }

    public static void dragEvent(float startX, float startY, float distanceX, float distanceY) {
        tellDrawerDragOccurred(startX, startY, distanceX, distanceY);
    }

    public static void dragEnded(float doneX, float doneY) {
        tellDrawerDragEnded(doneX, doneY);
    }

    public static void tapEvent(float x, float y) {
        tellDrawerTapOccurred(x, y);
    }

    /**
     * Native methods.
     */
    private static native void tellDrawerDragEnded(float doneX, float doneY);
    private static native void tellDrawerTapOccurred(float x, float y);
    private static native void tellDrawerDragOccurred(float startX, float startY, float distanceX, float distanceY);
    private static native void tellDrawerSwitchLevel(String level);
    private static native void tellDrawerSurfaceChanged(int width, int height, float rotationAngle);
    private static native void tellDrawerStop();

    private native void startGame(Surface drawingSurface, AssetManager manager, String saveData,
                                    GameReturnChannel notify, float rotationAngle);
}
