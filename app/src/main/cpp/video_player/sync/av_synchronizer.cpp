//
// Created by syy on 2022/1/22.
//


#include "av_synchronizer.h"
#define LOG_TAG "Player_AVSynchronizer"


/*
 * class UploaderCallbackImpl
 *
 */
void UploaderCallbackImpl::processVideoFrame(GLuint inputTexId, int width, int height, float position) {
//    if (mParent){
//        mParent->processVideoFrame(inputTexId, width, height, position);
//    }
}

int UploaderCallbackImpl::processAudioData(short *sample, int size, float position, byte** buffer) {
    int bufferSize = size * 2;
    (*buffer) = new byte[bufferSize];
    convertByteArrayFromShortArray(sample, size, *buffer);
    return bufferSize;
}

void UploaderCallbackImpl::onSeekCallback(float seek_seconds) {
//    if (mParent){
//        mParent->onSeek(seek_seconds);
//    }
}

void UploaderCallbackImpl::initFromUploaderGLContext(EGLCore* eglCore) {
    if (mParent){
        int videoFrameWidth = mParent->getVideoFrameWidth();
        int videoFrameHeight = mParent->getVideoFrameHeight();

        EGLContext eglContext = mParent->getUploaderEGLContext();
        mParent->OnInitFromUploaderGLContext(eglCore, videoFrameWidth, videoFrameHeight);
    }
}


EGLContext AVSynchronizer::getUploaderEGLContext() {
    if(decoder){
        return decoder->getTextureFrameUploaderEGLContext();
    }
    return NULL;
}


void UploaderCallbackImpl::destroyFromUploaderGLContext() {
//    if (mParent){
//        mParent->onDestroyFromUploaderGLContext();
//    }
}

bool AVSynchronizer::init(DecoderRequestHeader *requestHeader, JavaVM *g_jvm, jobject obj,
                          float minBufferedDuration, float maxBufferedDuration) {
    LOGI("enter AVSynchronizer::init");
    currentAudioFrame = NULL;
    currentAudioFramePos = 0;
    isCompleted = false;
    moviePosition = 0;
    buffered = false;
    bufferedDuration = 0;
    decoder = NULL;
    decodeVideoErrorState = 0;
    isLoading = false;
    isInitializeDecodeThread = false;
    this->minBufferedDuration = minBufferedDuration;
    this->maxBufferedDuration = maxBufferedDuration;
    this->g_jvm = g_jvm;
    this->obj = obj;
    isOnDecoding = false;
    isDestroyed = false;

    //1、创建decoder实例
    this->createDecoderInstance();
    //2、初始化成员变量
    this->initMeta();
    //3、打开流并且解析出来音视频流的Context
    int initCode = decoder->openFile(requestHeader);
    if (initCode < 0 || isDestroyed) {
        LOGI("VideoDecoder decode file fail...");
        closeDecoder();
        return false;
    }
    if (!decoder->isSubscribed() || isDestroyed) {
        LOGI("decoder has not Subscribed...");
        closeDecoder();
        return false;
    }
    //5、回调客户端视频宽高以及duration
    float duration = decoder->getDuration();
    int videoWidth = decoder->getVideoFrameWidth();
    int videoHeight = decoder->getVideoFrameHeight();
    if(videoWidth <= 0 || videoHeight <= 0){
        return false;
    }
    //6、启动decoder的uploader部分
    decoder->startUploader(&mUploaderCallback);
    this->viewStreamMetaCallback(videoWidth, videoHeight, duration);
    //7、increase for Only audio stream
    if (!decoder->validVideo()){
        this->minBufferedDuration *= 10.0;
    }

    LOGI("Leave AVSynchronizer::init");
    return true;
}

AVSynchronizer::AVSynchronizer() {
    audioFrameQueue = NULL;
    circleFrameTextureQueue = NULL;
    currentAudioFrame = NULL;
    decoder = NULL;
//    passThorughRender = NULL;

    mUploaderCallback.setParent(this);
}

AVSynchronizer::~AVSynchronizer() {

}

void AVSynchronizer::viewStreamMetaCallback(int videoWidth, int videoHeight,
                                            float duration) {
    jniCallbackWithArguments("viewStreamMetaCallback", "(IIF)V", videoWidth, videoHeight, duration);
}


void AVSynchronizer::closeDecoder() {
    if (NULL != decoder) {
        decoder->closeFile();
        delete decoder;
        decoder = NULL;
    }
}

