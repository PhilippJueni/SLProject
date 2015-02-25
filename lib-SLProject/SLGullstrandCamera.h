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
#include "SLRaytracer.h"
#include "SLScene.h"

//-----------------------------------------------------------------------------
class SLGullstrandCamera : public SLCamera, public SLRaytracer
{
public:
    SLGullstrandCamera();

    void        prepareImage();
    void        eyeToPixelRay(SLfloat x, SLfloat y, SLRay* ray);
    SLbool      onMouseWheel(SLint delta, const SLKey mod);
};

//-----------------------------------------------------------------------------
#endif //SLGULLSTRANDCAMERA_H