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

import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Surface;

public class Draw implements Runnable {
    private static String NATIVE = "Rainbow Dice Native";
    private Handler notify;
    private Surface drawingSurface;
    private AssetManager assetManager;
    private int level;
    public Draw(Handler inNotify, Surface inDrawingSurface, AssetManager inAssetManager, int inLevel) {
        notify = inNotify;
        drawingSurface = inDrawingSurface;
        assetManager = inAssetManager;
        level = inLevel;
    }
    public void run() {
        String error = startGame(drawingSurface, assetManager, level);
        if (error == null || error.length() == 0) {
            // no error, just return.
            return;
        }
        // tell the main thread, an error has occurred.
        Bundle bundle = new Bundle();
        bundle.putString(MainActivity.ERROR_STRING_KEY, error);
        Message msg = Message.obtain();
        msg.setData(bundle);
        notify.sendMessage(msg);
    }

    public native String startGame(Surface drawingSurface, AssetManager manager, int level);
}