void AVSynchronizer::initMeta() {
    this->minBufferedDuration = LOCAL_MIN_BUFFERED_DURATION;
    this->maxBufferedDuration = LOCAL_MAX_BUFFERED_DURATION;
    this->syncMaxTimeDiff = LOCAL_AV_SYNC_MAX_TIME_DIFF;
}

void AVSynchronizer::createDecoderInstance() {
    if(isHWCodecAvailable()) {

    } else {
        decoder = new FFMPEGVideoDecoder(g_jvm, obj);
    }
}

bool AVSynchronizer::isHWCodecAvailable() {
    bool useMediaCodecDecoder = false;
    JNIEnv *env = 0;
    int status = 0;
    bool needAttach = false;
    status = g_jvm->GetEnv((void **) (&env), JNI_VERSION_1_4);
    // don't know why, if detach directly, will crash
    if (status < 0) {
        if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
            LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
            return false;
        }
        needAttach = true;
    }
    jclass jcls = env->GetObjectClass(obj);
    jmethodID useMediaCodecFunc = env->GetMethodID(jcls, "isHWCodecAvailableFromNative", "()Z");
    useMediaCodecDecoder = env->CallBooleanMethod(obj,useMediaCodecFunc);
    if (needAttach) {
        if (g_jvm->DetachCurrentThread() != JNI_OK) {
            LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
        }
    }
    LOGI("useMediaCodecDecoder =  %s ", useMediaCodecDecoder ? "true" : "false");
    return useMediaCodecDecoder;
}

int AVSynchronizer::jniCallbackWithArguments(const char* signature, const char* params, ...){
    JNIEnv *env;
    if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
        LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
        return -1;
    }
    if (env == NULL) {
        LOGI("getJNIEnv failed");
        return -1;
    }
    jclass jcls = env->GetObjectClass(obj);
    if (NULL != jcls) {
        jmethodID jniCallback = env->GetMethodID(jcls, signature, params);
        if (NULL != jniCallback) {
            va_list arg_ptr;
            va_start(arg_ptr,params);
            env->CallVoidMethodV(obj, jniCallback, arg_ptr);
            va_end(arg_ptr);
        }
    }
    if (g_jvm->DetachCurrentThread() != JNI_OK) {
        LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
        return -1;
    }
    return 1;
}

void AVSynchronizer::start() {
    isOnDecoding = true;
//    pauseDecodeThreadFlag = false;

//    circleFrameTextureQueue->setIsFirstFrame(true);

    //开启解码线程
    initDecoderThread();
}

void AVSynchronizer::initDecoderThread() {
    isDecodingFrames = false;
    pthread_mutex_init(&videoDecoderLock, NULL);
    pthread_cond_init(&videoDecoderCondition, NULL);
    isInitializeDecodeThread = true;
    pthread_create(&videoDecoderThread, NULL, startDecoderThread, this);
    
    
}

void * AVSynchronizer::startDecoderThread(void* ptr) {
    AVSynchronizer* synchronizer = (AVSynchronizer *) ptr;

    while (synchronizer->isOnDecoding) {
        synchronizer->decode();
    }
    return 0;
}

void AVSynchronizer::decode(){
//    LOGI("before pthread_cond_wait");
    pthread_mutex_lock(&videoDecoderLock);
    pthread_cond_wait(&videoDecoderCondition, &videoDecoderLock);
    pthread_mutex_unlock(&videoDecoderLock);

    isDecodingFrames = true;
    decodeFrames();
    isDecodingFrames = false;

}

void AVSynchronizer::decodeFrames() {
    bool good = true;
    float duration = decoder->isNetwork() ? 0.0f : 0.1f;
    while (good) {
        good = false;
        if (canDecode()) {
            processDecodingFrame(good, duration);
        } else {
            break;
        }
    }
}

