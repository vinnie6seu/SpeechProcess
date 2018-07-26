#include "Util/DateUtil.hpp"

string DateUtil::parseTimestampToStr(int timestamp, string& timeFromat) {
    int unixTime = timestamp;
    time_t tick = (time_t)unixTime;

    struct tm tm;
    char timetr[100];
    tm = *localtime(&tick);

    strftime(timetr, sizeof(timetr), timeFromat.c_str(), &tm);

    return timetr;
}

int DateUtil::getCurTimestamp() {
    time_t now;
    int unixTime = (int) time(&now);
    return unixTime;
}

string DateUtil::getCurTimeFromat(string& timeFromat) {
    return parseTimestampToStr(getCurTimestamp(), timeFromat);
}

/************************************************************************/
// g++ -DDEBUG_TEST DateUtil.cpp -I/home/vinnie/my_project/SpeechProcess/include/

#ifdef DEBUG_TEST

#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
    string timeFromat = "%Y%m%d";
    cout << DateUtil::getCurTimeFromat(timeFromat) << endl;
    return 0;
}

#endif
