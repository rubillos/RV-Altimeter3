#ifndef _ZONE_CALC_
#define _ZONE_CALC_

#include "Timezone.h"

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

class ZoneCalc {
    public:
		float zoneOffsetForGPSCoord(float lat, float lon, bool summer);
		bool dateIsDST(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, int16_t zoneOffset);

    private:
		int orientation(FloatPt p, FloatPt q, FloatPt r);
		bool doIntersect(FloatPt p1, FloatPt q1, FloatPt p2, FloatPt q2);
		bool isInside(const FloatPt p, const FloatPt polygon[], int numPts);
};

#endif