void AVSynchronizer::processDecodingFrame(bool& good, float duration){
    std::list<MovieFrame*>* frames = decoder->decodeFrames(duration,&decodeVideoErrorState);
    if (NULL != frames) {
        if (!frames->empty()) {
            if (decoder->hasSeekReq()) {
                if (decoder->hasSeekResp()) {
                    if (NULL != audioFrameQueue) {
                        clearAudioFrameQueue();
                    }
                    bufferedDuration = 0.0f;
                    good = addFrames(frames);
                    int count = audioFrameQueue->size();
                    if (count > 0) {
                        AudioFrame *frame = audioFrameQueue->front();
                        moviePosition = frame->position;
                    }
                    buffered = false;
                    decoder->setSeekReq(false);
                } else {
                    std::list<MovieFrame*>::iterator i;
                    for (i = frames->begin(); i != frames->end(); ++i) {
                        MovieFrame* frame = *i;
                        delete frame;
                    }
                }
            } else {
                good = addFrames(frames);
            }
        } else {
            LOGI("frames is empty %d", (int )good);
        }
        delete frames;
    } else {
        LOGI("why frames is NULL tell me why?");
    }
}

bool AVSynchronizer::addFrames(std::list<MovieFrame*>* frames) {
    return addFrames(maxBufferedDuration, frames);
}

bool AVSynchronizer::addFrames(float thresholdDuration, std::list<MovieFrame*>* frames) {
    if (decoder->validAudio()) {
        pthread_mutex_lock(&audioFrameQueueMutex);
        std::list<MovieFrame*>::iterator i;
        for (i = frames->begin(); i != frames->end(); ++i) {
            MovieFrame* frame = *i;
            if (frame->getType() == MovieFrameTypeAudio) {
                AudioFrame* audioFrame = (AudioFrame*) frame;
                audioFrameQueue->push(audioFrame);
//				LOGI("audioFrameQueue->push(audioFrame) position is %.4f", audioFrame->position);
                bufferedDuration += audioFrame->duration;
            }
        }
        pthread_mutex_unlock(&audioFrameQueueMutex);
    }
//	LOGI("bufferDuration is %.3f thresholdDuration is %.3f buffered is %d", bufferedDuration, thresholdDuration, buffered);
    bool isBufferedDurationIncreasedToThreshold = (bufferedDuration >= thresholdDuration) &&
                                                  (circleFrameTextureQueue->getValidSize()  >= thresholdDuration * getVideoFPS());
    return  !isBufferedDurationIncreasedToThreshold;
}

void AVSynchronizer::signalDecodeThread(){
    if (NULL == decoder || isDestroyed) {
        LOGI("NULL == decoder || isDestroyed == true");
        return;
    }

    bool isBufferedDurationDecreasedToMin = true;
    if (!isDestroyed && (decoder->hasSeekReq()) || ((!isDecodingFrames) && isBufferedDurationDecreasedToMin)) {
        pthread_mutex_lock(&videoDecoderLock);
        pthread_cond_signal(&videoDecoderCondition);
        pthread_mutex_unlock(&videoDecoderLock);
    }
}

bool AVSynchronizer::validAudio() {
    if (NULL == decoder || isDestroyed){
        return false;
    }
    return decoder->validAudio();
}

int AVSynchronizer::getAudioChannels() {
    int channels = -1;
    if (NULL != decoder) {
        channels = decoder->getAudioChannels();
    }
    return channels;
}

int AVSynchronizer::getAudioSampleRate() {
    return decoder->getAudioSampleRate();
}

bool AVSynchronizer::isValid() {
    if (NULL != decoder && !decoder->validVideo() && !decoder->validAudio()) {
        return false;
    }
    if (isDestroyed) {
        return false;
    }
    return true;
}


int AVSynchronizer::videoDecodeException() {
    return jniCallbackWithNoArguments("videoDecodeException", "()V");
}

int AVSynchronizer::hideLoadingDialog() {
    return jniCallbackWithNoArguments("hideLoadingDialog", "()V");
}

int AVSynchronizer::showLoadingDialog() {
    return jniCallbackWithNoArguments("showLoadingDialog", "()V");
}

int AVSynchronizer::onCompletion() {
    return jniCallbackWithNoArguments("onCompletion", "()V");
}

int AVSynchronizer::jniCallbackWithNoArguments(char* signature, char* params){
    return jniCallbackWithArguments(signature, params);
}

