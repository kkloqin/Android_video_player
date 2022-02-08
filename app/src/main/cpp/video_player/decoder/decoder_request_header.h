
#ifndef ANDROID_VIDEO_PLAYER_DECODER_REQUEST_HEADER_H
#define ANDROID_VIDEO_PLAYER_DECODER_REQUEST_HEADER_H

#include <map>
#include "CommonTools.h"

class DecoderRequestHeader {
private:
    char *uri;
    int *maxAnalyzeDurations;
    int analyzeCnt;
    int probeSize;
    bool fpsProbeSizeConfigured;
    std::map<std::string, void *> *extraData;

public:
    DecoderRequestHeader();
    ~DecoderRequestHeader();

    void init(char *uriParams) {
        int length = strlen(uriParams);
        uri = new char[length + 1];
        memcpy(uri, uriParams, sizeof(char) * (length + 1));
        extraData = new std::map<std::string, void *>();
        fpsProbeSizeConfigured = false;
    };

    void init(char *uriParams, int *max_analyze_duration, int analyzeCnt, int probeSize,
              bool fpsProbeSizeConfigured) {
        int length = strlen(uriParams);
        uri = new char[length + 1];
        memcpy(uri, uriParams, sizeof(char) * (length + 1));
        extraData = new std::map<std::string, void *>();
        maxAnalyzeDurations = new int[analyzeCnt];
        memcmp(maxAnalyzeDurations, max_analyze_duration, sizeof(int) * analyzeCnt);
        this->analyzeCnt = analyzeCnt;
        this->probeSize = probeSize;
        this->fpsProbeSizeConfigured = fpsProbeSizeConfigured;
    };

    void destroy() {
        if (uri != NULL) {
            delete[] uri;
            uri = NULL;
        }
        if (maxAnalyzeDurations != NULL) {
            delete[] maxAnalyzeDurations;
            maxAnalyzeDurations = NULL;
        }
        if (extraData != NULL) {
            std::map<std::string, void *>::iterator itr = extraData->begin();
            for (; itr != extraData->end(); ++itr) {
                void *value = itr->second;
                delete value;
            }
            extraData->clear();
            delete extraData;
        }
    }

    char *getURI() {
        return uri;
    };

    int *getMaxAnalyzeDurations() {
        return maxAnalyzeDurations;
    };

    int getAnalyzeCnt() {
        return analyzeCnt;
    };

    int getProbeSize() {
        return probeSize;
    };

    bool getFPSProbeSizeConfigured() {
        return fpsProbeSizeConfigured;
    };

    void put(std::string key, void *value) {
        extraData->insert(std::pair<std::string, void *>(key, value));
    }

    void *get(std::string key) {
        std::map<std::string, void *>::iterator itr = extraData->find(key);
        if (itr != extraData->end()) {
            return itr->second;
        }
        return NULL;
    }
};


#endif //ANDROID_VIDEO_PLAYER_DECODER_REQUEST_HEADER_H
