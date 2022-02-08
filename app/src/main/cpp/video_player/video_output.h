
#ifndef ANDROID_VIDEO_PLAYER_VIDEO_OUTPUT_H
#define ANDROID_VIDEO_PLAYER_VIDEO_OUTPUT_H

#include <android/native_window.h>
#include "./common/circle_texture_queue.h"
#include "message_queue/message_queue.h"



typedef enum {
    VIDEO_OUTPUT_MESSAGE_CREATE_EGL_CONTEXT,
    VIDEO_OUTPUT_MESSAGE_CREATE_WINDOW_SURFACE,
    VIDEO_OUTPUT_MESSAGE_DESTROY_WINDOW_SURFACE,
    VIDEO_OUTPUT_MESSAGE_DESTROY_EGL_CONTEXT,
    VIDEO_OUTPUT_MESSAGE_RENDER_FRAME
} VideoOutputMSGType;

typedef int (*getTextureCallback)(FrameTexture** texture, void* ctx, bool forceGetFrame);
typedef int (*onPreDestroyCallback)(void* ctx);

class VideoOuputHandler;

class VideoOutput {
public:
    VideoOutput();
    virtual ~VideoOutput();

    bool initOutput(ANativeWindow* window, int screenWidth, int screenHeight, getTextureCallback produceDataCallback,
                    void* ctx);

    void signalFrameAvailable();

    void resetRenderSize(int left, int top, int width, int height);

    void onSurfaceCreated(ANativeWindow* window);

    void onSurfaceDestroyed();

    void stopOutput();

    bool createEglContext(ANativeWindow* window);

    void createWindowSurface(ANativeWindow* window);

    bool renderVideo();

    void DestroyEglContext();

    int getScreenWidth() {
        return screenWidth;
    }

    int getScreenHeight() {
        return screenHeight;
    }

    bool eglHasDestroyed;

protected:

private:
    getTextureCallback  produceDataCallback;
    void* ctx;

    VideoOuputHandler* handler;
    MessageQueue* queue;
    ANativeWindow* surfaceWindow;

    int screenWidth;
    int screenHeight;












};


#endif //ANDROID_VIDEO_PLAYER_VIDEO_OUTPUT_H
