//
// Created by yineng on 2019/6/21.
//

#include <jni.h>

//export to java

extern "C" {
#undef com_piv8_v8_V8_NULL
#define com_piv8_v8_V8_NULL 0L
#undef com_piv8_v8_V8_UNKNOWN
#define com_piv8_v8_V8_UNKNOWN 0L
#undef com_piv8_v8_V8_INTEGER
#define com_piv8_v8_V8_INTEGER 1L
#undef com_piv8_v8_V8_INT_32_ARRAY
#define com_piv8_v8_V8_INT_32_ARRAY 1L
#undef com_piv8_v8_V8_DOUBLE
#define com_piv8_v8_V8_DOUBLE 2L
#undef com_piv8_v8_V8_FLOAT_64_ARRAY
#define com_piv8_v8_V8_FLOAT_64_ARRAY 2L
#undef com_piv8_v8_V8_BOOLEAN
#define com_piv8_v8_V8_BOOLEAN 3L
#undef com_piv8_v8_V8_STRING
#define com_piv8_v8_V8_STRING 4L
#undef com_piv8_v8_V8_V8_ARRAY
#define com_piv8_v8_V8_V8_ARRAY 5L
#undef com_piv8_v8_V8_V8_OBJECT
#define com_piv8_v8_V8_V8_OBJECT 6L
#undef com_piv8_v8_V8_V8_FUNCTION
#define com_piv8_v8_V8_V8_FUNCTION 7L
#undef com_piv8_v8_V8_V8_TYPED_ARRAY
#define com_piv8_v8_V8_V8_TYPED_ARRAY 8L
#undef com_piv8_v8_V8_BYTE
#define com_piv8_v8_V8_BYTE 9L
#undef com_piv8_v8_V8_INT_8_ARRAY
#define com_piv8_v8_V8_INT_8_ARRAY 9L
#undef com_piv8_v8_V8_V8_ARRAY_BUFFER
#define com_piv8_v8_V8_V8_ARRAY_BUFFER 10L
#undef com_piv8_v8_V8_UNSIGNED_INT_8_ARRAY
#define com_piv8_v8_V8_UNSIGNED_INT_8_ARRAY 11L
#undef com_piv8_v8_V8_UNSIGNED_INT_8_CLAMPED_ARRAY
#define com_piv8_v8_V8_UNSIGNED_INT_8_CLAMPED_ARRAY 12L
#undef com_piv8_v8_V8_INT_16_ARRAY
#define com_piv8_v8_V8_INT_16_ARRAY 13L
#undef com_piv8_v8_V8_UNSIGNED_INT_16_ARRAY
#define com_piv8_v8_V8_UNSIGNED_INT_16_ARRAY 14L
#undef com_piv8_v8_V8_UNSIGNED_INT_32_ARRAY
#define com_piv8_v8_V8_UNSIGNED_INT_32_ARRAY 15L
#undef com_piv8_v8_V8_FLOAT_32_ARRAY
#define com_piv8_v8_V8_FLOAT_32_ARRAY 16L
#undef com_piv8_v8_V8_UNDEFINED
#define com_piv8_v8_V8_UNDEFINED 99L

JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8Object
        (JNIEnv *, jobject, jlong);



JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initEmptyContainer
        (JNIEnv *, jobject, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _acquireLock
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1acquireLock
        (JNIEnv *, jobject, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _releaseLock
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1releaseLock
        (JNIEnv *, jobject, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _lowMemoryNotification
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1lowMemoryNotification
        (JNIEnv *, jobject, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _createTwin
 * Signature: (JJJ)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1createTwin
        (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _releaseRuntime
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1releaseRuntime
        (JNIEnv *, jobject, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _createIsolate
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1createIsolate
        (JNIEnv *, jobject, jstring);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _executeIntegerScript
 * Signature: (JLjava/lang/String;Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1executeIntegerScript
        (JNIEnv *, jobject, jlong, jstring, jstring, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _executeDoubleScript
 * Signature: (JLjava/lang/String;Ljava/lang/String;I)D
 */
JNIEXPORT jdouble JNICALL Java_com_yineng_piv8_V8__1executeDoubleScript
        (JNIEnv *, jobject, jlong, jstring, jstring, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _executeStringScript
 * Signature: (JLjava/lang/String;Ljava/lang/String;I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_yineng_piv8_V8__1executeStringScript
        (JNIEnv *, jobject, jlong, jstring, jstring, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _executeBooleanScript
 * Signature: (JLjava/lang/String;Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1executeBooleanScript
        (JNIEnv *, jobject, jlong, jstring, jstring, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _executeScript
 * Signature: (JILjava/lang/String;Ljava/lang/String;I)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_com_yineng_piv8_V8__1executeScript
        (JNIEnv *, jobject, jlong, jint, jstring, jstring, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _executeVoidScript
 * Signature: (JLjava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1executeVoidScript
        (JNIEnv *, jobject, jlong, jstring, jstring, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _release
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1release
        (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _releaseMethodDescriptor
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1releaseMethodDescriptor
        (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _contains
 * Signature: (JJLjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1contains
        (JNIEnv *, jobject, jlong, jlong, jstring);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _getKeys
 * Signature: (JJ)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_yineng_piv8_V8__1getKeys
        (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _getInteger
 * Signature: (JJLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1getInteger
        (JNIEnv *, jobject, jlong, jlong, jstring);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _getBoolean
 * Signature: (JJLjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1getBoolean
        (JNIEnv *, jobject, jlong, jlong, jstring);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _getDouble
 * Signature: (JJLjava/lang/String;)D
 */
JNIEXPORT jdouble JNICALL Java_com_yineng_piv8_V8__1getDouble
        (JNIEnv *, jobject, jlong, jlong, jstring);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _getString
 * Signature: (JJLjava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_yineng_piv8_V8__1getString
        (JNIEnv *, jobject, jlong, jlong, jstring);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _get
 * Signature: (JIJLjava/lang/String;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_com_yineng_piv8_V8__1get
        (JNIEnv *, jobject, jlong, jint, jlong, jstring);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _executeIntegerFunction
 * Signature: (JJLjava/lang/String;J)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1executeIntegerFunction
        (JNIEnv *, jobject, jlong, jlong, jstring, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _executeDoubleFunction
 * Signature: (JJLjava/lang/String;J)D
 */
JNIEXPORT jdouble JNICALL Java_com_yineng_piv8_V8__1executeDoubleFunction
        (JNIEnv *, jobject, jlong, jlong, jstring, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _executeStringFunction
 * Signature: (JJLjava/lang/String;J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_yineng_piv8_V8__1executeStringFunction
        (JNIEnv *, jobject, jlong, jlong, jstring, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _executeBooleanFunction
 * Signature: (JJLjava/lang/String;J)Z
 */
JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1executeBooleanFunction
        (JNIEnv *, jobject, jlong, jlong, jstring, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _executeFunction
 * Signature: (JIJLjava/lang/String;J)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_com_yineng_piv8_V8__1executeFunction__JIJLjava_lang_String_2J
        (JNIEnv *, jobject, jlong, jint, jlong, jstring, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _executeFunction
 * Signature: (JJJJ)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_com_yineng_piv8_V8__1executeFunction__JJJJ
        (JNIEnv *, jobject, jlong, jlong, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _executeVoidFunction
 * Signature: (JJLjava/lang/String;J)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1executeVoidFunction
        (JNIEnv *, jobject, jlong, jlong, jstring, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _equals
 * Signature: (JJJ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1equals
        (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _toString
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_yineng_piv8_V8__1toString
        (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _strictEquals
 * Signature: (JJJ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1strictEquals
        (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _sameValue
 * Signature: (JJJ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1sameValue
        (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _identityHash
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1identityHash
        (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _add
 * Signature: (JJLjava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1add__JJLjava_lang_String_2I
        (JNIEnv *, jobject, jlong, jlong, jstring, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _addObject
 * Signature: (JJLjava/lang/String;J)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addObject
        (JNIEnv *, jobject, jlong, jlong, jstring, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _add
 * Signature: (JJLjava/lang/String;Z)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1add__JJLjava_lang_String_2Z
        (JNIEnv *, jobject, jlong, jlong, jstring, jboolean);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _add
 * Signature: (JJLjava/lang/String;D)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1add__JJLjava_lang_String_2D
        (JNIEnv *, jobject, jlong, jlong, jstring, jdouble);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _add
 * Signature: (JJLjava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1add__JJLjava_lang_String_2Ljava_lang_String_2
        (JNIEnv *, jobject, jlong, jlong, jstring, jstring);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _addUndefined
 * Signature: (JJLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addUndefined
        (JNIEnv *, jobject, jlong, jlong, jstring);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _addNull
 * Signature: (JJLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addNull
        (JNIEnv *, jobject, jlong, jlong, jstring);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _registerJavaMethod
 * Signature: (JJLjava/lang/String;Z)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1registerJavaMethod
        (JNIEnv *, jobject, jlong, jlong, jstring, jboolean);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _initNewV8Array
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8Array
        (JNIEnv *, jobject, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _initNewV8Function
 * Signature: (J)[J
 */
JNIEXPORT jlongArray JNICALL Java_com_yineng_piv8_V8__1initNewV8Function
        (JNIEnv *, jobject, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGetSize
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetSize
        (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGetInteger
 * Signature: (JJI)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetInteger
        (JNIEnv *, jobject, jlong, jlong, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGetBoolean
 * Signature: (JJI)Z
 */
JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1arrayGetBoolean
        (JNIEnv *, jobject, jlong, jlong, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGetByte
 * Signature: (JJI)B
 */
JNIEXPORT jbyte JNICALL Java_com_yineng_piv8_V8__1arrayGetByte
        (JNIEnv *, jobject, jlong, jlong, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGetDouble
 * Signature: (JJI)D
 */
JNIEXPORT jdouble JNICALL Java_com_yineng_piv8_V8__1arrayGetDouble
        (JNIEnv *, jobject, jlong, jlong, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGetString
 * Signature: (JJI)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_yineng_piv8_V8__1arrayGetString
        (JNIEnv *, jobject, jlong, jlong, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGet
 * Signature: (JIJI)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_com_yineng_piv8_V8__1arrayGet
        (JNIEnv *, jobject, jlong, jint, jlong, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _addArrayIntItem
 * Signature: (JJI)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addArrayIntItem
        (JNIEnv *, jobject, jlong, jlong, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _addArrayBooleanItem
 * Signature: (JJZ)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addArrayBooleanItem
        (JNIEnv *, jobject, jlong, jlong, jboolean);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _addArrayDoubleItem
 * Signature: (JJD)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addArrayDoubleItem
        (JNIEnv *, jobject, jlong, jlong, jdouble);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _addArrayStringItem
 * Signature: (JJLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addArrayStringItem
        (JNIEnv *, jobject, jlong, jlong, jstring);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _addArrayObjectItem
 * Signature: (JJJ)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addArrayObjectItem
        (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _addArrayUndefinedItem
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addArrayUndefinedItem
        (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _addArrayNullItem
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addArrayNullItem
        (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _getType
 * Signature: (JJLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1getType__JJLjava_lang_String_2
        (JNIEnv *, jobject, jlong, jlong, jstring);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _getType
 * Signature: (JJI)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1getType__JJI
        (JNIEnv *, jobject, jlong, jlong, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _getArrayType
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1getArrayType
        (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _setPrototype
 * Signature: (JJJ)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1setPrototype
        (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _getConstructorName
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_yineng_piv8_V8__1getConstructorName
        (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _getType
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1getType__JJ
        (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _getType
 * Signature: (JJII)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1getType__JJII
        (JNIEnv *, jobject, jlong, jlong, jint, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGetDoubles
 * Signature: (JJII)[D
 */
JNIEXPORT jdoubleArray JNICALL Java_com_yineng_piv8_V8__1arrayGetDoubles__JJII
        (JNIEnv *, jobject, jlong, jlong, jint, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGetIntegers
 * Signature: (JJII)[I
 */
JNIEXPORT jintArray JNICALL Java_com_yineng_piv8_V8__1arrayGetIntegers__JJII
        (JNIEnv *, jobject, jlong, jlong, jint, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGetBooleans
 * Signature: (JJII)[Z
 */
JNIEXPORT jbooleanArray JNICALL Java_com_yineng_piv8_V8__1arrayGetBooleans__JJII
        (JNIEnv *, jobject, jlong, jlong, jint, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGetBytes
 * Signature: (JJII)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_yineng_piv8_V8__1arrayGetBytes__JJII
        (JNIEnv *, jobject, jlong, jlong, jint, jint);


JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetDoubles__JJIILdouble_3_093_2
        (JNIEnv *env, jobject instance, jlong v8RuntimePtr,jlong objectHandle, jint index, jint length,jdoubleArray resultArray_);

JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetIntegers__JJIILint_3_093_2
        (JNIEnv *env, jobject instance, jlong v8RuntimePtr,jlong objectHandle, jint index, jint length,jintArray resultArray_);

JNIEXPORT jobjectArray JNICALL Java_com_yineng_piv8_V8__1arrayGetStrings__JJII
        (JNIEnv *env, jobject instance, jlong v8RuntimePtr, jlong objectHandle,jint index, jint length);

JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetBooleans__JJIILboolean_3_093_2
        (JNIEnv *env, jobject instance, jlong v8RuntimePtr,jlong objectHandle, jint index, jint length, jbooleanArray resultArray_);

JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetBytes__JJIILbyte_3_093_2
        (JNIEnv *env, jobject instance, jlong v8RuntimePtr,jlong objectHandle, jint index, jint length,jbyteArray resultArray_);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGetStrings
 * Signature: (JJII)[Ljava/lang/String;
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetStrings__JJIILjava_lang_String_3_093_2
        (JNIEnv *env, jobject instance, jlong v8RuntimePtr, jlong objectHandle, jint index, jint length, jobjectArray resultArray) ;

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _initNewV8ArrayBuffer
 * Signature: (JI)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8ArrayBuffer__JI
        (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _initNewV8ArrayBuffer
 * Signature: (JLjava/nio/ByteBuffer;I)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8ArrayBuffer__JLjava_nio_ByteBuffer_2I
        (JNIEnv *, jobject, jlong, jobject, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _initNewV8Int32Array
 * Signature: (JJII)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8Int32Array
        (JNIEnv *, jobject, jlong, jlong, jint, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _initNewV8UInt32Array
 * Signature: (JJII)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8UInt32Array
        (JNIEnv *, jobject, jlong, jlong, jint, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _initNewV8Float32Array
 * Signature: (JJII)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8Float32Array
        (JNIEnv *, jobject, jlong, jlong, jint, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _initNewV8Float64Array
 * Signature: (JJII)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8Float64Array
        (JNIEnv *, jobject, jlong, jlong, jint, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _initNewV8Int16Array
 * Signature: (JJII)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8Int16Array
        (JNIEnv *, jobject, jlong, jlong, jint, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _initNewV8UInt16Array
 * Signature: (JJII)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8UInt16Array
        (JNIEnv *, jobject, jlong, jlong, jint, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _initNewV8Int8Array
 * Signature: (JJII)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8Int8Array
        (JNIEnv *, jobject, jlong, jlong, jint, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _initNewV8UInt8Array
 * Signature: (JJII)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8UInt8Array
        (JNIEnv *, jobject, jlong, jlong, jint, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _initNewV8UInt8ClampedArray
 * Signature: (JJII)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8UInt8ClampedArray
        (JNIEnv *, jobject, jlong, jlong, jint, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _setWeak
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1setWeak
        (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _clearWeak
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1clearWeak
        (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _isWeak
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1isWeak
        (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _createV8ArrayBufferBackingStore
 * Signature: (JJI)Ljava/nio/ByteBuffer;
 */
JNIEXPORT jobject JNICALL Java_com_yineng_piv8_V8__1createV8ArrayBufferBackingStore
        (JNIEnv *, jobject, jlong, jlong, jint);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _getVersion
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_yineng_piv8_V8__1getVersion
        (JNIEnv *, jclass);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _setFlags
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1setFlags
        (JNIEnv *, jclass, jstring);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _terminateExecution
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1terminateExecution
        (JNIEnv *, jobject, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _getGlobalObject
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1getGlobalObject
        (JNIEnv *, jobject, jlong);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _getBuildID
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1getBuildID
        (JNIEnv *, jobject);


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1connect
        (JNIEnv *, jobject, jlong, jobject);

JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1disconnect
        (JNIEnv *, jobject, jlong );

JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1dispatchMessage
        (JNIEnv *, jobject, jlong,jstring);

JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1initDebugger
        (JNIEnv *env, jobject , jlong);

}

