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

public class GameReturnChannel {
    private final Handler m_notify;

    public GameReturnChannel(Handler inNotify) {
        m_notify = inNotify;
    }

    public void sendKeepAliveEnabled(boolean keepAliveEnabled) {
        Bundle bundle = new Bundle();
        bundle.putBoolean(Constants.KeyEnableKeepAlive, keepAliveEnabled);
        Message msg = new Message();
        msg.setData(bundle);
        m_notify.sendMessage(msg);
    }

    public void sendError(String error) {
        Bundle bundle = new Bundle();
        bundle.putString(Constants.KeyError, error);
        Message msg = new Message();
        msg.setData(bundle);
        m_notify.sendMessage(msg);
    }

    public void sendBundle(Bundle bundle) {
        Message msg = new Message();
        msg.setData(bundle);
        m_notify.sendMessage(msg);
    }
}
