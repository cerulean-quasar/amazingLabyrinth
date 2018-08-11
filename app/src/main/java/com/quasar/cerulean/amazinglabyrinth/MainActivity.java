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
import android.graphics.PixelFormat;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.TextView;

import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    public static final String ERROR_STRING_KEY = "errorKey";
    boolean surfaceReady = false;
    private Thread game = null;
    private AssetManager manager = null;

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

    public void startDrawing(SurfaceHolder holder) {
        boolean usingVulkan = false;
        Surface drawSurface = holder.getSurface();
        String err = initPipeline(usingVulkan, drawSurface, manager);
        if (err != null && err.length() != 0) {
            usingVulkan = false;
            // Vulkan failed, try with OpenGL instead
            err = initPipeline(usingVulkan, drawSurface, manager);
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
    public native String initPipeline(boolean usingVulkan, Surface drawingSurface, AssetManager manager);
    public native void tellDrawerStop();
    public native void destroyNativeSurface();
}
