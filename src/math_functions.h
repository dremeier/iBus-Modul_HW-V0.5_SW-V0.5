#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H

#include <math.h>

extern double distance;  // Deklaration als extern, um Linker-Fehler zu vermeiden
extern double angle;

double toRadians(double degree);
double calculateDistance(double lat1, double lon1, double lat2, double lon2);
double calculateAngle(double lat1, double lon1, double lat2, double lon2);

#endif