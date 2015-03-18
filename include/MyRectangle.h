#ifndef MYRECTANGLE_H
#define MYRECTANGLE_H

#include <stdafx.h>
#include "SLMesh.h"
#include "SLSurface.h"


class MyRectangle : public SLSurface
{
public:
    MyRectangle(SLVec2f min, SLVec2f max,
        SLuint resX, SLuint resY,
        SLstring name,
        SLMaterial* mat);
    
    MyRectangle(SLVec2f min, SLVec2f max,
        SLVec2f tmin, SLVec2f tmax,
        SLuint resX, SLuint resY,
        SLstring name = "Rectangle",
        SLMaterial* mat=0);

    void        buildMesh(SLMaterial* mat);

protected:
    SLVec3f    _min;     //!< min corner
    SLVec3f    _max;     //!< max corner
    SLVec2f    _tmin;    //!< min corner texCoord
    SLVec2f    _tmax;    //!< max corner texCoord
    SLuint     _resX;    //!< resolution in x direction
    SLuint     _resY;    //!< resolution in y direction
};
//-----------------------------------------------------------------------------
#endif