//#############################################################################
//  File:      SLGullstrandCamera.cpp
//  Author:    Philipp Jüni
//  Date:      October 2014
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


SLGullstrandCamera::SLGullstrandCamera( SLfloat retinaRadius, 
                                        SLfloat fieldOfViewDEG, 
                                        SLfloat nEyeWater, 
                                        SLGullstrandCameraType cameraType)
{
    // calculate retina diameter
    SLfloat fieldOfViewRAD = 120 * SL_DEG2RAD;
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
    //_retinaNode->rotate(-90, 1, 0, 0, TS_Local);
    _retinaNode->translate(0, -_eyeSize, 0, TS_Local);
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
            SLfloat p = edge * 2 / (_imgWidth + _imgHeight);
            // -1 to make sure the retina is always hit
            _hWidth = (p * _imgWidth - 1) * 0.5;
            _hHeight = (p * _imgHeight - 1) * 0.5;
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

void SLGullstrandCamera::addSurface(SLSphericalRefractionSurface* surface, 
                                    SLfloat position)
{
    surface->setPosition(SLVec3f(0,0,position));
    SLNode *node = new SLNode(surface,surface->name());
    node->rotate(-90, 1, 0, 0, TS_Local);
    node->translate(0, -position, 0, TS_Local);
    
    addChild(node);
    _surfaces.insert(_surfaces.begin(), surface);
    _surfNodes.insert(_surfNodes.begin(), node);
}

/*
// change from cartesian to polar
// get point on macula
SLVec3f SLGullstrandCamera::transferCoords(SLfloat x, SLfloat y)
{
    SLfloat halfPxSize = _pxSize * 0.5f;
    SLfloat maculaX = -_hWidth + halfPxSize + x;
    SLfloat maculaY = -_hHeight + halfPxSize + y;

    SLfloat radius = sqrt(maculaX*maculaX + maculaY*maculaY);
    SLfloat phi = atan(maculaY / maculaX);

    //SLVec3f point = _retina->getPoint(radius, phi);
    SLVec3f point(1, 2, 3);
    return point;
}
*/

void SLGullstrandCamera::generateCameraRay( SLRay* ray, 
                                            SLVec3f bl, 
                                            SLVec3f lr, 
                                            SLVec3f lu, 
                                            SLfloat pxSize)
{
    _pxSize = pxSize;
    _cameraPosition = ray->origin.z;

    // set startpoint to virtual image surface
    
    //ray->origin.x = ray->x * pxSize;
    //ray->origin.y = ray->y * pxSize;
    SLfloat myPxSize = ((_hWidth * 2) / _imgWidth) * 4;
    SLfloat myhPx = myPxSize / 2;
    //ray->origin.x += bl.x + (ray->x * pxSize *5)-_hWidth;
    //ray->origin.y += bl.y + (ray->y * pxSize * 5)+_hHeight;
    //ray->origin.x = -bl.x;
    //ray->origin.y = -bl.y;
    ray->origin.x = -5 + (ray->x * pxSize * 2);
    ray->origin.y = -1 + (ray->y * pxSize * 2);

    //SLVec3f bla = _retinaNode->position();
    //ray->origin.y = ray->y * myPxSize + myhPx;

    ray->origin.z += (_eyeSize + 2.0f);
    
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

    if (wasHit)
    {
        ray->origin = ray->hitPoint;
        ray->originOS.set(updateAndGetWMI().multVec(ray->origin));
        ray->kn = ray->hitMat->knI();
        ray->hitDir = true;

        //cameraHERT(ray);
        
        ray->type = PRIMARY;
    }
    else
    {        
        ray->type = BLOCKED;
    }

    // generate primary ray to start into scene
    ray->depth = 1;
    ray->length = FLT_MAX;
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
    ray->contrib = 1.0f;
    ray->kn = 1.0f;
    ray->isOutside = true;
    ray->isInsideVolume = false;
}
void SLGullstrandCamera::cameraHERT(SLRay *ray)
{
    // go through surfaces from lens and cornea
    for (int i = 1; i < _surfaces.size() ; i++)
    {
        SLSphericalRefractionSurface* surface = _surfaces[i];
        SLNode* surfNode = _surfNodes[i];

        if (i == 0)
        {
            // hit lens bark back randomly
            SLVec3f hitPoint = surface->getRandomPoint();
            hitPoint.z += _cameraPosition;

            SLVec3f dir = hitPoint - ray->origin;
            dir.normalize();
            ray->setDir(dir);
            ray->hitPoint = hitPoint;
            ray->hitMesh = surface;
            ray->hitNode = surfNode;
            ray->hitMat = surface->mat;
            ray->length = ray->origin.z - hitPoint.z;

            ray->refractHE(ray);
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
            
            // intersect against all faces
            SLbool wasHit = false;
            for (SLuint t = 0; t < surface->numI; t += 3)
            {
                if (!wasHit && surface->hitTriangleOS(ray, surfNode, t))
                {
                    wasHit = true;
                    surface->preShade(ray);
                }
            }
            ray->refractHE(ray);
        }
    }
}

//-----------------------------------------------------------------------------
void SLGullstrandCamera::drawMeshes(SLSceneView* sv)
{
    if (sv->camera() != this)
    {
        if (_projection == monoOrthographic)
        {
            const SLMat4f& vm = updateAndGetWMI();
            SLVec3f P[17 * 2];
            SLuint  i = 0;
            SLVec3f pos(vm.translation());
            SLfloat t = tan(SL_DEG2RAD*_fov*0.5f) * pos.length();
            SLfloat b = -t;
            SLfloat l = -sv->scrWdivH() * t;
            SLfloat r = -l;

            P[i++].set(0, -10, 0); P[i++].set(0, 10, 0);

            // small line in view direction
            P[i++].set(0, 0, 0); P[i++].set(0, 0, _clipNear * 4);

            // frustum pyramid lines
            P[i++].set(r, t, _clipNear); P[i++].set(r, t, -_clipFar);
            P[i++].set(l, t, _clipNear); P[i++].set(l, t, -_clipFar);
            P[i++].set(l, b, _clipNear); P[i++].set(l, b, -_clipFar);
            P[i++].set(r, b, _clipNear); P[i++].set(r, b, -_clipFar);

            // around far clipping plane
            P[i++].set(r, t, -_clipFar); P[i++].set(r, b, -_clipFar);
            P[i++].set(r, b, -_clipFar); P[i++].set(l, b, -_clipFar);
            P[i++].set(l, b, -_clipFar); P[i++].set(l, t, -_clipFar);
            P[i++].set(l, t, -_clipFar); P[i++].set(r, t, -_clipFar);

            // around projection plane at focal distance
            P[i++].set(r, t, -_focalDist); P[i++].set(r, b, -_focalDist);
            P[i++].set(r, b, -_focalDist); P[i++].set(l, b, -_focalDist);
            P[i++].set(l, b, -_focalDist); P[i++].set(l, t, -_focalDist);
            P[i++].set(l, t, -_focalDist); P[i++].set(r, t, -_focalDist);

            // around near clipping plane
            P[i++].set(r, t, _clipNear); P[i++].set(r, b, _clipNear);
            P[i++].set(r, b, _clipNear); P[i++].set(l, b, _clipNear);
            P[i++].set(l, b, _clipNear); P[i++].set(l, t, _clipNear);
            P[i++].set(l, t, _clipNear); P[i++].set(r, t, _clipNear);

            SLCamera::_bufP.generate(P, i, 3);
        }
        else
        {
            SLVec3f P[17 * 2];
            SLuint  i = 0;
            SLfloat aspect = sv->scrWdivH();
            SLfloat tanFov = tan(_fov*SL_DEG2RAD*0.5f);
            SLfloat tF = tanFov * _clipFar;    //top far
            SLfloat rF = tF * aspect;          //right far
            SLfloat lF = -rF;                   //left far
            SLfloat tP = tanFov * _focalDist;  //top projection at focal distance
            SLfloat rP = tP * aspect;          //right projection at focal distance
            SLfloat lP = -tP * aspect;          //left projection at focal distance
            SLfloat tN = tanFov * _clipNear;   //top near
            SLfloat rN = tN * aspect;          //right near
            SLfloat lN = -tN * aspect;          //left near

            // small line in view direction
            P[i++].set(0, 0, -5); P[i++].set(0, 0, 7);

            // frustum pyramid lines
            P[i++].set(0, 0, 0); P[i++].set(rF, tF, -_clipFar);
            P[i++].set(0, 0, 0); P[i++].set(lF, tF, -_clipFar);
            P[i++].set(0, 0, 0); P[i++].set(lF, -tF, -_clipFar);
            P[i++].set(0, 0, 0); P[i++].set(rF, -tF, -_clipFar);

            // around far clipping plane
            P[i++].set(rF, tF, -_clipFar); P[i++].set(rF, -tF, -_clipFar);
            P[i++].set(rF, -tF, -_clipFar); P[i++].set(lF, -tF, -_clipFar);
            P[i++].set(lF, -tF, -_clipFar); P[i++].set(lF, tF, -_clipFar);
            P[i++].set(lF, tF, -_clipFar); P[i++].set(rF, tF, -_clipFar);

            // around projection plane at focal distance
            P[i++].set(rP, tP, -_focalDist); P[i++].set(rP, -tP, -_focalDist);
            P[i++].set(rP, -tP, -_focalDist); P[i++].set(lP, -tP, -_focalDist);
            P[i++].set(lP, -tP, -_focalDist); P[i++].set(lP, tP, -_focalDist);
            P[i++].set(lP, tP, -_focalDist); P[i++].set(rP, tP, -_focalDist);

            // around near clipping plane
            P[i++].set(rN, tN, -_clipNear); P[i++].set(rN, -tN, -_clipNear);
            P[i++].set(rN, -tN, -_clipNear); P[i++].set(lN, -tN, -_clipNear);
            P[i++].set(lN, -tN, -_clipNear); P[i++].set(lN, tN, -_clipNear);
            P[i++].set(lN, tN, -_clipNear); P[i++].set(rN, tN, -_clipNear);

            SLCamera::_bufP.generate(P, i, 3);
        }

        SLCamera::_bufP.drawArrayAsConstantColorLines(SLCol3f::WHITE*0.7f);
    }
}
//-----------------------------------------------------------------------------