//
// Created by yineng on 2019/6/21.
//

#include <include/v8.h>
#include <iostream>
#include "com_yineng_piv8_impl.h"
#include <natives_blob.h>
#include <snapshot_blob.h>
#include <libplatform/libplatform.h>
#include "util.h"

using namespace std;
using namespace v8;

class MethodDescriptor {
public:
    jlong methodID;
    jlong v8RuntimePtr;
};

class WeakReferenceDescriptor {
public:
    jlong v8RuntimePtr;
    jlong objectHandle;
};

class V8Runtime {
public:
    Isolate* isolate;
    Persistent<Context> context_;
    Persistent<Object>* globalObject;
    Locker* locker;
    jobject v8;
    jthrowable pendingException;
};

v8::Platform* v8Platform;

const char* ToCString(const String::Utf8Value& value) {
    return *value ? *value : "<string conversion failed>";
}

JavaVM* jvm = NULL;
jclass v8cls = NULL;
jclass v8ObjectCls = NULL;
jclass v8ArrayCls = NULL;
jclass v8TypedArrayCls = NULL;
jclass v8ArrayBufferCls = NULL;
jclass v8FunctionCls = NULL;
jclass undefinedV8ObjectCls = NULL;
jclass undefinedV8ArrayCls = NULL;
jclass v8ResultsUndefinedCls = NULL;
jclass v8ScriptCompilationCls = NULL;
jclass v8ScriptExecutionException = NULL;
jclass v8RuntimeExceptionCls = NULL;
jclass throwableCls = NULL;
jclass stringCls = NULL;
jclass integerCls = NULL;
jclass doubleCls = NULL;
jclass booleanCls = NULL;
jclass errorCls = NULL;
jclass unsupportedOperationExceptionCls = NULL;
jmethodID v8ArrayInitMethodID = NULL;
jmethodID v8TypedArrayInitMethodID = NULL;
jmethodID v8ArrayBufferInitMethodID = NULL;
jmethodID v8ArrayGetHandleMethodID = NULL;
jmethodID v8CallVoidMethodID = NULL;
jmethodID v8ObjectReleaseMethodID = NULL;
jmethodID v8DisposeMethodID = NULL;
jmethodID v8WeakReferenceReleased = NULL;
jmethodID v8ArrayReleaseMethodID = NULL;
jmethodID v8ObjectIsUndefinedMethodID = NULL;
jmethodID v8ObjectGetHandleMethodID = NULL;
jmethodID throwableGetMessageMethodID = NULL;
jmethodID integerIntValueMethodID = NULL;
jmethodID booleanBoolValueMethodID = NULL;
jmethodID doubleDoubleValueMethodID = NULL;
jmethodID v8CallObjectJavaMethodMethodID = NULL;
jmethodID v8ScriptCompilationInitMethodID = NULL;
jmethodID v8ScriptExecutionExceptionInitMethodID = NULL;
jmethodID undefinedV8ArrayInitMethodID = NULL;
jmethodID undefinedV8ObjectInitMethodID = NULL;
jmethodID integerInitMethodID = NULL;
jmethodID doubleInitMethodID = NULL;
jmethodID booleanInitMethodID = NULL;
jmethodID v8FunctionInitMethodID = NULL;
jmethodID v8ObjectInitMethodID = NULL;
jmethodID v8RuntimeExceptionInitMethodID = NULL;

void throwParseException(JNIEnv *env, Isolate* isolate, TryCatch* tryCatch);
void throwExecutionException(JNIEnv *env, Isolate* isolate, TryCatch* tryCatch, jlong v8RuntimePtr);
void throwError(JNIEnv *env, const char *message);
void throwV8RuntimeException(JNIEnv *env,  String::Value *message);
void throwResultUndefinedException(JNIEnv *env, const char *message);
Isolate* getIsolate(JNIEnv *env, jlong handle);
int getType(Handle<Value> v8Value);
jobject getResult(JNIEnv *env, jobject &v8, jlong v8RuntimePtr, Handle<Value> &result, jint expectedType);

#define SETUP(env, v8RuntimePtr, errorReturnResult) getIsolate(env, v8RuntimePtr);\
    if ( isolate == NULL ) {\
      return errorReturnResult;\
                                }\
    V8Runtime* runtime = reinterpret_cast<V8Runtime*>(v8RuntimePtr);\
    Isolate::Scope isolateScope(isolate);\
    HandleScope handle_scope(isolate);\
    Local<Context> context = Local<Context>::New(isolate,runtime->context_);\
    Context::Scope context_scope(context);
