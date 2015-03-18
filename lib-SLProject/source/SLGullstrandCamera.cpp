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
//-----------------------------------------------------------------------------


SLGullstrandCamera::SLGullstrandCamera()
{

}

void SLGullstrandCamera::addLens(SLNode* node, SLfloat position)
{
    node->rotate(90, 1, 0, 0, TS_Local);
    node->translate(0, position, 0, TS_Local);
    addChild(node);
    
}

void SLGullstrandCamera::addSurface(SLSurface* surf, SLfloat position)
{
    _surfaces.push_back(surf);
    // todo: check that it is a surface
   
    SLNode *node = new SLNode( surf );
    node->translate(0, 0, position, TS_Local);
    addChild(node);
}

void SLGullstrandCamera::addLSurface(SLSurface* surf, SLfloat position)
{
    _surfaces.push_back(surf);
    // todo: check that it is a surface

    SLNode *node = new SLNode(surf);
    node->rotate(90, 1, 0, 0, TS_Local);
    node->translate(0, position, 0, TS_Local);
    addChild(node);
}

void SLGullstrandCamera::renderClassic(SLSceneView* sv)
{
    
    _sv = sv;
    SLRTState _state = rtBusy;                          // From here we state the RT as busy
    SLGLState* _stateGL = SLGLState::getInstance();     // OpenGL state shortcut
    SLint _numThreads = 1;                              // No. of threads
    SLint _pcRendered = 0;                              // % rendered
    SLfloat _renderSec = 0.0f;                          // reset time
    SLint _maxDepth = 5;
    //SLstring _infoText = SLScene::current->info(_sv)->text();  // keep original info string
    //SLCol4f _infoColor = SLScene::current->info(_sv)->color(); // keep original info color

    initStats(_maxDepth);               // init statistics
    prepareImage();                     // Setup image & precalculations

    // Measure time 
    double t1 = SLScene::current->timeSec();
    double tStart = t1;

    cout << "start render" << endl;
    for (SLuint x = 0; x<_img[0].width(); ++x)
    {
        for (SLuint y = 0; y<_img[0].height(); ++y)
        {
            SLRay primaryRay;
            setPrimaryRay((SLfloat)x, (SLfloat)y, &primaryRay);

            //////////////////////////////////////////
            SLCol4f color = traceClassic(&primaryRay);
            //////////////////////////////////////////

            _img[0].setPixeliRGB(x, y, color);

            SLRay::avgDepth += SLRay::depthReached;
            SLRay::maxDepthReached = SL_max(SLRay::depthReached, SLRay::maxDepthReached);
        }

        // Update image after 500 ms
        double t2 = SLScene::current->timeSec();
        if (t2 - t1 > 0.5)
        {
            _pcRendered = (SLint)((SLfloat)x / (SLfloat)_img[0].width() * 100);
            _sv->onWndUpdate();
            t1 = SLScene::current->timeSec();
        }
    }

    _renderSec = (SLfloat)(SLScene::current->timeSec() - tStart);
    _pcRendered = 100;

    if (_continuous)
    _state = rtReady;
    else
    {
        _state = rtFinished;
        //printStats(_renderSec);
    }
    
}
void SLGullstrandCamera::initStats(SLint depth)
{
    
    SLRay::maxDepth = (depth) ? depth : SL_MAXTRACE;
    SLRay::reflectedRays = 0;
    SLRay::refractedRays = 0;
    SLRay::shadowRays = 0;
    SLRay::subsampledRays = 0;
    SLRay::subsampledPixels = 0;
    SLRay::tests = 0;
    SLRay::intersections = 0;
    SLRay::maxDepthReached = 0;
    SLRay::avgDepth = 0.0f;
    
}
void SLGullstrandCamera::prepareImage()
{
    
    ///////////////////////
    //  PRECALCULATIONS  //
    ///////////////////////

    //_cam = _sv->_camera;                // camera shortcut

    // get camera vectors eye, lookAt, lookUp
    updateAndGetVM().lookAt(&_EYE, &_LA, &_LU, &_LR);
    /*
    if (_cam->projection() == monoOrthographic)
    {   /*
        In orthographic projection the bottom-left vector (_BL) points
        from the eye to the center of the bottom-left pixel of a plane that
        parallel to the projection plan at zero distance from the eye.
        *//*
        SLVec3f pos(_cam->updateAndGetVM().translation());
        SLfloat hh = tan(SL_DEG2RAD*_cam->fov()*0.5f) * pos.length();
        SLfloat hw = hh * _sv->scrWdivH();

        // calculate the size of a pixel in world coords.
        _pxSize = hw * 2 / _sv->scrW();

        _BL = _EYE - hw*_LR - hh*_LU + _pxSize / 2 * _LR - _pxSize / 2 * _LU;
    }
    else
    {  */ /*
        In perspective projection the bottom-left vector (_BL) points
        from the eye to the center of the bottom-left pixel on a projection
        plan in focal distance. See also the computergraphics script about
        primary ray calculation.
        */
        // calculate half window width & height in world coords
        SLfloat hh = tan(SL_DEG2RAD* fov()*0.5f) * focalDist();
        SLfloat hw = hh * _sv->scrWdivH();

        // calculate the size of a pixel in world coords.
        _pxSize = hw * 2 / _sv->scrW();

        // calculate a vector to the center (C) of the bottom left (BL) pixel
        SLVec3f C = _LA * focalDist();
        _BL = C - hw*_LR - hh*_LU + _pxSize / 2 * _LR + _pxSize / 2 * _LU;
    //}

    // Allocate image of the inherited texture class 
    if (_sv->scrW() != _img[0].width() || _sv->scrH() != _img[0].height())
    {
        // Delete the OpenGL Texture if it allready exists
        if (_texName)
        {
            glDeleteTextures(1, &_texName);
            //SL_LOG("glDeleteTextures id: %u   ", _texName);
            _texName = 0;
        }

        // Dispose VBO is they allready exist        
        SLGLTexture::_bufP.dispose();
        SLGLTexture::_bufT.dispose();
        SLGLTexture::_bufI.dispose();

        _img[0].allocate(_sv->scrW(), _sv->scrH(), GL_RGB);
    }

    // Fill image black for single RT
    if (!_continuous) _img[0].fill();
    
}
void SLGullstrandCamera::setPrimaryRay(SLfloat x, SLfloat y, SLRay* primaryRay)
{
    
    primaryRay->x = x;
    primaryRay->y = y;

    // calculate ray from eye to pixel (See also prepareImage())
    if (projection() == monoOrthographic)
    {
        primaryRay->setDir(_LA);
        primaryRay->origin = _BL + _pxSize*((SLfloat)x*_LR + (SLfloat)y*_LU);
    }
    else
    {
        SLVec3f primaryDir(_BL + _pxSize*((SLfloat)x*_LR + (SLfloat)y*_LU));
        primaryDir.normalize();
        primaryRay->setDir(primaryDir);
        primaryRay->origin = _EYE;
    }
    
}
SLCol4f SLGullstrandCamera::traceClassic(SLRay* ray)
{
    /*!
    This method is the classic recursive ray tracing method that checks the scene
    for intersection. If the ray hits an object the local color is calculated and
    if the material is reflective and/or transparent new rays are created and
    passed to this trace method again. If no object got intersected the
    background color is return.
    */
    SLScene* s = SLScene::current;
    SLCol4f color(s->backColor());

    hitRec(ray);

    

    if (ray->length < FLT_MAX)
    {
        color = shade(ray);

        if (ray->depth < SLRay::maxDepth && ray->contrib > SLRay::minContrib)
        {
            if (ray->hitMat->kt())
            {
                SLRay refracted;
                ray->refract(&refracted);
                color += ray->hitMat->kt() * traceClassic(&refracted);
            }
            if (ray->hitMat->kr())
            {
                SLRay reflected;
                ray->reflect(&reflected);
                color += ray->hitMat->kr() * traceClassic(&reflected);
            }
        }
    }

    color.clampMinMax(0, 1);
    return color;
}
SLbool SLGullstrandCamera::hitRec(SLRay *ray)
{
    
    assert(ray != 0);

    // Do not test hidden nodes
    if (_drawBits.get(SL_DB_HIDDEN))
        return false;

    // Do not test origin node for shadow rays 
    //if (this == ray->originNode && ray->type == SHADOW)
    //    return false;
    
    SLbool wasHit = false;

    // Test children nodes
    for (SLint i = 0; i<_surfaces.size(); ++i)
    {
//        if (_surfaces[i]->hitRec2(ray) && !wasHit)
//            wasHit = true;
        //if (ray->isShaded())
        //    return true;
    }

    return wasHit;
}
SLCol4f SLGullstrandCamera::shade(SLRay* ray)
{
    
    SLScene*    s = SLScene::current;
    SLCol4f     localColor = SLCol4f::BLACK;
    SLMaterial* mat = ray->hitMat;
    SLVGLTexture& texture = mat->textures();
    SLVec3f     L, N, H;
    SLfloat     lightDist, LdN, NdH, df, sf, spotEffect, att, lighted = 0.0f;
    SLCol4f     amdi, spec;
    SLCol4f     localSpec(0, 0, 0, 1);

    // Don't shade lights. Only take emissive color as material 
    //if (typeid(*ray->hitNode) == typeid(SLLightSphere) ||
    //    typeid(*ray->hitNode) == typeid(SLLightRect))
    //{
        localColor = mat->emission();
        return localColor;
    //}

    
    localColor = mat->emission() + (mat->ambient()&s->globalAmbiLight());

    //ray->hitMesh->preShade(ray);

    for (SLint i = 0; i<s->lights().size(); ++i)
    {
        SLLight* light = s->lights()[i];

        if (light && light->on())
        {
            // calculate light vector L and distance to light
            N.set(ray->hitNormal);
            L.sub(light->positionWS(), ray->hitPoint);
            lightDist = L.length();
            L /= lightDist;
            LdN = L.dot(N);

            // check shadow ray if hit point is towards the light
            lighted = (LdN>0) ? light->shadowTest(ray, L, lightDist) : 0;

            // calculate the ambient part
            amdi = light->ambient() & mat->ambient();
            spec.set(0, 0, 0);

            // calculate spot effect if light is a spotlight
            if (lighted > 0.0f && light->spotCutoff() < 180.0f)
            {
                SLfloat LdS = SL_max(-L.dot(light->spotDirWS()), 0.0f);

                // check if point is in spot cone
                if (LdS > light->spotCosCut())
                {
                    spotEffect = pow(LdS, (SLfloat)light->spotExponent());
                }
                else
                {
                    lighted = 0.0f;
                    spotEffect = 0.0f;
                }
            }
            else spotEffect = 1.0f;

            // calculate local illumination only if point is not shaded
            if (lighted > 0.0f)
            {
                H.sub(L, ray->dir); // half vector between light & eye
                H.normalize();
                df = SL_max(LdN, 0.0f);           // diffuse factor
                NdH = SL_max(N.dot(H), 0.0f);
                sf = pow(NdH, (SLfloat)mat->shininess()); // specular factor

                amdi += lighted * df * light->diffuse() & mat->diffuse();
                spec = lighted * sf * light->specular()& mat->specular();
            }

            // apply attenuation and spot effect
            att = light->attenuation(lightDist) * spotEffect;
            localColor += att * amdi;
            localSpec += att * spec;
        }
    }

    if (texture.size())
    {
        localColor &= ray->hitTexCol;    // componentwise multiply
        localColor += localSpec;         // add afterwards the specular component
    }
    else localColor += localSpec;
    
    localColor.clampMinMax(0, 1);
    return localColor;
    
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