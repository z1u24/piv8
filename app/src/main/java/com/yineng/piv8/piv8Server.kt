package com.yineng.piv8

import android.annotation.SuppressLint
import android.app.Service
import android.content.Context
import android.content.Intent
import android.os.IBinder
import android.util.Log
import android.support.v4.app.NotificationCompat.getExtras
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Base64
import com.yineng.piv8.utils.FileUtil
import org.json.JSONObject
import java.io.*


class piv8Service: Service() {

    private var runtime: V8? = null

    override fun onCreate() {
        super.onCreate()




    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {

        val bundle = intent!!.getExtras()
        val scriptString = bundle!!.getString("script")
        if (scriptString != null){
            runtime!!.executeVoidScript(scriptString)
        }
        return super.onStartCommand(intent, flags, startId)
    }


    override fun onDestroy() {
        runtime?.close()
        super.onDestroy()
    }


    override fun onBind(intent: Intent?): IBinder? {
        throw UnsupportedOperationException("Not yet implemented") as Throwable
    }

}








