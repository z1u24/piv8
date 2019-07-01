//
// Created by yineng on 2019/6/21.
//

#include <include/v8.h>
#include <iostream>
#include "com_yineng_piv8_impl.h"
#include <natives_blob.h>
#include <snapshot_blob.h>
#include <libplatform/libplatform.h>
#include "include/v8-inspector.h"
#include "src/inspector/v8-inspector-impl.h"
#include "src/inspector/v8-inspector-session-impl.h"
#include "util.h"
#include "log.h"

using namespace std;
using namespace v8_inspector;
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

//chrome debugger

class piv8_inspector : V8InspectorClient, V8Inspector::Channel {
public:
    static piv8_inspector* getInstance();
    void init(jlong v8ptr,Isolate *isolate);
    void connect(JNIEnv *env_, jobject connection);
//    void scheduleBreak();
    void createInspectorSession();
    void disconnect(JNIEnv *env_,Isolate *isolate);

    void doDispatchMessage(v8::Isolate* isolate, const std::string& message);

    void sendResponse(int callId, std::unique_ptr<StringBuffer> message) override;
    void sendNotification(const std::unique_ptr<StringBuffer> message) override;
    void flushProtocolNotifications() override;

//    void runMessageLoopOnPause(int contextGroupId) override;
//    void quitMessageLoopOnPause() override;
//    void runIfWaitingForDebugger(int contextGroupId) override;
//    v8::Local<v8::Context> ensureDefaultContextInGroup(int contextGroupId) override;

//    static void sendToFrontEndCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
//    static void consoleLogCallback(const std::string& message, const std::string& logLevel);

    std::unique_ptr<V8Inspector> inspector_;
    bool isConnected;

private:
    static piv8_inspector* instance;
    static int contextGroupId;

    std::unique_ptr<V8InspectorSession> session_;
    jobject connection;
    jlong v8RuntimePtr;
//    bool running_nested_loop_;
//    bool terminated_;

};

piv8_inspector* piv8_inspector::instance = nullptr;
int piv8_inspector::contextGroupId = 1;


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
jmethodID v8InspectorSend = NULL;
jmethodID v8InspectorSendToDevToolsConsole = NULL;
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
jobject getResult(JNIEnv *env, jobject &v8, jlong v8RuntimePtr, Isolate* isolate, Handle<Value> &result, jint expectedType);

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
    return object->Get(isolate->GetCurrentContext(),v8Key).ToLocalChecked();
}

void addValueWithKey(JNIEnv* env, Isolate* isolate, jlong &v8RuntimePtr, jlong objectHandle, jstring key, Handle<Value> value) {
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    const uint16_t* unicodeString_key = env->GetStringChars(key, NULL);
    int length = env->GetStringLength(key);
    Local<String> v8Key = String::NewFromTwoByte(isolate, unicodeString_key, String::NewStringType::kNormalString, length);
    v8::String::Utf8Value serverKeyUtf8(isolate,v8Key);
    std::string serverKeystd = std::string(*serverKeyUtf8);
    LOGV("%s",serverKeystd.c_str());
    object->Set(isolate->GetCurrentContext(),v8Key, value).ToChecked();
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
    ScriptOrigin* scriptOriginPtr = NULL;
    if (jscriptName != NULL) {
        scriptOriginPtr = createScriptOrigin(env, isolate, jscriptName, jlineNumber);
    }
    MaybeLocal<Script> spt = Script::Compile(isolate->GetCurrentContext(),source, scriptOriginPtr);
    if(!spt.IsEmpty()){
        script = spt.ToLocalChecked();
    }
    if (scriptOriginPtr != NULL) {
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

bool runScript(Isolate* isolate, JNIEnv *env, Local<Script> *script, TryCatch* tryCatch, Local<Value> &result, jlong v8RuntimePtr) {
    result = (*script)->Run(isolate->GetCurrentContext()).ToLocalChecked();
    if (tryCatch->HasCaught()) {
        throwExecutionException(env, isolate, tryCatch, v8RuntimePtr);
        return false;
    }
    return true;
}

bool invokeFunction(JNIEnv *env, Isolate* isolate, jlong &v8RuntimePtr, jlong &receiverHandle, jlong &functionHandle, jlong &parameterHandle, Handle<Value> &result) {
    int size = 0;
    Handle<Value>* args = NULL;
    if (parameterHandle != 0) {
        Handle<Object> parameters = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(parameterHandle));
        size = Array::Cast(*parameters)->Length();
        args = new Handle<Value>[size];
        for (int i = 0; i < size; i++) {
            args[i] = parameters->Get(i);
        }
    }
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(functionHandle));
    Handle<Object> receiver = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(receiverHandle));
    Handle<Function> func = Handle<Function>::Cast(object);
    TryCatch tryCatch(isolate);
    result = func->Call(isolate->GetCurrentContext(),receiver, size, args).ToLocalChecked();
    if (args != NULL) {
        delete(args);
    }
    if (tryCatch.HasCaught()) {
        throwExecutionException(env, isolate, &tryCatch, v8RuntimePtr);
        return false;
    }
    return true;
}

bool invokeFunction(JNIEnv *env, Isolate* isolate, jlong &v8RuntimePtr, jlong &objectHandle, jstring &jfunctionName, jlong &parameterHandle, Handle<Value> &result) {
    Local<String> functionName = createV8String(env, isolate, jfunctionName);
    Handle<Object> parentObject = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    int size = 0;
    Handle<Value>* args = NULL;
    if (parameterHandle != 0) {
        Handle<Object> parameters = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(parameterHandle));
        size = Array::Cast(*parameters)->Length();
        args = new Handle<Value>[size];
        for (int i = 0; i < size; i++) {
            args[i] = parameters->Get(i);
        }
    }
    Handle<Value> value = parentObject->Get(functionName);
    Handle<Function> func = Handle<Function>::Cast(value);
    TryCatch tryCatch(isolate);
    result = func->Call(isolate->GetCurrentContext(),parentObject, size, args).ToLocalChecked();
    if (args != NULL) {
        delete(args);
    }
    if (tryCatch.HasCaught()) {
        throwExecutionException(env, isolate, &tryCatch, v8RuntimePtr);
        return false;
    }
    return true;
}

bool isNumber(int type) {
    return type == com_piv8_v8_V8_DOUBLE || type == com_piv8_v8_V8_INTEGER;
}

bool isObject(int type) {
    return type == com_piv8_v8_V8_V8_OBJECT || type == com_piv8_v8_V8_V8_ARRAY;
}

bool isNumber(int type1, int type2) {
    return isNumber(type1) && isNumber(type2);
}

bool isObject(int type1, int type2) {
    return isObject(type1) && isObject(type2);
}


int fillIntArray(JNIEnv *env, Isolate* isolate, Handle<Object> &array, int start, int length, jintArray &result) {
    jint * fill = new jint[length];
    for (int i = start; i < start + length; i++) {
        Handle<Value> v8Value = array->Get(i);
        ASSERT_IS_NUMBER(v8Value);
        fill[i - start] = v8Value->Int32Value(isolate->GetCurrentContext()).ToChecked();
    }
    (env)->SetIntArrayRegion(result, 0, length, fill);
    delete[] fill;
    return length;
}

int fillDoubleArray(JNIEnv *env, Isolate* isolate, Handle<Object> &array, int start, int length, jdoubleArray &result) {
    jdouble * fill = new jdouble[length];
    for (int i = start; i < start + length; i++) {
        Handle<Value> v8Value = array->Get(i);
        ASSERT_IS_NUMBER(v8Value);
        fill[i - start] = v8Value->NumberValue(isolate->GetCurrentContext()).ToChecked();
    }
    (env)->SetDoubleArrayRegion(result, 0, length, fill);
    delete[] fill;
    return length;
}

