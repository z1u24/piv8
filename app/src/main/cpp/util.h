//
// Created by yineng on 2019/6/5.
//

#ifndef V8TEST_UTIL_H
#define V8TEST_UTIL_H

#include "jni.h"
#include "string"

class util{
public:
    static std::string jstring2string(JNIEnv *env, jstring jStr);
private:
};


#endif //V8TEST_UTIL_H
