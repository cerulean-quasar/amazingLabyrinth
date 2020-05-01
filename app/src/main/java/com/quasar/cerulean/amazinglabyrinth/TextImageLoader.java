package com.quasar.cerulean.amazinglabyrinth;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;

import java.nio.ByteBuffer;

import static android.graphics.Bitmap.Config.ARGB_8888;

public class TextImageLoader {
    private final int TEXWIDTH = 500;
    private final int TEXHEIGHT = 500;
    private byte[] bytes;

    public TextImageLoader(String inText) {
        String[] texts = inText.split("\n");
        Bitmap bitmap = Bitmap.createBitmap(TEXWIDTH, TEXHEIGHT, ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        Paint paint = new Paint();
        paint.setTextAlign(Paint.Align.CENTER);
        paint.setTextSize(50.0f);
        canvas.drawARGB(200,128,128,128);
        paint.setARGB(255, 0, 255, 0);
        for (int i=0; i < texts.length; i++) {
            canvas.drawText(texts[i], 200, 100+50*i, paint);
        }
        int bitmapSize = bitmap.getAllocationByteCount();
        ByteBuffer imageBuffer = ByteBuffer.allocate(bitmapSize);
        bitmap.copyPixelsToBuffer(imageBuffer);
        bytes = new byte[bitmapSize];
        try {
            imageBuffer.position(0);
            imageBuffer.get(bytes);
        } catch (Exception e) {
            bytes = null;
        }
    }

    public int getImageWidth() {
        if (bytes == null) {
            return 0;
        } else {
            return TEXWIDTH;
        }
    }

    public int getImageHeight() {
        if (bytes == null) {
            return 0;
        } else {
            return TEXHEIGHT;
        }
    }

    public int getImageSize() {
        if (bytes == null) {
            return 0;
        } else {
            return bytes.length;
        }
    }

    public void getImageData(byte[] inBytes) {
        System.arraycopy(bytes, 0, inBytes, 0, bytes.length);
    }
}
