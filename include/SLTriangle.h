//#############################################################################
//  File:      SLTriangle.cpp
//  Author:    Philipp Jüni
//  Date:      June 2015
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: 2002-2014 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLTRIANGLE_H
#define SLTRIANGLE_H

#include "SLMesh.h"

//-----------------------------------------------------------------------------
//! Creates a triangle mesh wit 3 vertexes
/*!
Creates the simplest possible mesh.
*/
class SLTriangle : public SLMesh
{
public:
    SLTriangle( SLMaterial *mat,
                SLstring name = "Triangle",
                SLVec3f v0 = SLVec3f(0, 0, 0),
                SLVec3f v1 = SLVec3f(1, 0, 0),
                SLVec3f v2 = SLVec3f(0, 1, 0),
                SLVec2f t0 = SLVec2f(0, 0),
                SLVec2f t1 = SLVec2f(1, 0),
                SLVec2f t2 = SLVec2f(0, 1));

    void        buildMesh(SLMaterial* mat);

protected:
    SLVec3f    v[3];  //!< Array of vertex positions
    SLVec2f    t[3];  //!< Array of vertex tex. coords. (opt.)

};
//-----------------------------------------------------------------------------
#endif //SLTRIANGLE_H