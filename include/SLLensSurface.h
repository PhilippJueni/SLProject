//#############################################################################
//  File:      SLLensSurface.h
//  Author:    Philipp Jüni
//  Date:      October 2014
//  Copyright: 2002-2014 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLLENSSURFACE_H
#define SLLENSSURFACE_H

#include <stdafx.h>
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
class SLLensSurface : public SLRevolver
{
public:
    //! Create a lense with given sphere, cylinder, diameter and thickness
    SLLensSurface(SLfloat diameter,
        SLfloat knBot = 1.0f,
        SLfloat knTop = 1.0f,
        SLint stacks = 32,
        SLint slices = 32,
        SLstring name = "LensSurface");

    SLfloat addBottom(SLfloat radius);

    SLfloat addTop(SLfloat radius);

    SLfloat addPlane(SLfloat xStart, SLfloat yStart);

    void setSpaceBetweenSurfaces(SLfloat surfaceSpace);

    ~SLLensSurface() { ; }

private:
    SLfloat generateLensSurface(SLfloat radius,
        SLfloat xStart,
        SLfloat yStart,
        SLfloat yTranslate,
        SLfloat startAngle);

    SLfloat calcAngle(SLfloat radius);

    SLfloat calcSagitta(SLfloat radius);

    SLint   _stacks;        //!< NO. of stacks 
    SLfloat _diameter;      //!< The diameter of the lens
    SLfloat _thickness;     //!< The space between the primary planes of lens sides

    SLfloat _knBot;
    SLfloat _knTop;
    SLMaterial *_mat;


};
//-----------------------------------------------------------------------------
#endif //SLLENSSURFACE_H