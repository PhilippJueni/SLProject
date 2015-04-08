#ifndef SLTRIANGLE_H
#define SLTRIANGLE_H

//#include <stdafx.h>
#include "SLMesh.h"

class SLTriangle : public SLMesh
{
public:
    SLTriangle(SLMaterial *mat);

    SLTriangle(SLVec2f min, SLVec2f max,
        SLuint resX, SLuint resY,
        SLstring name,
        SLMaterial* mat);

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
#endif //SLTRIANGLE_H