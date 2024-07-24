

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

int main(){
    std::cout << "ffmpeg version:" << av_version_info() << std::endl;

    avdevice_register_all();
    std::cout << "avdevice_register_all ended" << std::endl;

    AVPacket *pck = av_packet_alloc();
    av_packet_ref(pck , pck);
    av_packet_free(&pck);
    
    return 0;
}