int fillByteArray(JNIEnv *env, Isolate* isolate, Handle<Object> &array, int start, int length, jbyteArray &result) {
    jbyte * fill = new jbyte[length];
    for (int i = start; i < start + length; i++) {
        Handle<Value> v8Value = array->Get(i);
        ASSERT_IS_NUMBER(v8Value);
        fill[i - start] = (jbyte)v8Value->Int32Value(isolate->GetCurrentContext()).ToChecked();
    }
    (env)->SetByteArrayRegion(result, 0, length, fill);
    delete[] fill;
    return length;
}

int fillBooleanArray(JNIEnv *env, Isolate* isolate, Handle<Object> &array, int start, int length, jbooleanArray &result) {
    jboolean * fill = new jboolean[length];
    for (int i = start; i < start + length; i++) {
        Handle<Value> v8Value = array->Get(i);
        ASSERT_IS_BOOLEAN(v8Value);
        fill[i - start] = v8Value->BooleanValue(isolate->GetCurrentContext()).ToChecked();
    }
    (env)->SetBooleanArrayRegion(result, 0, length, fill);
    delete[] fill;
    return length;
}

int fillStringArray(JNIEnv *env, Isolate* isolate, Handle<Object> &array, int start, int length, jobjectArray &result) {
    for (int i = start; i < start + length; i++) {
        Handle<Value> v8Value = array->Get(i);
        ASSERT_IS_STRING(v8Value);
        String::Value unicodeString(isolate,v8Value->ToString(isolate));
        jstring string = env->NewString(*unicodeString, unicodeString.length());
        env->SetObjectArrayElement(result, i - start, string);
        (env)->DeleteLocalRef(string);
    }

    return length;
}

int getType(Handle<Value> v8Value) {
    if (v8Value.IsEmpty() || v8Value->IsUndefined()) {
        return com_piv8_v8_V8_UNDEFINED;
    }
    else if (v8Value->IsNull()) {
        return com_piv8_v8_V8_NULL;
    }
    else if (v8Value->IsInt32()) {
        return com_piv8_v8_V8_INTEGER;
    }
    else if (v8Value->IsNumber()) {
        return com_piv8_v8_V8_DOUBLE;
    }
    else if (v8Value->IsBoolean()) {
        return com_piv8_v8_V8_BOOLEAN;
    }
    else if (v8Value->IsString()) {
        return com_piv8_v8_V8_STRING;
    }
    else if (v8Value->IsFunction()) {
        return com_piv8_v8_V8_V8_FUNCTION;
    }
    else if (v8Value->IsArrayBuffer()) {
        return com_piv8_v8_V8_V8_ARRAY_BUFFER;
    }
    else if (v8Value->IsTypedArray()) {
        return com_piv8_v8_V8_V8_TYPED_ARRAY;
    }
    else if (v8Value->IsArray()) {
        return com_piv8_v8_V8_V8_ARRAY;
    }
    else if (v8Value->IsObject()) {
        return com_piv8_v8_V8_V8_OBJECT;
    }
    return -1;
}

jobject createParameterArray(JNIEnv* env, jlong v8RuntimePtr, jobject v8, int size, const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = getIsolate(env, v8RuntimePtr);
    jobject result = env->NewObject(v8ArrayCls, v8ArrayInitMethodID, v8);
    jlong parameterHandle = env->CallLongMethod(result, v8ArrayGetHandleMethodID);
    Handle<Object> parameters = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(parameterHandle));
    for (int i = 0; i < size; i++) {
        parameters->Set(i, args[i]);
    }
    return result;
}

void voidCallback(const FunctionCallbackInfo<Value>& args) {
    int size = args.Length();
    Local<External> data = Local<External>::Cast(args.Data());
    void *methodDescriptorPtr = data->Value();
    MethodDescriptor* md = static_cast<MethodDescriptor*>(methodDescriptorPtr);
    jobject v8 = reinterpret_cast<V8Runtime*>(md->v8RuntimePtr)->v8;
    Isolate* isolate = reinterpret_cast<V8Runtime*>(md->v8RuntimePtr)->isolate;
    Isolate::Scope isolateScope(isolate);
    JNIEnv * env;
    getJNIEnv(env);
    jobject parameters = createParameterArray(env, md->v8RuntimePtr, v8, size, args);
    Handle<Value> receiver = args.This();
    jobject jreceiver = getResult(env, v8, md->v8RuntimePtr, isolate, receiver, com_piv8_v8_V8_UNKNOWN);
    env->CallVoidMethod(v8, v8CallVoidMethodID, md->methodID, jreceiver, parameters);
    if (env->ExceptionCheck()) {
        Isolate* isolate = getIsolate(env, md->v8RuntimePtr);
        reinterpret_cast<V8Runtime*>(md->v8RuntimePtr)->pendingException = env->ExceptionOccurred();
        env->ExceptionClear();
        jstring exceptionMessage = (jstring)env->CallObjectMethod(reinterpret_cast<V8Runtime*>(md->v8RuntimePtr)->pendingException, throwableGetMessageMethodID);
        if (exceptionMessage != NULL) {
            Local<String> v8String = createV8String(env, isolate, exceptionMessage);
            isolate->ThrowException(v8String);
        }
        else {
            isolate->ThrowException(String::NewFromUtf8(isolate, "Unhandled Java Exception"));
        }
    }
    env->CallVoidMethod(parameters, v8ArrayReleaseMethodID);
    env->CallVoidMethod(jreceiver, v8ObjectReleaseMethodID);
    env->DeleteLocalRef(jreceiver);
    env->DeleteLocalRef(parameters);
}

int getReturnType(JNIEnv* env, jobject &object) {
    int result = com_piv8_v8_V8_NULL;
    if (env->IsInstanceOf(object, integerCls)) {
        result = com_piv8_v8_V8_INTEGER;
    }
    else if (env->IsInstanceOf(object, doubleCls)) {
        result = com_piv8_v8_V8_DOUBLE;
    }
    else if (env->IsInstanceOf(object, booleanCls)) {
        result = com_piv8_v8_V8_BOOLEAN;
    }
    else if (env->IsInstanceOf(object, stringCls)) {
        result = com_piv8_v8_V8_STRING;
    }
    else if (env->IsInstanceOf(object, v8ArrayCls)) {
        result = com_piv8_v8_V8_V8_ARRAY;
    }
    else if (env->IsInstanceOf(object, v8ObjectCls)) {
        result = com_piv8_v8_V8_V8_OBJECT;
    }
    else if (env->IsInstanceOf(object, v8ArrayBufferCls)) {
        result = com_piv8_v8_V8_V8_ARRAY_BUFFER;
    }
    return result;
}

int getInteger(JNIEnv* env, jobject &object) {
    return env->CallIntMethod(object, integerIntValueMethodID);
}

bool getBoolean(JNIEnv* env, jobject &object) {
    return env->CallBooleanMethod(object, booleanBoolValueMethodID);
}

double getDouble(JNIEnv* env, jobject &object) {
    return env->CallDoubleMethod(object, doubleDoubleValueMethodID);
}

