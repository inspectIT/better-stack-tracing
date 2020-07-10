#include "StackTrace.h"

StackTrace::StackTrace(jint frameCount, jvmtiFrameInfo* jvmtiFrames) {
    this->frameCount = frameCount;
    if (frameCount > 0) {
        frames = new jmethodID[frameCount];
        for (jint i = 0; i < frameCount; i++) {
            frames[i] = jvmtiFrames[i].method;
        }
    } else {
        frames = NULL;
    }
}

StackTrace::StackTrace(AsyncCallTrace* trace) {
    this->frameCount = trace->frameCount;
    if (frameCount > 0) {
        frames = new jmethodID[frameCount];
        for (jint i = 0; i < frameCount; i++) {
            frames[i] = trace->frames[i].method;
        }
    }
    else {
        frames = NULL;
    }
}

StackTrace::~StackTrace() {
    if (frames != NULL) {
        delete[] frames;
    }
}

jobject StackTrace::toJavaObject(JNIEnv* env, jvmtiEnv* jvmti, jclass stackFrameClass) {
    jmethodID constructorId = env->GetMethodID(stackFrameClass, "<init>", "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/String;)V");
    jobjectArray result = env->NewObjectArray(this->frameCount, stackFrameClass, NULL);
    for (jint i = 0; i < this->frameCount; i++) {
        if (this->frames[i] != NULL) {
            char* name;
            char* signature;
            jclass clazz;
            if (jvmti->GetMethodDeclaringClass(this->frames[i], &clazz) != 0) {
                continue;
            }
            if (jvmti->GetMethodName(this->frames[i], &name, &signature, NULL) != 0) {
                continue;
            }
            jobject frame = env->NewObject(stackFrameClass, constructorId, clazz, env->NewStringUTF(name), env->NewStringUTF(signature));
            jvmti->Deallocate((unsigned char*) name);
            jvmti->Deallocate((unsigned char*) signature);
            env->SetObjectArrayElement(result, i, frame);
        }
    }
    return result;
}