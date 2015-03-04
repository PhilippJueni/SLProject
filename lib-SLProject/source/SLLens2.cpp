//#############################################################################
//  File:      SLLens.cpp
//  Author:    Philipp Jüni
//  Date:      October 2014
//  Copyright: 2002-2014 Marcus Hudritsch, Philipp Jüni
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>           // precompiled headers
#ifdef SL_MEMLEAKDETECT       // set in SL.h for debug config only
#include <debug_new.h>        // memory leak detector
#endif

using namespace std;
#include <SLMaterial.h>
#include <SLLens2.h>
#include <SLLensSurface.h>

//-----------------------------------------------------------------------------
/*!
SLLens::SLLens ctor for lens revolution object around the y-axis. <br>
Create the lens with the eye prescription card.
\image html EyePrescriptionCard.png
The first values in the prescription card is called Sphere. It is also the
diopter of the front side of the lens. <br>
The second value from the prescription card is called Cylinder. The sum from
the spere and the cylinder is the diopter of the back side of the lens. <br>
The diopter is the inverse of the focal distance (f) of the lens. <br>
To correct myopic, negative diopters are used. <br>
To correct presbyopic, positive diopters are used.<br>
\image html Lens.png

\param sphere SLfloat taken from the eyeglass passport.
\param cylinder SLfloat taken from the eyeglass passport.
\param diameter SLfloat The diameter (h) of the lens
\param thickness SLfloat The space between the primary planes of lens sides (d)
\param stacks SLint
\param slices SLint
\param name SLstring of the SLRevolver Mesh
\param mat SLMaterial* The Material of the lens

The diopter of the front side is equal to the sphere. <br>
The diopter of the backside is the sum of the spehere and the cylinder. <br>
From the diopter, the radius (R1, R2) can be calculated: <br>
radiusFront = (LensMaterial - OutsideMaterial) / FrontDiopter) * diameter;<br>
radiusBack = (OutsideMaterial - LensMaterial) / BackDiopter) * diameter;<br>
*/
SLLens2::SLLens2(double sphere,
    double cylinder,
    SLfloat diameter,
    SLfloat thickness,
    SLint stacks,
    SLint slices,
    SLstring name,
    SLMaterial* mat)
{
    assert(slices >= 3 && "Error: Not enough slices.");
    assert(slices >  0 && "Error: Not enough stacks.");

    // /100 to let the user input values he is used to
    int shrink = 100;

    SLfloat diopterBot = (SLfloat)(sphere / shrink); // D1 = sphere
    SLfloat diopterTop = (SLfloat)((sphere + cylinder) / shrink); // D2 = sphere + cylinder

    init(diopterBot, diopterTop, diameter, thickness, stacks, slices, name, mat);
}

/*!
SLLens::SLLens ctor for lens revolution object around the y-axis. <br>
Create the lens with radius.<br>
To correct presbyopic (far-sightedness) a converging lens is needed.
To correct myopic (short-sightedness) a diverging lens is needed.
\image html Lens.png

\param radiusBot SLfloat The radius of the front side of the lens
\param radiusTop SLfloat The radius of the back side of the lens
\param diameter SLfloat The diameter (h) of the lens
\param thickness SLfloat The space between the primary planes of lens sides (d)
\param stacks SLint
\param slices SLint
\param name SLstring of the SLRevolver Mesh
\param mat SLMaterial* The Material of the lens

Positive radius creates a convex lens side. <br>
Negative radius creates a concave lens side. <br>
Setting the radius to 0 creates a plane. <br>
Combine the two radius to get the required lens.
*/
SLLens2::SLLens2(SLfloat radiusBot,
    SLfloat radiusTop,
    SLfloat diameter,
    SLfloat thickness,
    SLint stacks,
    SLint slices,
    SLstring name,
    SLMaterial* mat)
{
    SLfloat nOut = 1.00;            // kn material outside lens
    SLfloat nLens = mat->kn();      // kn material of the lens
    SLfloat diopterBot = (SLfloat)((nLens - nOut) * diameter / radiusBot);
    SLfloat diopterTop = (SLfloat)((nOut - nLens) * diameter / radiusTop);

    init(diopterBot, diopterTop, diameter, thickness, stacks, slices, name, mat);
}

/*!
\brief Initialize the lens
\param diopterBot SLfloat The diopter of the bot (front) part of the lens
\param diopterTop SLfloat The diopter of the top (back) part of the lens
\param diameter SLfloat The diameter of the lens
\param thickness SLfloat d The space between the primary planes of lens sides
\param stacks SLint
\param slices SLint
\param mat SLMaterial* The Material of the lens
*/
void SLLens2::init(SLfloat diopterBot,
    SLfloat diopterTop,
    SLfloat diameter,
    SLfloat thickness,
    SLint stacks,
    SLint slices,
    SLstring name,
    SLMaterial* mat)
{
    assert(slices >= 3 && "Error: Not enough slices.");
    assert(slices > 0 && "Error: Not enough stacks.");

    // Gullstrand-Formel
    // D = D1 + D2 - delta * D1 * D2
    SLfloat diopter = diopterBot + diopterTop;

    // calc radius
    SLfloat nOut = 1.00;         // kn material outside lens
    SLfloat nLens = mat->kn();   // kn material of the lens
    SLfloat delta = thickness / nLens; // d/n

    // calc radius
    _diameter = diameter;
    _radiusBot = (SLfloat)((nLens - nOut) / diopterBot) * _diameter;
    _radiusTop = (SLfloat)((nOut - nLens) / diopterTop) * _diameter;

    SLLensSurface *lensSurf = new SLLensSurface(_diameter, 1.0f, 1.5f, stacks, slices, name);
    lensSurf->addBottom(_radiusBot);
    lensSurf->setSpaceBetweenSurfaces(thickness);
    lensSurf->addTop(_radiusTop);
    lensSurf->buildMesh(mat);

    addMesh(lensSurf);
}
//-----------------------------------------------------------------------------