void objectCallback(const FunctionCallbackInfo<Value>& args) {
    int size = args.Length();
    Local<External> data = Local<External>::Cast(args.Data());
    void *methodDescriptorPtr = data->Value();
    MethodDescriptor* md = static_cast<MethodDescriptor*>(methodDescriptorPtr);
    jobject v8 = reinterpret_cast<V8Runtime*>(md->v8RuntimePtr)->v8;
    Isolate* isolate = reinterpret_cast<V8Runtime*>(md->v8RuntimePtr)->isolate;
    Isolate::Scope isolateScope(isolate);
    JNIEnv * env;
    getJNIEnv(env);
    jobject parameters = createParameterArray(env, md->v8RuntimePtr, v8, size, args);
    Handle<Value> receiver = args.This();
    jobject jreceiver = getResult(env, v8, md->v8RuntimePtr, isolate, receiver, com_piv8_v8_V8_UNKNOWN);
    jobject resultObject = env->CallObjectMethod(v8, v8CallObjectJavaMethodMethodID, md->methodID, jreceiver, parameters);
    if (env->ExceptionCheck()) {
        resultObject = NULL;
        Isolate* isolate = getIsolate(env, md->v8RuntimePtr);
        reinterpret_cast<V8Runtime*>(md->v8RuntimePtr)->pendingException = env->ExceptionOccurred();
        env->ExceptionClear();
        jstring exceptionMessage = (jstring)env->CallObjectMethod(reinterpret_cast<V8Runtime*>(md->v8RuntimePtr)->pendingException, throwableGetMessageMethodID);
        if (exceptionMessage != NULL) {
            Local<String> v8String = createV8String(env, isolate, exceptionMessage);
            isolate->ThrowException(v8String);
        }
        else {
            isolate->ThrowException(String::NewFromUtf8(isolate, "Unhandled Java Exception"));
        }
    }
    else if (resultObject == NULL) {
        args.GetReturnValue().SetNull();
    }
    else {
        int returnType = getReturnType(env, resultObject);
        if (returnType == com_piv8_v8_V8_INTEGER) {
            args.GetReturnValue().Set(getInteger(env, resultObject));
        }
        else if (returnType == com_piv8_v8_V8_BOOLEAN) {
            args.GetReturnValue().Set(getBoolean(env, resultObject));
        }
        else if (returnType == com_piv8_v8_V8_DOUBLE) {
            args.GetReturnValue().Set(getDouble(env, resultObject));
        }
        else if (returnType == com_piv8_v8_V8_STRING) {
            jstring stringResult = (jstring)resultObject;
            Local<String> result = createV8String(env, reinterpret_cast<V8Runtime*>(md->v8RuntimePtr)->isolate, stringResult);
            args.GetReturnValue().Set(result);
        }
        else if (returnType == com_piv8_v8_V8_V8_ARRAY) {
            if (isUndefined(env, resultObject)) {
                args.GetReturnValue().SetUndefined();
            }
            else {
                jlong resultHandle = getHandle(env, resultObject);
                Handle<Object> result = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(resultHandle));
                releaseArray(env, resultObject);
                args.GetReturnValue().Set(result);
            }
        }
        else if (returnType == com_piv8_v8_V8_V8_OBJECT) {
            if (isUndefined(env, resultObject)) {
                args.GetReturnValue().SetUndefined();
            }
            else {
                jlong resultHandle = getHandle(env, resultObject);
                Handle<Object> result = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(resultHandle));
                release(env, resultObject);
                args.GetReturnValue().Set(result);
            }
        }
        else if (returnType == com_piv8_v8_V8_V8_ARRAY_BUFFER) {
            if (isUndefined(env, resultObject)) {
                args.GetReturnValue().SetUndefined();
            }
            else {
                jlong resultHandle = getHandle(env, resultObject);
                Handle<Object> result = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(resultHandle));
                release(env, resultObject);
                args.GetReturnValue().Set(result);
            }
        }
        else {
            args.GetReturnValue().SetUndefined();
        }
    }
    if (resultObject != NULL) {
        env->DeleteLocalRef(resultObject);
    }
    env->CallVoidMethod(parameters, v8ArrayReleaseMethodID);
    env->CallVoidMethod(jreceiver, v8ObjectReleaseMethodID);
    env->DeleteLocalRef(jreceiver);
    env->DeleteLocalRef(parameters);
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
    v8ObjectReleaseMethodID = env->GetMethodID(v8ObjectCls, "close", "()V");
    v8ArrayReleaseMethodID = env->GetMethodID(v8ArrayCls, "close", "()V");
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
    v8InspectorSend = env->GetMethodID(v8cls,"send","(Ljava/lang/Object;Ljava/lang/String;)V");
    v8InspectorSendToDevToolsConsole = env->GetMethodID(v8cls,"sendToDevToolsConsole","(Ljava/lang/Object;Ljava/lang/String;Ljava/lang/String;)V");



    return JNI_VERSION_1_6;

}

piv8_inspector* piv8_inspector::getInstance(){
    if(piv8_inspector::instance == NULL){
        instance = new piv8_inspector();
    }
    return instance;
}

void piv8_inspector::init(jlong v8ptr,Isolate *isolate) {
    LOGV("init inspector start");
    this->v8RuntimePtr = v8ptr;
    inspector_ = V8Inspector::create(isolate,getInstance());
    inspector_->contextCreated(v8_inspector::V8ContextInfo(isolate->GetCurrentContext(),piv8_inspector::contextGroupId,v8_inspector::StringView()));
    this->createInspectorSession();
    this->isConnected = false;
}

void piv8_inspector::connect(JNIEnv *env_,jobject connection) {
    this->connection = env_->NewGlobalRef(connection);
    this->isConnected = true;
}

void piv8_inspector::disconnect(JNIEnv *env_,Isolate *isolate) {
    if (this->connection == nullptr || this->isConnected == false) {
        return;
    }
    session_->resume();
    session_.reset();
    env_->DeleteGlobalRef(this->connection);
    this->connection = nullptr;
    this->isConnected = false;
    this->createInspectorSession();
}

void piv8_inspector::createInspectorSession() {
    session_ = inspector_->connect(piv8_inspector::contextGroupId,this, v8_inspector::StringView());
}


void piv8_inspector::doDispatchMessage(v8::Isolate *isolate, const std::string &message) {
    if (session_ == nullptr) {
        return;
    }
    const v8_inspector::String16 msg = v8_inspector::String16::fromUTF8(message.c_str(), message.length());
    v8_inspector::StringView message_view = toStringView(msg);
    session_->dispatchProtocolMessage(message_view);
}

static v8_inspector::String16 ToString16(const v8_inspector::StringView& string) {
    if (string.is8Bit()) {
        return v8_inspector::String16(reinterpret_cast<const char*>(string.characters8()), string.length());
    }
    return v8_inspector::String16(reinterpret_cast<const uint16_t*>(string.characters16()), string.length());
}

void piv8_inspector::flushProtocolNotifications() {

}

void piv8_inspector::sendNotification(const std::unique_ptr<StringBuffer> message) {
    if (this->isConnected == false || this->connection == nullptr) {
        return;
    }
    JNIEnv * env;
    getJNIEnv(env);
    v8_inspector::String16 msg = ToString16(message->string());
    jstring jmsg = env->NewStringUTF(msg.utf8().c_str());
    env->CallVoidMethod(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->v8,v8InspectorSend,this->connection,jmsg);

}

void piv8_inspector::sendResponse(int callId, std::unique_ptr<StringBuffer> message) {
    sendNotification(std::move(message));
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


JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1executeIntegerScript(JNIEnv * env, jobject v8, jlong v8RuntimePtr, jstring jjstring, jstring jscriptName = NULL, jint jlineNumber = 0) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    TryCatch tryCatch(isolate);
    Local<Script> script;
    Local<Value> result;
    if (!compileScript(isolate, jjstring, env, jscriptName, jlineNumber, script, &tryCatch))
        return 0;
    if (!runScript(isolate, env, &script, &tryCatch, result, v8RuntimePtr))
        return 0;
    ASSERT_IS_NUMBER(result);
    return result->Int32Value(isolate->GetCurrentContext()).ToChecked();
}


