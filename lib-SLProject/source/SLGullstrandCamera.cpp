//#############################################################################
//  File:      SLGullstrandCamera.cpp
//  Author:    Philipp Jüni
//  Date:      June 2015
//  Copyright: 2002-2014 Marcus Hudritsch, Philipp Jüni
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>           // precompiled headers
#include <SLGullstrandCamera.h>
#include <SLLens.h>
#include <SLSphere.h>
#include <SLSceneView.h>
#include <SLTriangle.h>
//-----------------------------------------------------------------------------
/*!
Create the gullstrand-camera retina and image plane from the given parameters
\param retinaRadius - the radius of the eye globe
\param fieldOfViewDEG - the frustumangle in deg
\param nEyeWater - the kn of the eye water
\param sv - SLSceneView
\param cameraType - STANDARD, CORNER-FISHEYE, FULL-FISCHEYE
*/
SLGullstrandCamera::SLGullstrandCamera( SLfloat retinaRadius, 
                                        SLfloat fieldOfViewDEG, 
                                        SLfloat nEyeWater, 
                                        SLSceneView* sv,
                                        SLGullstrandCameraType cameraType)
{
    // calculate retina diameter
    SLfloat fieldOfViewRAD = fieldOfViewDEG * SL_DEG2RAD;
    SLfloat retinaDiameter = 2 * retinaRadius * sin(fieldOfViewRAD*0.5);

    // create retina
    SLMaterial* matRetina = new SLMaterial(     "matRetina", 
                                                SLCol4f(0.0f, 0.0f, 0.5f), 
                                                SLCol4f(0.5f, 0.5f, 0.5f), 
                                                100, 
                                                0.0f, 
                                                0.8f, 
                                                nEyeWater, 
                                                1.0f);
    _retina = new SLSphericalRefractionSurface( retinaDiameter, 
                                                retinaRadius, 
                                                32, 
                                                32, 
                                                matRetina,
                                                "retina");
    _retinaNode = new SLNode( _retina, _retina->name() );
    _retinaNode->translate(0, 0, _eyeSize, TS_Local);
    addChild(_retinaNode);    
    
    // calculate image plane size    
    switch (cameraType)
    {
        case 2:
            // fish eye corners
            _hWidth = (retinaDiameter +1) * 0.5;
            _hHeight = _hWidth * 0.75;
            break;
        case 1:
            // full fish eye
            _hHeight = (retinaDiameter +1) * 0.5;
            _hWidth = _hHeight * 4 / 3;
            break;
        case 0:
        default:            
            // eye not visible
            SLfloat edge = sin(SL_HALFPI*0.5f) * retinaDiameter;
            SLfloat p = edge * 2 / (sv->scrW() + sv->scrH());
            // -1 to make sure the retina is always hit
            _hWidth = (p * sv->scrW() - 1) * 0.5;
            _hHeight = (p * sv->scrH() -1) * 0.5;
    }
        
    // create image plane
    SLGLTexture* texRec = new SLGLTexture("tron_floor2.png");
    SLMaterial* matRec = new SLMaterial("texRec", texRec);
    _imageRectangle = new SLRectangle(  SLVec2f(-_hWidth, -_hHeight), 
                                        SLVec2f(_hWidth, _hHeight), 
                                        1, 
                                        1, 
                                        "imagePlane", 
                                        matRec);
    _rectNode = new SLNode(_imageRectangle,"imagePlane");
    _rectNode->translate(0, 0, _eyeSize + _imagePlaneGap, TS_Local);
    addChild(_rectNode);
}

/*!
\brief Add a refracting surface to the camera
\param surface
\param z-position from the front end of the eye
*/
void SLGullstrandCamera::addSurface(SLSphericalRefractionSurface* surface, 
                                    SLfloat position)
{
    surface->setPosition(SLVec3f(0,0,position));
    SLNode *node = new SLNode(surface,surface->name());
    node->translate(0, 0, position, TS_Local);
    
    addChild(node);
    _surfaces.insert(_surfaces.begin(), surface);
    _surfNodes.insert(_surfNodes.begin(), node);
}

