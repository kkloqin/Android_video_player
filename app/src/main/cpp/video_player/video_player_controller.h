
#ifndef ANDROID_VIDEO_PLAYER_VIDEO_PLAYER_CONTROLLER_H
#define ANDROID_VIDEO_PLAYER_VIDEO_PLAYER_CONTROLLER_H


#include "CommonTools.h"
#include <android/native_window.h>
#include "video_output.h"
#include "audio_output.h"
#include "decoder/decoder_request_header.h"
#include "sync/av_synchronizer.h"

class VideoPlayerController {
public:
    VideoPlayerController();

    virtual ~VideoPlayerController();

    bool init(char *srcFilenameParams, JavaVM *g_jvm, jobject obj, int *maxAnalyzeDuration,
              int analyzeCmt, int probeSize, bool fpsProbeSizeConfigured, float minBufferedDuration,
              float maxBufferedDuration);

    void play();

    void seekToPosition(float position);

    void pause();

    virtual void destroy();

    float getDuration();

    int getVideoFrameWidth();

    int getVideoFrameHeight();

    float getBufferedProgress();

    float getPlayProgress();

    void resetRenderSize(int left, int top, int width, int height);

    void onSurfaceCreated(ANativeWindow *window, int width, int height);

    void onSurfaceDestroyed();

    void signalOutputFrameAvailable();

    int getScreenWidth() {
        return 0;
    };

    //当音频播放器播放完毕一段buffer之后，会回调这个方法，这个方法要做的就是用数据将这个buffer再填充起来
    static int audioCallbackFillData(byte *buffer, size_t bufferSize, void *ctx);

protected:
    ANativeWindow *window;
    int screenWidth;
    int screenHeight;
    bool userCancelled;
    VideoOutput *videoOutput;
    AudioOutput *audioOutput;
    bool isPlaying;
    DecoderRequestHeader *requestHeader;
    float minBufferedDuration;
    float maxBufferedDuration;

    JavaVM *g_jvm;
    jobject obj;
    pthread_t initThreadThreadId;
    AVSynchronizer *synchronizer;

    static void *initThreadCallback(void *myself);

    bool startAVSynchronizer();

    bool initAVSynchronizer();

    bool initAudioOutput();

    int getAudioChannels();

    int consumeAudioFrames(byte *string, size_t i);

    void setInitializedStatus(bool initCode);

    void initVideoOutput(ANativeWindow *pWindow);

    static int videoCallbackGetTex(FrameTexture** frameTex, void* ctx, bool forceGetFrame);

    int getCorrectRenderTexture(FrameTexture **frameTex, bool forceGetFrame);
};


#endif //ANDROID_VIDEO_PLAYER_VIDEO_PLAYER_CONTROLLER_H