JNIEXPORT jdouble JNICALL Java_com_yineng_piv8_V8__1executeDoubleScript(JNIEnv * env, jobject v8, jlong v8RuntimePtr, jstring jjstring, jstring jscriptName = NULL, jint jlineNumber = 0) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    TryCatch tryCatch(isolate);
    Local<Script> script;
    Local<Value> result;
    if (!compileScript(isolate, jjstring, env, jscriptName, jlineNumber, script, &tryCatch))
        return 0;
    if (!runScript(isolate, env, &script, &tryCatch, result, v8RuntimePtr))
        return 0;
    ASSERT_IS_NUMBER(result);
    return result->NumberValue(isolate->GetCurrentContext()).ToChecked();
}


JNIEXPORT jstring JNICALL Java_com_yineng_piv8_V8__1executeStringScript(JNIEnv *env, jobject v8, jlong v8RuntimePtr, jstring jjstring, jstring jscriptName = NULL, jint jlineNumber = 0) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, NULL);
    TryCatch tryCatch(isolate);
    Local<Script> script;
    Local<Value> result;
    if (!compileScript(isolate, jjstring, env, jscriptName, jlineNumber, script, &tryCatch))
        return NULL;
    if (!runScript(isolate, env, &script, &tryCatch, result, v8RuntimePtr))
        return NULL;
    ASSERT_IS_STRING(result);
    String::Value unicodeString(isolate,result->ToString(isolate));

    return env->NewString(*unicodeString, unicodeString.length());
}



JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1executeBooleanScript(JNIEnv *env, jobject v8, jlong v8RuntimePtr, jstring jjstring, jstring jscriptName = NULL, jint jlineNumber = 0) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, false);
    TryCatch tryCatch(isolate);
    Local<Script> script;
    Local<Value> result;
    if (!compileScript(isolate, jjstring, env, jscriptName, jlineNumber, script, &tryCatch))
        return false;
    if (!runScript(isolate, env, &script, &tryCatch, result, v8RuntimePtr))
        return false;
    ASSERT_IS_BOOLEAN(result);
    return result->BooleanValue(isolate->GetCurrentContext()).ToChecked();
}


JNIEXPORT jobject JNICALL Java_com_yineng_piv8_V8__1executeScript(JNIEnv *env, jobject v8, jlong v8RuntimePtr, jint expectedType, jstring jjstring, jstring jscriptName = NULL, jint jlineNumber = 0) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, NULL);
    TryCatch tryCatch(isolate);
    Local<Script> script;
    Local<Value> result;
    if (!compileScript(isolate, jjstring, env, jscriptName, jlineNumber, script, &tryCatch)) { return NULL; }
    if (!runScript(isolate, env, &script, &tryCatch, result, v8RuntimePtr)) { return NULL; }
    return getResult(env, v8, v8RuntimePtr, isolate, result, expectedType);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1executeVoidScript(JNIEnv * env, jobject v8, jlong v8RuntimePtr, jstring jjstring, jstring jscriptName = NULL, jint jlineNumber = 0) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    TryCatch tryCatch(isolate);
    Local<Script> script;
    if (!compileScript(isolate, jjstring, env, jscriptName, jlineNumber, script, &tryCatch))
        return;
    runScript(isolate, env, &script, &tryCatch, v8RuntimePtr);
}


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


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1releaseMethodDescriptor(JNIEnv *, jobject, jlong, jlong methodDescriptorPtr) {
    MethodDescriptor* md = reinterpret_cast<MethodDescriptor*>(methodDescriptorPtr);
    delete(md);
}


JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1contains(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jstring key) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, false);
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    Local<String> v8Key = createV8String(env, isolate, key);
    return object->Has(isolate->GetCurrentContext(),v8Key).ToChecked();
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


JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1getInteger(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jstring key) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Value> v8Value = getValueWithKey(env, isolate, v8RuntimePtr, objectHandle, key);
    ASSERT_IS_NUMBER(v8Value);
    return v8Value->Int32Value(isolate->GetCurrentContext()).ToChecked();
}


JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1getBoolean(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jstring key) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, false);
    Handle<Value> v8Value = getValueWithKey(env, isolate, v8RuntimePtr, objectHandle, key);
    ASSERT_IS_BOOLEAN(v8Value);
    return v8Value->BooleanValue(isolate->GetCurrentContext()).ToChecked();
}


JNIEXPORT jdouble JNICALL Java_com_yineng_piv8_V8__1getDouble(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jstring key) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Value> v8Value = getValueWithKey(env, isolate, v8RuntimePtr, objectHandle, key);
    ASSERT_IS_NUMBER(v8Value);
    return v8Value->NumberValue(isolate->GetCurrentContext()).ToChecked();
}


JNIEXPORT jstring JNICALL Java_com_yineng_piv8_V8__1getString(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jstring key) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Value> v8Value = getValueWithKey(env, isolate, v8RuntimePtr, objectHandle, key);
    ASSERT_IS_STRING(v8Value);
    String::Value unicode(isolate,v8Value->ToString(isolate));
    return env->NewString(*unicode, unicode.length());
}


JNIEXPORT jobject JNICALL Java_com_yineng_piv8_V8__1get(JNIEnv *env, jobject v8, jlong v8RuntimePtr, jint expectedType, jlong objectHandle, jstring key) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, NULL);
    Handle<Value> result = getValueWithKey(env, isolate, v8RuntimePtr, objectHandle, key);
    return getResult(env, v8, v8RuntimePtr, isolate, result, expectedType);
}


JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1executeIntegerFunction(JNIEnv *env, jobject v8, jlong v8RuntimePtr, jlong objectHandle, jstring jfunctionName, jlong parameterHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Value> result;
    if (!invokeFunction(env, isolate, v8RuntimePtr, objectHandle, jfunctionName, parameterHandle, result))
        return 0;
    ASSERT_IS_NUMBER(result);
    return result->Int32Value(isolate->GetCurrentContext()).ToChecked();
}


JNIEXPORT jdouble JNICALL Java_com_yineng_piv8_V8__1executeDoubleFunction(JNIEnv *env, jobject v8, jlong v8RuntimePtr, jlong objectHandle, jstring jfunctionName, jlong parameterHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Value> result;
    if (!invokeFunction(env, isolate, v8RuntimePtr, objectHandle, jfunctionName, parameterHandle, result))
        return 0;
    ASSERT_IS_NUMBER(result);
    return result->NumberValue(isolate->GetCurrentContext()).ToChecked();
}


JNIEXPORT jstring JNICALL Java_com_yineng_piv8_V8__1executeStringFunction(JNIEnv *env, jobject v8, jlong v8RuntimePtr, jlong objectHandle, jstring jfunctionName, jlong parameterHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, NULL);
    Handle<Value> result;
    if (!invokeFunction(env, isolate, v8RuntimePtr, objectHandle, jfunctionName, parameterHandle, result))
        return NULL;
    ASSERT_IS_STRING(result);
    String::Value unicodeString(isolate,result->ToString(isolate));

    return env->NewString(*unicodeString, unicodeString.length());
}

JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1executeBooleanFunction(JNIEnv *env, jobject v8, jlong v8RuntimePtr, jlong objectHandle, jstring jfunctionName, jlong parameterHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, false);
    Handle<Value> result;
    if (!invokeFunction(env, isolate, v8RuntimePtr, objectHandle, jfunctionName, parameterHandle, result))
        return false;
    ASSERT_IS_BOOLEAN(result);
    return result->BooleanValue(isolate->GetCurrentContext()).ToChecked();
}


JNIEXPORT jobject JNICALL Java_com_yineng_piv8_V8__1executeFunction__JIJLjava_lang_String_2J(JNIEnv *env, jobject v8, jlong v8RuntimePtr, jint expectedType, jlong objectHandle, jstring jfunctionName, jlong parameterHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, NULL);
    Handle<Value> result;
    if (!invokeFunction(env, isolate, v8RuntimePtr, objectHandle, jfunctionName, parameterHandle, result))
        return NULL;
    return getResult(env, v8, v8RuntimePtr, isolate, result, expectedType);
}


