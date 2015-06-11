//#############################################################################
//  File:      SLSphericalRefractionSurface.cpp
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

// random number
extern SLfloat rnd01();

/*!
\brief ctor of the spherical refraction surface
\param diameter
\param radius
\param stacks
\param slices
\param material
\param name
*/
SLSphericalRefractionSurface::SLSphericalRefractionSurface(SLfloat diameter,
    SLfloat radius,
    SLint stacks,
    SLint slices,
    SLMaterial* mat,
    SLstring name) : SLRevolver(name)
{
    _diameter = diameter;
    _radius = radius;    
    _stacks = stacks;
    _slices = slices;

    _smoothFirst = false;
    _revAxis.set(0, 0, 1);
    _accelStruct = NULL;

    SLfloat sagitta = calcSagitta();
    SLfloat alphaRAD = calcAngle();    
    if (sagitta < 0)
    {   // plane
        generatePlane(_diameter / 2);
    }
    else
    {
        if (_radius > 0)
        {   // konvex
            SLfloat startAngle = SL_HALFPI - (alphaRAD * 0.5);
            generateLensSurface(startAngle);
        }
        else
        {   // konkav
            SLfloat startAngle = -SL_HALFPI - (alphaRAD * 0.5);
            generateLensSurface(startAngle);
        }
    }
    buildMesh(mat);
}

/*!
\brief Returns a random point on this surface
\return point SLVec3f
*/
SLVec3f SLSphericalRefractionSurface::getRandomPoint()
{
    SLfloat alphaRAD = 2 * asin(_diameter / (2 * _radius));
    SLfloat alphaDEG = alphaRAD * SL_RAD2DEG;
    
    // get random angles in the segment
    SLfloat phiRAD = rnd01() * alphaRAD;
    SLfloat etaRAD = rnd01() * alphaRAD*0.5;

    SLfloat phiDEG = phiRAD * SL_RAD2DEG;
    SLfloat etaDEG = etaRAD * SL_RAD2DEG;

    SLfloat sagitta = calcSagitta();
    SLfloat h = _radius - sagitta;
    SLfloat diff = _radius - sagitta;

    // cartesian coordinates
    // formula www.wikipwdia.org
    // x = _radius * sin(phiRAD) * cos(etaRAD);
    // y = _radius * sin(phiRAD) * sin(etaRAD);
    // z = _radius * cos(phiRAD);
    SLfloat x = _radius * sin(phiRAD) * cos(etaRAD);
    SLfloat z = -((_radius * cos(etaRAD)) - _radius);
    SLfloat y = _radius - (_radius * sin(phiRAD) * sin(etaRAD));
    SLVec3f point(x, y, z);

    // add surface position
    point = _position + point;
    return point;    
}

/*!
\brief generate a plane surface
\param xStart - start x coordinate
*/
void SLSphericalRefractionSurface::generatePlane(SLfloat xStart)
{
    // get segment size
    SLfloat cutX = _diameter / _stacks;

    SLVec3f p;
    for (int i = 0; i <= (_stacks / 2); i++)
    {
        // set point
        p.x = (xStart <= 0) ? cutX * i : xStart - (cutX * i);
        p.y = 0;
        p.z = 0;
        _revPoints.push_back(p);
    }
}

/*!
\brief Generate the spherical surface
\param alpha - the current angle of the x,z position
*/
void SLSphericalRefractionSurface::generateLensSurface(SLfloat alphaRAD)
{
    SLfloat radiusAmount = std::fabs(_radius);
    SLint halfStacks = _stacks / 2;
    SLfloat dAlphaRAD = (calcAngle() * 0.5f) / halfStacks;
    
    SLVec3f p;
    // calc lens surface
    for (int i = 0; i < halfStacks; i++)
    {
        // change angle
        alphaRAD += dAlphaRAD;

        // set point
        p.x = cos(alphaRAD) * radiusAmount;
        p.y = 0;
        p.z = ((sin(alphaRAD)) * radiusAmount - _radius);

        _revPoints.push_back(p);
    }
}

/*!
\brief Calculate the delta of the angle
\return deltaAlpha
*/
SLfloat SLSphericalRefractionSurface::calcAngle()
{
    SLfloat alphaAsin = _diameter / (2.0f * _radius);
    alphaAsin = (alphaAsin > 1) ? 1 : alphaAsin;  // correct rounding errors
    alphaAsin = (alphaAsin < -1) ? -1 : alphaAsin;// correct rounding errors
    SLfloat alphaRAD = 2.0f * (SLfloat)asin(alphaAsin);
    return alphaRAD;
}

/*!
\brief Calculate the sagitta (s) for a given radius (r) and diameter (l+l)
l: half of the lens diameter
\return sagitta s of the surface
\image html Sagitta.png
http://en.wikipedia.org/wiki/Sagitta_%28geometry%29
*/
SLfloat SLSphericalRefractionSurface::calcSagitta()
{
    // take the amount of the radius
    SLfloat radiusAmount = (_radius < 0) ? -_radius : _radius;
    SLfloat l = _diameter * 0.5f;

    // sagitta = radius - sqrt( radius*radius - l*l )
    // calc this part to sort out negative numbers in square root
    SLfloat part = radiusAmount*radiusAmount - l*l;

    // set sagitta negative if no bulge is given -> plane
    SLfloat sagitta = (part >= 0) ? (radiusAmount - sqrt(part)) : -1;
    return sagitta;
}

//-----------------------------------------------------------------------------
