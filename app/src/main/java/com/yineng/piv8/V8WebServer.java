package com.yineng.piv8;

import android.app.Activity;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import okhttp3.Response;
import okhttp3.WebSocket;
import okhttp3.WebSocketListener;
import okhttp3.mockwebserver.MockResponse;
import okhttp3.mockwebserver.MockWebServer;
import okio.ByteString;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.net.InetAddress;

public class V8WebServer {

    private static String TAG = "piv8";
    private V8 runtime;
    private static int port = 41868;
    private Handler mainHandler;

    public V8WebServer(V8 v8){
        this.runtime = v8;
        mainHandler = new Handler(Looper.getMainLooper());
        createWebSocketServer();
    }

    private void createWebSocketServer(){
        new Thread(new Runnable() {
            @Override
            public void run() {
                MockWebServer mockWebServer = new MockWebServer();
                mockWebServer.enqueue(new MockResponse().withWebSocketUpgrade(new WebSocketListener() {
                    @Override
                    public void onOpen(final WebSocket webSocket, Response response) {
                        super.onOpen(webSocket, response);
                        Log.d(TAG,"mock webServer onOpen");
                        mainHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                runtime.disconnect();
                                runtime.connect(webSocket);
                            }
                        });
                    }

                    @Override
                    public void onMessage(WebSocket webSocket, final String text) {
                        super.onMessage(webSocket, text);
                        Log.d(TAG,"mock webServer onMessage: " + text);
                        mainHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                runtime.dispatchMessage(text);
                            }
                        });
                    }

                    @Override
                    public void onMessage(WebSocket webSocket, ByteString bytes) {
                        super.onMessage(webSocket, bytes);
                        Log.d(TAG,"mock webServer onMessage: this is a bytes");
                    }

                    @Override
                    public void onClosing(WebSocket webSocket, int code, String reason) {
                        super.onClosing(webSocket, code, reason);
                        Log.d(TAG,"mock webServer onClosing code = "+ code + "; reason = " + reason);
                    }

                    @Override
                    public void onClosed(WebSocket webSocket, int code, String reason) {
                        super.onClosed(webSocket, code, reason);
                        Log.d(TAG,"mock webServer onClosed code = "+ code + "; reason = " + reason);
                        mainHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                runtime.disconnect();
                            }
                        });
                    }

                    @Override
                    public void onFailure(WebSocket webSocket, Throwable t, Response response) {
                        super.onFailure(webSocket, t, response);
                        Log.d(TAG,"mock webServer onFailure reason = " + t.getMessage());
                    }
                }));
                try {
                    mockWebServer.start(InetAddress.getByName("192.168.28.15"),port);
                    Log.d(TAG,"start debugger port");
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }


    public static void send(Object websocket, String playload){
        WebSocket ws = (WebSocket)websocket;
        Log.d(TAG,"send message: " + playload);
        ws.send(playload);
    }

    public static void sendToDevToolsConsole(Object connection, String message, String level){
        try{
            JSONObject consoleMessage = new JSONObject();
            JSONObject params = new JSONObject();
            params.put("type", level);
            params.put("executionContextId", 0);
            params.put("timestamp", 0.000000000000000);
            JSONArray args = new JSONArray();
            args.put(message);
            params.put("args", args);
            consoleMessage.put("method", "Runtime.consoleAPICalled");
            consoleMessage.put("params", params);
            String sendingText = consoleMessage.toString();
            V8WebServer.send(connection,sendingText);
        }catch (JSONException e){
            e.printStackTrace();
        }
    }
}