JNIEXPORT jobject JNICALL Java_com_yineng_piv8_V8__1executeFunction__JJJJ(JNIEnv *env, jobject v8, jlong v8RuntimePtr, jlong receiverHandle, jlong functionHandle, jlong parameterHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, NULL);
    Handle<Value> result;
    if (!invokeFunction(env, isolate, v8RuntimePtr, receiverHandle, functionHandle, parameterHandle, result))
        return NULL;
    return getResult(env, v8, v8RuntimePtr, isolate, result, com_piv8_v8_V8_UNKNOWN);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1executeVoidFunction(JNIEnv *env, jobject v8, jlong v8RuntimePtr, jlong objectHandle, jstring jfunctionName, jlong parameterHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    Handle<Value> result;
    invokeFunction(env, isolate, v8RuntimePtr, objectHandle, jfunctionName, parameterHandle, result);
}


JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1equals(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jlong thatHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, false);
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    Handle<Object> that = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    if (objectHandle == 0) {
        object = context->Global();
    }
    if (thatHandle == 0) {
        that = context->Global();
    }
    return object->Equals(isolate->GetCurrentContext(),that).ToChecked();
}


JNIEXPORT jstring JNICALL Java_com_yineng_piv8_V8__1toString(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    String::Value unicodeString(isolate,object->ToString(isolate));
    return env->NewString(*unicodeString, unicodeString.length());
}


JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1strictEquals(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jlong thatHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, false);
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    Handle<Object> that = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(thatHandle));
    if (objectHandle == reinterpret_cast<jlong>(runtime->globalObject)) {
        object = context->Global();
    }
    if (thatHandle == reinterpret_cast<jlong>(runtime->globalObject)) {
        that = context->Global();
    }
    return object->StrictEquals(that);
}


JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1sameValue(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jlong thatHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, false);
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    Handle<Object> that = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    if (objectHandle == reinterpret_cast<jlong>(runtime->globalObject)) {
        object = context->Global();
    }
    if (thatHandle == reinterpret_cast<jlong>(runtime->globalObject)) {
        that = context->Global();
    }
    return object->SameValue(that);
}


JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1identityHash(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, false);
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    if (objectHandle == reinterpret_cast<jlong>(runtime->globalObject)) {
        object = context->Global();
    }
    return object->GetIdentityHash();
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1add__JJLjava_lang_String_2I(JNIEnv * env, jobject, jlong v8RuntimePtr, jlong objectHandle, jstring key, jint value) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    addValueWithKey(env, isolate, v8RuntimePtr, objectHandle, key, Int32::New(isolate, value));
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addObject(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jstring key, jlong valueHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    Handle<Value> value = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(valueHandle));
    addValueWithKey(env, isolate, v8RuntimePtr, objectHandle, key, value);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1add__JJLjava_lang_String_2Z(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jstring key, jboolean value) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    addValueWithKey(env, isolate, v8RuntimePtr, objectHandle, key, Boolean::New(isolate, value));
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1add__JJLjava_lang_String_2D(JNIEnv * env, jobject, jlong v8RuntimePtr, jlong objectHandle, jstring key, jdouble value) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    addValueWithKey(env, isolate, v8RuntimePtr, objectHandle, key, Number::New(isolate, value));
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1add__JJLjava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jstring key, jstring value) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    Handle<Value> v8Value = createV8String(env, isolate, value);
    addValueWithKey(env, isolate, v8RuntimePtr, objectHandle, key, v8Value);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addUndefined(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jstring key) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    addValueWithKey(env, isolate, v8RuntimePtr, objectHandle, key, Undefined(isolate));
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addNull(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jstring key) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    addValueWithKey(env, isolate, v8RuntimePtr, objectHandle, key, Null(isolate));
}


JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1registerJavaMethod(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jstring functionName, jboolean voidMethod) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    FunctionCallback callback = voidCallback;
    if (!voidMethod) {
        callback = objectCallback;
    }
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    Local<String> v8FunctionName = createV8String(env, isolate, functionName);
    isolate->IdleNotificationDeadline(1.0);
    MethodDescriptor* md= new MethodDescriptor();
    Local<External> ext =  External::New(isolate, md);
    Persistent<External> pext(isolate, ext);
    pext.SetWeak(md, [](v8::WeakCallbackInfo<MethodDescriptor> const& data) {
        MethodDescriptor* md = data.GetParameter();
        jobject v8 = reinterpret_cast<V8Runtime*>(md->v8RuntimePtr)->v8;
        JNIEnv * env;
        getJNIEnv(env);
        env->CallVoidMethod(v8, v8DisposeMethodID, md->methodID);
        delete(md);
    }, WeakCallbackType::kParameter);

    md->methodID = reinterpret_cast<jlong>(md);
    md->v8RuntimePtr = v8RuntimePtr;
    object->Set(isolate->GetCurrentContext(),v8FunctionName, Function::New(isolate->GetCurrentContext(), callback, ext).ToLocalChecked());
    return md->methodID;
}

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


JNIEXPORT jlongArray JNICALL Java_com_yineng_piv8_V8__1initNewV8Function(JNIEnv *env, jobject, jlong v8RuntimePtr) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    MethodDescriptor* md = new MethodDescriptor();
    Local<External> ext = External::New(isolate, md);
    Persistent<External> pext(isolate, ext);
    isolate->IdleNotificationDeadline(1.0);
    pext.SetWeak(md, [](v8::WeakCallbackInfo<MethodDescriptor> const& data) {
        MethodDescriptor* md = data.GetParameter();
        jobject v8 = reinterpret_cast<V8Runtime*>(md->v8RuntimePtr)->v8;
        JNIEnv * env;
        getJNIEnv(env);
        env->CallVoidMethod(v8, v8DisposeMethodID, md->methodID);
        delete(md);
    }, WeakCallbackType::kParameter);

    Local<Function> function = Function::New(isolate->GetCurrentContext(), objectCallback, ext).ToLocalChecked();
    md->v8RuntimePtr = v8RuntimePtr;
    Persistent<Object>* container = new Persistent<Object>;
    container->Reset(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate, function);
    md->methodID = reinterpret_cast<jlong>(md);

    // Position 0 is the pointer to the container, position 1 is the pointer to the descriptor
    jlongArray result = env->NewLongArray(2);
    jlong * fill = new jlong[2];
    fill[0] = reinterpret_cast<jlong>(container);
    fill[1] = md->methodID;
    (env)->SetLongArrayRegion(result, 0, 2, fill);
    return result;
}


JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetSize(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    if ( array->IsTypedArray() ) {
        return TypedArray::Cast(*array)->Length();
    }
    return Array::Cast(*array)->Length();
}


JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetInteger(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint index) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    Handle<Value> v8Value = array->Get(index);
    ASSERT_IS_NUMBER(v8Value);
    return v8Value->Int32Value(isolate->GetCurrentContext()).ToChecked();
}


JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1arrayGetBoolean(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint index) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, false);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    Handle<Value> v8Value = array->Get(index);
    ASSERT_IS_BOOLEAN(v8Value);
    return v8Value->BooleanValue(isolate->GetCurrentContext()).ToChecked();
}


JNIEXPORT jbyte JNICALL Java_com_yineng_piv8_V8__1arrayGetByte(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint index) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, false);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    Handle<Value> v8Value = array->Get(index);
    ASSERT_IS_NUMBER(v8Value);
    return v8Value->Int32Value(isolate->GetCurrentContext()).ToChecked();
}


JNIEXPORT jdouble JNICALL Java_com_yineng_piv8_V8__1arrayGetDouble(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint index) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    Handle<Value> v8Value = array->Get(index);
    ASSERT_IS_NUMBER(v8Value);
    return v8Value->NumberValue(isolate->GetCurrentContext()).ToChecked();
}


