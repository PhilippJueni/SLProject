#ifndef SLSURFACE_H
#define SLSURFACE_H

#include <stdafx.h>
#include "SLMesh.h"
#include "SLGullstrandRay.h"


class SLSurface : public SLMesh
{
public: 
    SLSurface(SLstring name);

    void setSurface(SLfloat knIn, SLfloat knOut);

    bool hitRec2(SLGullstrandRay* ray);
   
    SLMesh getMesh();
protected:
    SLVec3f    _knIn;     
    SLVec3f    _knOut;     

private:
    SLMesh _mesh;
};
//-----------------------------------------------------------------------------
#endif //SLSURFACE_H