#define ASSERT_IS_NUMBER(v8Value) \
    if (v8Value.IsEmpty() || v8Value->IsUndefined() || !v8Value->IsNumber()) {\
      throwResultUndefinedException(env, "");\
      return 0;\
                                }
#define ASSERT_IS_STRING(v8Value)\
    if (v8Value.IsEmpty() || v8Value->IsUndefined() || !v8Value->IsString()) {\
      if ( v8Value->IsNull() ) {\
        return 0;\
      }\
      throwResultUndefinedException(env, "");\
      return 0;\
                                }
#define ASSERT_IS_BOOLEAN(v8Value)\
    if (v8Value.IsEmpty() || v8Value->IsUndefined() || !v8Value->IsBoolean() ) {\
      throwResultUndefinedException(env, "");\
      return 0;\
                                }
void release(JNIEnv* env, jobject object) {
    env->CallVoidMethod(object, v8ObjectReleaseMethodID);
}

void releaseArray(JNIEnv* env, jobject object) {
    env->CallVoidMethod(object, v8ArrayReleaseMethodID);
}

int isUndefined(JNIEnv* env, jobject object) {
    return env->CallBooleanMethod(object, v8ObjectIsUndefinedMethodID);
}

jlong getHandle(JNIEnv* env, jobject object) {
    return env->CallLongMethod(object, v8ObjectGetHandleMethodID);
}


Local<String> createV8String(JNIEnv *env, Isolate *isolate, jstring &string) {
    const uint16_t* unicodeString = env->GetStringChars(string, NULL);
    int length = env->GetStringLength(string);
    Local<String> result = String::NewFromTwoByte(isolate, unicodeString, String::NewStringType::kNormalString, length);
    env->ReleaseStringChars(string, unicodeString);
    return result;
}

Handle<Value> getValueWithKey(JNIEnv* env, Isolate* isolate, jlong &v8RuntimePtr, jlong &objectHandle, jstring &key) {
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    Local<String> v8Key = createV8String(env, isolate, key);
    return object->Get(v8Key);
}

void addValueWithKey(JNIEnv* env, Isolate* isolate, jlong &v8RuntimePtr, jlong &objectHandle, jstring &key, Handle<Value> value) {
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    const uint16_t* unicodeString_key = env->GetStringChars(key, NULL);
    int length = env->GetStringLength(key);
    Local<String> v8Key = String::NewFromTwoByte(isolate, unicodeString_key, String::NewStringType::kNormalString, length);
    object->Set(v8Key, value);
    env->ReleaseStringChars(key, unicodeString_key);
}

void getJNIEnv(JNIEnv*& env) {
    int getEnvStat = jvm->GetEnv((void **)&env, JNI_VERSION_1_6);
    if (getEnvStat == JNI_EDETACHED) {
        if (jvm->AttachCurrentThread(&env, NULL) != 0) {
            std::cout << "Failed to attach" << std::endl;
        }
    }
}

static void jsWindowObjectAccessor(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    info.GetReturnValue().Set(info.GetIsolate()->GetCurrentContext()->Global());
}

class ShellArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
public:
    virtual void* Allocate(size_t length) {
        void* data = AllocateUninitialized(length);
        return data == NULL ? data : memset(data, 0, length);
    }
    virtual void* AllocateUninitialized(size_t length) { return malloc(length); }
    virtual void Free(void* data, size_t) { free(data); }
};


ScriptOrigin * createScriptOrigin(JNIEnv * env, Isolate* isolate, jstring &jscriptName, jint jlineNumber = 0) {
    Local<String> scriptName = createV8String(env, isolate, jscriptName);
    return new ScriptOrigin(scriptName, Integer::New(isolate, jlineNumber));
}

bool compileScript(Isolate *isolate, jstring &jscript, JNIEnv *env, jstring jscriptName, jint &jlineNumber, Local<Script> &script, TryCatch* tryCatch) {
    Local<String> source = createV8String(env, isolate, jscript);
    MaybeLocal<String> scriptOriginPtr = NULL;
    if (jscriptName != NULL) {
        scriptOriginPtr = createScriptOrigin(env, isolate, jscriptName, jlineNumber);
    }
    script = Script::Compile(source, scriptOriginPtr.ToLocalChecked()).ToLocalChecked();
    if (scriptOriginPtr.IsEmpty()) {
        delete(scriptOriginPtr);
    }
    if (tryCatch->HasCaught()) {
        throwParseException(env, isolate, tryCatch);
        return false;
    }
    return true;
}

