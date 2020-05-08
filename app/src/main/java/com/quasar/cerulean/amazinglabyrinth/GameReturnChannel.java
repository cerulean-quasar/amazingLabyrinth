package com.quasar.cerulean.amazinglabyrinth;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

public class GameReturnChannel {
    private Handler m_notify;

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