JNIEXPORT jstring JNICALL Java_com_yineng_piv8_V8__1arrayGetString(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint index) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, NULL);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    Handle<Value> v8Value = array->Get(index);
    ASSERT_IS_STRING(v8Value);
    String::Value unicodeString(isolate,v8Value->ToString(isolate));

    return env->NewString(*unicodeString, unicodeString.length());
}


JNIEXPORT jobject JNICALL Java_com_yineng_piv8_V8__1arrayGet(JNIEnv *env, jobject v8, jlong v8RuntimePtr, jint expectedType, jlong arrayHandle, jint index) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, NULL);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    Handle<Value> result = array->Get(index);
    return getResult(env, v8, v8RuntimePtr, isolate, result, expectedType);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addArrayIntItem(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint value) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    if ( array->IsTypedArray() ) {
        Local<String> string = String::NewFromUtf8(isolate, "Cannot push to a Typed Array.");
        v8::String::Value strValue(isolate,string);
        throwV8RuntimeException(env, &strValue);
        return;
    }
    Local<Value> v8Value = Int32::New(isolate, value);
    int index = Array::Cast(*array)->Length();
    array->Set(index, v8Value);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addArrayBooleanItem(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jboolean value) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    if ( array->IsTypedArray() ) {
        Local<String> string = String::NewFromUtf8(isolate, "Cannot push to a Typed Array.");
        v8::String::Value strValue(isolate,string);
        throwV8RuntimeException(env, &strValue);
        return;
    }
    Local<Value> v8Value = Boolean::New(isolate, value);
    int index = Array::Cast(*array)->Length();
    array->Set(index, v8Value);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addArrayDoubleItem(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jdouble value) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    if ( array->IsTypedArray() ) {
        Local<String> string = String::NewFromUtf8(isolate, "Cannot push to a Typed Array.");
        v8::String::Value strValue(isolate,string);
        throwV8RuntimeException(env, &strValue);
        return;
    }
    Local<Value> v8Value = Number::New(isolate, value);
    int index = Array::Cast(*array)->Length();
    array->Set(index, v8Value);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addArrayStringItem(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jstring value) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    if ( array->IsTypedArray() ) {
        Local<String> string = String::NewFromUtf8(isolate, "Cannot push to a Typed Array.");
        v8::String::Value strValue(isolate,string);
        throwV8RuntimeException(env, &strValue);
        return;
    }
    int index = Array::Cast(*array)->Length();
    Local<String> v8Value = createV8String(env, isolate, value);
    array->Set(index, v8Value);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addArrayObjectItem(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jlong valueHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    if ( array->IsTypedArray() ) {
        Local<String> string = String::NewFromUtf8(isolate, "Cannot push to a Typed Array.");
        v8::String::Value strValue(isolate,string);
        throwV8RuntimeException(env, &strValue);
        return;
    }
    int index = Array::Cast(*array)->Length();
    Local<Value> v8Value = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(valueHandle));
    array->Set(index, v8Value);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addArrayUndefinedItem(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    if ( array->IsTypedArray() ) {
        Local<String> string = String::NewFromUtf8(isolate, "Cannot push to a Typed Array.");
        v8::String::Value strValue(isolate,string);
        throwV8RuntimeException(env, &strValue);
        return;
    }
    int index = Array::Cast(*array)->Length();
    array->Set(index, Undefined(isolate));
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1addArrayNullItem(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    if ( array->IsTypedArray() ) {
        Local<String> string = String::NewFromUtf8(isolate, "Cannot push to a Typed Array.");
        v8::String::Value strValue(isolate,string);
        throwV8RuntimeException(env, &strValue);
        return;
    }
    int index = Array::Cast(*array)->Length();
    array->Set(index, Null(isolate));
}


JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1getType__JJLjava_lang_String_2(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jstring key) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Value> v8Value = getValueWithKey(env, isolate, v8RuntimePtr, objectHandle, key);
    int type = getType(v8Value);
    if (type < 0) {
        throwResultUndefinedException(env, "");
    }
    return type;
}


JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1getType__JJI(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jint index) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    Handle<Value> v8Value = array->Get(index);
    int type = getType(v8Value);
    if (type < 0) {
        throwResultUndefinedException(env, "");
    }
    return type;
}


JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1getArrayType(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    int length = 0;
    if ( array->IsTypedArray() ) {
        if ( array->IsFloat64Array() ) {
            return com_piv8_v8_V8_DOUBLE;
        } else if ( array->IsFloat32Array() ) {
            return com_piv8_v8_V8_FLOAT_32_ARRAY;
        } else if ( array->IsInt32Array() ) {
            return com_piv8_v8_V8_INT_32_ARRAY;
        } else if ( array->IsUint32Array() ) {
            return com_piv8_v8_V8_UNSIGNED_INT_32_ARRAY;
        } else if ( array->IsInt16Array() ) {
            return com_piv8_v8_V8_INT_16_ARRAY;
        } else if ( array->IsUint16Array() ) {
            return com_piv8_v8_V8_UNSIGNED_INT_16_ARRAY;
        } else if ( array->IsInt8Array() ) {
            return com_piv8_v8_V8_INT_8_ARRAY;
        } else if ( array->IsUint8Array() ) {
            return com_piv8_v8_V8_UNSIGNED_INT_8_ARRAY;
        } else if ( array->IsUint8ClampedArray() ) {
            return com_piv8_v8_V8_UNSIGNED_INT_8_CLAMPED_ARRAY;
        }
        return com_piv8_v8_V8_INTEGER;
    } else {
        length = Array::Cast(*array)->Length();
    }
    int arrayType = com_piv8_v8_V8_UNDEFINED;
    for (int index = 0; index < length; index++) {
        int type = getType(array->Get(index));
        if (type < 0) {
            throwResultUndefinedException(env, "");
        }
        else if (index == 0) {
            arrayType = type;
        }
        else if (type == arrayType) {
            // continue
        }
        else if (isNumber(arrayType, type)) {
            arrayType = com_piv8_v8_V8_DOUBLE;
        }
        else if (isObject(arrayType, type)) {
            arrayType = com_piv8_v8_V8_V8_OBJECT;
        }
        else {
            return com_piv8_v8_V8_UNDEFINED;
        }
    }
    return arrayType;
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1setPrototype(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle, jlong prototypeHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    Handle<Object> prototype = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(prototypeHandle));
    object->SetPrototype(isolate->GetCurrentContext(),prototype);
}


JNIEXPORT jstring JNICALL Java_com_yineng_piv8_V8__1getConstructorName(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle){
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Object> object = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(objectHandle));
    String::Value unicodeString(isolate,object->GetConstructorName());
    return env->NewString(*unicodeString, unicodeString.length());
}


JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1getType__JJ(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong objectHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Value> v8Value = Local<Value>::New(isolate, *reinterpret_cast<Persistent<Value>*>(objectHandle));
    return getType(v8Value);
}


JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1getType__JJII(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint start, jint length) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    int result = -1;
    for (int i = start; i < start + length; i++) {
        Handle<Value> v8Value = array->Get(i);
        int type = getType(v8Value);
        if (result >= 0 && result != type) {
            throwResultUndefinedException(env, "");
            return -1;
        }
        else if (type < 0) {
            throwResultUndefinedException(env, "");
            return -1;
        }
        result = type;
    }
    if (result < 0) {
        throwResultUndefinedException(env, "");
    }
    return result;
}


JNIEXPORT jdoubleArray JNICALL Java_com_yineng_piv8_V8__1arrayGetDoubles__JJII(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint start, jint length) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, NULL);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    jdoubleArray result = env->NewDoubleArray(length);
    fillDoubleArray(env, isolate, array, start, length, result);
    return result;
}


