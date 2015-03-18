//#############################################################################
//  File:      SLGullstrandCamera.h
//  Author:    Philipp J�ni
//  Date:      October 2014
//  Copyright: 2002-2014 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLGULLSTRANDCAMERA_H
#define SLGULLSTRANDCAMERA_H

#include <stdafx.h>
#include "SLCamera.h"
#include "SLSurface.h"

typedef std::vector<SLSurface*>  SLVSurface;

//-----------------------------------------------------------------------------
class SLGullstrandCamera : public SLCamera, public SLGLTexture
{
public:
    SLGullstrandCamera();

    void        addLens(SLNode* node, SLfloat position = 0);
    void        addSurface(SLSurface* surf, SLfloat position = 0);
    void        addLSurface(SLSurface* surf, SLfloat position = 0);

    void        renderClassic(SLSceneView* sv);
    void        initStats(SLint depth);
    void        prepareImage();

    void        drawMeshes(SLSceneView* sv);

    void        setPrimaryRay(SLfloat x, SLfloat y, SLRay* primaryRay);
    
    SLbool hitRec(SLRay *ray);
      
    SLCol4f traceClassic(SLRay* ray);
    SLCol4f shade(SLRay* ray);
   
private:
    SLSceneView* _sv;
    SLImage     _img[6];        //!< max 6 images for cube map
    SLuint      _texName;       //!< OpenGL texture "name" (= ID)
    SLfloat     _pxSize;        //!< Pixel size
    SLVec3f     _EYE;           //!< Camera position
    SLVec3f     _LA, _LU, _LR;  //!< Camera lookat, lookup, lookright
    
    SLbool      _continuous;    //!< if true state goes into ready again
    SLVec3f     _BL;            //!< Bottom left vector
    SLuint      _next;          //!< next index to render RT

    SLVSurface      _surfaces;         //!< vector of children surfaces

    
};

//-----------------------------------------------------------------------------
#endif //SLGULLSTRANDCAMERA_H