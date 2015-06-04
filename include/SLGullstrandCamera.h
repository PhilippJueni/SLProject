//#############################################################################
//  File:      SLGullstrandCamera.h
//  Author:    Philipp Jüni
//  Date:      October 2014
//  Copyright: 2002-2014 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLGULLSTRANDCAMERA_H
#define SLGULLSTRANDCAMERA_H

#include <stdafx.h>
#include "SLCamera.h"
#include "SLSphericalRefractionSurface.h"
#include "SLRectangle.h"
#include "SLTriangle.h"

//#include "SLSurface.h"
//typedef std::vector<SLSurface*>  SLVSurface;

//-----------------------------------------------------------------------------
class SLGullstrandCamera : public SLCamera, public SLGLTexture
{
public:
    SLGullstrandCamera(SLfloat retinaRadius, SLfloat fieldOfViewDEG, SLfloat nEyeWater);

   
    void        addSurface(SLSphericalRefractionSurface* mesh, SLfloat position = 0);

    void        drawMeshes(SLSceneView* sv);
    void        generateCameraRay(SLRay* primaryRay, SLVec3f lb, SLVec3f lr, SLVec3f lu, SLfloat pxSize );

    // RT
    /*
    void        renderClassic(SLSceneView* sv);
    void        initStats(SLint depth);
    void        prepareImage();
    void        setPrimaryRay(SLfloat x, SLfloat y, SLRay* primaryRay);    
    SLbool hitRec(SLRay *ray);      
    SLCol4f traceClassic(SLRay* ray);
    SLCol4f shade(SLRay* ray);
    */

    void refract(SLRay* primaryRay, SLSphericalRefractionSurface* surface);


    //void    generateCameraRay(SLfloat x, SLfloat y, SLRay* primaryRay);
    
   
private:
    // RT 
    /*
    SLSceneView* _sv;
    SLImage     _img[6];        //!< max 6 images for cube map
    SLuint      _texName;       //!< OpenGL texture "name" (= ID)
    SLfloat     _pxSize;        //!< Pixel size
    SLVec3f     _EYE;           //!< Camera position
    SLVec3f     _LA, _LU, _LR;  //!< Camera lookat, lookup, lookright
    
    SLbool      _continuous;    //!< if true state goes into ready again
    SLVec3f     _BL;            //!< Bottom left vector
    SLuint      _next;          //!< next index to render RT
    */

    SLfloat _eyeSize = 24.0f;

    std::vector<SLSphericalRefractionSurface*>  _surfaces;         //!< vector of children surfaces
    std::vector<SLNode*>  _surfNodes;         //!< vector of children surfaces

    SLNode* _retinaNode;
    SLSphericalRefractionSurface* _retina;

    //SLTriangle* _retina;
    SLRectangle* _imageRectangle;
    SLfloat _hWidth;
    SLfloat _hHeight;
    SLfloat _pxSize;
    SLfloat _imagePlaneGap;

    SLVec3f transferCoords(SLfloat x, SLfloat y);

    void startRT(SLRay *ray);
    void surfaceRT(SLRay *ray, int i);
    
    
};

//-----------------------------------------------------------------------------
#endif //SLGULLSTRANDCAMERA_H