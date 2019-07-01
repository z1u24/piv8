package com.yineng.piv8

import android.app.Activity
import android.os.Handler
import android.os.Looper
import java.util.*
import kotlin.concurrent.timerTask

class piv8Timer() {

    private val mtimer = Timer()
    var i = 0
    var managerTimeMap = HashMap<String, TimerTask>()
    var managerV8FunctionMap = HashMap<String, V8Function>()

    var mainHandler = Handler(Looper.getMainLooper())

    fun setTimeout(func: V8Function, time: Int) : Int{
        val v8Function = func.twin()
        val timerTask = timerTask {
            mainHandler.post {
                v8Function.call(null,null)
                v8Function.close()
            }
            this.cancel()
            removeFromMap(this)
        }

        mtimer.schedule(timerTask, time.toLong())
        i += 1
        managerTimeMap.set(i.toString(),timerTask)
        return i
    }

    fun clearTimeout(time:Int){
        if (time.toString() in managerTimeMap.keys) {
            val timer = managerTimeMap.get(time.toString())
            timer!!.cancel()
            if (time.toString() in managerV8FunctionMap.keys) {
                mainHandler.post { managerV8FunctionMap.get(time.toString())!!.close() }
                managerV8FunctionMap.remove(time.toString())
            }
            managerTimeMap.remove(time.toString())
        }
    }

    fun setInterval(func: V8Function, time: Int) : Int{
        val v8Function = func.twin()
        val timerTask = timerTask {
            mainHandler.post { if (!v8Function.isReleased)v8Function.call(null,null) }
        }
        mtimer.schedule(timerTask, time.toLong(), time.toLong())
        i += 1
        managerTimeMap.set(i.toString(),timerTask)
        managerV8FunctionMap.set(i.toString(), v8Function)
        return i
    }

    private fun removeFromMap(timer: TimerTask) {
        val iter = managerTimeMap.keys.iterator()
        while (iter.hasNext()) {
            val key = iter.next()
            val target = managerTimeMap[key]
            if (target === timer) {
                managerTimeMap.remove(key)
                break
            }
        }
    }

}