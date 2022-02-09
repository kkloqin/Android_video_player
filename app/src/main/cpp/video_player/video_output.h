
#ifndef ANDROID_VIDEO_PLAYER_VIDEO_OUTPUT_H
#define ANDROID_VIDEO_PLAYER_VIDEO_OUTPUT_H

#include <unistd.h>
#include <android/native_window.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "opengl_media/render/video_gl_surface_render.h"
#include "./common/circle_texture_queue.h"
#include "egl_core/egl_core.h"
#include "CommonTools.h"
#include "message_queue/handler.h"
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

    bool createEGLContext(ANativeWindow* window);

    void destroyWindowSurface();

    void destroyEGLContext();

protected:

private:
    getTextureCallback  produceDataCallback;
    void* ctx;

    VideoOuputHandler* handler;
    MessageQueue* queue;
    ANativeWindow* surfaceWindow;

    int screenWidth;
    int screenHeight;
    VideoGLSurfaceRender* renderer;
    bool surfaceExists;
    bool isANativeWindowValid;
    bool forceGetFrame;
    EGLCore* eglCore;
    EGLSurface renderTexSurface;
    pthread_t _threadId;


    static void *threadStartCallback(void *myself);

    void processMessage();
};

class VideoOuputHandler: public Handler {
private:
    VideoOutput* videoOutput;
    bool initPlayerResourceFlag;
public:
    VideoOuputHandler(VideoOutput* videoOutput, MessageQueue* queue) :
            Handler(queue) {
        this->videoOutput = videoOutput;
        initPlayerResourceFlag = false;
    }

    void handleMessage(Message* msg) {
        int what = msg->getWhat();
        ANativeWindow* obj;
        switch (what) {
            case VIDEO_OUTPUT_MESSAGE_CREATE_EGL_CONTEXT:
                if (videoOutput->eglHasDestroyed){
                    break;
                }

                obj = (ANativeWindow*) (msg->getObj());
                initPlayerResourceFlag = videoOutput->createEGLContext(obj);
                break;
            case VIDEO_OUTPUT_MESSAGE_RENDER_FRAME:
                if (videoOutput->eglHasDestroyed) {
                    break;
                }

                if(initPlayerResourceFlag){
                    videoOutput->renderVideo();
                }
                break;
            case VIDEO_OUTPUT_MESSAGE_CREATE_WINDOW_SURFACE:
                if (videoOutput->eglHasDestroyed) {
                    break;
                }
                if(initPlayerResourceFlag){
                    obj = (ANativeWindow*) (msg->getObj());
                    videoOutput->createWindowSurface(obj);
                }
                break;
            case VIDEO_OUTPUT_MESSAGE_DESTROY_WINDOW_SURFACE:
                if(initPlayerResourceFlag){
                    videoOutput->destroyWindowSurface();
                }
                break;
            case VIDEO_OUTPUT_MESSAGE_DESTROY_EGL_CONTEXT:
                videoOutput->destroyEGLContext();
                break;
        }
    }
};

#endif //ANDROID_VIDEO_PLAYER_VIDEO_OUTPUT_H