bool runScript(Isolate* isolate, JNIEnv *env, Local<Script> *script, TryCatch* tryCatch, jlong v8RuntimePtr) {
    (*script)->Run(isolate->GetCurrentContext());
    if (tryCatch->HasCaught()) {
        throwExecutionException(env, isolate, tryCatch, v8RuntimePtr);
        return false;
    }
    return true;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    jint onLoad_err = -1;
    if ( vm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK ) {
        return onLoad_err;
    }
    if (env == NULL) {
        return onLoad_err;
    }
    //快照
    auto* nativesBlobStartupData = new StartupData();
    nativesBlobStartupData->data = reinterpret_cast<const char*>(&natives_blob_bin[0]);
    nativesBlobStartupData->raw_size = natives_blob_bin_len;
    V8::SetNativesDataBlob(nativesBlobStartupData);
    auto* snapshotBlobStartupData = new StartupData();
    snapshotBlobStartupData->data = reinterpret_cast<const char *>(&snapshot_blob_bin[0]);
    snapshotBlobStartupData->raw_size = snapshot_blob_bin_len;
    V8::SetSnapshotDataBlob(snapshotBlobStartupData);
    v8::V8::InitializeICU();
    v8Platform = v8::platform::NewDefaultPlatform().release();
    v8::V8::InitializePlatform(v8Platform);
    v8::V8::Initialize();

    jvm = vm;
    v8cls = (jclass)env->NewGlobalRef((env)->FindClass("com/yineng/piv8/V8"));
    v8ObjectCls = (jclass)env->NewGlobalRef((env)->FindClass("com/yineng/piv8/V8Object"));
    v8ArrayCls = (jclass)env->NewGlobalRef((env)->FindClass("com/yineng/piv8/V8Array"));
    v8TypedArrayCls = (jclass)env->NewGlobalRef((env)->FindClass("com/yineng/piv8/V8TypedArray"));
    v8ArrayBufferCls = (jclass)env->NewGlobalRef((env)->FindClass("com/yineng/piv8/V8ArrayBuffer"));
    v8FunctionCls = (jclass)env->NewGlobalRef((env)->FindClass("com/yineng/piv8/V8Function"));
    undefinedV8ObjectCls = (jclass)env->NewGlobalRef((env)->FindClass("com/yineng/piv8/V8Object$Undefined"));
    undefinedV8ArrayCls = (jclass)env->NewGlobalRef((env)->FindClass("com/yineng/piv8/V8Array$Undefined"));
    stringCls = (jclass)env->NewGlobalRef((env)->FindClass("java/lang/String"));
    integerCls = (jclass)env->NewGlobalRef((env)->FindClass("java/lang/Integer"));
    doubleCls = (jclass)env->NewGlobalRef((env)->FindClass("java/lang/Double"));
    booleanCls = (jclass)env->NewGlobalRef((env)->FindClass("java/lang/Boolean"));
    throwableCls = (jclass)env->NewGlobalRef((env)->FindClass("java/lang/Throwable"));
    v8ResultsUndefinedCls = (jclass)env->NewGlobalRef((env)->FindClass("com/yineng/piv8/V8ResultUndefined"));
    v8ScriptCompilationCls = (jclass)env->NewGlobalRef((env)->FindClass("com/yineng/piv8/V8ScriptCompilationException"));
    v8ScriptExecutionException = (jclass)env->NewGlobalRef((env)->FindClass("com/yineng/piv8/V8ScriptExecutionException"));
    v8RuntimeExceptionCls = (jclass)env->NewGlobalRef((env)->FindClass("com/yineng/piv8/V8RuntimeException"));
    errorCls = (jclass)env->NewGlobalRef((env)->FindClass("java/lang/Error"));
    unsupportedOperationExceptionCls = (jclass)env->NewGlobalRef((env)->FindClass("java/lang/UnsupportedOperationException"));

    // Get all method IDs
    v8ArrayInitMethodID = env->GetMethodID(v8ArrayCls, "<init>", "(Lcom/yineng/piv8/V8;)V");
    v8TypedArrayInitMethodID = env->GetMethodID(v8TypedArrayCls, "<init>", "(Lcom/yineng/piv8/V8;)V");
    v8ArrayBufferInitMethodID = env->GetMethodID(v8ArrayBufferCls, "<init>", "(Lcom/yineng/piv8/V8;Ljava/nio/ByteBuffer;)V");
    v8ArrayGetHandleMethodID = env->GetMethodID(v8ArrayCls, "getHandle", "()J");
    v8CallVoidMethodID = (env)->GetMethodID(v8cls, "callVoidJavaMethod", "(JLcom/yineng/piv8/V8Object;Lcom/yineng/piv8/V8Array;)V");
    v8ObjectReleaseMethodID = env->GetMethodID(v8ObjectCls, "release", "()V");
    v8ArrayReleaseMethodID = env->GetMethodID(v8ArrayCls, "release", "()V");
    v8ObjectIsUndefinedMethodID = env->GetMethodID(v8ObjectCls, "isUndefined", "()Z");
    v8ObjectGetHandleMethodID = env->GetMethodID(v8ObjectCls, "getHandle", "()J");
    throwableGetMessageMethodID = env->GetMethodID(throwableCls, "getMessage", "()Ljava/lang/String;");
    integerIntValueMethodID = env->GetMethodID(integerCls, "intValue", "()I");
    booleanBoolValueMethodID = env->GetMethodID(booleanCls, "booleanValue", "()Z");
    doubleDoubleValueMethodID = env->GetMethodID(doubleCls, "doubleValue", "()D");
    v8CallObjectJavaMethodMethodID = (env)->GetMethodID(v8cls, "callObjectJavaMethod", "(JLcom/yineng/piv8/V8Object;Lcom/yineng/piv8/V8Array;)Ljava/lang/Object;");
    v8DisposeMethodID = (env)->GetMethodID(v8cls, "disposeMethodID", "(J)V");
    v8WeakReferenceReleased = (env)->GetMethodID(v8cls, "weakReferenceReleased", "(J)V");
    v8ScriptCompilationInitMethodID = env->GetMethodID(v8ScriptCompilationCls, "<init>", "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;II)V");
    v8ScriptExecutionExceptionInitMethodID = env->GetMethodID(v8ScriptExecutionException, "<init>", "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;IILjava/lang/String;Ljava/lang/Throwable;)V");
    undefinedV8ArrayInitMethodID = env->GetMethodID(undefinedV8ArrayCls, "<init>", "()V");
    undefinedV8ObjectInitMethodID = env->GetMethodID(undefinedV8ObjectCls, "<init>", "()V");
    v8RuntimeExceptionInitMethodID = env->GetMethodID(v8RuntimeExceptionCls, "<init>", "(Ljava/lang/String;)V");
    integerInitMethodID = env->GetMethodID(integerCls, "<init>", "(I)V");
    doubleInitMethodID = env->GetMethodID(doubleCls, "<init>", "(D)V");
    booleanInitMethodID = env->GetMethodID(booleanCls, "<init>", "(Z)V");
    v8FunctionInitMethodID = env->GetMethodID(v8FunctionCls, "<init>", "(Lcom/yineng/piv8/V8;)V");
    v8ObjectInitMethodID = env->GetMethodID(v8ObjectCls, "<init>", "(Lcom/yineng/piv8/V8;)V");

    return JNI_VERSION_1_6;

}


