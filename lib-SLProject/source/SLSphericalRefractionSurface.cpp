//#############################################################################
//  File:      SLRectangle.cpp
//  Author:    Philipp Jüni
//  Date:      Mai 2015
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: 2002-2014 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>           // precompiled headers
#ifdef SL_MEMLEAKDETECT       // set in SL.h for debug config only
#include <debug_new.h>        // memory leak detector
#endif


#include "stdafx.h"
#include "SLSphericalRefractionSurface.h"

SLSphericalRefractionSurface::SLSphericalRefractionSurface(SLfloat diameter,
    SLfloat radius,
    SLint stacks,
    SLint slices,
    SLstring name) : SLRevolver(name)
{
    _diameter = diameter;
    
    _stacks = stacks;
    _slices = slices;

    _smoothFirst = true;
    _smoothLast = true;
    _revAxis.set(0, 1, 0);

    SLfloat sagitta = calcSagitta(radius);    
    if (radius > 0)
    {
        generateLensSurface(radius, 0, 0, radius, -SL_HALFPI);
    }
    else
    {
        generateLensSurface(radius, 0, 0, radius, SL_HALFPI);
    }
}

SLVec3f SLSphericalRefractionSurface::getPoint(SLfloat radius, SLfloat phi)
{
    
    
    SLVec3f correspondingPoint(1,1,23);

    return correspondingPoint;
}

SLVec3f SLSphericalRefractionSurface::getRandomPoint()
{
    //SLfloat max = (_diameter / 2) - (_diameter * 0.05);
    //SLfloat rad = SL_random(0.0f, max);
    //SLfloat phi = SL_random(0.0f, SL_2PI);
    //SLfloat random = rand() % _revPoints.size();
    //SLVec3f point = this->_revPoints[random];
    SLVec3f point(3,4,7);

    return point;    
}

/*!
\brief Calculate the delta of the angle
\param radius - the radius of the surface
*/
SLfloat SLSphericalRefractionSurface::calcAngle(SLfloat radius)
{
    SLfloat alphaAsin = _diameter / (2.0f * radius);
    alphaAsin = (alphaAsin > 1) ? 1 : alphaAsin;  // correct rounding errors
    alphaAsin = (alphaAsin < -1) ? -1 : alphaAsin;// correct rounding errors
    SLfloat alphaRAD = 2.0f * (SLfloat)asin(alphaAsin);
    return alphaRAD;
}

/*!
\brief generate a plane surface
\param xStart - start value of x
\param yStart - start value of y
*/
void SLSphericalRefractionSurface::generatePlane(SLfloat xStart, SLfloat yStart)
{
    SLfloat cutX = _diameter / _stacks;

    SLVec3f p;
    SLfloat y;
    SLfloat x = xStart;

    // Draw plane
    for (int i = 0; i <= (_stacks / 2); i++)
    {
        //check if bot or top side
        x = (xStart <= 0) ? cutX * i : xStart - (cutX * i);
        y = yStart;

        // set point
        p.x = x;
        p.y = y;
        p.z = 0;
        _revPoints.push_back(p);
    }
}


/*!
\brief Generate the spherical surface
\param radius of the surface
\param xStart - start value of x
\param yStart - start value of y
\param yTranslate - y translation
\param startAngle - the current angle of the x,y position
*/
void SLSphericalRefractionSurface::generateLensSurface(SLfloat radius,
    SLfloat xStart,
    SLfloat yStart,
    SLfloat yTranslate,
    SLfloat startAngle
    )
{
    SLfloat radiusAmount = std::fabs(radius);
    SLint halfStacks = _stacks / 2;
    SLfloat dAlphaRAD = (calcAngle(radius) * 0.5f) / halfStacks;
    SLfloat sagitta = calcSagitta(radius);

    if (sagitta < 0)
        generatePlane(xStart, yStart);

    // Start Point
    SLfloat currentAlphaRAD = startAngle;
    SLfloat x = xStart;
    SLfloat y = yStart;

    SLVec3f p;
    p.x = x;
    p.y = y;
    p.z = 0;
    _revPoints.push_back(p);

    SLfloat oldX = x;
    SLfloat oldY = y;

    // calc lens surface
    for (int i = 0; i < halfStacks; i++)
    {
        // change angle
        currentAlphaRAD += dAlphaRAD;

        if ((i + 1 == halfStacks))
        {
            // x wird grösser bot
            if (oldX <= x)
            {
                // y wird grösser bot konvex
                if (oldY <= y)
                { // 1
                    x = _diameter / 2;
                    y = sagitta;
                }
                else{ // y wird kleiner 2 bot konkav
                    x = _diameter / 2;
                    y = -sagitta;
                }
            }
            else{ // x wird kleiner top
                // y wird grösser top konvex
                if (oldY <= y)
                { // 3
                    x = 0;
                    y = sagitta;
                }
                else{ // y wird kleiner 4 top konkav
                    x = 0;
                    y = 0;
                }
            }
        }
        else
        {
            oldX = x;
            oldY = y;
            x = cos(currentAlphaRAD) * radiusAmount;
            y = ((sin(currentAlphaRAD)) * radiusAmount + yTranslate);
        }

        // set point
        p.x = x;
        p.y = y;
        p.z = 0;
        _revPoints.push_back(p);
    }
}

/*!
\brief Calculate the sagitta (s) for a given radius (r) and diameter (l+l)
l: half of the lens diameter
\param radius r of the surface
\return sagitta s of the surface
\image html Sagitta.png
http://en.wikipedia.org/wiki/Sagitta_%28geometry%29
*/
SLfloat SLSphericalRefractionSurface::calcSagitta(SLfloat radius)
{
    // take the amount of the radius
    SLfloat radiusAmount = (radius < 0) ? -radius : radius;
    SLfloat l = _diameter * 0.5f;

    // sagitta = radius - sqrt( radius*radius - l*l )
    // calc this part to sort out negative numbers in square root
    SLfloat part = radiusAmount*radiusAmount - l*l;

    // set sagitta negative if no bulge is given -> plane
    SLfloat sagitta = (part >= 0) ? (radiusAmount - sqrt(part)) : -1;
    return sagitta;
}

//-----------------------------------------------------------------------------
