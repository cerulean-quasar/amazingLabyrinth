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

import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.Surface;

public class Draw implements Runnable {
    public static String ERROR_STRING_KEY = "errorString";
    public static String SAVE_GAME_DATA = "saveGameData";

    private Handler m_notify;
    private Surface m_drawingSurface;
    private AssetManager m_assetManager;
    private String m_saveGameDir;

    public Draw(Handler inNotify, Surface inDrawingSurface, AssetManager inAssetManager, String inSaveGameDir) {
        m_notify = inNotify;
        m_drawingSurface = inDrawingSurface;
        m_assetManager = inAssetManager;
        m_saveGameDir = inSaveGameDir;
    }

    public void run() {
        startGame(m_drawingSurface, m_assetManager, m_saveGameDir, new GameReturnChannel(m_notify));
    }

    public static String[] levelList() {
        return getLevelList();
    }

    public static void switchLevel(int level) {
        tellDrawerSwitchLevel(level);
    }

    public static void stopDrawer() {
        tellDrawerStop();
    }

    public static void surfaceChanged(int width, int height) {
        tellDrawerSurfaceChanged(width, height);
    }

    /**
     * Native methods.
     */
    private static native void tellDrawerSwitchLevel(int level);
    private static native void tellDrawerSurfaceChanged(int width, int height);
    private static native void tellDrawerStop();

    private static native String[] getLevelList();

    private native void startGame(Surface drawingSurface, AssetManager manager, String saveData,
                                    GameReturnChannel notify);
}
