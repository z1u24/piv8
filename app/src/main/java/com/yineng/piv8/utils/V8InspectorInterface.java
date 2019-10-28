package com.yineng.piv8.utils;

public interface V8InspectorInterface {

    public void send(Object connection, String message);

    public void sendToDevToolsConsole(Object connection, String message, String level);

}
