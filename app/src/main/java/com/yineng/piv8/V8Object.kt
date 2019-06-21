package com.yineng.piv8

import java.lang.reflect.Array.getDouble
import java.lang.Runtime.getRuntime
import jdk.nashorn.internal.runtime.Undefined.getUndefined
import jdk.nashorn.internal.runtime.Undefined.getUndefined
import java.lang.reflect.AccessibleObject.setAccessible











class V8Object(val v8: V8?, private val data: Any? = null): V8Value(v8){

    init {
        if (v8 != null) {
            this.v8.checkThread();
            initialize(this.v8.getV8RuntimePtr(), data!!)
        }
    }

    override fun createTwin(): V8Object {
        return V8Object(v8)
    }

    override fun twin(): V8Object {
        return super.twin() as V8Object
    }

    operator fun contains(key: String): Boolean {
        v8!!.checkThread()
        checkReleased()
        checkKey(key)
        return v8.contains(v8.getV8RuntimePtr(), objectHandle, key)
    }

    fun getKeys(): Array<String> {
        v8!!.checkThread()
        checkReleased()
        return v8.getKeys(v8.getV8RuntimePtr(), objectHandle)
    }

    operator fun get(key: String): Any {
        v8!!.checkThread()
        checkReleased()
        checkKey(key)
        return v8.get(v8.getV8RuntimePtr(), V8_OBJECT, objectHandle, key)
    }

    fun getInteger(key: String): Int {
        v8!!.checkThread()
        checkReleased()
        checkKey(key)
        return v8.getInteger(v8.getV8RuntimePtr(), objectHandle, key)
    }

    fun getBoolean(key: String): Boolean {
        v8!!.checkThread()
        checkReleased()
        checkKey(key)
        return v8.getBoolean(v8.getV8RuntimePtr(), objectHandle, key)
    }

    fun getDouble(key: String): Double {
        v8!!.checkThread()
        checkReleased()
        checkKey(key)
        return v8.getDouble(v8.getV8RuntimePtr(), objectHandle, key)
    }

    fun getString(key: String): String {
        v8!!.checkThread()
        checkReleased()
        checkKey(key)
        return v8.getString(v8.getV8RuntimePtr(), objectHandle, key)
    }

    fun getArray(key: String): V8Array {
        v8!!.checkThread()
        checkReleased()
        checkKey(key)
        val result = v8.get(v8.getV8RuntimePtr(), V8_ARRAY, objectHandle, key)
        if (result == null || result is V8Array) {
            return result as V8Array
        }
        throw V8ResultUndefined()
    }

    fun getObject(key: String): V8Object? {
        v8!!.checkThread()
        checkReleased()
        checkKey(key)
        val result = v8.get(v8.getV8RuntimePtr(), V8_OBJECT, objectHandle, key)
        if (result == null || result is V8Object) {
            return result
        }
        throw V8ResultUndefined()
    }

    fun executeIntegerFunction(name: String, parameters: V8Array?): Int {
        v8!!.checkThread()
        checkReleased()
        v8.checkRuntime(parameters)
        val parametersHandle = (if (parameters == null) 0 else parameters!!.getHandle()).toLong()
        return v8.executeIntegerFunction(v8.getV8RuntimePtr(), getHandle(), name, parametersHandle)
    }

    fun executeDoubleFunction(name: String, parameters: V8Array?): Double {
        v8!!.checkThread()
        checkReleased()
        v8.checkRuntime(parameters)
        val parametersHandle = (if (parameters == null) 0 else parameters!!.getHandle()).toLong()
        return v8.executeDoubleFunction(v8.getV8RuntimePtr(), getHandle(), name, parametersHandle)
    }

    fun executeStringFunction(name: String, parameters: V8Array?): String {
        v8!!.checkThread()
        checkReleased()
        v8.checkRuntime(parameters)
        val parametersHandle = (if (parameters == null) 0 else parameters!!.getHandle()).toLong()
        return v8.executeStringFunction(v8.getV8RuntimePtr(), getHandle(), name, parametersHandle)
    }

    fun executeBooleanFunction(name: String, parameters: V8Array?): Boolean {
        v8!!.checkThread()
        checkReleased()
        v8.checkRuntime(parameters)
        val parametersHandle = (if (parameters == null) 0 else parameters!!.getHandle()).toLong()
        return v8.executeBooleanFunction(v8.getV8RuntimePtr(), getHandle(), name, parametersHandle)
    }

    fun executeArrayFunction(name: String, parameters: V8Array?): V8Array {
        v8!!.checkThread()
        checkReleased()
        v8.checkRuntime(parameters)
        val parametersHandle = (if (parameters == null) 0 else parameters!!.getHandle()).toLong()
        val result = v8.executeFunction(v8.getV8RuntimePtr(), V8_ARRAY, objectHandle, name, parametersHandle)
        if (result is V8Array) {
            return result as V8Array
        }
        throw V8ResultUndefined()
    }

    fun executeObjectFunction(name: String, parameters: V8Array?): V8Object {
        v8!!.checkThread()
        checkReleased()
        v8.checkRuntime(parameters)
        val parametersHandle = (if (parameters == null) 0 else parameters!!.getHandle()).toLong()
        val result = v8.executeFunction(v8.getV8RuntimePtr(), V8_OBJECT, objectHandle, name, parametersHandle)
        if (result is V8Object) {
            return result
        }
        throw V8ResultUndefined()
    }

    fun executeFunction(name: String, parameters: V8Array?): Any {
        v8!!.checkThread()
        checkReleased()
        v8.checkRuntime(parameters)
        val parametersHandle = (if (parameters == null) 0 else parameters!!.getHandle()).toLong()
        return v8.executeFunction(v8.getV8RuntimePtr(), UNKNOWN, objectHandle, name, parametersHandle)
    }

