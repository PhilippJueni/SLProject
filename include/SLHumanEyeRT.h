//#############################################################################
//  File:      SLHumanEyeRT.h
//  Author:    Philipp J�ni
//  Date:      June 2015
//  Copyright: 2002-2014 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLHUMANEYERT_H
#define SLHUMANEYERT_H

#include <stdafx.h>
#include <SLGLTexture.h>
#include <SLGLBuffer.h>
#include <SLEventHandler.h>
#include <SLRaytracer.h>

class SLScene;
class SLSceneView;
class SLRay;
class SLMaterial;
class SLCamera;

//-----------------------------------------------------------------------------
//! SLHumanEyeRT hold all the methods for Whitted style Ray Tracing.
/*!      
This class is similar to the SLRaytracer Class
SLRaytracer implements the methods render, eyeToPixel, trace and shade for
classic Whitted style Ray Tracing. This class is a friend class of SLScene and
can access via the pointer _s all members of SLScene. The scene traversal for
the ray intersection tests is done within the intersection method of all nodes. 
*/
class SLHumanEyeRT: public SLGLTexture, public SLEventHandler
{
    public:
                        SLHumanEyeRT ();
            virtual    ~SLHumanEyeRT ();
            
            // ray tracer functions
            SLbool      renderClassic   (SLSceneView* sv);
            SLCol4f     traceClassic    (SLRay* ray);
            SLCol4f     shade           (SLRay* ray);
            
            // additional ray tracer functions
            void        setPrimaryRay   (SLfloat x, SLfloat y, SLRay* primaryRay); 
            SLCol4f     fogBlend        (SLfloat z, SLCol4f color);
            void        printStats      (SLfloat sec);
            void        initStats       (SLint depth);
            
            // Setters
            void        state           (SLRTState state) {if (_state!=rtBusy) _state=state;}
            void        maxDepth        (SLint depth)     {_maxDepth = depth; state(rtReady);}
            void        distributed     (SLbool distrib)  {_distributed = distrib;}
            void        continuous      (SLbool cont)     {_continuous = cont; state(rtReady);}
            void        aaSamples       (SLint samples)   {_aaSamples = samples; state(rtReady);}
            
            // Getters
            SLRTState   state           () {return _state;}
            SLint       maxDepth        () {return _maxDepth;}
            SLbool      distributed     () {return _distributed;}
            SLbool      continuous      () {return _continuous;}
            SLint       aaSamples       () {return _aaSamples;}
            SLint       numThreads      () {return _numThreads;}
            SLint       pcRendered      () {return _pcRendered;}
            SLfloat     aaThreshold     () {return _aaThreshold;}
            SLfloat     renderSec       () {return _renderSec;}
            
            // Render target image
            void        prepareImage    ();
            void        renderImage     ();
            void        saveImage       ();
                        
   protected:
            SLSceneView* _sv;           //!< Parent sceneview
            SLGLState*  _stateGL;       //!< Pointer to the global state
            SLRTState   _state;         //!< RT state;
            SLCamera*   _cam;           //!< shortcut to the camera
            SLint       _maxDepth;      //!< Max. allowed recursion depth
            SLbool      _continuous;    //!< if true state goes into ready again
            SLbool      _distributed;   //!< Flag for parallel distributed RT
            SLint       _pcRendered;    //!< % rendered
            SLfloat     _renderSec;     //!< Rendering time in seconds
            SLstring    _infoText;      //!< Original info string
            SLCol4f     _infoColor;     //!< Original info string color

            SLfloat     _pxSize;        //!< Pixel size
            SLVec3f     _EYE;           //!< Camera position
            SLVec3f     _LA, _LU, _LR;  //!< Camera lookat, lookup, lookright
            SLVec3f     _BL;            //!< Bottom left vector
            SLuint      _next;          //!< next index to render RT
            SLVPixel    _aaPixels;      //!< Vector for antialiasing pixels

            // variables for distributed ray tracing
            SLfloat     _aaThreshold;   //!< threshold for anti aliasing
            SLint       _aaSamples;     //!< SQRT of uneven num. of AA samples
            SLint       _numThreads;    //!< Num. of threads used for RT
};
//-----------------------------------------------------------------------------
#endif
