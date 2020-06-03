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

import static android.view.MotionEvent.INVALID_POINTER_ID;

public class AmazingLabyrinthSurfaceView extends SurfaceView {
    float m_firstTouchedX;
    float m_firstTouchedY;
    float m_lastTouchX;
    float m_lastTouchY;
    int m_activePointerId;
    private class GestureListener extends GestureDetector.SimpleOnGestureListener {
        boolean m_dragOccurring;

        public GestureListener() {
            super();
            m_dragOccurring = false;
        }
/*
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
*/
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
        m_activePointerId = INVALID_POINTER_ID;
    }

    public AmazingLabyrinthSurfaceView(Context ctx, AttributeSet attrs) {
        super(ctx, attrs);
        gestureDetector = new GestureDetector(ctx, new GestureListener());
        m_activePointerId = INVALID_POINTER_ID;
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        boolean gest2res = gestureDetector.onTouchEvent(ev);
        boolean gest1res = super.performClick();

        final int action = ev.getActionMasked();
        switch (action) {
            case MotionEvent.ACTION_DOWN: {
                final int pointerIndex = ev.getActionIndex();
                final float x = ev.getX(pointerIndex);
                final float y = ev.getY(pointerIndex);

                // Remember where we started dragging
                m_firstTouchedX = x;
                m_firstTouchedY = y;

                // used to calculate the incremental drag distance
                m_lastTouchX = x;
                m_lastTouchY = y;

                // Save the ID of this pointer (for dragging)
                m_activePointerId = ev.getPointerId(0);
                break;
            }

            case MotionEvent.ACTION_MOVE: {
                if (m_activePointerId == INVALID_POINTER_ID) {
                    break;
                }

                // Find the index of the active pointer and fetch its position
                final int pointerIndex = ev.findPointerIndex(m_activePointerId);

                final float x = ev.getX(pointerIndex);
                final float y = ev.getY(pointerIndex);

                // Calculate the distance moved
                final float dx = x - m_lastTouchX;
                final float dy = y - m_lastTouchY;

                // Remember this touch position for the next move event
                m_lastTouchX = x;
                m_lastTouchY = y;

                Draw.dragEvent(m_firstTouchedX, m_firstTouchedY, dx, dy);
                break;
            }

            case MotionEvent.ACTION_CANCEL:
            case MotionEvent.ACTION_UP: {
                Draw.dragEnded(ev.getAxisValue(MotionEvent.AXIS_X), ev.getAxisValue(MotionEvent.AXIS_Y));
                m_activePointerId = INVALID_POINTER_ID;
                break;
            }

            case MotionEvent.ACTION_POINTER_UP: {
                final int pointerIndex = ev.getActionIndex();
                final int pointerId = ev.getPointerId(pointerIndex);

                if (pointerId == m_activePointerId) {
                    // This was our active pointer going up. Choose a new
                    // active pointer and adjust accordingly.
                    final int newPointerIndex = pointerIndex == 0 ? 1 : 0;
                    m_lastTouchX = ev.getX(newPointerIndex);
                    m_lastTouchY = ev.getY(newPointerIndex);
                    m_activePointerId = ev.getPointerId(newPointerIndex);
                }
                break;
            }
        }

        return true;
    }

    @Override
    public boolean performClick() {
        return super.performClick();
    }
}
