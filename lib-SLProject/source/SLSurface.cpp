#include <stdafx.h>           // precompiled headers
#ifdef SL_MEMLEAKDETECT       // set in SL.h for debug config only
#include <debug_new.h>        // memory leak detector
#endif

/*
#include "stdafx.h"
#include "SLSurface.h"

SLSurface::SLSurface(SLstring name)
{

    cout << "asdf2" << endl;
}

SLMesh SLSurface::getMesh()
{
    return _mesh;
}

void SLSurface::setSurface(SLfloat knIn, SLfloat knOut)
{
    cout << "asdf" << endl;
}


bool SLSurface::hitRec2(SLGullstrandRay* ray)
{
    
    assert(ray != 0);

    // Do not test hidden nodes
    if (_drawBits.get(SL_DB_HIDDEN))
        return false;

    // Do not test origin node for shadow rays 
    if (this == ray->originNode && ray->type == SHADOW)
        return false;

    // Check first AABB for intersection
    if (!_aabb.isHitInWS(ray))
        return false;


    SLbool wasHit = false;

    // Transform ray to object space for non-groups
    if (_meshes.size() > 0)
    {
        // transform origin position to object space
        ray->originOS.set(updateAndGetWMI().multVec(ray->origin));

        // transform the direction only with the linear sub matrix
        ray->setDirOS(_wmI.mat3() * ray->dir);

        // test all meshes
        for (SLint i = 0; i<_meshes.size(); ++i)
        {
            if (_meshes[i]->hit(ray, this) && !wasHit)
                wasHit = true;
            if (ray->isShaded())
                return true;
        }
    }

    // Test children nodes
    for (SLint i = 0; i<_children.size(); ++i)
    {
        if (_children[i]->hitRec(ray) && !wasHit)
            wasHit = true;
        if (ray->isShaded())
            return true;
    }

    return wasHit;
    
    return false;
}

//-----------------------------------------------------------------------------
*/