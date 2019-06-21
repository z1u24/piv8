package com.yineng.piv8



abstract class V8Value(v8: V8?) : Releasable{

    val NULL = 0
    val UNKNOWN = 0
    val INTEGER = 1
    val INT_32_ARRAY = 1
    val DOUBLE = 2
    val FLOAT_64_ARRAY = 2
    val BOOLEAN = 3
    val STRING = 4
    val V8_ARRAY = 5
    val V8_OBJECT = 6
    val V8_FUNCTION = 7
    val V8_TYPED_ARRAY = 8
    val BYTE = 9
    val INT_8_ARRAY = 9
    val V8_ARRAY_BUFFER = 10
    val UNSIGNED_INT_8_ARRAY = 11
    val UNSIGNED_INT_8_CLAMPED_ARRAY = 12
    val INT_16_ARRAY = 13
    val UNSIGNED_INT_16_ARRAY = 14
    val UNSIGNED_INT_32_ARRAY = 15
    val FLOAT_32_ARRAY = 16
    val UNDEFINED = 99

    protected var v8: V8? = null
    protected var objectHandle: Long = 0
    protected var released = true

    init {
        if (v8 == null){
            this.v8 = this as V8
        } else {
            this.v8 = v8
        }
    }


    protected fun initialize(runtimePtr: Long, data: Any) {
        val objectHandle = v8.initNewV8Object(runtimePtr)
        released = false
        addObjectReference(objectHandle)
    }


    @Throws(Error::class)
    protected fun addObjectReference(objectHandle: Long) {
        this.objectHandle = objectHandle
        try {
//            v8.addObjRef(this)
        } catch (e: Error) {
            close()
            throw e
        } catch (e: RuntimeException) {
            close()
            throw e
        }

    }


    fun getStringRepresentation(type: Int): String {
        when (type) {
            NULL -> return "Null"
            INTEGER -> return "Integer"
            DOUBLE -> return "Double"
            BOOLEAN -> return "Boolean"
            STRING -> return "String"
            V8_ARRAY -> return "V8Array"
            V8_OBJECT -> return "V8Object"
            V8_FUNCTION -> return "V8Function"
            V8_TYPED_ARRAY -> return "V8TypedArray"
            BYTE -> return "Byte"
            V8_ARRAY_BUFFER -> return "V8ArrayBuffer"
            UNSIGNED_INT_8_ARRAY -> return "UInt8Array"
            UNSIGNED_INT_8_CLAMPED_ARRAY -> return "UInt8ClampedArray"
            INT_16_ARRAY -> return "Int16Array"
            UNSIGNED_INT_16_ARRAY -> return "UInt16Array"
            UNSIGNED_INT_32_ARRAY -> return "UInt32Array"
            FLOAT_32_ARRAY -> return "Float32Array"
            UNDEFINED -> return "Undefined"
            else -> throw IllegalArgumentException("Invalid V8 type: $type")
        }
    }


    fun getConstructorName(): String {
        v8.checkThread()
        v8.checkReleased()
        return v8.getConstructorName(v8.getV8RuntimePtr(), objectHandle)
    }


    fun isUndefined(): Boolean {
        return false
    }

    fun getRuntime(): V8 {
        return v8!!
    }

    fun getV8Type(): Int {
        if (isUndefined()) {
            return UNDEFINED
        }
        v8.checkThread()
        v8.checkReleased()
        return v8.getType(v8.getV8RuntimePtr(), objectHandle)
    }

    open fun twin(): V8Value {
        if (isUndefined()) {
            return this
        }
        v8.checkThread()
        v8.checkReleased()
        val twin = createTwin()
        v8.createTwin(this, twin)
        return twin
    }

    fun setWeak(): V8Value {
        v8.checkThread()
        v8.checkReleased()
        v8.v8WeakReferences.put(getHandle(), this)
        v8.setWeak(v8.getV8RuntimePtr(), getHandle())
        return this
    }

    //weak to strong
    fun clearWeak(): V8Value {
        v8.checkThread()
        v8.checkReleased()
        v8.v8WeakReferences.remove(getHandle())
        v8.clearWeak(v8.getV8RuntimePtr(), getHandle())
        return this
    }

    //check value is weak?
    fun isWeak(): Boolean {
        v8.checkThread()
        v8.checkReleased()
        return v8.isWeak(v8.getV8RuntimePtr(), getHandle())
    }


    override fun close() {
        v8.checkThread();
        if (!released) {
            try {
                v8.releaseObjRef(this);
            } finally {
                released = true;
                v8.release(v8.getV8RuntimePtr(), objectHandle);
            }
        }
    }

    fun isReleased(): Boolean {
        return released
    }

    fun strictEquals(that: Any?): Boolean {
        v8.checkThread()
        checkReleased()
        if (that === this) {
            return true
        }
        if (that == null) {
            return false
        }
        if (that !is V8Value) {
            return false
        }
        if (isUndefined() && that.isUndefined()) {
            return true
        }
        return if (that.isUndefined()) {
            false
        } else v8.strictEquals(v8.getV8RuntimePtr(), getHandle(), that.getHandle())
    }

    protected fun getHandle(): Long {
        checkReleased()
        return objectHandle
    }

    protected abstract fun createTwin(): V8Value

    override fun equals(other: Any?): Boolean {
        return strictEquals(other)
    }

    fun jsEquals(that: Any?): Boolean {
        v8.checkThread()
        checkReleased()
        if (that === this) {
            return true
        }
        if (that == null) {
            return false
        }
        if (that !is V8Value) {
            return false
        }
        if (isUndefined() && that.isUndefined()) {
            return true
        }
        return if (that.isUndefined()) {
            false
        } else v8.equals(v8.getV8RuntimePtr(), getHandle(), that.getHandle())
    }

    override fun hashCode(): Int {
        v8.checkThread();
        checkReleased();
        return v8.identityHash(v8.getV8RuntimePtr(), getHandle());
    }

    protected fun checkReleased() {
        if (released) {
            throw IllegalStateException("Object released")
        }
    }

}