ShellArrayBufferAllocator array_buffer_allocator;


JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8Object(JNIEnv *env, jobject, jlong v8RuntimePtr){
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Local<Object> obj = Object::New(isolate);
    Persistent<Object>* container = new Persistent<Object>;
    container->Reset(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate, obj);
    return reinterpret_cast<jlong>(container);
}



JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initEmptyContainer(JNIEnv *env, jobject, jlong v8RuntimePtr){
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Persistent<Object>* container = new Persistent<Object>;
    return reinterpret_cast<jlong>(container);
}

JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1acquireLock(JNIEnv *env, jobject, jlong v8RuntimePtr){
    V8Runtime* runtime = reinterpret_cast<V8Runtime*>(v8RuntimePtr);
    if(runtime->isolate->InContext()) {
        jstring exceptionString = env->NewStringUTF("Cannot acquire lock while in a V8 Context");
        jthrowable exception = (jthrowable)env->NewObject(v8RuntimeExceptionCls, v8RuntimeExceptionInitMethodID, exceptionString);
        (env)->Throw(exception);
        env->DeleteLocalRef(exceptionString);
        return;
    }
    runtime->locker = new Locker(runtime->isolate);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1releaseLock(JNIEnv *env, jobject, jlong v8RuntimePtr){
    V8Runtime* runtime = reinterpret_cast<V8Runtime*>(v8RuntimePtr);
    if(runtime->isolate->InContext()) {
        jstring exceptionString = env->NewStringUTF("Cannot release lock while in a V8 Context");
        jthrowable exception = (jthrowable)env->NewObject(v8RuntimeExceptionCls, v8RuntimeExceptionInitMethodID, exceptionString);
        (env)->Throw(exception);
        env->DeleteLocalRef(exceptionString);
        return;
    }
    delete(runtime->locker);
    runtime->locker = NULL;
}

JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1lowMemoryNotification(JNIEnv *env, jobject, jlong v8RuntimePtr){
    V8Runtime* runtime = reinterpret_cast<V8Runtime*>(v8RuntimePtr);
    runtime->isolate->LowMemoryNotification();
}

JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1createTwin(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jlong twinObjectHandle){
    Isolate* isolate = SETUP(env, v8RuntimePtr,);
    Handle<Object> obj = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    reinterpret_cast<Persistent<Object>*>(twinObjectHandle)->Reset(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate, obj);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1releaseRuntime(JNIEnv *env, jobject, jlong v8RuntimePtr) {
    if (v8RuntimePtr == 0) {
        return;
    }
    Isolate* isolate = getIsolate(env, v8RuntimePtr);
    reinterpret_cast<V8Runtime*>(v8RuntimePtr)->context_.Reset();
    reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate->Dispose();
    env->DeleteGlobalRef(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->v8);
    V8Runtime* runtime = reinterpret_cast<V8Runtime*>(v8RuntimePtr);
    delete(reinterpret_cast<V8Runtime*>(v8RuntimePtr));
}


JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1createIsolate(JNIEnv *env, jobject v8, jstring globalAlias){
    V8Runtime *runtime = new V8Runtime();
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = &array_buffer_allocator;
    runtime->isolate = v8::Isolate::New(create_params);
    runtime->locker = new Locker(runtime->isolate);
    v8::Isolate::Scope isolate_scope(runtime->isolate);
    runtime->v8 = env->NewGlobalRef(v8);
    runtime->pendingException = NULL;
    HandleScope handle_scope(runtime->isolate);
    Handle<ObjectTemplate> globalObject = ObjectTemplate::New(runtime->isolate);
    if (globalAlias == NULL) {
        Handle<Context> context = Context::New(runtime->isolate, NULL, globalObject);
        runtime->context_.Reset(runtime->isolate, context);
        runtime->globalObject = new Persistent<Object>;
        runtime->globalObject->Reset(runtime->isolate, context->Global()->GetPrototype()->ToObject(runtime->isolate));
    }
    else {
        Local<String> utfAlias = createV8String(env, runtime->isolate, globalAlias);
        globalObject->SetAccessor(utfAlias, jsWindowObjectAccessor);
        Handle<Context> context = Context::New(runtime->isolate, NULL, globalObject);
        runtime->context_.Reset(runtime->isolate, context);
        runtime->globalObject = new Persistent<Object>;
        runtime->globalObject->Reset(runtime->isolate, context->Global()->GetPrototype()->ToObject(runtime->isolate));
    }
    delete(runtime->locker);
    return reinterpret_cast<jlong>(runtime);
}

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


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1release(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle) {
    if (v8RuntimePtr == 0) {
        return;
    }
    Isolate* isolate = getIsolate(env, v8RuntimePtr);
    Locker locker(isolate);
    HandleScope handle_scope(isolate);
    reinterpret_cast<Persistent<Object>*>(objectHandle)->Reset();
    delete(reinterpret_cast<Persistent<Object>*>(objectHandle));
}

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _releaseMethodDescriptor
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1releaseMethodDescriptor
        (JNIEnv *, jobject, jlong, jlong);


JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1contains(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jstring key) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, false);
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    Local<String> v8Key = createV8String(env, isolate, key);
    return util::ToJBool(object->Has(isolate->GetCurrentContext(),v8Key).ToChecked());
}


