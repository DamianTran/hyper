#pragma once

#ifndef HYPER_TIME
#define HYPER_TIME

#include <chrono>
#include <ctime>
#include <string>
#include <vector>
#include <iomanip>

#include <stdexcept>
#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>

#define YEARS(N)        hyperC::TimePoint( N, 0, 0, 0, 0, 0 )
#define MONTHS(N)       hyperC::TimePoint( 0, N, 0, 0, 0, 0 )
#define DAYS(N)         hyperC::TimePoint( 0, 0, N, 0, 0, 0 )
#define HOURS(N)        hyperC::TimePoint( 0, 0, 0, N, 0, 0 )
#define MINUTES(N)      hyperC::TimePoint( 0, 0, 0, 0, N, 0 )
#define SECONDS(N)      hyperC::TimePoint( 0, 0, 0, 0, 0, N )

#define EPOCH_TIME      hyperC::TimePoint(2018, 9, 16, 0, 0, 0)

#define TIME_NOW                        std::chrono::high_resolution_clock::now()
#define CLOCK_NOW                       std::chrono::system_clock::to_time_t(TIME_NOW)
#define TIME_EPOCH                      std::chrono::duration_cast<std::chrono::seconds>(TIME_NOW.time_since_epoch()).count()

#define DURATION(t0, t1)                std::chrono::duration<float>(t1 - t0).count()

namespace hyperC{

// Time scales used for calendar and other time-tracking apps

inline std::string getTimeString(const char* format,
                                 const time_t& time)
{
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), format);
    return ss.str();
}

enum time_scale{
    any,
    millenium,
    century,
    decade,
    year,
    month,
    week,
    day,
    hour,
    minute,
    second
};

const static std::vector<std::string> calendar_days = {
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday"
};

const static std::vector<std::string> calendar_months = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

inline unsigned int days_in_month(const unsigned int& month, const bool& leap_year = false){
    switch(month){
        case 1:{
            return 31;
        }
        case 2:{
            if(leap_year) return 29;
            return 28;
        }
        case 3:{
            return 31;
        }
        case 4:{
            return 30;
        }
        case 5:{
            return 31;
        }
        case 6:{
            return 30;
        }
        case 7:{
            return 31;
        }
        case 8:{
            return 31;
        }
        case 9:{
            return 30;
        }
        case 10:{
            return 31;
        }
        case 11:{
            return 30;
        }
        case 12:{
            return 31;
        }
        default:{
            throw std::invalid_argument("In days_in_month: integer does not correspond to month");
        }
    }
}

int days_between_months(const int& start_month,
                           const int& end_month,
                           const bool& leap_year = false);
std::string monthStr(const unsigned int& monthInt);
int monthInt(const std::string& month);

std::string num_to_time(unsigned int hour,
                        unsigned int minute = 0,
                        unsigned int second = 0);

std::string getDurationStr(const float& system_count);
inline std::string getDurationStr(const std::chrono::duration<float>& duration)
{
    return getDurationStr(duration.count());
}

class TimePoint{
protected:
    std::vector<int> timeInfo;
    std::string annotation;

    friend class CVCalendarPanel;
    friend class CVCalendarRecord;

public:

    inline const int& year() const{ return timeInfo[0]; }
    inline const int& month() const{ return timeInfo[1]; }
    inline const int& day() const{ return timeInfo[2]; }
    inline const int& hour() const{ return timeInfo[3]; }
    inline const int& minute() const{ return timeInfo[4]; }
    inline const int& second() const{ return timeInfo[5]; }

    inline int& year(){ return timeInfo[0]; }
    inline int& month(){ return timeInfo[1]; }
    inline int& day(){ return timeInfo[2]; }
    inline int& hour(){ return timeInfo[3]; }
    inline int& minute(){ return timeInfo[4]; }
    inline int& second(){ return timeInfo[5]; }

    inline void setYear(const int& newValue){ timeInfo[0] = newValue; }
    inline void setMonth(const int& newValue){ timeInfo[1] = newValue; }
    inline void setDay(const int& newValue){ timeInfo[2] = newValue; }
    inline void setHour(const int& newValue){ timeInfo[3] = newValue; }
    inline void setMinute(const int& newValue){ timeInfo[4] = newValue; }
    inline void setSecond(const int& newValue){ timeInfo[5] = newValue; }

    int total_months() const;
    int total_days() const;
    int64_t total_hours() const;
    int64_t total_minutes() const;
    int64_t total_seconds() const;

    void clear();
    bool empty() const;

    inline const int* data() const{ return timeInfo.data(); }

    void setFromEpoch(const int& timeCount);    // Set using integer based on TIME_EPOCH reference

    inline int& operator[](const unsigned int& index){ return timeInfo[index]; }
    friend std::ostream& operator<<(std::ostream& output, const TimePoint& T);
    friend TimePoint abs(const TimePoint& other);

    // Operator overloads

    bool operator==(const TimePoint& other) const;
    bool operator!=(const TimePoint& other) const;

    bool operator<(const TimePoint& other) const;
    bool operator<=(const TimePoint& other) const;
    bool operator>(const TimePoint& other) const;
    bool operator>=(const TimePoint& other) const;

    TimePoint operator+(const TimePoint& other) const;
    TimePoint operator-(const TimePoint& other) const;
    TimePoint operator-() const;

    TimePoint& operator=(const std::string& str);
    void parse(const std::string& str);

    TimePoint& operator=(const int& seconds);

    void operator+=(const TimePoint& other);
    void operator-=(const TimePoint& other);

    friend void advance_day(TimePoint& time, const std::string& day);

    std::string getDate() const;
    std::string getTime(bool _24hr = false, bool seconds = false) const;
    std::string getDurationStr() const;

    void getTimeNow();

    friend void fread(TimePoint& timePoint, FILE* inFILE);
    inline void read(TimePoint& timePoint, FILE* inFILE)
    {
        fread(timePoint, inFILE);
    }

    friend void fwrite(const TimePoint& timePoint, FILE* outFILE);
    inline void write(const TimePoint& timePoint, FILE* outFILE)
    {
        fwrite(timePoint, outFILE);
    }

    int& getIndex(const std::string& text);

    void distribute(); // Normalize time categories to metric maxima

    TimePoint();
    TimePoint(const int* timeInfo);
    TimePoint(const std::vector<int>& timeInfo);
    TimePoint(const int& seconds);
    TimePoint(const int& years,
              const int& months,
              const int& days,
              const int& hours,
              const int& minutes,
              const int& seconds);
    TimePoint(const std::string& text);

    inline void setTag(const std::string& tag){
        annotation = tag;
    }
    inline const std::string& getTag() const{ return annotation; }

    ~TimePoint() = default;
};

std::string day_of_week(const TimePoint& time);
int day_int(const std::string& day);

inline std::string current_time_str(){
    TimePoint t;
    t.getTimeNow();
    return t.getTime();
}

struct TimeLog : public std::vector<TimePoint>{

    inline void addTimePoint(const unsigned int& index){ insert(begin() + index, TimePoint()); }
    inline void removeTimePoint(const unsigned int& index){ erase(begin() + index); }
    inline void addTimePoint(){ emplace_back(); }
    inline void removeTimePoint(){ pop_back(); }

};

}

#endif // HYPER_TIME
