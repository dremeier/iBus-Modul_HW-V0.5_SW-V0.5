#include "math_functions.h"

const double EARTH_RADIUS = 6371000.0; // Radius der Erde in Metern (ca. 6.371 Kilometer)

double distance;  // Definition der globalen Variablen
double angle;

double toRadians(double degree) {
    return degree * (M_PI / 180.0);
}

double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    double dLat = toRadians(lat2 - lat1);
    double dLon = toRadians(lon2 - lon1);
    double a = sin(dLat / 2) * sin(dLat / 2) + cos(toRadians(lat1)) * cos(toRadians(lat2)) * sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    double distance =  EARTH_RADIUS * c; // Radius der Erde in Metern
    return distance;
}

double calculateAngle(double lat1, double lon1, double lat2, double lon2) {
    double dLon = toRadians(lon2 - lon1);
    double x = sin(dLon) * cos(toRadians(lat2));
    double y = cos(toRadians(lat1)) * sin(toRadians(lat2)) - sin(toRadians(lat1)) * cos(toRadians(lat2)) * cos(dLon);
    double angle = atan2(x, y) * (180.0 / M_PI);
    if (angle < 0) {
        angle += 360.0;
    }
    return angle;
}