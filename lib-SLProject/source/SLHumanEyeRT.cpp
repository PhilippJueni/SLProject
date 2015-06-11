//#############################################################################
//  File:      SLHumanEyeRT.cpp
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Copyright: 2002-2014 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>           // precompiled headers
#ifdef SL_MEMLEAKDETECT       // set in SL.h for debug config only
#include <debug_new.h>        // memory leak detector
#endif

#ifdef SL_CPP11
using namespace std::placeholders;
using namespace std::chrono;
#endif

#include <SLRay.h>
#include <SLHumanEyeRT.h>
#include <SLCamera.h>
#include <SLSceneView.h>
#include <SLLightSphere.h>
#include <SLLightRect.h>
#include <SLLight.h>
#include <SLNode.h>
#include <SLMesh.h>
#include <SLText.h>
#include <SLGLTexture.h>
#include <SLSamples2D.h>
#include <SLGLProgram.h>

//-----------------------------------------------------------------------------
SLHumanEyeRT::SLHumanEyeRT()
{  
    name("myCoolRaytracer");
   
    _state = rtReady;
    _distributed = true;
    _maxDepth = 5;
    _aaThreshold = 0.3f; // = 10% color difference
    _aaSamples = 3;
   
    // set texture properies
    _min_filter   = GL_NEAREST;
    _mag_filter   = GL_NEAREST;
    _wrap_s       = GL_CLAMP_TO_EDGE;
    _wrap_t       = GL_CLAMP_TO_EDGE;
    _resizeToPow2 = false;
   
    _numThreads = 1;
    _continuous = false;
    _distributed = true;
}
//-----------------------------------------------------------------------------
SLHumanEyeRT::~SLHumanEyeRT()
{  
    SL_LOG("~SLHumanEyeRT\n");
}
//-----------------------------------------------------------------------------
/*!
This is the main rendering method for the classic ray tracing. It loops over all 
lines and pixels and determines for each pixel a color with a partly global 
illumination calculation.
*/
SLbool SLHumanEyeRT::renderClassic(SLSceneView* sv)
{
    _sv = sv;
    _state = rtBusy;                    // From here we state the RT as busy
    _stateGL = SLGLState::getInstance();// OpenGL state shortcut
    _numThreads = 1;                    // No. of threads
    _pcRendered = 0;                    // % rendered
    _renderSec = 0.0f;                  // reset time
    _infoText  = SLScene::current->info(_sv)->text();  // keep original info string
    _infoColor = SLScene::current->info(_sv)->color(); // keep original info color

    initStats(_maxDepth);               // init statistics
    prepareImage();                     // Setup image & precalculations

    // Measure time 
    double t1 = SLScene::current->timeSec();
    double tStart = t1;

    for (SLuint y = 0/*240*/; y < _img[0].height(); ++y)
    {
        for (SLuint x = 0/*320*/; x < _img[0].width(); ++x)
        {
       
            SLRay primaryRay;
            setPrimaryRay((SLfloat)x, (SLfloat)y, &primaryRay);

            SLCol4f color;
            if (primaryRay.type != BLOCKED)
            {
                //////////////////////////////////////////
                color = traceClassic(&primaryRay);
                //////////////////////////////////////////
            }
            else
            {   //set color black to create fisheye
                color.set(0,0,0);
            }
            _img[0].setPixeliRGB(x, y, color);

            SLRay::avgDepth += SLRay::depthReached;
            SLRay::maxDepthReached = SL_max(SLRay::depthReached, SLRay::maxDepthReached);
        }

        // Update image after 500 ms
        double t2 = SLScene::current->timeSec();
        if (t2-t1 > 0.5)
        {   _pcRendered = (SLint)((SLfloat)y/(SLfloat)_img[0].height()*100);
            _sv->onWndUpdate();
            t1 = SLScene::current->timeSec();
        }
    }

    _renderSec = (SLfloat)(SLScene::current->timeSec() - tStart);
    _pcRendered = 100;

    if (_continuous)
        _state = rtReady;
    else
    {   _state = rtFinished;
        printStats(_renderSec);
    }
    return true;
}

