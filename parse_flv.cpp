
#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <memory>

std::string SoundRateName(uint8_t rate){
    std::string name = "";
    switch(rate){
        case 0:
        name = "5.5-kHz";
        break;
        case 1:
        name = "11-kHz";
        break;
        case 2:
        name = "22-kHz";
        break;
        case 3:
        name = "44-kHz";
        break;
    }
    return name;
}

std::string SoundFormatName(uint8_t format){
    std::string name = "";
    switch(format){
        case 0:
        name = "Linear PCM, platform endian";
        break;
        case 1:
        name = "ADPCM";
        break;
        case 2:
        name = "MP3";
        break;
        case 3:
        name = "Linear PCM, little endian";
        break;
        case 4:
        name = "Nellymoser 16kHz mono";
        break;
        case 5:
        name = "Nellymoser 8kHz mono";
        break;
        case 6:
        name = "Nellymoser";
        break;
        case 7:
        name = "G.711 A-law logarithmic PCM";
        break;
        case 8:
        name = "G.711 mu-law logarithmic PCM";
        break;
        case 9:
        name = "reserved";
        break;
        case 10:
        name = "AAC";
        break;
        case 11:
        name = "Speex";
        break;
        case 14:
        name = "MP3 8-KHz";
        break;
        case 15:
        name = "Device-specific sound";
        break;
    }//end switch
    return name;
}

std::string hexChar(uint8_t value){
    if(value <= 9){
        return std::to_string(value);
    }else{
        std::string str = "";
        switch (value)
        {
        case 10://A
            str = "A";
            break;
        case 11://B
            str = "B";
            break;
        case 12://C
            str = "C";
            break;
        case 13://D
            str = "D";
            break;
        case 14://E
            str = "E";
            break;
         case 15://F
            str = "F";
            break;
        default:
            break;
        }//end switch
        return str;
    }
}

std::string hexCharStr(uint8_t value){
    uint8_t low = 0x0f & value;
    uint8_t high = value >> 4;
    return hexChar(high)+hexChar(low);
}

void printCharArray(char *data , int size){
    const int lineWidth = 16;
    int index = 0;
    while(index < size){
        std::cout << hexCharStr(static_cast<uint8_t>(data[index])) << " ";
        index++;
        if(index % lineWidth == 0){
            std::cout << std::endl;
        }
    }//end while
}

const int TAG_TYPE_SCRIPT = 0x12;
const int TAG_TYPE_AUDIO = 0x8;
const int TAG_TYPE_VIDEO = 0x9;

struct FlvHeader{
    char signature[3];
    uint8_t version;
    uint8_t flags;
    uint32_t dataOffset;
};

struct TagHeader{
    uint32_t preTagSize;//4字节 前一个tag的尺寸
    uint8_t tagType;//1字节 标签类型 0x8 = 音频 , 0x9 = 视频 , 0x12 = 脚本数据
    uint32_t dataSize;//3字节  数据区域大小 不包含标签头
    uint32_t timestamp;//3字节 时间戳
    uint8_t timestampExtent;//1字节  时间戳扩展
    uint32_t streamId;//3 字节 保留，总是 0x00_00_00
};

class FlvParser{
public:
    FlvParser(std::string &path) : filepath(path){
    }

    ~FlvParser(){
    }

    void parseFile(){
        std::ifstream file(filepath , std::ios::binary);
        if(!file.is_open()){
            return;    
        }
        
        parseFlvHeader(file);
        parseFlvBody(file);
        file.close();
    }

    int parseFlvHeader(std::ifstream &file){
        const unsigned int HeadSize = 9;
        uint8_t headDataBuf[HeadSize];
        file.read(reinterpret_cast<char*>(headDataBuf),HeadSize);
        // printCharArray(reinterpret_cast<char*>(headDataBuf), HeadSize);

        std::cout << "signature : "
        << static_cast<char>( headDataBuf[0])
        << static_cast<char>( headDataBuf[1])
        << static_cast<char>( headDataBuf[2]);
        std::cout << std::endl;

        unsigned int version = headDataBuf[3];
        std::cout << "version : " << version << std::endl;

        uint8_t flag = headDataBuf[4];
        bool hasAudio = (((flag & 0x04) >> 2) == 1);
        bool hasVideo = (flag & 0x01 == 1);

        std::cout << "is Have Audio : " << hasAudio << std::endl;
        std::cout << "is Have Video : " << hasVideo << std::endl;

        uint32_t dataOffset = 
              (headDataBuf[5] << 24) 
            | (headDataBuf[6] << 16)  
            | (headDataBuf[7] << 8)
            | (headDataBuf[8]);

        std::cout << "Data Offset:" << dataOffset << std::endl;

        return 0;
    }

