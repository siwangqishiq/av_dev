

#include <iostream>
#include <stdint.h>
#include <string>

// #define SDL_MAIN_HANDLED 

extern "C"
{
#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
#include "libavdevice/avdevice.h"
}

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

std::string GetCodecTypeName(int type){
    std::string result = "UNKNOWN";
    switch(type){
        case AVMEDIA_TYPE_VIDEO:
        result = "VIDEO";
        break;
        case AVMEDIA_TYPE_AUDIO:
        result = "AUDIO";
        break;
        case AVMEDIA_TYPE_DATA:
        result = "DATA";
        break;
        case AVMEDIA_TYPE_SUBTITLE:
        result = "SUBTITLE";
        break;
        case AVMEDIA_TYPE_ATTACHMENT:
        result = "ATTACHMENT";
        break;
    }//end switch
    return result;
}


void test1(){
    std::cout << "ffmpeg version:" << av_version_info() << std::endl;

    avdevice_register_all();
    std::cout << "avdevice_register_all ended" << std::endl;

    AVPacket *pck = av_packet_alloc();
    av_packet_ref(pck , pck);
    av_packet_free(&pck);
}

int testReadFile(int argc , char **argv){
    if(argc < 2){
        std::cerr << "need input a file url!" << std::endl;
        return -1;
    }
    std::string url = argv[1];
    std::cout << "file url:" << url << std::endl;

    AVFormatContext *formatContext = nullptr;
    int errCode = avformat_open_input(&formatContext , url.c_str(), nullptr, nullptr);
    if(errCode < 0){
        std::cerr << "open input error " << errCode << std::endl;
        return -1;
    }
    std::cout << "open success" << std::endl;
    int streamCount = formatContext->nb_streams;
    std::cout << "streamCount = " << streamCount << std::endl;

    AVDictionary *metaData = formatContext->metadata;
    std::cout << "metadata:" << std::endl;
    AVDictionaryEntry* entry = nullptr;
    while ((entry = av_dict_get(metaData, "", entry, AV_DICT_IGNORE_SUFFIX))) {
        std::cout << entry->key << ": " << entry->value << std::endl;
    }//end while

    int videoIndex = -1;
    int audioIndex = -1;

    for(int i = 0 ; i < streamCount;i++){
        AVStream *stream = formatContext->streams[i];
        std::cout << "==============STREAM#"<< i <<"=====================" << std::endl;
        std::cout << "codec type :" << GetCodecTypeName(stream->codecpar->codec_type) << std::endl;
        std::cout << "stream index : " << stream->index << std::endl;
        std::cout << "nb_frames : " << stream->nb_frames << std::endl;
        std::cout << "codec name:" << avcodec_get_name(stream->codecpar->codec_id) << std::endl;
        double duration = stream->duration * av_q2d(stream->time_base);
        std::cout << "duration (s) : " << duration << std::endl;
        std::cout << "start time : " << stream->start_time * av_q2d(stream->time_base) << std::endl;
        std::cout << std::endl;

        int codecType = stream->codecpar->codec_type; 
        if(codecType == AVMediaType::AVMEDIA_TYPE_AUDIO){
            audioIndex = stream->index;
        }else if(codecType == AVMediaType::AVMEDIA_TYPE_VIDEO){
            videoIndex = stream->index;
        }
    }//end for i

    AVStream *videoSteam = nullptr;
    if(videoIndex < 0){
        std::cout << "Not Found Video Steam" << std::endl;
    }else{
        videoSteam = formatContext->streams[videoIndex];
    }

    AVStream *audioStream = nullptr;
    if(audioIndex < 0){
        std::cout << "Not Found Audio Steam" << std::endl;
    }else{
        audioStream = formatContext->streams[audioIndex];
    }

    if(videoSteam != nullptr){
        
    }

    if(audioStream != nullptr){

    }
    
    return 0;
}

int main(int argc , char **argv){
    return testReadFile(argc , argv);
}
