#ifndef FEATURE_H
#define FEATURE_H

#include <iostream>

/**
 * "Tags" for output properties
 */
class Feature
{
    public:
        std::string name;

        Feature(std::string name) :
            name(name)
        {
        }

        friend std::ostream & operator<<(std::ostream &strm, const Feature &f)
        {
            return strm << f.name;
        }
};

#endif