JNIEXPORT jintArray JNICALL Java_com_yineng_piv8_V8__1arrayGetIntegers__JJII(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint start, jint length) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, NULL);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    jintArray result = env->NewIntArray(length);
    fillIntArray(env, isolate, array, start, length, result);
    return result;
}


JNIEXPORT jbooleanArray JNICALL Java_com_yineng_piv8_V8__1arrayGetBooleans__JJII(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint start, jint length) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, NULL);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    jbooleanArray result = env->NewBooleanArray(length);
    fillBooleanArray(env, isolate, array, start, length, result);
    return result;
}


JNIEXPORT jbyteArray JNICALL Java_com_yineng_piv8_V8__1arrayGetBytes__JJII(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint start, jint length) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, NULL);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    jbyteArray result = env->NewByteArray(length);
    fillByteArray(env, isolate, array, start, length, result);
    return result;
}


JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetDoubles__JJIILdouble_3_093_2(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint start, jint length, jdoubleArray result) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    return fillDoubleArray(env, isolate,array, start, length, result);
}



JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetIntegers__JJIILint_3_093_2(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint start, jint length, jintArray result) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));

    return fillIntArray(env, isolate, array, start, length, result);
}


JNIEXPORT jobjectArray JNICALL Java_com_yineng_piv8_V8__1arrayGetStrings__JJII(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint start, jint length) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, NULL);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    jobjectArray result = env->NewObjectArray(length, stringCls, NULL);
    fillStringArray(env, isolate, array, start, length, result);

    return result;
}

JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetBooleans__JJIILboolean_3_093_2(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint start, jint length, jbooleanArray result) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    return fillBooleanArray(env, isolate, array, start, length, result);
}



JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetBytes__JJIILbyte_3_093_2(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint start, jint length, jbyteArray result) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    return fillByteArray(env, isolate, array, start, length, result);
}


JNIEXPORT jint JNICALL Java_com_yineng_piv8_V8__1arrayGetStrings__JJIILjava_lang_String_3_093_2(JNIEnv * env, jobject, jlong v8RuntimePtr, jlong arrayHandle, jint start, jint length, jobjectArray result) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<Object> array = Local<Object>::New(isolate, *reinterpret_cast<Persistent<Object>*>(arrayHandle));
    return fillStringArray(env, isolate, array, start, length, result);
}



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


JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1initNewV8UInt16Array(JNIEnv *env, jobject, jlong v8RuntimePtr, jlong bufferHandle, jint offset, jint length) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    Handle<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::New(isolate, *reinterpret_cast<Persistent<ArrayBuffer>*>(bufferHandle));
    Local<Uint16Array> array = Uint16Array::New(arrayBuffer, offset, length);
    Persistent<Object>* container = new Persistent<Object>;
    container->Reset(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate, array);
    return reinterpret_cast<jlong>(container);
}


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


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1setWeak(JNIEnv * env, jobject, jlong v8RuntimePtr, jlong objectHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    WeakReferenceDescriptor* wrd = new WeakReferenceDescriptor();
    wrd->v8RuntimePtr = v8RuntimePtr;
    wrd->objectHandle = objectHandle;
    reinterpret_cast<Persistent<Object>*>(objectHandle)->SetWeak(wrd, [](v8::WeakCallbackInfo<WeakReferenceDescriptor> const& data) {
        WeakReferenceDescriptor* wrd = data.GetParameter();
        JNIEnv * env;
        getJNIEnv(env);
        jobject v8 = reinterpret_cast<V8Runtime*>(wrd->v8RuntimePtr)->v8;
        env->CallVoidMethod(v8, v8WeakReferenceReleased, wrd->objectHandle);
        delete(wrd);
    }, WeakCallbackType::kFinalizer);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1clearWeak(JNIEnv * env, jobject, jlong v8RuntimePtr, jlong objectHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, );
    reinterpret_cast<Persistent<Object>*>(objectHandle)->ClearWeak();
}


JNIEXPORT jboolean JNICALL Java_com_yineng_piv8_V8__1isWeak(JNIEnv * env, jobject, jlong v8RuntimePtr, jlong objectHandle) {
    Isolate* isolate = SETUP(env, v8RuntimePtr, 0);
    return reinterpret_cast<Persistent<Object>*>(objectHandle)->IsWeak();
}


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
    Local<Object> obj = isolate->GetCurrentContext()->Global();
//    Local<Object> obj = Object::New(isolate);
    reinterpret_cast<V8Runtime*>(v8RuntimePtr)->globalObject->Reset(isolate,obj);
    return reinterpret_cast<jlong>(reinterpret_cast<V8Runtime*>(v8RuntimePtr)->globalObject);
}


JNIEXPORT jlong JNICALL Java_com_yineng_piv8_V8__1getBuildID(JNIEnv *, jobject) {
    return 1;
}



Isolate* getIsolate(JNIEnv *env, jlong v8RuntimePtr) {
    if (v8RuntimePtr == 0) {
        throwError(env, "V8 isolate not found.");
        return NULL;
    }
    V8Runtime* runtime = reinterpret_cast<V8Runtime*>(v8RuntimePtr);
    return runtime->isolate;
}

void throwResultUndefinedException(JNIEnv *env, const char *message) {
    (env)->ThrowNew(v8ResultsUndefinedCls, message);
}

void throwParseException(JNIEnv *env, const char* fileName, int lineNumber, String::Value *message,
                         String::Value *sourceLine, int startColumn, int endColumn) {
    jstring jfileName = env->NewStringUTF(fileName);
    jstring jmessage = env->NewString(**message, message->length());
    jstring jsourceLine = env->NewString(**sourceLine, sourceLine->length());
    jthrowable result = (jthrowable)env->NewObject(v8ScriptCompilationCls, v8ScriptCompilationInitMethodID, jfileName, lineNumber, jmessage, jsourceLine, startColumn, endColumn);
    env->DeleteLocalRef(jfileName);
    env->DeleteLocalRef(jmessage);
    env->DeleteLocalRef(jsourceLine);
    (env)->Throw(result);
}

void throwExecutionException(JNIEnv *env, const char* fileName, int lineNumber, String::Value *message,
                             String::Value* sourceLine, int startColumn, int endColumn, const char* stackTrace, jlong v8RuntimePtr) {
    jstring jfileName = env->NewStringUTF(fileName);
    jstring jmessage = env->NewString(**message, message->length());
    jstring jsourceLine = env->NewString(**sourceLine, sourceLine->length());
    jstring jstackTrace = NULL;
    if (stackTrace != NULL) {
        jstackTrace = env->NewStringUTF(stackTrace);
    }
    jthrowable wrappedException = NULL;
    if (env->ExceptionCheck()) {
        wrappedException = env->ExceptionOccurred();
        env->ExceptionClear();
    }
    if (reinterpret_cast<V8Runtime*>(v8RuntimePtr)->pendingException != NULL) {
        wrappedException = reinterpret_cast<V8Runtime*>(v8RuntimePtr)->pendingException;
        reinterpret_cast<V8Runtime*>(v8RuntimePtr)->pendingException = NULL;
    }
    if ( wrappedException != NULL && !env->IsInstanceOf( wrappedException, throwableCls) ) {
        std::cout << "Wrapped Exception is not a Throwable" << std::endl;
        wrappedException = NULL;
    }
    jthrowable result = (jthrowable)env->NewObject(v8ScriptExecutionException, v8ScriptExecutionExceptionInitMethodID, jfileName, lineNumber, jmessage, jsourceLine, startColumn, endColumn, jstackTrace, wrappedException);
    env->DeleteLocalRef(jfileName);
    env->DeleteLocalRef(jmessage);
    env->DeleteLocalRef(jsourceLine);
    (env)->Throw(result);
}

