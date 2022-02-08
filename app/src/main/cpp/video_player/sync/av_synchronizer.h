//
// Created by syy on 2022/1/22.
//

#ifndef ANDROID_VIDEO_PLAYER_AV_SYNCHRONIZER_H
#define ANDROID_VIDEO_PLAYER_AV_SYNCHRONIZER_H

#include "CommonTools.h"
#include "../decoder/decoder_request_header.h"
#include "../decoder/video_decoder.h"
#include "../decoder/ffmpeg_video_decoder.h"
#include "../../common/opengl_media/movie_frame.h"
#include <queue>

using namespace std;
// 以下是解码本地文件的时候的缓冲区的最小和最大值
#define LOCAL_MIN_BUFFERED_DURATION   			0.5
#define LOCAL_MAX_BUFFERED_DURATION   			0.8
#define LOCAL_AV_SYNC_MAX_TIME_DIFF         		0.05

class AVSynchronizer;

class UploaderCallbackImpl : public UploaderCallback {
public:
    void setParent(AVSynchronizer * pParent) {
        mParent = pParent;
    }

public:
    void processVideoFrame(GLuint inputTexId, int width, int height, float position);

    int processAudioData(short *sample, int size, float position, byte** buffer);

    void onSeekCallback(float seek_seconds);

    void initFromUploaderGLContext(EGLCore* eglCore);

    void destroyFromUploaderGLContext();

    void createEncoderOutputRender() { };

protected:
    AVSynchronizer * mParent;
};

class AVSynchronizer {
public:
    AVSynchronizer();

    virtual ~AVSynchronizer();

    //当前缓冲区是否有数据
    bool buffered;
    bool isCompleted;
    bool init(DecoderRequestHeader *requestHeader, JavaVM *g_jvm, jobject obj,
              float minBufferedDuration, float maxBufferedDuration);
    UploaderCallbackImpl mUploaderCallback;

    void start();

    bool validAudio();

    int getAudioChannels();

    int getAudioSampleRate();

    bool isValid();
    bool isDestroyed;

    bool isPlayCompleted() {
        return isCompleted;
    }

    int fillAudioData(byte *string, int i);

    int getVideoFrameWidth();

    float getVideoFPS();

    int getVideoFrameHeight();

    EGLContext getUploaderEGLContext();

    void OnInitFromUploaderGLContext(EGLCore *pCore, int i, int i1);

    void initCircleQueue(int videoWidth, int videoHeight);

    void interruptRequest();

    virtual void destroy();

    void clearFrameMeta();

protected:
    std::queue<AudioFrame*> *audioFrameQueue;
    /** 解码出来的videoFrame与audioFrame的容器，以及同步操作信号量 **/
    pthread_mutex_t audioFrameQueueMutex;
    /** 这里是为了将audioFrame中的数据，缓冲到播放音频的buffer中，有可能需要积攒几个frame，所以记录位置以及保存当前frame **/
    AudioFrame* currentAudioFrame;
    int currentAudioFramePos;
    /** 当前movie的position，用于同步音画 **/
    double moviePosition;
    /** 根据缓冲区来控制是否需要编解码的变量 **/
    float bufferedDuration;//当前缓冲区时长
    //init 方法中调用的私有方法
    VideoDecoder* decoder;
    int decodeVideoErrorState;
    /** 是否初始化解码线程 **/
    bool isInitializeDecodeThread;
    //回调客户端的方法比如书显示或者隐藏progressbar
    JavaVM *g_jvm;
    jobject obj;
    //表示客户端界面上是否显示loading progressbar
    bool isLoading;
    float syncMaxTimeDiff;
    float minBufferedDuration;//缓冲区的最短时长
    float maxBufferedDuration;//缓冲区的最长时长
    /** 当客户端调用destroy方法之后 只为true **/
    bool isOnDecoding;
    bool isDecodingFrames;
    pthread_mutex_t  videoDecoderLock;
    pthread_cond_t videoDecoderCondition;
    pthread_t videoDecoderThread;
    CircleFrameTextureQueue* circleFrameTextureQueue;

    virtual void createDecoderInstance();
    virtual void initMeta();
    bool isHWCodecAvailable();

    void closeDecoder();

    void viewStreamMetaCallback(int videoWidth, int videoHeight, float duration);

    int jniCallbackWithNoArguments(char* signature, char* params);
    int jniCallbackWithArguments(const char* signature, const char* params, ...);

    void initDecoderThread();

    static void* startDecoderThread(void* ptr);

    void decode();

    void decodeFrames();

    void signalDecodeThread();

    virtual void destroyDecoderThread();

    void clearAudioFrameQueue();

    inline bool canDecode(){
        return !pauseDecodeThreadFlag && !isDestroyed && decoder && (decoder->validVideo() || decoder->validAudio()) && !decoder->isEOF();
    }

    bool pauseDecodeThreadFlag;

    void processDecodingFrame(bool& good, float duration);

    bool addFrames(std::list<MovieFrame*>* frames);

    bool addFrames(float thresholdDuration, std::list<MovieFrame*>* frames);

    bool checkPlayState();

    int videoDecodeException();

    int hideLoadingDialog();

    int showLoadingDialog();

    int onCompletion();

    virtual void useForstatistic(int leftVideoFrames){};
};


#endif //ANDROID_VIDEO_PLAYER_AV_SYNCHRONIZER_H
