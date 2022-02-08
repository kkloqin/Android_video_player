

#include "video_player_controller.h"

#define LOG_TAG "Player_VideoPlayerController"

VideoPlayerController::VideoPlayerController() {
    LOGI("VideoPlayerController");
    userCancelled = false;
    videoOutput = NULL;
    screenWidth = 0;
    screenHeight = 0;
}

VideoPlayerController::~VideoPlayerController() {
    LOGI("~VideoPlayerController");
    videoOutput = NULL;
}

bool VideoPlayerController::init(char *srcFilenameParams, JavaVM *g_jvm, jobject obj,
                                 int *maxAnalyzeDuration, int analyzeCmt, int probeSize,
                                 bool fpsProbeSizeConfigured, float minBufferedDuration,
                                 float maxBufferedDuration) {
    isPlaying = false;
    videoOutput = NULL;

    requestHeader = new DecoderRequestHeader();
    requestHeader->init(srcFilenameParams, maxAnalyzeDuration, analyzeCmt, probeSize,
                        fpsProbeSizeConfigured);
    this->g_jvm = g_jvm;
    this->obj = obj;
    this->minBufferedDuration = minBufferedDuration;
    this->maxBufferedDuration = maxBufferedDuration;
    pthread_create(&initThreadThreadId, 0, initThreadCallback, this);

    userCancelled = false;
    return true;
}

void VideoPlayerController::destroy() {
    LOGI("enter VideoPlayerController::destroy...");

    userCancelled = true;

    if (synchronizer){
        //中断request
        synchronizer->interruptRequest();
    }

    pthread_join(initThreadThreadId, 0);

    //todo syy
//    if (NULL != videoOutput) {
//        videoOutput->stopOutput();
//        delete videoOutput;
//        videoOutput = NULL;
//    }

    if (NULL != synchronizer) {
        synchronizer->isDestroyed = true;
        this->pause();
        LOGI("stop synchronizer ...");
        synchronizer->destroy();

        LOGI("stop audioOutput ...");
        if (NULL != audioOutput) {
            audioOutput->stop();
            delete audioOutput;
            audioOutput = NULL;
        }
        synchronizer->clearFrameMeta();
        delete synchronizer;
        synchronizer = NULL;
    }
    if(NULL != requestHeader){
        requestHeader->destroy();
        delete requestHeader;
        requestHeader = NULL;
    }

    LOGI("leave VideoPlayerController::destroy...");
}


void VideoPlayerController::pause() {
    LOGI("VideoPlayerController::pause");
    if (!this->isPlaying)
        return;
    this->isPlaying = false;
    if (NULL != audioOutput) {
        audioOutput->pause();
    }
}

void VideoPlayerController::onSurfaceCreated(ANativeWindow *window, int width, int height) {

}

void *VideoPlayerController::initThreadCallback(void *myself) {
    VideoPlayerController *controller = (VideoPlayerController *) myself;
    controller->startAVSynchronizer();
    pthread_exit(0);
}

bool VideoPlayerController::startAVSynchronizer() {
    LOGI("enter  startAVSynchronizer");
    bool ret = false;
    if (userCancelled) {
        return ret;
    }
    if (this->initAVSynchronizer()) {
        if (synchronizer->validAudio()) {
            ret = this->initAudioOutput();
        }
    }
    if (ret) {
        if (NULL != synchronizer && !synchronizer->isValid()) {
            ret = false;
        } else {
            isPlaying = true;
            synchronizer->start();
            LOGI("call audioOutput start...");
            if (NULL != audioOutput) {
                audioOutput->start();
            }
            LOGI("After call audioOutput start...");
        }
    }
    LOGI("VideoPlayerController::startAVSynchronizer() init result:%s", (ret? "success" : "fail"));
    this->setInitializedStatus(ret);
    return ret;
}

void VideoPlayerController::setInitializedStatus(bool initCode) {
    LOGI("enter VideoPlayerController::setInitializedStatus...");
    JNIEnv *env = 0;
    int status = 0;
    bool needAttach = false;
    status = g_jvm->GetEnv((void **) (&env), JNI_VERSION_1_4);

    // don't know why, if detach directly, will crash
    if (status < 0) {
        if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
            LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
            return;
        }

        needAttach = true;
    }

    jclass jcls = env->GetObjectClass(obj);

    jmethodID onInitializedFunc = env->GetMethodID(jcls, "onInitializedFromNative", "(Z)V");
    env->CallVoidMethod(obj, onInitializedFunc, initCode);

    if (needAttach) {
        if (g_jvm->DetachCurrentThread() != JNI_OK) {
            LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
        }
    }
    LOGI("leave VideoPlayerController::setInitializedStatus...");
}

bool VideoPlayerController::initAVSynchronizer() {
    synchronizer = new AVSynchronizer();
    return synchronizer->init(requestHeader, g_jvm, obj, minBufferedDuration, maxBufferedDuration);
}

bool VideoPlayerController::initAudioOutput() {
    LOGI("VideoPlayerController::initAudioOutput");
    int channels = this->getAudioChannels();
    if (channels < 0) {
        LOGI("VideoDecoder get channels failed ...");
        return false;
    }
    int sampleRate = synchronizer->getAudioSampleRate();
    if (sampleRate < 0) {
        LOGI("VideoDecoder get sampleRate failed ...");
        return false;
    }
    audioOutput = new AudioOutput();
    SLresult result = audioOutput->initSoundTrack(channels, sampleRate, audioCallbackFillData,
                                                  this);
    if (SL_RESULT_SUCCESS != result) {
        LOGI("audio manager failed on initialized...");
        delete audioOutput;
        audioOutput = NULL;
        return false;
    }
    return true;
}

int VideoPlayerController::audioCallbackFillData(byte *outData, size_t bufferSize, void *ctx) {
    VideoPlayerController *playerController = (VideoPlayerController *) ctx;
    return playerController->consumeAudioFrames(outData, bufferSize);
}


int VideoPlayerController::getAudioChannels() {
    int channels = -1;
    if (NULL != synchronizer) {
        channels = synchronizer->getAudioChannels();
    }
    return channels;
}

int VideoPlayerController::consumeAudioFrames(byte *outData, size_t bufferSize) {
    int ret = bufferSize;
    if (this->isPlaying &&
        synchronizer && !synchronizer->isDestroyed && !synchronizer->isPlayCompleted()) {
//        LOGI("Before synchronizer fillAudioData...");
        ret = synchronizer->fillAudioData(outData, bufferSize);
//        LOGI("After synchronizer fillAudioData... ");
        signalOutputFrameAvailable();
    } else {
        LOGI("VideoPlayerController::consumeAudioFrames set 0");
        memset(outData, 0, bufferSize);
    }
    return ret;
}

void VideoPlayerController::signalOutputFrameAvailable() {
//  LOGI("signalOutputFrameAvailable");
    if (NULL != videoOutput) {
//        videoOutput->signalFrameAvailable();
    }
}