    fun executeJSFunction(name: String): Any {
        return executeFunction(name, null)
    }

    fun executeJSFunction(name: String, vararg parameters: Any): Any {
        if (parameters == null) {
            return executeFunction(name, null)
        }
        val parameterArray = V8Array(v8!!.getRuntime())
        try {
            for (`object` in parameters) {
                if (`object` == null) {
                    parameterArray.pushNull()
                } else if (`object` is V8Value) {
                    parameterArray.push(`object`)
                } else if (`object` is Int) {
                    parameterArray.push(`object`)
                } else if (`object` is Double) {
                    parameterArray.push(`object`)
                } else if (`object` is Long) {
                    parameterArray.push(`object`.toDouble())
                } else if (`object` is Float) {
                    parameterArray.push(`object`.toFloat())
                } else if (`object` is Boolean) {
                    parameterArray.push(`object`)
                } else if (`object` is String) {
                    parameterArray.push(`object`)
                } else {
                    throw IllegalArgumentException("Unsupported Object of type: " + `object`.javaClass)
                }
            }
            return executeFunction(name, parameterArray)
        } finally {
            parameterArray.close()
        }
    }

    fun executeVoidFunction(name: String, parameters: V8Array?) {
        v8!!.checkThread()
        checkReleased()
        v8.checkRuntime(parameters)
        val parametersHandle = (if (parameters == null) 0 else parameters!!.getHandle()).toLong()
        v8.executeVoidFunction(v8.getV8RuntimePtr(), objectHandle, name, parametersHandle)
    }

    fun add(key: String, value: Int): V8Object {
        v8!!.checkThread()
        checkReleased()
        v8.add(v8.getV8RuntimePtr(), objectHandle, key, value)
        return this
    }

    fun add(key: String, value: Boolean): V8Object {
        v8!!.checkThread()
        checkReleased()
        v8.add(v8.getV8RuntimePtr(), objectHandle, key, value)
        return this
    }

    fun add(key: String, value: Double): V8Object {
        v8!!.checkThread()
        checkReleased()
        v8.add(v8.getV8RuntimePtr(), objectHandle, key, value)
        return this
    }

    fun add(key: String, value: String?): V8Object {
        v8!!.checkThread()
        checkReleased()
        if (value == null) {
            v8.addNull(v8.getV8RuntimePtr(), objectHandle, key)
        } else if (value == V8.getUndefined()) {
            v8.addUndefined(v8.getV8RuntimePtr(), objectHandle, key)
        } else {
            v8.add(v8.getV8RuntimePtr(), objectHandle, key, value)
        }
        return this
    }

    fun add(key: String, value: V8Value?): V8Object {
        v8!!.checkThread()
        checkReleased()
        v8.checkRuntime(value)
        if (value == null) {
            v8.addNull(v8.getV8RuntimePtr(), objectHandle, key)
        } else if (value == V8.getUndefined()) {
            v8.addUndefined(v8.getV8RuntimePtr(), objectHandle, key)
        } else {
            v8.addObject(v8.getV8RuntimePtr(), objectHandle, key, value.getHandle())
        }
        return this
    }

    fun addUndefined(key: String): V8Object {
        v8!!.checkThread()
        checkReleased()
        v8.addUndefined(v8.getV8RuntimePtr(), objectHandle, key)
        return this
    }

    fun addNull(key: String): V8Object {
        v8!!.checkThread()
        checkReleased()
        v8.addNull(v8.getV8RuntimePtr(), objectHandle, key)
        return this
    }

    fun setPrototype(value: V8Object): V8Object {
        v8!!.checkThread()
        checkReleased()
        v8.setPrototype(v8.getV8RuntimePtr(), objectHandle, value.getHandle())
        return this
    }

    fun registerJavaMethod(callback: JavaCallback, jsFunctionName: String): V8Object {
        v8!!.checkThread()
        checkReleased()
        v8.registerCallback(callback, getHandle(), jsFunctionName)
        return this
    }

    fun registerJavaMethod(callback: JavaVoidCallback, jsFunctionName: String): V8Object {
        v8!!.checkThread()
        checkReleased()
        v8.registerVoidCallback(callback, getHandle(), jsFunctionName)
        return this
    }

    fun registerJavaMethod(
        `object`: Any,
        methodName: String,
        jsFunctionName: String,
        parameterTypes: Array<Class<*>>
    ): V8Object {
        return registerJavaMethod(`object`, methodName, jsFunctionName, parameterTypes, false)
    }

    fun registerJavaMethod(
        `object`: Any,
        methodName: String,
        jsFunctionName: String,
        parameterTypes: Array<Class<*>>,
        includeReceiver: Boolean
    ): V8Object {
        v8!!.checkThread()
        checkReleased()
        try {
            val method = `object`.javaClass.getMethod(methodName, *parameterTypes)
            method.isAccessible = true
            v8.registerCallback(`object`, method, getHandle(), jsFunctionName, includeReceiver)
        } catch (e: NoSuchMethodException) {
            throw IllegalStateException(e)
        } catch (e: SecurityException) {
            throw IllegalStateException(e)
        }

        return this
    }

    override fun toString(): String {
        if (isReleased() || v8.isReleased()) {
            return "[Object released]";
        }
        v8.checkThread();
        return v8.toString(v8.getV8RuntimePtr(), getHandle());
    }

    private fun checkKey(key: String?) {
        if (key == null) {
            throw IllegalArgumentException("Key cannot be null")
        }
    }

    class Undefined{

    }

}