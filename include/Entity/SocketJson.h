#pragma once  
#include <string>  
  
struct SocketJson {  
    // Gson@required
    int id;        
    // Gson@optional           
    int speechOffset;   
    // Gson@optional      
    int speechLen;       
    // Gson@optional     
    int ttsFlag;              
};
