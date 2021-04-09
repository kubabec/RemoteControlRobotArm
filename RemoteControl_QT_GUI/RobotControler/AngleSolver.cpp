#include "AngleSolver.h"
#include <cmath>
#define M_PI   3.14159265358979323846264338327950288
#define degToRad(angleInDegrees) ((angleInDegrees) * M_PI / 180.0)



void AngleSolver::getAnglesAlgorithm(int xparam, int yparam, int* ang1, int* ang2, int* ang3){
        int destinationPointX = xparam;
        int destinationPointY = yparam;
        int element1xBegin = 0;
        int element1yBegin = 0;
        // Arm elements length
        int element1Length = 8;
        int element2Length = 6;
        int element3Length = 15;

        // Algorithm config
        int element1minAngle = 25;
        int element1maxAngle = 166;
        int element2minAngle = 30;
        int element2maxAngle = 175;
        int element3minAngle = 30;
        int element3maxAngle = 175;

        // Precisionz
        int searchprecission = 1;
        double eps = 0.1;

        // X Max and Min range
        int minXrange = 4;
        int maxXrange = 33;

        double singlePointIncrease = (double)(element1maxAngle-element1minAngle)/(double)(maxXrange-minXrange);
        int element1Angle = (int)floor(element1maxAngle-(singlePointIncrease*destinationPointX));
        int beginAngleBackup = element1Angle;
        int element2Angle = element2minAngle;
        int element3Angle = element3minAngle;

        double x = element1Length * cos(degToRad(element1Angle));
        double y = element1Length * sin(degToRad(element1Angle));
        double element1xEnd = element1xBegin + x;
        double element1yEnd = element1yBegin + y;

        // PART 2
        double element2xBegin = element1xEnd;
        double element2yBegin = element1yEnd;
        x = element2Length * cos(degToRad(element2Angle));
        y = element2Length * sin(degToRad(element2Angle));
        double element2xEnd = element2xBegin + x;
        double element2yEnd = element2yBegin + y;

        // PART 3
        double element3xBegin = element2xEnd;
        double element3yBegin = element2yEnd;
        x = element3Length * cos(degToRad(element3Angle));
        y = element3Length * sin(degToRad(element3Angle));
        double element3xEnd = element3xBegin + x;
        double element3yEnd = element3yBegin + y;


        double DistanceToDestinationPoint= sqrt(pow(destinationPointX-element3xEnd,2) + pow(destinationPointY-element3yEnd,2) );
        bool succeed = true;
        int mode = 1;
        while(DistanceToDestinationPoint > eps){
            for(int i = element2maxAngle; i > element2minAngle; i = i - searchprecission){
                element2Angle = element1Angle - i;
                x = element2Length * cos(degToRad(element2Angle));
                y = element2Length * sin(degToRad(element2Angle));
                element2xEnd = element2xBegin + x;
                element2yEnd = element2yBegin + y;
                for(int j = element3minAngle; j < element3maxAngle; j = j + searchprecission){
                    element3Angle = element2Angle - j;
                    element3xBegin = element2xEnd;
                    element3yBegin = element2yEnd;
                    x = element3Length * cos(degToRad(element3Angle));
                    y = element3Length * sin(degToRad(element3Angle));
                    element3xEnd = element3xBegin + x;
                    element3yEnd = element3yBegin + y;
                    DistanceToDestinationPoint= sqrt(pow(destinationPointX-element3xEnd,2) + pow(destinationPointY-element3yEnd,2) );
                    if(DistanceToDestinationPoint < eps) break;
                }
                if(DistanceToDestinationPoint < eps ) break;
            }
            if(DistanceToDestinationPoint < eps ) break;
            else {
                if(mode == 1 ) element1Angle += searchprecission;
                else element1Angle -= searchprecission;

                if (element1Angle > element1maxAngle){
                    mode = 2;
                    element1Angle = beginAngleBackup;
                }

                if (element1Angle < element1minAngle){
                    succeed = false;
                    break;
                }
                x = element1Length * cos(degToRad(element1Angle));
                y = element1Length * sin(degToRad(element1Angle));
                element1xEnd = element1xBegin + x;
                element1yEnd = element1yBegin + y;
                element2xBegin = element1xEnd;
                element2yBegin = element1yEnd;
            }
        }

        if(element1Angle < element1minAngle || element1Angle > element1maxAngle) succeed = false;
        int element2Result = 180 - element1Angle + element2Angle;
        int element3Result = 180 - element2Angle + element3Angle;

        if(element2Result < element2minAngle || element2Result > element2maxAngle) succeed = false;
        if(element3Result < element3minAngle || element3Result > element3maxAngle) succeed = false;
        if(succeed){
            *ang1 = element1Angle;
            *ang2 = element2Result;
            *ang3 = element3Result;
        }else {
            *ang1 = -1;
            *ang2 = -1;
            *ang3 = -1;
        }
}


