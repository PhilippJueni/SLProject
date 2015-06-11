//#############################################################################
//  File:      SLGullstrandCamera.h
//  Author:    Philipp Jüni
//  Date:      June 2015
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

//! Defines the size of the image plane.
// STANDARD, FULL_FISHEYE, CORNER_FISCHEYE
enum SLGullstrandCameraType { STANDARD = 0, FULL_FISHEYE = 1, CORNER_FISHEYE = 2 };

//-----------------------------------------------------------------------------
//! Active or visible camera node class, simulation a Gullstrand-Eye
/*! An instance of this SLNode derived class serves as an active camera
node with camera body and its view frustum. 
Because the SLNode class is inherited from the abstract SLEventHandler class a
camera can handle mouse & keyboard event. All camera animations are handled in
these eventhandlers.
These class contains childnotes to handle in preparation of the primaryray
for the HERT. 
*/
class SLGullstrandCamera : public SLCamera, public SLGLTexture
{
public:
    SLGullstrandCamera( SLfloat retinaRadius, 
                        SLfloat fieldOfViewDEG, 
                        SLfloat nEyeWater, 
                        SLSceneView* sv,
                        SLGullstrandCameraType cameraType=STANDARD);
    void        addSurface(SLSphericalRefractionSurface* surface, SLfloat position = 0);
    
    void        generateCameraRay(SLRay* primaryRay, SLSceneView* sv);   

private:
    SLfloat _eyeSize = 24.0f;                                //!< the size of the eye globe
    SLfloat _imagePlaneGap = 2.0f;                           //!< the gap between retina and image plane

    std::vector<SLSphericalRefractionSurface*>  _surfaces;   //!< vector of children surfaces
    std::vector<SLNode*>  _surfNodes;                        //!< vector of children nodes
    SLNode* _retinaNode;                                     //!< the retina node
    SLNode *_rectNode;                                       //!< the image plane node
    SLSphericalRefractionSurface* _retina;                   //!< the retina surface
    SLRectangle* _imageRectangle;                            //!< the image plane rectangle
    SLfloat _hWidth;                                         //!< half width of the image plane
    SLfloat _hHeight;                                        //!< half height of the image plane
    SLfloat _pxSize;                                         //!< size of a pixel
    SLfloat _cameraPosition;                                 //!< the position of the camera

    void cameraHERT(SLRay *ray);    
};

//-----------------------------------------------------------------------------
#endif //SLGULLSTRANDCAMERA_H