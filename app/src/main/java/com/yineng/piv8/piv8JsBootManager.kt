package com.yineng.piv8

import android.content.Context
import android.os.Handler
import android.os.Looper
import android.util.Base64
import android.util.Log
import com.yineng.piv8.utils.FileUtil
import org.json.JSONObject

class PiV8JsBootManager(private val context: Context, private val v8: V8){

    private val mainHander = Handler(Looper.getMainLooper())
    private val bootPath: String = "/data/data/" + context.packageName
    val htmlPath: String = "$bootPath/html/vm"
    private var mBootFilePaths: JSONObject? = null
    private val mConfigPath: String = "$htmlPath/bootFilePaths.json"

    init {
        val context = FileUtil.readFile(mConfigPath)
        try {
            if(context == ""){
                mBootFilePaths = JSONObject()
            }else{
                mBootFilePaths = JSONObject(context)
            }
        }catch (e: Exception){
            e.printStackTrace()
        }
    }


    fun saveFile(path: String, base64str: String, callback: V8Function){
        var sPath = path
        val cb = callback.twin()
        try {
            if (sPath.contains(".depend")) {
                sPath = sPath.replace(".depend", "depend")
            }

            val fullPath = "$htmlPath/$path"
            val content = Base64.decode(base64str, Base64.NO_WRAP)
            FileUtil.writeFile(fullPath, content, false)

            sPath = sPath.substring(sPath.lastIndexOf("/") + 1)
            mBootFilePaths!!.put(sPath, fullPath)
            FileUtil.writeFile(mConfigPath, mBootFilePaths!!.toString().toByteArray(Charsets.UTF_8),false)

            Log.d("Intercept", "JSIntercept.saveFile: $fullPath")
            mainHander.post { val v8Array = V8Array(v8); v8Array.push("SUCCESS"); cb.call(null,v8Array); v8Array.close(); cb.close() }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    fun getBootFiles(callback: V8Function){
        val cb = callback.twin()
        val result = JSONObject()
        try {
            val iterator = mBootFilePaths!!.keys()
            while (iterator.hasNext()) {
                val key = iterator.next() as String
                val fullPath = mBootFilePaths!!.getString(key)
                result.put(key, Base64.encodeToString(FileUtil.readFileToData(fullPath), Base64.NO_WRAP))
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
        mainHander.post { val v8Array = V8Array(v8); v8Array.push(result.toString()); cb.call(null,v8Array); v8Array.close(); cb.close() }
    }

    fun restartJSVM(){
        mainHander.post { JSVMManager.get().restartJSVM() }
    }

}