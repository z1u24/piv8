package com.yineng.piv8

import android.annotation.SuppressLint
import android.app.Service
import android.content.Context
import android.content.Intent
import android.os.IBinder
import android.util.Log
import java.io.BufferedReader
import java.io.InputStreamReader
import android.support.v4.app.NotificationCompat.getExtras
import android.os.Bundle



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


class JSVMManager private constructor(){

    private var runtime: V8? = null
    private var ctx: Context? = null

    fun createV8(context: Context):V8{

        val logCallBack = JavaVoidCallback { receiver, parameters ->
            var log = ""
            for (i in 0 until parameters.length()) {
                val x = parameters.get(i)
                log += x.toString()
                if (x is Releasable) x.close()
            }
            Log.d("piv8","javascript打印=$log")
        }

        this.ctx = context
        runtime = V8.createV8Runtime("window")
        val v8Console = V8Object(runtime)
        runtime!!.add("console",v8Console)
        v8Console.registerJavaMethod(logCallBack,"log")
        val piv8timer = piv8Timer()
        val piv8http = piv8Http(runtime!!)
        val piv8ws = piv8WebSocket(runtime!!)
        val piv8db = piv8DB(ctx!!,runtime!!)
        runtime!!.executeVoidScript("var JSVM = {};")
        runtime!!.executeVoidScript("var piv8WebSocket = {};")
        runtime!!.executeVoidScript("JSVM.store = {};")
        val jsWS = runtime!!.getObject("piv8WebSocket")
        val jsvm = runtime!!.getObject("JSVM")
        val store = jsvm.getObject("store")
        jsvm.registerJavaMethod(piv8http, "request", "request", arrayOf<Class<*>>(String::class.java,String::class.java,String::class.java,String::class.java,String::class.java,String::class.java,V8Function::class.java,V8Function::class.java,V8Function::class.java))
        jsWS.registerJavaMethod(piv8ws, "startWebSocket", "startWebSocket", arrayOf<Class<*>>(String::class.java))
        jsWS.registerJavaMethod(piv8ws, "onOpen", "onOpen", arrayOf<Class<*>>(String::class.java,V8Function::class.java))
        jsWS.registerJavaMethod(piv8ws, "onFail", "onFail", arrayOf<Class<*>>(String::class.java,V8Function::class.java))
        jsWS.registerJavaMethod(piv8ws, "onMessage", "onMessage", arrayOf<Class<*>>(String::class.java,V8Function::class.java))
        jsWS.registerJavaMethod(piv8ws, "onClose", "onClose", arrayOf<Class<*>>(String::class.java,V8Function::class.java))
        jsWS.registerJavaMethod(piv8ws, "close", "close", arrayOf<Class<*>>(String::class.java))
        jsWS.registerJavaMethod(piv8ws, "sendMsg", "sendMsg", arrayOf<Class<*>>(String::class.java,String::class.java,String::class.java))
        runtime!!.registerJavaMethod(piv8timer, "setTimeout", "setTimeout", arrayOf<Class<*>>(V8Function::class.java,Int::class.java))
        runtime!!.registerJavaMethod(piv8timer, "clearTimeout", "clearTimeout", arrayOf<Class<*>>(Int::class.java))
        runtime!!.registerJavaMethod(piv8timer, "setInterval", "setInterval", arrayOf<Class<*>>(V8Function::class.java,Int::class.java))
        store.registerJavaMethod(piv8db,"create","create",arrayOf<Class<*>>(String::class.java,V8Function::class.java,V8Function::class.java,V8Function::class.java))
        store.registerJavaMethod(piv8db,"delete","delete",arrayOf<Class<*>>(String::class.java,V8Function::class.java,V8Function::class.java,V8Function::class.java))
        store.registerJavaMethod(piv8db,"iterate","iterate",arrayOf<Class<*>>(String::class.java,V8Function::class.java,V8Function::class.java,V8Function::class.java))
        store.registerJavaMethod(piv8db,"read","read",arrayOf<Class<*>>(String::class.java,String::class.java,V8Function::class.java,V8Function::class.java,V8Function::class.java))
        store.registerJavaMethod(piv8db,"remove","remove",arrayOf<Class<*>>(String::class.java,String::class.java,V8Function::class.java,V8Function::class.java,V8Function::class.java))
        store.registerJavaMethod(piv8db,"write","write",arrayOf<Class<*>>(String::class.java,String::class.java,String::class.java,V8Function::class.java,V8Function::class.java,V8Function::class.java))



        val base64js = getLocalScript("base64js.min.js")
        runtime!!.executeScript(base64js!!)
//        runtime?.startDebugger()

        v8Console.close()
        jsWS.close()
        jsvm.close()
        store.close()
        return runtime!!
    }

    fun restartJSVM(){
        runtime = null
        createV8(ctx!!)
    }

    private fun getLocalScript(fileName: String): String? {
        try {
            val temp = ctx!!.getAssets().open(fileName)
            val br = BufferedReader(InputStreamReader(temp))
            val sb = StringBuffer()
            var strLine = br.readLine()
            while (strLine  != null) {
                sb.append(strLine).append("\r\n")
                strLine = br.readLine()
            }
            br.close()
            return sb.toString()
        } catch (e: Exception) {
            e.printStackTrace()
        }

        return null
    }


    companion object {
        @SuppressLint("StaticFieldLeak")
        private var instance: JSVMManager? = null
            get() {
                if (field == null) {
                    field = JSVMManager()
                }
                return field
            }
        @Synchronized
        fun get(): JSVMManager{
            return instance!!
        }
    }

}


class JSBootManager(private val ctx: Context, private val v8: V8){

    fun initManager(){

        val restartJSVM = JavaVoidCallback { receiver, parameters ->
            JSVMManager.get().restartJSVM()
        }

        val jsvm = v8.getObject("JSVM")
        val boot = jsvm.getObject("Boot")

        boot.registerJavaMethod(restartJSVM,"restart")

        boot.close()
        jsvm.close()
    }

}