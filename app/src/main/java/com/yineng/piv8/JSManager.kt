package com.yineng.piv8

import android.annotation.SuppressLint
import android.content.Context
import android.util.Log
import java.io.BufferedReader
import java.io.InputStreamReader
import java.security.SecureRandom

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
        val bootManager = PiV8JsBootManager(ctx!!,runtime!!)
        runtime!!.executeVoidScript("var JSVM = {};")
        runtime!!.executeVoidScript("var piv8WebSocket = {};")
        runtime!!.executeVoidScript("JSVM.store = {};")
        runtime!!.executeVoidScript("JSVM.Boot = {}")
        val jsWS = runtime!!.getObject("piv8WebSocket")
        val jsvm = runtime!!.getObject("JSVM")
        val store = jsvm.getObject("store")
        val boot = jsvm.getObject("Boot")
//        jsvm.registerJavaMethod(this,"getRandomValues","getRandomValues", arrayOf())
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
        boot.registerJavaMethod(bootManager, "saveFile", "saveFile", arrayOf(String::class.java, String::class.java, V8Function::class.java))
        boot.registerJavaMethod(bootManager, "getBootFiles", "getBootFiles", arrayOf(V8Function::class.java))
        boot.registerJavaMethod(bootManager, "restartJSVM", "restartJSVM", arrayOf())

        val base64js = getLocalJSScript("base64js.min.js")
        runtime!!.executeScript(base64js!!)
//        runtime?.startDebugger()


        v8Console.close()
        jsWS.close()
        boot.close()
        store.close()
        jsvm.close()

        return runtime!!
    }

    fun restartJSVM(){
        runtime = null
        createV8(ctx!!)
    }

    fun getRandomValues():Long{
        val sercurirandom = SecureRandom()
        val bytes = ByteArray(4)
        sercurirandom.nextBytes(bytes)
        val results = bytes.getUIntAt(0)
        return results
    }

    fun ByteArray.getUIntAt(idx: Int) =
        ((this[3].toInt() and 0xFF) shl 24 or (this[2].toInt() and 0xFF) shl 16 or (this[1].toInt() and 0xFF) shl 8 or (this[0].toInt() and 0xFF)).toLong()



    private fun getLocalJSScript(fileName: String): String? {
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