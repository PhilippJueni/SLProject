//#############################################################################
//  File:      SLSphericalRefractionSurface.cpp
//  Author:    Philipp Jüni
//  Date:      Mai 2015
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: 2002-2014 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLSPHERICALREFRACTIONSURFACE_H
#define SLSPHERICALREFRACTIONSURFACE_H

#include <stdafx.h>
#include "SLMesh.h"
#include "SLRefractionSurface.h"
#include "SLRevolver.h"

//-----------------------------------------------------------------------------
//! SLLens creates a lens mesh based on SLRevolver
/*!
SLLens creates a lens mesh based on SLRevolver.
Different constructors allow to either create the lens from the values
written in the eyeglass prescription card or from the radius of the lens.<br>
<br>
<b>Lens types:</b>
\image html LensTypes.png
<b>Myopia</b> ( http://en.wikipedia.org/wiki/Myopia )<br>
The eye is too long for its optical power.<br>
To correct myopic (short-sightedness) a diverging lens is needed.
\image html Myopia.png
<b>Hypermetropia</b> ( http://en.wikipedia.org/wiki/Hyperopia )<br>
The eye is too short for its optical power.<br>
To correct presbyopic (far-sightedness) a converging lens is needed.
\image html Hypermetropia.png
*/
class SLSphericalRefractionSurface : public SLRefractionSurface, public SLRevolver
{
public:
    SLSphericalRefractionSurface(   SLfloat diameter,
                                    SLfloat radius = 1.0f,
                                    SLint stacks = 32,
                                    SLint slices = 32,
                                    SLMaterial* mat = 0,
                                    SLstring name = "SphericalRefractionSurface");

    // saves the position in gullstrand camera when the surface is added
    void setPosition(SLVec3f pos){ _position = pos; };
    
    SLVec3f getRandomPoint();

    ~SLSphericalRefractionSurface() { ; }

    

private:
    void generateLensSurface( SLfloat startAngle );

    void generatePlane(SLfloat xStart);

    SLfloat calcAngle();

    SLfloat calcSagitta();

    SLint   _stacks;        //!< NO. of stacks 
    SLfloat _diameter;      //!< The diameter of the lens
    SLfloat _radius;        //!< radius of the spherical surface
    SLVec3f _position;      //!< position in the gullstrand-camera
};
//-----------------------------------------------------------------------------
#endif