    int parseTag(std::ifstream &file){
        const unsigned int size = 15;
        uint8_t tagDataBuf[size];
        file.read(reinterpret_cast<char*>(tagDataBuf),size);
        if(file.eof()){
            return -2;
        }

        uint8_t tagType = tagDataBuf[4];
        uint32_t preTagSize = (tagDataBuf[0] << 24) 
            | (tagDataBuf[1] << 16)  
            | (tagDataBuf[2] << 8)
            | (tagDataBuf[3]);
       
        uint32_t tagDataSize = (tagDataBuf[5] << 16)  
            | (tagDataBuf[6] << 8)
            | (tagDataBuf[7]);
        
        uint32_t timestamp = (tagDataBuf[8] << 16)  
            | (tagDataBuf[9] << 8)
            | (tagDataBuf[10]);
        uint8_t timestampExtent = tagDataBuf[11];

        uint32_t streamId = (tagDataBuf[12] << 16)  
            | (tagDataBuf[13] << 8)
            | (tagDataBuf[14]);


        std::cout << "========= tag header =========" << std::endl;
        std::cout << "Tag Head file read count : " << file.gcount() << std::endl;

        std::cout << "preTagSize: " << preTagSize <<std::endl;
        if(tagType == TAG_TYPE_AUDIO){
            std::cout << "tagType: " << "Audio" <<std::endl;
            tagAudioCount++;
        }else if(tagType == TAG_TYPE_VIDEO){
            std::cout << "tagType: " << "Video" <<std::endl;
            tagVideoCount++;
        }else if(tagType == TAG_TYPE_SCRIPT){
            std::cout << "tagType: " << "Script" <<std::endl;
            tagScriptCount++;
        }else{
            std::cout << "tagType: " << (int)tagType <<std::endl;
        }
        std::cout << "tagDataSize: " << tagDataSize <<std::endl;
        std::cout << "timestamp: " << timestamp <<std::endl;
        std::cout << "streamId: " << streamId <<std::endl;

        uint8_t *buf = new uint8_t[tagDataSize];
        file.read(reinterpret_cast<char*>(buf),tagDataSize);


        std::cout << "Tag Data file read count : " << file.gcount() << std::endl;
        if(file.eof()){
            return -2;
        }

        switch (tagType)
        {
        case TAG_TYPE_AUDIO:
            parseAudioTagData(buf , tagDataSize);
            break;
        case TAG_TYPE_VIDEO:
            parseVideoTagData(buf , tagDataSize);
            break;
        case TAG_TYPE_SCRIPT:
            parseScriptTagData(buf , tagDataSize);
            break;
        default:
            break;
        }//end switch

        delete[] buf;
        buf = nullptr;
        return 0;
    }

    int parseVideoTagData(uint8_t *dataBuf , uint32_t bufSize){
        uint8_t field = dataBuf[0];
        std::cout << "video field: " << hexCharStr(field) << std::endl;

        // uint8_t soundFormat = (field & 0xF0) >> 4;

        return 0;
    }

    int parseAudioTagData(uint8_t *dataBuf , uint32_t bufSize){
        uint8_t field = dataBuf[0];
        std::cout << "audio field: " << hexCharStr(field) << std::endl;

        uint8_t soundFormat = (field & 0xf0) >> 4;
        uint8_t soundRate = (field & 0x0c) >> 2;
        uint8_t soundSize = (field & 0x02) >> 1;
        uint8_t soundType = (field & 0x01);

        std::cout << "Sound Format : " << SoundFormatName(soundFormat) << std::endl;
        std::cout << "Sound Rate : " << SoundRateName(soundRate) << std::endl;
        std::cout << "Sound Size : " << ((soundSize == 1)?"16bit":"8bit") << std::endl;
        std::cout << "Sound Type : " << ((soundType == 1)?"Stereo":"Mono") << std::endl;
        return 0;
    }

    int parseScriptTagData(uint8_t *dataBuf , uint32_t bufSize){
        return 0;
    }
    
    int parseFlvBody(std::ifstream &file){
        int readTagCount = 0;

        while(true){
            if(parseTag(file) < 0){
                readTagCount++;
                break;
            }
            readTagCount++;
        }

        
        std::cout << "VideoTag : " << tagVideoCount << std::endl;
        std::cout << "AudioTag : " << tagAudioCount << std::endl;
        std::cout << "ScriptTag : " << tagScriptCount << std::endl;
        std::cout << "TagCount : " << readTagCount << std::endl;
        return 0;
    }
private:
    std::string filepath;

    int tagVideoCount = 0;
    int tagAudioCount = 0;
    int tagScriptCount = 0;
};


int main(int argc , char **argv){
    if(argc != 2){
        return -1;
    }

    std::string path = argv[1];
    FlvParser flvParser(path);
    flvParser.parseFile();

    // for(int i = 0 ; i < 32;i++){
    //     std::cout << hexCharStr(static_cast<uint8_t>(i)) << " ";
    // }
    // std::cout << std::endl;
    return 0;
}

