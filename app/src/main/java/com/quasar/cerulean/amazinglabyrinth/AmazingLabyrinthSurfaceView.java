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

import android.content.Context;
import android.util.AttributeSet;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.SurfaceView;

public class AmazingLabyrinthSurfaceView extends SurfaceView {
    private class GestureListener extends GestureDetector.SimpleOnGestureListener {
        boolean m_dragOccurring;

        public GestureListener() {
            super();
            m_dragOccurring = false;
        }

        @Override
        public boolean onScroll(MotionEvent ev1, MotionEvent ev2, float distanceX, float distanceY) {
            m_dragOccurring = true;
            Draw.dragEvent(ev1.getAxisValue(MotionEvent.AXIS_X), ev1.getAxisValue(MotionEvent.AXIS_Y), distanceX, distanceY);
            return true;
        }

        @Override
        public boolean onDown(MotionEvent ev) {
            return true;
        }

        @Override
        public boolean onSingleTapUp(MotionEvent ev) {
            if (m_dragOccurring) {
                Draw.dragEnded(ev.getAxisValue(MotionEvent.AXIS_X), ev.getAxisValue(MotionEvent.AXIS_Y));
                m_dragOccurring = false;
            }
            return true;
        }

        @Override
        public boolean onSingleTapConfirmed(MotionEvent ev) {
            Draw.tapEvent(ev.getAxisValue(MotionEvent.AXIS_X), ev.getAxisValue(MotionEvent.AXIS_Y));
            return AmazingLabyrinthSurfaceView.this.performClick();
        }
    }
    private GestureDetector gestureDetector;

    public AmazingLabyrinthSurfaceView(Context ctx) {
        super(ctx);
        gestureDetector = new GestureDetector(ctx, new GestureListener());
    }

    public AmazingLabyrinthSurfaceView(Context ctx, AttributeSet attrs) {
        super(ctx, attrs);
        gestureDetector = new GestureDetector(ctx, new GestureListener());
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        boolean gest2res = gestureDetector.onTouchEvent(ev);
        boolean gest1res = super.performClick();
        return gest1res || gest2res;
    }

    @Override
    public boolean performClick() {
        return super.performClick();
    }
}
