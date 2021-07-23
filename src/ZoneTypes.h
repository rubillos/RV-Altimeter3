#ifndef _ZoneTypes_
#define _ZoneTypes_

typedef struct {
    float lat;
    float lon;
} FloatPt;

typedef struct {
    const char* name;
    float zoneOffset;
    int numPts;
    float latMin;
    float latMax;
    float lonMin;
    float lonMax;
    const FloatPt* pts;
} ZoneRec;

#endif
