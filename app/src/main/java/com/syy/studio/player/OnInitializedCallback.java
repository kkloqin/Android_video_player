package com.syy.studio.player;

public interface OnInitializedCallback {

    enum OnInitialStatus {
        CONNECT_SUCESS,
        CONNECT_FAILED,
        CLIENT_CANCEL
    };

    public void onInitialized(OnInitialStatus onInitialStatus);
}
