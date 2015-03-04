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

//-----------------------------------------------------------------------------
class SLGullstrandCamera : public SLCamera
{
public:
    SLGullstrandCamera();

    void        eyeToPixelRay(SLfloat x, SLfloat y, SLRay* ray);
    void        drawMeshes(SLSceneView* sv);
    void        addLens(SLNode* node,SLfloat position=0);
};

//-----------------------------------------------------------------------------
#endif //SLGULLSTRANDCAMERA_H