bool AVSynchronizer::checkPlayState() {
    if (NULL == decoder || NULL == circleFrameTextureQueue || NULL == audioFrameQueue) {
        LOGI("NULL == decoder || NULL == circleFrameTextureQueue || NULL == audioFrameQueue");
        return false;
    }
    //判断是否是视频解码错误
    if (1 == decodeVideoErrorState) {
        decodeVideoErrorState = 0;
        this->videoDecodeException();
    }

    int leftVideoFrames = decoder->validVideo() ? circleFrameTextureQueue->getValidSize() : 0;
    int leftAudioFrames = decoder->validAudio() ? audioFrameQueue->size() : 0;
    const int leftFrames = leftVideoFrames + leftAudioFrames;
//	LOGI("leftAudioFrames is %d, leftVideoFrames is %d, bufferedDuration is %f",
//			leftAudioFrames, leftVideoFrames, bufferedDuration);


    this->useForstatistic(leftVideoFrames);

    if (leftVideoFrames == 1 || leftAudioFrames == 0) {
//		LOGI("Setting Buffered is True : leftAudioFrames is %d, leftVideoFrames is %d, bufferedDuration is %f",
//				leftAudioFrames, leftVideoFrames, bufferedDuration);
        buffered = true;
        if (!isLoading) {
            isLoading = true;
            showLoadingDialog();
        }
        if (decoder->isEOF()) {
            //由于OpenSLES 暂停之后有一些数据 还在buffer里面，暂停200ms让他播放完毕
            usleep(0.2 * 1000000);
            isCompleted = true;
            onCompletion();
//			LOGI("onCompletion...");
            return true;
        }
    } else {
        bool isBufferedDurationIncreasedToMin = leftVideoFrames >= int(minBufferedDuration*getVideoFPS()) && (bufferedDuration >= minBufferedDuration);

        if (!decoder->hasSeekReq() && (isBufferedDurationIncreasedToMin || decoder->isEOF())) {
//			LOGI("Setting Buffered is False : leftAudioFrames is %d, leftVideoFrames is %d, bufferedDuration is %f, minBufferedDuration is %f",
//					leftAudioFrames, leftVideoFrames, bufferedDuration, minBufferedDuration);
            buffered = false;
            //回调android客户端hide loading dialog
            if (isLoading) {
                isLoading = false;
                hideLoadingDialog();
            }
        }
    }

    return false;
}

int AVSynchronizer::fillAudioData(byte* outData, int bufferSize) {
    //	LOGI("enter AVSynchronizer::fillAudioData... buffered is %d", buffered);
    this->signalDecodeThread();
    //todo syy
//    this->checkPlayState();
    if(buffered) {
//		LOGI("fillAudioData if(buffered) circleFrameTextureQueue->getValidSize() %d", circleFrameTextureQueue->getValidSize());
        memset(outData, 0, bufferSize);
        return bufferSize;
    }
    int needBufferSize = bufferSize;
    while (bufferSize > 0) {
        if (NULL == currentAudioFrame) {
            pthread_mutex_lock(&audioFrameQueueMutex);
            int count = audioFrameQueue->size();
//			LOGI("audioFrameQueue->size() is %d", count);
            if (count > 0) {
                AudioFrame *frame = audioFrameQueue->front();
                bufferedDuration -= frame->duration;
                audioFrameQueue->pop();
                if (!decoder->hasSeekReq()) {
                    //resolve when drag seek bar position changed Frequent
                    moviePosition = frame->position;
                }
                currentAudioFrame = new AudioFrame();
                currentAudioFramePos = 0;
                int frameSize = frame->size;
                currentAudioFrame->samples = new byte[frameSize];
                memcpy(currentAudioFrame->samples, frame->samples, frameSize);
                currentAudioFrame->size = frameSize;
                delete frame;
            }
            pthread_mutex_unlock(&audioFrameQueueMutex);
        }

        if (NULL != currentAudioFrame) {
            //从frame的samples数据放入到buffer中
            byte* bytes = currentAudioFrame->samples + currentAudioFramePos;
            int bytesLeft = currentAudioFrame->size - currentAudioFramePos;
            int bytesCopy = std::min(bufferSize, bytesLeft);
            memcpy(outData, bytes, bytesCopy);
            bufferSize -= bytesCopy;
            outData += bytesCopy;
            if (bytesCopy < bytesLeft)
                currentAudioFramePos += bytesCopy;
            else {
                delete currentAudioFrame;
                currentAudioFrame = NULL;
            }
        } else {
            LOGI("fillAudioData NULL == currentAudioFrame");
            memset(outData, 0, bufferSize);
            bufferSize = 0;
            break;
        }
    }
//	LOGI("leave AVSynchronizer::fillAudioData...");
    return needBufferSize - bufferSize;
}

