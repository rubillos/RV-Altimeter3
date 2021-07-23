#include "ZoneTypes.h"
#include "ZoneSummer.h"
#include "ZoneWinter.h"

int orientation(FloatPt p, FloatPt q, FloatPt r) {
	float val = (q.lat - p.lat) * (r.lon - q.lon) -
			(q.lon - p.lon) * (r.lat - q.lat);

	if (val == 0) return 0; // colinear
	return (val > 0)? 1: 2; // clock or counterclock wise
}

bool doIntersect(FloatPt p1, FloatPt q1, FloatPt p2, FloatPt q2) {
	// Find the four orientations needed for general and
	// special cases
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

bool isInside(const FloatPt p, const FloatPt polygon[], int numPts) {
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

float zoneOffsetForGPSCoord(float lat, float lon, bool summer) {
    // elapsedMillis duration;
	const ZoneRec** zones = (summer) ? zonesSummer : zonesWinter;
	int zoneCount = (summer) ? SummerZoneCount : WinterZoneCount;
	FloatPt p = { lat, lon };

	for (int i=0; i<zoneCount; i++) {
		const ZoneRec* zone = zones[i];

        if (lat>=zone->latMin && lat<=zone->latMax && lon>=zone->lonMin && lon<=zone->lonMax) {
            if (isInside(p, zone->pts, zone->numPts)) {
                // Debug_print(duration);
                // Debug_println("ms");
                return zone->zoneOffset;
            }
        }
	}

	return 0;
}

void printPolyMinMax(const FloatPt poly[], int numPts) {
	float latMin = 90;
	float latMax = 0;
	float lonMin = 180;
	float lonMax = -180;

	for (int i=0; i<numPts; i++) {
		if (poly[i].lat < latMin) {
			latMin = poly[i].lat;
		}
		if (poly[i].lat > latMax) {
			latMax = poly[i].lat;
		}
		if (poly[i].lon < lonMin) {
			lonMin = poly[i].lon;
		}
		if (poly[i].lon > lonMax) {
			lonMax = poly[i].lon;
		}
	}

	Serial.print(latMin);
	Serial.print(", ");
	Serial.print(latMax);
	Serial.println(", ");
	Serial.print(lonMin);
	Serial.print(", ");
	Serial.print(lonMax);
	Serial.println(", ");
	Serial.println();
}

void zonesPrintMinMax() {
	Serial.println("Winter");
    Serial.println();
	for (int i=0; i<(int)WinterZoneCount; i++) {
        Serial.println(zonesWinter[i]->name);
		printPolyMinMax(zonesWinter[i]->pts, zonesWinter[i]->numPts);
	}

	Serial.println("Summer");
    Serial.println();
	for (int i=0; i<(int)SummerZoneCount; i++) {
        Serial.println(zonesSummer[i]->name);
		printPolyMinMax(zonesSummer[i]->pts, zonesSummer[i]->numPts);
	}
}
