//
// Created by shadowiterator on 18-10-27.
//

#include "sistring.h"

int sistrcmp(char* sa, char* sb, int s1, int s2, int len)
{
    if((!sa) || (!sb)) return -2;
    for(int i = 0; i < len; ++i)
    {
        if(sa[i + s1] < sb[i + s2])
            return -1;
        if(sa[i + s1] > sb[i + s2])
            return 1;
        if(sa[i + s1] == '\0')
            break;
    }
    return 0;
}