int AVSynchronizer::getVideoFrameWidth() {
    int videoWidth = 0;
    if(decoder){
        videoWidth = decoder->getVideoFrameWidth();
    }
    return videoWidth;
}

float AVSynchronizer::getVideoFPS() {
    float fps = 0.0f;
    if (decoder) {
        fps = decoder->getVideoFPS();
    }
    return fps;
}

int AVSynchronizer::getVideoFrameHeight() {
    int videoHeight = 0;
    if(decoder){
        videoHeight = decoder->getVideoFrameHeight();
    }
    return videoHeight;
}

void AVSynchronizer::OnInitFromUploaderGLContext(EGLCore *pCore, int videoFrameWidth, int videoFrameHeight) {
//    if (NULL == passThorughRender) {
//        passThorughRender = new VideoGLSurfaceRender();
//        bool isGLViewInitialized = passThorughRender->init(videoFrameWidth,videoFrameHeight);
//        if (!isGLViewInitialized) {
//            LOGI("GL View failed on initialized...");
//        }
//    }
    initCircleQueue(videoFrameWidth, videoFrameHeight);
//    eglCore->doneCurrent();
}


void AVSynchronizer::initCircleQueue(int videoWidth, int videoHeight) {
    // 初始化audioQueue与videoQueue
    float fps = decoder->getVideoFPS();

//	LOGI("decoder->getVideoFPS() is %.3f maxBufferedDuration is %.3f", fps, maxBufferedDuration);
    //此处修正fps，因为某些平台得到的fps很大，导致计算出来要申请很多的显存，因此，这里做一下限定
    if (fps > 30.0f) {
        fps = 30.0f;
    }

    int queueSize = (maxBufferedDuration + 1.0) * fps;
    circleFrameTextureQueue = new CircleFrameTextureQueue(
            "decode frame texture queue");
    circleFrameTextureQueue->init(videoWidth, videoHeight, queueSize);
    audioFrameQueue = new std::queue<AudioFrame*>();
    pthread_mutex_init(&audioFrameQueueMutex, NULL);
}

void AVSynchronizer::interruptRequest() {
    if (decoder){
        decoder->interrupt();
    }
}


void AVSynchronizer::destroyDecoderThread() {
//	LOGI("AVSynchronizer::destroyDecoderThread ...");
    isOnDecoding = false;
    if (!isInitializeDecodeThread) {
        return;
    }
    void* status;
    int getLockCode = pthread_mutex_lock(&videoDecoderLock);
    pthread_cond_signal(&videoDecoderCondition);
    pthread_mutex_unlock(&videoDecoderLock);
    pthread_join(videoDecoderThread, &status);
    pthread_mutex_destroy(&videoDecoderLock);
    pthread_cond_destroy(&videoDecoderCondition);
}


void AVSynchronizer::destroy() {
    LOGI("enter AVSynchronizer::destroy ...");
    isDestroyed = true;
    //先停止掉解码线程
    destroyDecoderThread();
    isLoading = true;

    LOGI("clear and destroy audio frame queue ...");
    //清空并且销毁音频帧队列
    if (NULL != audioFrameQueue) {
        clearAudioFrameQueue();
        pthread_mutex_lock(&audioFrameQueueMutex);
        delete audioFrameQueue;
        audioFrameQueue = NULL;
        pthread_mutex_unlock(&audioFrameQueueMutex);
        pthread_mutex_destroy(&audioFrameQueueMutex);
    }

    LOGI("call decoder close video source URI ...");
    if (NULL != decoder) {
        if (decoder->isOpenInput()) {
            LOGI("call decoder closeFile ...");
            closeDecoder();
        } else {
            LOGI("call decoder stopSubscribe ...");
            decoder->stopSubscribe();
        }
    }

}

void AVSynchronizer::clearAudioFrameQueue() {
    pthread_mutex_lock(&audioFrameQueueMutex);
    while (!audioFrameQueue->empty()) {
        AudioFrame* frame = audioFrameQueue->front();
        audioFrameQueue->pop();
        delete frame;
    }

    bufferedDuration = 0;
    pthread_mutex_unlock(&audioFrameQueueMutex);
}


void AVSynchronizer::clearFrameMeta() {
    LOGI("destroy AudioFrame ...");
    if (NULL != currentAudioFrame) {
        delete currentAudioFrame;
        currentAudioFrame = NULL;
    }
}