/*!
This method is the classic recursive ray tracing method that checks the scene
for intersection. If the ray hits an object the local color is calculated and
if the material is reflective and/or transparent new rays are created and
passed to this trace method again. If no object got intersected the
background color is return.
*/
SLCol4f SLHumanEyeRT::traceClassic(SLRay* ray)
{
    SLScene* s = SLScene::current;
    SLCol4f color(s->backColor());

    s->root3D()->hitRec(ray);

    if (ray->length < FLT_MAX)
    {
        color = shade(ray);

        if (ray->depth < SLRay::maxDepth && ray->contrib > SLRay::minContrib)
        {
            if (ray->hitMat->kt())
            {   SLRay refracted;
                ray->refractHE(&refracted);
                SLfloat kt = ray->hitMat->kt();         
                SLCol4f tC = traceClassic(&refracted);  
                color += kt * tC;                       
            }
            if (ray->hitMat->kr())
            {   SLRay reflected;
                ray->reflect(&reflected);
                color += ray->hitMat->kr() * traceClassic(&reflected);
            }
        }
    }

    color.clampMinMax(0,1);
    return color;
}
//-----------------------------------------------------------------------------
//! Set the parameters of a primary ray for a pixel position at x, y.
void SLHumanEyeRT::setPrimaryRay(SLfloat x, SLfloat y, SLRay* primaryRay)
{   
    primaryRay->x = x;
    primaryRay->y = y;

    // calculate ray from eye to pixel (See also prepareImage())
    if (_cam->projection() == monoOrthographic)
    {   primaryRay->setDir(_LA);
        primaryRay->origin = _BL + _pxSize*((SLfloat)x*_LR + (SLfloat)y*_LU);
    } else
    {   
        SLVec3f primaryDir(_BL + _pxSize*((SLfloat)x*_LR + (SLfloat)y*_LU));
        primaryDir.normalize();
        primaryRay->setDir(primaryDir);
        primaryRay->origin = _cam->position();

        // in gullstrand camera, the primaryRay goes through eye surfaces
        _cam->generateCameraRay(primaryRay,_sv);
    }
}
//-----------------------------------------------------------------------------
/*!
This method calculates the local illumination at the rays intersection point. 
It uses the OpenGL local light model where the color is calculated as 
follows:
color = material emission + 
        global ambient light scaled by the material's ambient color + 
        ambient, diffuse, and specular contributions from all lights, 
        properly attenuated
*/
SLCol4f SLHumanEyeRT::shade(SLRay* ray)
{  
    SLScene*    s = SLScene::current;
    SLCol4f     localColor = SLCol4f::BLACK;
    SLMaterial* mat = ray->hitMat;
    SLVGLTexture& texture = mat->textures();
    SLVec3f     L,N,H;
    SLfloat     lightDist, LdN, NdH, df, sf, spotEffect, att, lighted = 0.0f;
    SLCol4f     amdi, spec;
    SLCol4f     localSpec(0,0,0,1);
   
    // Don't shade lights. Only take emissive color as material 
    if (typeid(*ray->hitNode)==typeid(SLLightSphere) || 
        typeid(*ray->hitNode)==typeid(SLLightRect))
    {   localColor = mat->emission();
        return localColor;
    } 

    localColor = mat->emission() + (mat->ambient()&s->globalAmbiLight());
  
    ray->hitMesh->preShade(ray);
      
    for (SLint i=0; i<s->lights().size(); ++i) 
    {  SLLight* light = s->lights()[i];
   
        if (light && light->on())
        {              
            // calculate light vector L and distance to light
            N.set(ray->hitNormal);
            L.sub(light->positionWS(), ray->hitPoint);
            lightDist = L.length();
            L/=lightDist; 
            LdN = L.dot(N);

            // check shadow ray if hit point is towards the light
            lighted = (LdN>0) ? light->shadowTest(ray, L, lightDist) : 0;
         
            // calculate the ambient part
            amdi = light->ambient() & mat->ambient();
            spec.set(0,0,0);
      
            // calculate spot effect if light is a spotlight
            if (lighted > 0.0f && light->spotCutoff() < 180.0f)
            {  SLfloat LdS = SL_max(-L.dot(light->spotDirWS()), 0.0f);
         
            // check if point is in spot cone
            if (LdS > light->spotCosCut())
            {  spotEffect = pow(LdS, (SLfloat)light->spotExponent());
            } else 
            {   lighted = 0.0f;
                spotEffect = 0.0f;
            }
            } else spotEffect = 1.0f;
         
            // calculate local illumination only if point is not shaded
            if (lighted > 0.0f) 
            {   H.sub(L,ray->dir); // half vector between light & eye
                H.normalize();
                df   = SL_max(LdN     , 0.0f);           // diffuse factor
                NdH  = SL_max(N.dot(H), 0.0f);
                sf = pow(NdH, (SLfloat)mat->shininess()); // specular factor
         
                amdi += lighted * df * light->diffuse() & mat->diffuse();
                spec  = lighted * sf * light->specular()& mat->specular();
            }
      
            // apply attenuation and spot effect
            att = light->attenuation(lightDist) * spotEffect;
            localColor += att * amdi;
            localSpec  += att * spec;
        }
    }

    if (texture.size()) 
    {   localColor &= ray->hitTexCol;    // componentwise multiply
        localColor += localSpec;         // add afterwards the specular component
    } else localColor += localSpec; 
         
    localColor.clampMinMax(0, 1); 
    return localColor;  
}
//-----------------------------------------------------------------------------
/*! 
fogBlend: Blends the a fog color to the passed color according to to OpenGL fog 
calculation. See OpenGL docs for more information on fog properties.
*/
SLCol4f SLHumanEyeRT::fogBlend(SLfloat z, SLCol4f color)
{  
    SLfloat f=0.0f;
    if (z > _sv->_camera->clipFar()) z = _sv->_camera->clipFar();
    switch (_stateGL->fogMode)
    {   case 0:  f = (_stateGL->fogDistEnd-z)/
                     (_stateGL->fogDistEnd-_stateGL->fogDistStart); break;
        case 1:  f = exp(-_stateGL->fogDensity*z); break;
        default: f = exp(-_stateGL->fogDensity*z*_stateGL->fogDensity*z); break;
    }
    color = f*color + (1-f)*_stateGL->fogColor;
    color.clampMinMax(0, 1);
    return color;   
}
//-----------------------------------------------------------------------------
/*!
Initialises the statistic variables in SLRay to zero
*/
void SLHumanEyeRT::initStats(SLint depth)
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
//-----------------------------------------------------------------------------
/*! 
Prints some statistics after the rendering
*/
void SLHumanEyeRT::printStats(SLfloat sec)
{
    SL_LOG("\nRender time  : %10.2f sec.", sec);
    SL_LOG("\nImage size   : %10d x %d",_img[0].width(), _img[0].height());
    SL_LOG("\nNum. Threads : %10d", _numThreads);
    SL_LOG("\nAllowed depth: %10d", SLRay::maxDepth);

    #if _DEBUG
    SLint  primarys = _sv->scrW()*_sv->scrH();
    SLuint total = primarys +
                   SLRay::reflectedRays +
                   SLRay::subsampledRays +
                   SLRay::refractedRays +
                   SLRay::shadowRays;

    SL_LOG("\nMaximum depth     : %10d", SLRay::maxDepthReached);
    SL_LOG("\nAverage depth     : %10.6f", SLRay::avgDepth/primarys);
    SL_LOG("\nAA threshold      : %10.1f", _aaThreshold);
    SL_LOG("\nAA subsampling    : %8dx%d\n", _aaSamples, _aaSamples);
    SL_LOG("\nSubsampled pixels : %10u, %4.1f%% of total", SLRay::subsampledPixels,  
            (SLfloat)SLRay::subsampledPixels/primarys*100.0f);   
    SL_LOG("\nPrimary rays      : %10u, %4.1f%% of total", primarys,               
            (SLfloat)primarys/total*100.0f);
    SL_LOG("\nReflected rays    : %10u, %4.1f%% of total", SLRay::reflectedRays,   
            (SLfloat)SLRay::reflectedRays/total*100.0f);
    SL_LOG("\nTransmitted rays  : %10u, %4.1f%% of total", SLRay::refractedRays, 
            (SLfloat)SLRay::refractedRays/total*100.0f);
    SL_LOG("\nTIR rays          : %10u, %4.1f%% of total", SLRay::tirRays,         
            (SLfloat)SLRay::tirRays/total*100.0f);
    SL_LOG("\nShadow rays       : %10u, %4.1f%% of total", SLRay::shadowRays,      
            (SLfloat)SLRay::shadowRays/total*100.0f);
    SL_LOG("\nAA subsampled rays: %10u, %4.1f%% of total", SLRay::subsampledRays,  
            (SLfloat)SLRay::subsampledRays/total*100.0f);
    SL_LOG("\nTotal rays        : %10u,100.0%%\n", total);
   
    SL_LOG("\nRays per second   : %10u", (SLuint)(total / sec));
    SL_LOG("\nIntersection tests: %10u", SLRay::tests);
    SL_LOG("\nIntersections     : %10u, %4.1f%%", SLRay::intersections, 
            SLRay::intersections/(SLfloat)SLRay::tests*100.0f);
    #endif
    SL_LOG("\n\n");
}
//-----------------------------------------------------------------------------
/*!
Creates the inherited image in the texture class. The RT is drawn into
a texture map that is displayed with OpenGL in 2D-orthographic projection.
Also precalculate as much as possible.
*/
void SLHumanEyeRT::prepareImage()
{
    ///////////////////////
    //  PRECALCULATIONS  //
    ///////////////////////

    _cam = _sv->_camera;                // camera shortcut

    // get camera vectors eye, lookAt, lookUp
    _cam->updateAndGetVM().lookAt(&_EYE, &_LA, &_LU, &_LR);

    if (_cam->projection() == monoOrthographic)
    {   /*
        In orthographic projection the bottom-left vector (_BL) points
        from the eye to the center of the bottom-left pixel of a plane that
        parallel to the projection plan at zero distance from the eye.
        */
        SLVec3f pos(_cam->updateAndGetVM().translation());
        SLfloat hh = tan(SL_DEG2RAD*_cam->fov()*0.5f) * pos.length();
        SLfloat hw = hh * _sv->scrWdivH();

        // calculate the size of a pixel in world coords.
        _pxSize = hw * 2 / _sv->scrW();

        _BL = _EYE - hw*_LR - hh*_LU  +  _pxSize/2*_LR - _pxSize/2*_LU;
    }
    else
    {   /* 
        In perspective projection the bottom-left vector (_BL) points
        from the eye to the center of the bottom-left pixel on a projection
        plan in focal distance. See also the computergraphics script about
        primary ray calculation.
        */
        // calculate half window width & height in world coords
        SLfloat hh = tan(SL_DEG2RAD*_cam->fov()*0.5f) * _cam->focalDist();
        SLfloat hw = hh * _sv->scrWdivH();

        // calculate the size of a pixel in world coords.
        _pxSize = hw * 2 / _sv->scrW();

        // calculate a vector to the center (C) of the bottom left (BL) pixel
        SLVec3f C  = _LA * _cam->focalDist();
        _BL = C - hw*_LR - hh*_LU  +  _pxSize/2*_LR + _pxSize/2*_LU;
    }

    // Allocate image of the inherited texture class 
    if (_sv->scrW() != _img[0].width() || _sv->scrH() != _img[0].height())
    {  
        // Delete the OpenGL Texture if it allready exists
        if (_texName) 
        {   glDeleteTextures(1, &_texName);
            //SL_LOG("glDeleteTextures id: %u   ", _texName);
            _texName = 0;
        }

        // Dispose VBO is they allready exist
        _bufP.dispose();
        _bufT.dispose();
        _bufI.dispose();

        _img[0].allocate(_sv->scrW(), _sv->scrH(), GL_RGB);
    }
   
    // Fill image black for single RT
    if (!_continuous) _img[0].fill();
}
//-----------------------------------------------------------------------------
/*! 
Draw the RT-Image as a textured quad in 2D-Orthographic projection
*/
void SLHumanEyeRT::renderImage()
{
    SLfloat w = (SLfloat)_sv->scrW();
    SLfloat h = (SLfloat)_sv->scrH();
    if (w != _img[0].width()) return;
    if (h != _img[0].height()) return;
      
    // Set orthographic projection with the size of the window
    _stateGL->projectionMatrix.ortho(0.0f, w, 0.0f, h, -1.0f, 0.0f);
    _stateGL->modelViewMatrix.identity();
    _stateGL->clearColorBuffer();
    _stateGL->depthTest(false);
    _stateGL->multiSample(false);
    _stateGL->polygonLine(false);

    drawSprite(true);
   
    // Write progress into info text
    if (_pcRendered < 100)
    {  SLchar str[255];  
        sprintf(str,"%s Tracing: Threads: %d, Progess: %d%%", 
                _infoText.c_str(), _numThreads, _pcRendered);
        SLScene::current->info(_sv, str, _infoColor);
    } else SLScene::current->info(_sv, _infoText.c_str(), _infoColor);

    _stateGL->depthTest(true);
    GET_GL_ERROR;
}
//-----------------------------------------------------------------------------
//! Saves the current RT image as PNG image
void SLHumanEyeRT::saveImage()
{
    static SLint no = 0;
    SLchar filename[255];  
    sprintf(filename,"Raytraced_%d_%d.png", _maxDepth, no++);
    _img[0].savePNG(filename);
}
//-----------------------------------------------------------------------------
