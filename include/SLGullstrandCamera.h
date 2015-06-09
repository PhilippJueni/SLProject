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

enum SLGullstrandCameraType { STANDARD = 0, FULL_FISHEYE = 1, CORNER_FISHEYE = 2 };

//-----------------------------------------------------------------------------
class SLGullstrandCamera : public SLCamera, public SLGLTexture
{
public:
    SLGullstrandCamera( SLfloat retinaRadius, 
                        SLfloat fieldOfViewDEG, 
                        SLfloat nEyeWater, 
                        SLGullstrandCameraType cameraType=STANDARD);
    void        addSurface(SLSphericalRefractionSurface* surface, SLfloat position = 0);
    void        drawMeshes(SLSceneView* sv);
    void        generateCameraRay(SLRay* primaryRay, SLVec3f lb, SLVec3f lr, SLVec3f lu, SLfloat pxSize );
    void refract(SLRay* primaryRay, SLSphericalRefractionSurface* surface);
   
private:
    SLfloat _eyeSize = 24.0f;
    SLfloat _imgWidth = 640;
    SLfloat _imgHeight = 480;
    SLfloat _imagePlaneGap = 2.0f;

    std::vector<SLSphericalRefractionSurface*>  _surfaces;      //!< vector of children surfaces
    std::vector<SLNode*>  _surfNodes;                           //!< vector of children nodes
    SLNode* _retinaNode;
    SLNode *_rectNode;
    SLSphericalRefractionSurface* _retina;
    SLRectangle* _imageRectangle;
    SLfloat _hWidth;
    SLfloat _hHeight;
    SLfloat _pxSize;
    SLfloat _cameraPosition;

    //SLVec3f transferCoords(SLfloat x, SLfloat y);
    void cameraHERT(SLRay *ray);    
};

//-----------------------------------------------------------------------------
#endif //SLGULLSTRANDCAMERA_H