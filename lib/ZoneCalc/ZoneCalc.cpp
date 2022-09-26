#include "ZoneCalc.h"

// Timezone boundaries derived from:
// // http://ontimezone.com

#include "ZoneSummer.h"
#include "ZoneWinter.h"

int ZoneCalc::orientation(FloatPt p, FloatPt q, FloatPt r) {
	float val = (q.lat - p.lat) * (r.lon - q.lon) -
			(q.lon - p.lon) * (r.lat - q.lat);

	if (val == 0) return 0; // colinear
	return (val > 0)? 1: 2; // clock or counterclock wise
}

bool ZoneCalc::doIntersect(FloatPt p1, FloatPt q1, FloatPt p2, FloatPt q2) {
	// Find the four orientations needed for general case
	int o1 = orientation(p1, q1, p2);
	int o2 = orientation(p1, q1, q2);
	int o3 = orientation(p2, q2, p1);
	int o4 = orientation(p2, q2, q1);

	// General case
	if (o1 != o2 && o3 != o4)
		return true;
    else
        return false;
}

#define lonInf 1000000

bool ZoneCalc::isInside(const FloatPt p, const FloatPt polygon[], int numPts) {
	// Create a point for line segment from p to infinite
	FloatPt extreme = {p.lat, lonInf};

	// Count intersections of the above line with sides of polygon
	int count = 0;
	
	for (int i=0; i<numPts-1; i++) {
		// Check if the line segment from 'p' to 'extreme' intersects
		// with the line segment from 'polygon[i]' to 'polygon[next]'
		if (doIntersect(polygon[i], polygon[i+1], p, extreme)) {
			count++;
		}
	}

	// Return true if count is odd, false otherwise
	return count&1;
}

float ZoneCalc::zoneOffsetForGPSCoord(float lat, float lon, bool summer) {
	const ZoneRec** zones = (summer) ? zonesSummer : zonesWinter;
	int zoneCount = (summer) ? SummerZoneCount : WinterZoneCount;
	FloatPt p = { lat, lon };

	for (int i=0; i<zoneCount; i++) {
		const ZoneRec* zone = zones[i];

        if (lat>=zone->latMin && lat<=zone->latMax && lon>=zone->lonMin && lon<=zone->lonMax) {
            if (isInside(p, zone->pts, zone->numPts)) {
                return zone->zoneOffset;
            }
        }
	}

	return 0;
}

TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -4 * 60};
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -5 * 60};
Timezone tzEastern(usEDT, usEST);

TimeChangeRule usCDT = {"CDT", Second, Sun, Mar, 2, -5 * 60};
TimeChangeRule usCST = {"CST", First, Sun, Nov, 2, -6 * 60};
Timezone tzCentral(usCDT, usCST);

TimeChangeRule usMDT = {"MDT", Second, Sun, Mar, 2, -6 * 60};
TimeChangeRule usMST = {"MST", First, Sun, Nov, 2, -7 * 60};
Timezone tzMountain(usMDT, usMST);

TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -7 * 60};
TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -8 * 60};
Timezone tzPacific(usPDT, usPST);

TimeChangeRule usADT = {"ADT", Second, Sun, Mar, 2, -8 * 60};
TimeChangeRule usAST = {"AST", First, Sun, Nov, 2, -9 * 60};
Timezone tzAlaska(usADT, usAST);

Timezone* timezoneForOffset(int16_t zoneOffset) {
    if (zoneOffset == -9) { return &tzAlaska; }
    else if (zoneOffset == -8) { return &tzPacific; }
    else if (zoneOffset == -7) { return &tzMountain; }
    else if (zoneOffset == -6) { return &tzCentral; }
    else return &tzEastern;
}

bool ZoneCalc::dateIsDST(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, int16_t zoneOffset) {
    Timezone* timezone = timezoneForOffset(zoneOffset);
    TimeElements timeParts = { 0, minute, hour, 0, day, month, (uint8_t)(year - 1970) };

    return timezone->utcIsDST(makeTime(timeParts));
}

