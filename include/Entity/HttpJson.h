#pragma once  
#include <string>  
  
struct HttpJson {  
    // Gson@required
    int id;     
    // Gson@optional              
    int speechOffset;
    // Gson@optional         
    int speechLen;    
    // Gson@optional        
    int asrFlag;           
    // Gson@optional   
    std::string fileName;  
    // Gson@optional
    int asrSpeechPackStatus;   
    // Gson@optional
    int curSendPacketNum;
    // Gson@optional
    int totalSendPacketNum;                
};