JNIEXPORT jobjectArray JNICALL Java_com_yineng_piv8_V8__1getKeys(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, NULL);
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    Local<Array> properties = object->GetOwnPropertyNames(isolate->GetCurrentContext()).ToLocalChecked();
    int size = properties->Length();
    jobjectArray keys = (env)->NewObjectArray(size, stringCls, NULL);
    for (int i = 0; i < size; i++) {
        String::Value unicodeString(isolate,properties->Get(i)->ToString(isolate));
        jobject key = (env)->NewString(*unicodeString, unicodeString.length());
        (env)->SetObjectArrayElement(keys, i, key);
        (env)->DeleteLocalRef(key);
    }
    return keys;
}

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
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8Array(JNIEnv *env, jobject, jlong v8RuntimePtr){
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Local<Array> array = Array::New(isolate);
    Persistent<Object>* container = new Persistent<Object>;
    container->Reset(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate, array);
    return reinterpret_cast<jlong>(container);
}

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

JNIEXPORT jstring JNICALL Java_com_yineng_piv8_V8__1getConstructorName(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle){
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    String::Value unicodeString(isolate,object->GetConstructorName());
    return env->NewString(*unicodeString, unicodeString.length());
}

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
 * Method:    _arrayGetIntegers
 * Signature: (JJII[I)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetIntegers__JJII_3I
        (JNIEnv *, jobject, jlong, jlong, jint, jint, jintArray);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGetDoubles
 * Signature: (JJII[D)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetDoubles__JJII_3D
        (JNIEnv *, jobject, jlong, jlong, jint, jint, jdoubleArray);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGetBooleans
 * Signature: (JJII[Z)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetBooleans__JJII_3Z
        (JNIEnv *, jobject, jlong, jlong, jint, jint, jbooleanArray);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGetBytes
 * Signature: (JJII[B)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetBytes__JJII_3B
        (JNIEnv *, jobject, jlong, jlong, jint, jint, jbyteArray);

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _arrayGetStrings
 * Signature: (JJII[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetStrings__JJII_3Ljava_lang_String_2
        (JNIEnv *, jobject, jlong, jlong, jint, jint, jobjectArray);


JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8ArrayBuffer__JI(JNIEnv *env, jobject, jlong v8RuntimePtr, jint capacity) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(isolate, capacity);
    Persistent<Object>* container = new Persistent<Object>;
    container->Reset(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate, arrayBuffer);
    return reinterpret_cast<jlong>(container);
}


JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8ArrayBuffer__JLjava_nio_ByteBuffer_2I(JNIEnv *env, jobject, jlong v8RuntimePtr, jobject byteBuffer, jint capacity) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(isolate, env->GetDirectBufferAddress(byteBuffer), capacity);
    Persistent<Object>* container = new Persistent<Object>;
    container->Reset(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate, arrayBuffer);
    return reinterpret_cast<jlong>(container);
}


JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8Int32Array(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong bufferHandle, jint offset, jint length) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::New(isolate, *reinterpret_cast<Persistent<ArrayBuffer>*>(bufferHandle));
    Local<Int32Array> array = Int32Array::New(arrayBuffer, offset, length);
    Persistent<Object>* container = new Persistent<Object>;
    container->Reset(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate, array);
    return reinterpret_cast<jlong>(container);
}


JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8UInt32Array(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong bufferHandle, jint offset, jint length) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::New(isolate, *reinterpret_cast<Persistent<ArrayBuffer>*>(bufferHandle));
    Local<Uint32Array> array = Uint32Array::New(arrayBuffer, offset, length);
    Persistent<Object>* container = new Persistent<Object>;
    container->Reset(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate, array);
    return reinterpret_cast<jlong>(container);
}


JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8Float32Array(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong bufferHandle, jint offset, jint length) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::New(isolate, *reinterpret_cast<Persistent<ArrayBuffer>*>(bufferHandle));
    Local<Float32Array> array = Float32Array::New(arrayBuffer, offset, length);
    Persistent<Object>* container = new Persistent<Object>;
    container->Reset(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate, array);
    return reinterpret_cast<jlong>(container);
}


JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8Float64Array(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong bufferHandle, jint offset, jint length) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::New(isolate, *reinterpret_cast<Persistent<ArrayBuffer>*>(bufferHandle));
    Local<Float64Array> array = Float64Array::New(arrayBuffer, offset, length);
    Persistent<Object>* container = new Persistent<Object>;
    container->Reset(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate, array);
    return reinterpret_cast<jlong>(container);
}

JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8Int16Array(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong bufferHandle, jint offset, jint length) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::New(isolate, *reinterpret_cast<Persistent<ArrayBuffer>*>(bufferHandle));
    Local<Int16Array> array = Int16Array::New(arrayBuffer, offset, length);
    Persistent<Object>* container = new Persistent<Object>;
    container->Reset(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate, array);
    return reinterpret_cast<jlong>(container);
}

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _initNewV8UInt16Array
 * Signature: (JJII)J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8UInt16Array
        (JNIEnv *, jobject, jlong, jlong, jint, jint);


JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8Int8Array(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong bufferHandle, jint offset, jint length){
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::New(isolate, *reinterpret_cast<Persistent<ArrayBuffer>*>(bufferHandle));
    Local<Int8Array> array = Int8Array::New(arrayBuffer, offset, length);
    Persistent<Object>* container = new Persistent<Object>;
    container->Reset(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate, array);
    return reinterpret_cast<jlong>(container);
}


JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8UInt8Array(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong bufferHandle, jint offset, jint length){
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::New(isolate, *reinterpret_cast<Persistent<ArrayBuffer>*>(bufferHandle));
    Local<Uint8Array> array = Uint8Array::New(arrayBuffer, offset, length);
    Persistent<Object>* container = new Persistent<Object>;
    container->Reset(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate, array);
    return reinterpret_cast<jlong>(container);
}


JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8UInt8ClampedArray(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong bufferHandle, jint offset, jint length) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::New(isolate, *reinterpret_cast<Persistent<ArrayBuffer>*>(bufferHandle));
    Local<Uint8ClampedArray> array = Uint8ClampedArray::New(arrayBuffer, offset, length);
    Persistent<Object>* container = new Persistent<Object>;
    container->Reset(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate, array);
    return reinterpret_cast<jlong>(container);
}

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


JNIEXPORT jobject JNICALL Java_com_yineng_piv8_V8__1createV8ArrayBufferBackingStore(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jint capacity) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::New(isolate, *reinterpret_cast<Persistent<ArrayBuffer>*>(objectHandle));
    void* dataPtr = arrayBuffer->GetContents().Data();
    jobject byteBuffer = env->NewDirectByteBuffer(arrayBuffer->GetContents().Data(), capacity);
    return byteBuffer;
}

JNIEXPORT jstring JNICALL Java_com_yineng_piv8_V8__1getVersion(JNIEnv *env, jclass){
    const char *utfstring = v8::V8::GetVersion();
    return env->NewStringUTF(utfstring);
}

JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1setFlags(JNIEnv *env, jclass, jstring v8flags){
    if (v8flags) {
        char const* str = env->GetStringUTFChars(v8flags, NULL);
        v8::V8::SetFlagsFromString(str, env->GetStringUTFLength(v8flags));
        env->ReleaseStringUTFChars(v8flags, str);
    }
    v8::V8::Initialize();
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1terminateExecution(JNIEnv * env, jobject, jlong v8RuntimePtr) {
    if (v8RuntimePtr == 0) {
        return;
    }
    Isolate* isolate = getIsolate(env, v8RuntimePtr);
    isolate->TerminateExecution();
    return;
}

JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1getGlobalObject(JNIEnv *env, jobject, jlong v8RuntimePtr){
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Local<Object> obj = Object::New(isolate);
    reinterpret_cast<V8Runtime*>(v8RuntimePtr)->globalObject->Reset(isolate,obj);
    return reinterpret_cast<jlong>(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->globalObject);
}

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _getBuildID
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1getBuildID
        (JNIEnv *, jobject);


JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1pumpMessageLoop(JNIEnv *env, jclass, jlong){
    (env)->ThrowNew(unsupportedOperationExceptionCls, "pumpMessageLoop Not Supported.");
    return false;
}


JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1isRunning(JNIEnv *env, jclass, jlong){
    (env)->ThrowNew(unsupportedOperationExceptionCls, "isRunning Not Supported.");
    return false;
}

/*
 * Class:     com_yineng_piv8_V8
 * Method:    _isNodeCompatible
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1isNodeCompatible(JNIEnv *, jclass){
    return false;
}


