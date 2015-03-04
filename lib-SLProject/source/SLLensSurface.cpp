//#############################################################################
//  File:      SLLensSurface.cpp
//  Author:    Philipp Jüni
//  Date:      October 2014
//  Copyright: 2002-2014 Marcus Hudritsch, Philipp Jüni
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>           // precompiled headers
#ifdef SL_MEMLEAKDETECT       // set in SL.h for debug config only
#include <debug_new.h>        // memory leak detector
#endif

#include <SLLensSurface.h>

SLLensSurface::SLLensSurface(SLfloat diameter,
    SLfloat knBot,
    SLfloat knTop,
    SLint stacks,
    SLint slices,
    SLstring name ) : SLRevolver(name)
{
    _diameter = diameter;
    _knBot = knBot;
    _knTop = knTop;
    _stacks = stacks;
    _slices = slices;
    _mat = mat;

    _smoothFirst = true;
    _smoothLast = true;
    _revAxis.set(0, 1, 0);

    _thickness = 0.0f;
}

SLfloat SLLensSurface::addBottom(SLfloat radius)
{
    SLfloat sagitta = calcSagitta(radius);
    SLfloat x;
    if (radius > 0)
    {
        x = generateLensSurface(radius, 0, -sagitta + _thickness, radius - sagitta, -SL_HALFPI);
    }
    else
    {
        x = generateLensSurface(radius, 0, _thickness, radius, SL_HALFPI);
    }
    return x;
}

SLfloat SLLensSurface::addTop(SLfloat radius)
{
    SLfloat sagitta = calcSagitta(radius);
    SLfloat x;
    SLfloat startAngle;
    SLfloat alphaRAD = calcAngle(radius);
    if (radius > 0)
    {
        startAngle = SL_HALFPI - (alphaRAD * 0.5f);
        x = generateLensSurface(radius, _diameter / 2, _thickness, -radius + sagitta, startAngle);
    }
    else
    {
        startAngle = -SL_HALFPI - (alphaRAD * 0.5f);
        x = generateLensSurface(radius, _diameter / 2, _thickness + sagitta, -radius, startAngle);
    }
    return x;
}

SLfloat SLLensSurface::calcAngle(SLfloat radius)
{
    SLfloat alphaAsin = _diameter / (2.0f * radius);
    alphaAsin = (alphaAsin > 1) ? 1 : alphaAsin;  // correct rounding errors
    alphaAsin = (alphaAsin < -1) ? -1 : alphaAsin;// correct rounding errors
    SLfloat alphaRAD = 2.0f * (SLfloat)asin(alphaAsin);
    return alphaRAD;
}


void SLLensSurface::setSpaceBetweenSurfaces(SLfloat surfaceSpace)
{
    _thickness = surfaceSpace;
}

SLfloat SLLensSurface::addPlane(SLfloat xStart, SLfloat yStart)
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
        cout << "A" << "  x: " << x << "  y: " << y << " _A" << endl;
    }
    return x;
}


/*!
\brief Generate the bottom (front) part of the lens
\param radius of the lens
\return x the x coordinate of the last point of the bulge
*/
SLfloat SLLensSurface::generateLensSurface(SLfloat radius,
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

    cout << "1. radius: " << radius << " x: " << xStart << " y: " << yStart << " yT: " << yTranslate << " angle: " << startAngle << endl;

    if (sagitta < 0)
        return addPlane(xStart, yStart);

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
                    y = _thickness;
                }
                else{ // y wird kleiner 2 bot konkav
                    x = _diameter / 2;
                    y = -sagitta + _thickness;
                }
            }
            else{ // x wird kleiner top
                // y wird grösser top konvex
                if (oldY <= y)
                { // 3
                    x = 0;
                    y = sagitta + _thickness;
                }
                else{ // y wird kleiner 4 top konkav
                    x = 0;
                    y = _thickness;
                }
            }
        }
        else
        {
            oldX = x;
            oldY = y;
            x = cos(currentAlphaRAD) * radiusAmount;
            y = ((sin(currentAlphaRAD)) * radiusAmount + yTranslate + _thickness);
        }

        // set point
        p.x = x;
        p.y = y;
        p.z = 0;
        _revPoints.push_back(p);
    }
    return x;
}

/*!
\brief Calculate the sagitta (s) for a given radius (r) and diameter (l+l)
l: half of the lens diameter
\param radius r of the lens
\return sagitta s of the lens
\image html Sagitta.png
http://en.wikipedia.org/wiki/Sagitta_%28geometry%29
*/
SLfloat SLLensSurface::calcSagitta(SLfloat radius)
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