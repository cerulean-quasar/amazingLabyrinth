package com.quasar.cerulean.amazinglabyrinth;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

public class GameReturnChannel {
    private Handler m_notify;

    public GameReturnChannel(Handler inNotify) {
        m_notify = inNotify;
    }

    public void sendError(String error) {
        Bundle bundle = new Bundle();
        bundle.putString(MySurfaceCallback.KeyError, error);
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
