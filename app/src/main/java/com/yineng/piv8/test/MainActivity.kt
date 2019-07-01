package com.yineng.piv8.test

import android.app.Service
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import com.yineng.piv8.*
import kotlinx.android.synthetic.main.activity_main.*
import java.io.BufferedReader
import java.io.InputStreamReader

class MainActivity : AppCompatActivity() {

    private var runtime: V8? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val logCallBack = JavaVoidCallback { receiver, parameters ->
            var log = ""
            for (i in 0 until parameters.length()) {
                val x = parameters.get(i)
                log += x.toString()
                if (x is Releasable) x.close()
            }
            Log.d("piv8","javascript打印=$log")
        }

        runtime = V8.createV8Runtime("window")
        val v8Console = V8Object(runtime)
        runtime!!.add("console",v8Console)
        v8Console.registerJavaMethod(logCallBack,"log")
        val piv8timer = piv8Timer()
        val piv8http = piv8Http(runtime!!)
        val piv8ws = piv8WebSocket(runtime!!)
        val piv8db = piv8DB(runtime!!)
        runtime!!.executeScript("var JSVM = {};")
        runtime!!.executeScript("var piv8WebSocket = {};")
        runtime!!.executeScript("JSVM.store = {};")
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

        runtime!!.executeVoidScript("console.log('123')")
        runtime!!.executeVoidScript("setTimeout(function(){console.log('123');},1000)")


        val base64js = getLocalScript("base64js.min.js")
        runtime!!.executeScript(base64js!!)
//        runtime?.startDebugger(this)

        v8Console.close()
        jsWS.close()
        jsvm.close()
        store.close()
    }

    private fun getLocalScript(fileName: String): String? {
        try {
            val temp = getAssets().open(fileName)
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


    override fun onDestroy() {
        super.onDestroy()
        runtime?.close()
    }





}