void throwParseException(JNIEnv *env, Isolate* isolate, TryCatch* tryCatch) {
    String::Value exception(isolate,tryCatch->Exception());
    Handle<Message> message = tryCatch->Message();
    if (message.IsEmpty()) {
        throwV8RuntimeException(env, &exception);
    }
    else {
        String::Utf8Value filename(isolate,message->GetScriptResourceName());
        int lineNumber = message->GetLineNumber(isolate->GetCurrentContext()).ToChecked();
        String::Value sourceline(isolate,message->GetSourceLine(isolate->GetCurrentContext()).ToLocalChecked());
        int start = message->GetStartColumn();
        int end = message->GetEndColumn();
        const char* filenameString = ToCString(filename);
        throwParseException(env, filenameString, lineNumber, &exception, &sourceline, start, end);
    }
}

void throwExecutionException(JNIEnv *env, Isolate* isolate, TryCatch* tryCatch, jlong v8RuntimePtr) {
    String::Value exception(isolate,tryCatch->Exception());
    Handle<Message> message = tryCatch->Message();
    if (message.IsEmpty()) {
        throwV8RuntimeException(env, &exception);
    }
    else {
        String::Utf8Value filename(isolate,message->GetScriptResourceName());
        int lineNumber = message->GetLineNumber(isolate->GetCurrentContext()).ToChecked();
        String::Value sourceline(isolate,message->GetSourceLine(isolate->GetCurrentContext()).ToLocalChecked());
        int start = message->GetStartColumn();
        int end = message->GetEndColumn();
        const char* filenameString = ToCString(filename);
        String::Utf8Value stack_trace(isolate,tryCatch->StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
        const char* stackTrace = NULL;
        if (stack_trace.length() > 0) {
            stackTrace = ToCString(stack_trace);
        }
        throwExecutionException(env, filenameString, lineNumber, &exception, &sourceline, start, end, stackTrace, v8RuntimePtr);
    }
}

void throwV8RuntimeException(JNIEnv *env, String::Value *message) {
    jstring exceptionString = env->NewString(**message, message->length());
    jthrowable exception = (jthrowable)env->NewObject(v8RuntimeExceptionCls, v8RuntimeExceptionInitMethodID, exceptionString);
    (env)->Throw(exception);
    env->DeleteLocalRef(exceptionString);
}

void throwError(JNIEnv *env, const char *message) {
    (env)->ThrowNew(errorCls, message);
}

jobject getResult(JNIEnv *env, jobject &v8, jlong v8RuntimePtr, Isolate* isolate, Handle<Value> &result, jint expectedType) {

    if (result->IsUndefined() && expectedType == com_piv8_v8_V8_V8_ARRAY) {
        jobject objectResult = env->NewObject(undefinedV8ArrayCls, undefinedV8ArrayInitMethodID, v8);
        return objectResult;
    }
    else if (result->IsUndefined() && (expectedType == com_piv8_v8_V8_V8_OBJECT || expectedType == com_piv8_v8_V8_NULL)) {
        jobject objectResult = env->NewObject(undefinedV8ObjectCls, undefinedV8ObjectInitMethodID, v8);
        return objectResult;
    }
    else if (result->IsInt32()) {
        return env->NewObject(integerCls, integerInitMethodID, result->Int32Value(isolate->GetCurrentContext()).ToChecked());
    }
    else if (result->IsNumber()) {
        return env->NewObject(doubleCls, doubleInitMethodID, result->NumberValue(isolate->GetCurrentContext()).ToChecked());
    }
    else if (result->IsBoolean()) {
        return env->NewObject(booleanCls, booleanInitMethodID, result->BooleanValue(isolate->GetCurrentContext()).ToChecked());
    }
    else if (result->IsString()) {
        v8::Isolate* isolate = reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate;

        String::Value unicodeString(isolate,result->ToString(isolate));

        return env->NewString(*unicodeString, unicodeString.length());
    }
    else if (result->IsFunction()) {
        jobject objectResult = env->NewObject(v8FunctionCls, v8FunctionInitMethodID, v8);
        jlong resultHandle = getHandle(env, objectResult);

        v8::Isolate* isolate = reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate;

        reinterpret_cast<Persistent<Object>*>(resultHandle)->Reset(isolate, result->ToObject(isolate));

        return objectResult;
    }
    else if (result->IsArray()) {
        jobject objectResult = env->NewObject(v8ArrayCls, v8ArrayInitMethodID, v8);
        jlong resultHandle = getHandle(env, objectResult);

        v8::Isolate* isolate = reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate;

        reinterpret_cast<Persistent<Object>*>(resultHandle)->Reset(isolate, result->ToObject(isolate));

        return objectResult;
    }
    else if (result->IsTypedArray()) {
        jobject objectResult = env->NewObject(v8TypedArrayCls, v8TypedArrayInitMethodID, v8);
        jlong resultHandle = getHandle(env, objectResult);

        v8::Isolate* isolate = reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate;

        reinterpret_cast<Persistent<Object>*>(resultHandle)->Reset(isolate, result->ToObject(isolate));

        return objectResult;
    }
    else if (result->IsArrayBuffer()) {
        ArrayBuffer* arrayBuffer = ArrayBuffer::Cast(*result);
        if ( arrayBuffer->ByteLength() == 0 || arrayBuffer->GetContents().Data() == NULL ) {
            jobject objectResult = env->NewObject(v8ArrayBufferCls, v8ArrayBufferInitMethodID, v8, NULL);
            jlong resultHandle = getHandle(env, objectResult);
            v8::Isolate* isolate = reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate;
            reinterpret_cast<Persistent<Object>*>(resultHandle)->Reset(isolate, result->ToObject(isolate));
            return objectResult;
        }
        jobject byteBuffer = env->NewDirectByteBuffer(arrayBuffer->GetContents().Data(), arrayBuffer->ByteLength());
        jobject objectResult = env->NewObject(v8ArrayBufferCls, v8ArrayBufferInitMethodID, v8, byteBuffer);
        jlong resultHandle = getHandle(env, objectResult);

        v8::Isolate* isolate = reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate;

        reinterpret_cast<Persistent<Object>*>(resultHandle)->Reset(isolate, result->ToObject(isolate));

        return objectResult;
    }
    else if (result->IsObject()) {
        jobject objectResult = env->NewObject(v8ObjectCls, v8ObjectInitMethodID, v8);
        jlong resultHandle = getHandle(env, objectResult);

        v8::Isolate* isolate = reinterpret_cast<V8Runtime*>(v8RuntimePtr)->isolate;

        reinterpret_cast<Persistent<Object>*>(resultHandle)->Reset(isolate, result->ToObject(isolate));

        return objectResult;
    }

    return NULL;
}

JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1connect(JNIEnv *env, jobject, jlong v8RuntimePtr, jobject connection){
    Isolate *isolate = SETUP(env,v8RuntimePtr,);
    piv8_inspector::getInstance()->connect(env,connection);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1disconnect(JNIEnv *env, jobject, jlong v8RuntimePtr){
    Isolate *isolate = SETUP(env,v8RuntimePtr,);
    piv8_inspector::getInstance()->disconnect(env,isolate);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1dispatchMessage(JNIEnv *env, jobject, jlong v8RuntimePtr,jstring message){
    Isolate *isolate = SETUP(env,v8RuntimePtr,);
    std::string msg = util::jstring2string(env,message);
    piv8_inspector::getInstance()->doDispatchMessage(isolate,msg);
}


JNIEXPORT void JNICALL Java_com_yineng_piv8_V8__1initDebugger(JNIEnv *env, jobject , jlong v8RuntimePtr){
    Isolate *isolate = SETUP(env,v8RuntimePtr,);
    piv8_inspector::getInstance()->init(v8RuntimePtr,isolate);
}


