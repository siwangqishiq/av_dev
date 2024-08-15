
#include <iostream>
#include <stdint.h>
#include <string>

extern "C"
{
#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
#include "libavdevice/avdevice.h"
}

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

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

struct MaParams{
    AVCodecContext *codecContext;
    AVFormatContext *formatContext;
    AVPacket *packet;
    AVFrame *frame;
    int audioIndex;
};

static void PlayPcmCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount){
    std::cout << "data callback " << frameCount << std::endl;

    MaParams *params = static_cast<MaParams *>(pDevice->pUserData);

    auto formatContext = params->formatContext;
    auto audioIndex = params->audioIndex;
    auto codecContext = params->codecContext;
    auto packet = params->packet;
    auto frame = params->frame;

    while(av_read_frame(formatContext , packet) >= 0){
        if(packet->stream_index != audioIndex){
            continue;
        }
        
        int ret = avcodec_send_packet(codecContext , packet);
        if(ret < 0){
            std::cerr << "audio decode error!" << std::endl;
        }

        while(ret >= 0){
            ret = avcodec_receive_frame(codecContext , frame);
            if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
                break;
            }else if(ret < 0){
                break;
            }

            int pcmDataSize = 
                av_get_bytes_per_sample(codecContext->sample_fmt) 
                * frame->nb_samples 
                * codecContext->ch_layout.nb_channels;
            std::cout << "read pcm data size : " << pcmDataSize << std::endl;

            int readPcmFileCount = frameCount * pDevice->playback.channels * ma_get_bytes_per_sample(pDevice->playback.format);
            std::cout << "readPcmFileCount = " << readPcmFileCount <<" frameCount :" << frameCount << std::endl;
            memcpy(pOutput, (uint8_t *)frame->data[0], frameCount * pDevice->playback.channels * ma_get_bytes_per_sample(pDevice->playback.format));
            // output_file.write((char*)frame->data[0], data_size);
        }//end while
        av_packet_unref(packet);
    }//end while
}


int main(int argc , char **argv){

     if(argc < 2){
        std::cerr << "need input a file url!" << std::endl;
        return -1;
    }
    std::string url = argv[1];
    std::cout << "file url:" << url << std::endl;

    avdevice_register_all();

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

    int audioIndex = -1;
    for(int i = 0 ; i < streamCount ;i++){
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
        }
    }//end for i

    if(audioIndex < 0){
        std::cerr << "Not found audio stream" << std::endl;
        avformat_close_input(&formatContext);
        return -1;
    }
        
    AVStream *audioStream = formatContext->streams[audioIndex];
    if(audioStream == nullptr){
        std::cerr << "Not found audio stream" << std::endl;
        avformat_close_input(&formatContext);
        return -1;
    }
        
    std::cout << "use audio steam" << std::endl;

    int ret = -1;
    AVCodecParameters *codecParams = audioStream->codecpar;
    std::cout << "audio codec :" << avcodec_get_name(codecParams->codec_id) << std::endl;
    const AVCodec *audioCodec = avcodec_find_decoder(codecParams->codec_id);
    if(audioCodec == nullptr){
        std::cerr << "Not found audio codec " << codecParams->codec_id << std::endl;
        avformat_close_input(&formatContext);
        return -1;
    }

    AVCodecContext *codecContext = avcodec_alloc_context3(audioCodec);

    if(codecContext == nullptr){
        std::cerr << "Audio Condec Context alloc failed!" << std::endl;
        return -1;
    }
    ret = avcodec_parameters_to_context(codecContext , codecParams);
    if(ret < 0){
        std::cerr << "Audio Codec Context Set params error!" << std::endl;
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        return -1;
    }

    ret = avcodec_open2(codecContext, audioCodec, nullptr);
    if(ret < 0){
        std::cerr << "Audio Codec Context Open Codec Failed!" << std::endl;
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        return -1;
    }

    std::cout << "codec name : " << audioCodec->long_name << std::endl;
    std::cout << "audio sampleFormat : " << av_get_sample_fmt_name(codecContext->sample_fmt) << std::endl;
    std::cout << "audio sample_rate : " << codecParams->sample_rate << std::endl;
    std::cout << "audio channels : " << codecParams->ch_layout.nb_channels << std::endl;

    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    MaParams *params = new MaParams();
    params->audioIndex = audioIndex;
    params->codecContext = codecContext;
    params->formatContext = formatContext;
    params->frame = frame;
    params->packet = packet;

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = codecParams->ch_layout.nb_channels;
    deviceConfig.sampleRate = codecParams->sample_rate;
    deviceConfig.pUserData = params;
    deviceConfig.dataCallback = PlayPcmCallback;

    ma_device playDevice;
    if(ma_device_init(nullptr, &deviceConfig, &playDevice) != MB_OK){
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        std::cerr << "Play device init Failed!" << std::endl;
        return -1;
    }

    if(ma_device_start(&playDevice)!= MB_OK){
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        std::cerr << "Play device start Failed!" << std::endl;
        return -1;
    }

    int input;
    std::cin>> input;


    // while(av_read_frame(formatContext , packet) >= 0){
    //     if(packet->stream_index != audioIndex){
    //         continue;
    //     }
        
    //     int ret = avcodec_send_packet(codecContext , packet);
    //     if(ret < 0){
    //         std::cerr << "audio decode error!" << std::endl;
    //     }

    //     while(ret >= 0){
    //         ret = avcodec_receive_frame(codecContext , frame);
    //         if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
    //             break;
    //         }else if(ret < 0){
    //             break;
    //         }

    //         int pcmDataSize = 
    //             av_get_bytes_per_sample(codecContext->sample_fmt) 
    //             * frame->nb_samples 
    //             * codecContext->ch_layout.nb_channels;
    //         std::cout << "read pcm data size : " << pcmDataSize << std::endl;
    //         // output_file.write((char*)frame->data[0], data_size);
    //     }//end while
    //     av_packet_unref(packet);
    // }//end while
end:
    delete params;
    params = nullptr;

    ma_device_uninit(&playDevice);
    av_frame_free(&frame);        
    av_packet_free(&packet);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return 0;
}