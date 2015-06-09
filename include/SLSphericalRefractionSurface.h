#ifndef SLSPHERICALREFRACTIONSURFACE_H
#define SLSPHERICALREFRACTIONSURFACE_H

#include <stdafx.h>
#include "SLMesh.h"
#include "SLRefractionSurface.h"
#include "SLRevolver.h"


class SLSphericalRefractionSurface : public SLRefractionSurface, public SLRevolver
{
public:
    SLSphericalRefractionSurface(   SLfloat diameter,
                                    SLfloat radius = 1.0f,
                                    SLint stacks = 32,
                                    SLint slices = 32,
                                    SLMaterial* mat = 0,
                                    SLstring name = "SphericalRefractionSurface");

    void setPosition(SLVec3f pos){ _position = pos; };
    SLVec3f getRandomPoint();
    
    ~SLSphericalRefractionSurface() { ; }

    SLfloat getDiameter() { return _diameter; }

private:
    void generateLensSurface(       SLfloat radius,
                                    SLfloat xStart,
                                    SLfloat yStart,
                                    SLfloat yTranslate,
                                    SLfloat startAngle);

    void generatePlane(SLfloat xStart, SLfloat yStart);

    SLfloat calcAngle(SLfloat radius);

    SLfloat calcSagitta(SLfloat radius);

    SLint   _stacks;        //!< NO. of stacks 
    SLfloat _diameter;      //!< The diameter of the lens
    SLfloat _radius;
    SLVec3f _position;
};
//-----------------------------------------------------------------------------
#endif