/*!
\brief prepare the primaryRay for the raytracer.
\Find the intersection point for each ray on the retina
\param ray
\param sv
*/
void SLGullstrandCamera::generateCameraRay( SLRay* ray, SLSceneView* sv)
{
    _cameraPosition = ray->origin.z;

    // set startpoint to virtual image surface
    SLfloat pxSize = ((_hWidth * 2) / sv->scrW());
    SLfloat hPxSize = pxSize * 0.5;
    ray->origin.z = _eyeSize + _imagePlaneGap;
    ray->origin.x = -_hWidth + (hPxSize + (ray->x * pxSize));
    ray->origin.y = -_hHeight + (hPxSize + (ray->y * pxSize));

    // go to macula    
    SLVec3f dir(0,0,-1);
    ray->setDir(dir);
    ray->setDirOS(dir);
    ray->originOS.set(updateAndGetWMI().multVec(ray->origin));

    // intersect against all retina faces
    SLbool wasHit = false;
    for (SLuint t = 0; t < _retina->numI; t += 3)
    {
        if (!wasHit && _retina->hitTriangleOS(ray, _retinaNode, t) )
        {
            wasHit = true;
            _retina->preShade(ray);
        }
    }

    if (!wasHit)
    {
        ray->type = BLOCKED;
    }else{
        ray->origin = ray->hitPoint;
        ray->originOS.set(updateAndGetWMI().multVec(ray->origin));
        ray->kn = ray->hitMat->knI();
        ray->hitDir = true;

        cameraHERT(ray);
        
        ray->type = PRIMARY;
    }
    // generate primary ray to start into scene
    ray->depth = 1;
    ray->hitTriangle = -1;
    ray->hitPoint = SLVec3f::ZERO;
    ray->hitNormal = SLVec3f::ZERO;
    ray->hitTexCol = SLCol4f::BLACK;
    ray->hitNode = 0;
    ray->hitMesh = 0;
    ray->hitMat = 0;
    ray->originNode = 0;
    ray->originMesh = 0;
    ray->originTria = -1;
    ray->originMat = 0;
    ray->kn = 1.0f;
}

/*!
\brief Go through refraction surfaces of the eye
\param ray
*/
void SLGullstrandCamera::cameraHERT(SLRay *ray)
{
    // go through surfaces from lens and cornea
    for (int i = 0; i < _surfaces.size() ; i++)
    {
        SLSphericalRefractionSurface* surface = _surfaces[i];
        SLNode* surfNode = _surfNodes[i];

        if (i == -1)
        {
            // hit lens bark back randomly
            SLVec3f hitPoint = surface->getRandomPoint();
            
            SLVec3f dir = hitPoint - ray->origin;
            dir.normalize();
            ray->setDir(dir);
            ray->hitPoint = hitPoint;
            ray->hitMesh = surface;
            ray->hitNode = surfNode;
            ray->hitMat = surface->mat;
            ray->length = ray->origin.z - hitPoint.z;
        }
        else
        {
            ray->setDirOS(ray->dir);
            ray->originOS.set(updateAndGetWMI().multVec(ray->origin));
            ray->isOutside = true;

            ray->hitMesh = surface;
            ray->hitNode = surfNode;
            ray->hitMat = surface->mat;
            ray->hitDir = true;
            
            // intersect against all faces of this surface
            SLbool wasHit = false;
            for (SLuint t = 0; t < surface->numI; t += 3)
            {
                if (!wasHit && surface->hitTriangleOS(ray, surfNode, t))
                {
                    wasHit = true;
                    surface->preShade(ray);
                }
            }
        }
        ray->refractHE(ray);
    }
}

//-----------------------------------------------------------------------------