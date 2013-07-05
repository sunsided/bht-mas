#include <string>
#include "Stats.h"

using namespace std;

ostream& operator<< (ostream& stream, const shared_ptr<Stats>& stats) {
    stream << *stats;
    return stream;
}

ostream& operator<< (ostream& stream, const Stats& stats) {
    stream << "Range " << to_string(stats.min) << " .. " << to_string(stats.max) << ", mean " << to_string(stats.mean) << " +/- " << to_string(stats.standard_deviation); 
    return stream;
}