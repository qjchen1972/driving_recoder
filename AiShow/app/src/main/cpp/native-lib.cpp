#include <jni.h>
#include <string>
#include <cstring>
#include "jniimp/impproc.h"
#include "utils/native_debug.h"

ImpProc  *pImpId = nullptr;



extern "C" JNIEXPORT jlong JNICALL
Java_cqj_aishow_MainActivity_createImpId(JNIEnv *env,jobject instance) {
    pImpId = new ImpProc(env, instance);
    return reinterpret_cast<jlong>(pImpId);
}

extern "C" JNIEXPORT void JNICALL
Java_cqj_aishow_MainActivity_initNet(JNIEnv *env,
                                      jobject instance,
                                      jlong impId,
                                      jstring netPath) {
    ImpProc *pApp = reinterpret_cast<ImpProc *>(impId);
    ASSERT(pApp == pImpId, "Imp Obj mismatch");

    const char* chstr;
    chstr = env->GetStringUTFChars(netPath, 0);
    pImpId->init_facerec(chstr);
    return;
}

extern "C" JNIEXPORT void JNICALL
Java_cqj_aishow_MainActivity_initAudio(JNIEnv *env,
                                     jobject instance,
                                     jlong impId,
                                     jint sampleRate, jint framesPerBuf) {
    ImpProc *pApp = reinterpret_cast<ImpProc *>(impId);
    ASSERT(pApp == pImpId, "Imp Obj mismatch");
    pApp->init_audio(sampleRate,framesPerBuf);
    return;
}

extern "C" JNIEXPORT void JNICALL
Java_cqj_aishow_CameraView_initCamera(JNIEnv *env,
                                      jobject instance,
                                      jlong impId,
                                      jint width, jint height,jboolean isback) {
    ImpProc *pApp = reinterpret_cast<ImpProc *>(impId);
    ASSERT(pApp == pImpId, "Imp Obj mismatch");
    pApp->init_camera(height,width, isback);
    return;
}

extern "C" JNIEXPORT jstring JNICALL
Java_cqj_aishow_MainActivity_getText( JNIEnv* env,
                                      jobject thiz,
                                      jlong impId)
{
    ImpProc *pApp = reinterpret_cast<ImpProc *>(impId);
    ASSERT(pApp == pImpId, "Imp Obj mismatch");
    return env->NewStringUTF(pApp->get_text().c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_cqj_aishow_MainActivity_addtext( JNIEnv* env,
                                      jobject thiz,
                                      jlong impId,
                                      jstring str)
{
    ImpProc *pApp = reinterpret_cast<ImpProc *>(impId);
    ASSERT(pApp == pImpId, "Imp Obj mismatch");
    const char* chstr;
    chstr = env->GetStringUTFChars(str, 0);
    pApp->add_text(chstr);
}

extern "C" JNIEXPORT jint JNICALL
Java_cqj_aishow_MainActivity_waitFor( JNIEnv* env,
                                      jobject thiz,
                                      jlong impId,
                                      jint time){

    ImpProc *pApp = reinterpret_cast<ImpProc *>(impId);
    ASSERT(pApp == pImpId, "Imp Obj mismatch");
    return pApp->wait(time);
}

extern "C" JNIEXPORT void JNICALL
Java_cqj_aishow_MainActivity_addface( JNIEnv* env,
                                      jobject thiz,
                                      jlong impId,
                                      jstring name,
                                      jintArray imageDate_,
                                      jint imageWidth,
                                      jint imageHeight){

    ImpProc *pApp = reinterpret_cast<ImpProc *>(impId);
    ASSERT(pApp == pImpId, "Imp Obj mismatch");

    jint *cbuf;
    jboolean ptfalse = false;
    cbuf = env->GetIntArrayElements(imageDate_, &ptfalse);

    const char* chstr;
    chstr = env->GetStringUTFChars(name, 0);
    pApp->add_faceSet(cbuf,imageHeight, imageWidth,chstr);
}

extern "C" JNIEXPORT jobject JNICALL
Java_cqj_aishow_CameraView_getMinimumCompatiblePreviewSize( JNIEnv *env,
                                                            jobject instance,
                                                            jlong impId) {

    ImpProc *pApp = reinterpret_cast<ImpProc *>(impId);
    ASSERT(pApp == pImpId, "Imp Obj mismatch");

    jclass cls = env->FindClass("android/util/Size");
    jobject previewSize = env->NewObject(cls, env->GetMethodID(cls, "<init>", "(II)V"),
                           pApp->GetCompatibleCameraRes().width,
                           pApp->GetCompatibleCameraRes().height);
    return previewSize;
}

extern "C" JNIEXPORT void JNICALL
Java_cqj_aishow_CameraView_setPreviewWindow( JNIEnv *env,
                                             jobject instance,
                                             jlong impId,
                                             jobject surface ) {
    ImpProc *pApp = reinterpret_cast<ImpProc *>(impId);
    ASSERT(pApp == pImpId, "Imp Obj mismatch");
    pApp->set_Window(surface);
}



extern "C" JNIEXPORT void JNICALL
Java_cqj_aishow_CameraView_setPreviewStatus( JNIEnv *env,
                                             jobject instance,
                                             jlong impId,
                                             jboolean status) {

    ImpProc *pApp = reinterpret_cast<ImpProc *>(impId);
    ASSERT(pApp == pImpId, "Imp Obj mismatch");
    pApp->startPreview(status);
    pApp->start_audio(status);
}

extern "C" JNIEXPORT void JNICALL
Java_cqj_aishow_MainActivity_deleteImpid(JNIEnv *env,
                                       jobject instance,
                                       jlong impid) {

    if (!pImpId || !impid) {
        return;
    }
    ImpProc *pApp = reinterpret_cast<ImpProc *>(impid);
    ASSERT(pApp == pImpId, "ImpId Obj mismatch");

    delete pApp;
    // also reset the private global object
    pImpId = nullptr;
}