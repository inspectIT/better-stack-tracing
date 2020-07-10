#include "AsyncTracer.h"
#include <sstream>
#include <dlfcn.h>
#include <iostream>

AsyncGetCallTraceType AsyncTracer::asyncGetCallTrace = NULL;
const int AsyncTracer::SIGNAL_CODE = SIGRTMIN + 7;
std::atomic<pthread_t> AsyncTracer::armedThread = { AsyncTracer::INVALID_TID };
volatile bool  AsyncTracer::sampleFinished = false;
JavaVM* AsyncTracer::vm = NULL;
jint  AsyncTracer::maxDepth = 0;
AsyncCallTrace  AsyncTracer::globalTrace = {};
AsyncCallFrame* AsyncTracer::globalFrames = NULL;


jstring AsyncTracer::init(JNIEnv* env, jvmtiEnv* jvmti, jint maxDepth) {

    if (env->GetJavaVM(&vm) != JNI_OK) {
        return env->NewStringUTF("Failed to get JavaVM instance");
    }
    jstring error;

    error = locateAsyncGetCallTrace(env);
    if (error != NULL) {
        return error;
    }

    error = initMethodIdPopulation(env, jvmti);
    if (error != NULL) {
        return error;
    }

    error = registerSignalHandler(env, maxDepth);
    if (error != NULL) {
        return error;
    }

    return NULL;
}

jstring AsyncTracer::locateAsyncGetCallTrace(JNIEnv* env) {
    dlerror();
    asyncGetCallTrace = reinterpret_cast<AsyncGetCallTraceType>(dlsym(RTLD_DEFAULT, "AsyncGetCallTrace"));
    const char* err = dlerror();
    if (err != NULL) {
        std::ostringstream msg;
        msg << "Failed to locate AsyncGetCallTrace: " << err;
        return env->NewStringUTF(msg.str().c_str());
    }
    return NULL;
}

jstring AsyncTracer::initMethodIdPopulation(JNIEnv* env, jvmtiEnv* jvmti) {
    jvmtiCapabilities capabilities = { 0 };
    capabilities.can_generate_all_class_hook_events = 1;
    jvmtiEventCallbacks callbacks = { 0 };
    callbacks.ClassLoad = AsyncTracer::classLoadCallback;
    if (jvmti->AddCapabilities(&capabilities) != 0) {
        return env->NewStringUTF("Failed to add class hook capabilitiy");
    }
    if (jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks)) != 0) {
        return env->NewStringUTF("Failed to configure class load callback");
    }
    if (jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_LOAD, NULL) != 0) {
        return env->NewStringUTF("Failed to enable class load callbacks");
    }
    populateMethodIDsOfLoadedClasses(jvmti);
    return NULL;
}



jstring AsyncTracer::registerSignalHandler(JNIEnv* env, jint maxDepth) {
    if (!armedThread.is_lock_free()) {
        return env->NewStringUTF("atomic<pthread_t> is not lock free!");
    }
    AsyncTracer::maxDepth = maxDepth;
    AsyncTracer::globalFrames = new AsyncCallFrame[maxDepth];
    AsyncTracer::globalTrace.frames = globalFrames;

    AsyncTracer::armedThread = INVALID_TID;
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = AsyncTracer::signalHandler;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigaction(AsyncTracer::SIGNAL_CODE, &sa, NULL);

    return NULL;
}

void AsyncTracer::classLoadCallback(jvmtiEnv* jvmti, JNIEnv* jni, jthread thread, jclass clazz) {
    populateMethodIDs(jvmti, clazz);
}

void AsyncTracer::populateMethodIDs(jvmtiEnv* jvmti, jclass clazz) {
    jint count;
    jmethodID* methods;
    int err = jvmti->GetClassMethods(clazz, &count, &methods);
    if (err == 0) {
        jvmti->Deallocate((unsigned char*)methods);
    }
}

void AsyncTracer::populateMethodIDsOfLoadedClasses(jvmtiEnv* jvmti) {
    jint count;
    jclass* classes;
    if (jvmti->GetLoadedClasses(&count, &classes) == 0) {
        for (int i = 0; i < count; i++) {
            populateMethodIDs(jvmti, classes[i]);
        }
        jvmti->Deallocate((unsigned char*)classes);
    }
}

void AsyncTracer::signalHandler(int signal, siginfo_t* siginfo, void* context) {
    pthread_t selfId = pthread_self();
    if (armedThread.compare_exchange_strong(selfId, INVALID_TID)) {
        JNIEnv* env;
        if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
            AsyncTracer::globalTrace.frameCount = -40;
        }
        else {
            AsyncTracer::globalTrace.env = env;
            (*asyncGetCallTrace)(&AsyncTracer::globalTrace, AsyncTracer::maxDepth, context);
        }
        sampleFinished = true;
    }
}


jstring AsyncTracer::destroy( jvmtiEnv* jvmti, JNIEnv* jni) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = SIG_DFL;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigaction(AsyncTracer::SIGNAL_CODE, &sa, NULL);

    delete[] AsyncTracer::globalFrames;
    jvmtiEventCallbacks callbacks = { 0 };
    if (jvmti->SetEventNotificationMode(JVMTI_DISABLE, JVMTI_EVENT_CLASS_LOAD, NULL) != 0) {
        return jni->NewStringUTF("Failed to disable class load callbacks");
    }
    if (jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks)) != 0) {
        return jni->NewStringUTF("Failed to disable class load callback");
    }
    return NULL;
}

StackTrace* AsyncTracer::getStackTrace(pthread_t target) {
    sampleFinished = false;
    armedThread.store(target);
    if (pthread_kill(target, AsyncTracer::SIGNAL_CODE) != 0) {
        return new StackTrace(-42, NULL);
    }
    while (!sampleFinished) {
        pthread_yield();
    }
    return new StackTrace(&globalTrace);
}
