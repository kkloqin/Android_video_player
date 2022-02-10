#include "./com_syy_studio_player_Player.h"
#include "../cpp/common/CommonTools.h"

#define LOG_TAG "Player_Player_JNI_Layer"

#include "video_player/video_player_controller.h"
#include <android/native_window_jni.h>

VideoPlayerController *videoPlayerController = NULL;
static ANativeWindow *window = 0;

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_syy_studio_player_Player_prepare(JNIEnv *env, jobject obj, jstring src_filename_param,
                                          jintArray max_analyze_durations, jint size,
                                          jint probesize, jboolean fps_probesize_configured,
                                          jfloat min_buffered_duration,
                                          jfloat max_buffered_duration, jint width, jint height,
                                          jobject surface) {
    LOGI(" enter Java_com_syy_studio_player_Player_prepare");
    JavaVM *g_jvm = NULL;
    env->GetJavaVM(&g_jvm);
    jobject g_obj = env->NewGlobalRef(obj);
    char *videoFilePath = (char *) env->GetStringUTFChars(src_filename_param, NULL);
    if (videoPlayerController == NULL) {
        videoPlayerController = new VideoPlayerController();
    }
    window = ANativeWindow_fromSurface(env, surface);
    jint *max_analyze_duration_params = env->GetIntArrayElements(max_analyze_durations, 0);
    jboolean initCode = videoPlayerController->init(videoFilePath, g_jvm, g_obj,
                                                    max_analyze_duration_params,
                                                    size, probesize, fps_probesize_configured,
                                                    min_buffered_duration, max_buffered_duration);
    videoPlayerController->onSurfaceCreated(window, width, height);
    env->ReleaseIntArrayElements(max_analyze_durations, max_analyze_duration_params, 0);
    env->ReleaseStringUTFChars(src_filename_param, videoFilePath);

    return initCode;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_syy_studio_player_Player_pause(JNIEnv *env, jobject thiz) {
    if(NULL != videoPlayerController) {
        videoPlayerController->pause();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_syy_studio_player_Player_onSurfaceCreated(JNIEnv *env, jobject thiz, jobject surface) {
    if (NULL != videoPlayerController) {
        window = ANativeWindow_fromSurface(env, surface);
//        videoPlayerController->onSurfaceCreated(window, width, height);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_syy_studio_player_Player_onSurfaceDestroyed(JNIEnv *env, jobject thiz, jobject surface) {
    if (NULL != videoPlayerController) {
        videoPlayerController->onSurfaceDestroyed();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_syy_studio_player_Player_resetRenderSize(JNIEnv *env, jobject thiz, jint left, jint top,
                                                  jint width, jint height) {
    if(NULL != videoPlayerController) {
        videoPlayerController->resetRenderSize(left, top, width, height);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_syy_studio_player_Player_play(JNIEnv *env, jobject thiz) {
    if(NULL != videoPlayerController) {
        videoPlayerController->play();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_syy_studio_player_Player_stop(JNIEnv *env, jobject thiz) {
    if(NULL != videoPlayerController) {
        videoPlayerController->destroy();
        delete videoPlayerController;
        videoPlayerController = NULL;
    }
}
extern "C"
JNIEXPORT jfloat JNICALL
Java_com_syy_studio_player_Player_getBufferedProgress(JNIEnv *env, jobject thiz) {
    if (NULL != videoPlayerController) {
        return videoPlayerController->getBufferedProgress();
    }
    return 0.0f;
}
extern "C"
JNIEXPORT jfloat JNICALL
Java_com_syy_studio_player_Player_getPlayProgress(JNIEnv *env, jobject thiz) {
    if (NULL != videoPlayerController) {
        return videoPlayerController->getPlayProgress();
    }
    return 0.0f;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_syy_studio_player_Player_seekToPosition(JNIEnv *env, jobject thiz, jfloat position) {
    if(NULL != videoPlayerController) {
        videoPlayerController->seekToPosition(position);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_syy_studio_player_Player_seekCurrent(JNIEnv *env, jobject thiz, jfloat position) {
    if(NULL != videoPlayerController) {
//		videoPlayerController->seekCurrent(position);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_syy_studio_player_Player_beforeSeekCurrent(JNIEnv *env, jobject thiz) {
    if(NULL != videoPlayerController) {
//		videoPlayerController->beforeSeekCurrent();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_syy_studio_player_Player_afterSeekCurrent(JNIEnv *env, jobject thiz) {
    if(NULL != videoPlayerController) {
//		videoPlayerController->afterSeekCurrent();
    }
}