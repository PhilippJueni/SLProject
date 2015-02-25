//#############################################################################
//  File:      SLLens.cpp
//  Author:    Philipp Jüni
//  Date:      October 2014
//  Copyright: 2002-2014 Marcus Hudritsch, Philipp Jüni
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>           // precompiled headers
#include <SLGullstrandCamera.h>
#include <SLLens.h>
#include <SLSphere.h>
//-----------------------------------------------------------------------------


SLGullstrandCamera::SLGullstrandCamera()
{
    SLMaterial* matLens = new SLMaterial("lens", SLCol4f(0.0f, 0.0f, 0.0f), SLCol4f(0.5f, 0.5f, 0.5f), 100, 0.5f, 0.5f, 1.5f);
    SLMaterial* matEye = new SLMaterial("lens", SLCol4f(0.0f, 0.0f, 0.2f), SLCol4f(0.5f, 0.5f, 0.5f), 100, 0.0f, 0.9f, 0.0f);
    SLNode* eye = new SLNode(new SLSphere(1.5f,32,32,"eye",matEye));
    eye->rotate(90, 1, 0, 0, TS_Local);
    eye->translate(0, 1.5, 0, TS_Local);
    SLNode* lens = new SLNode(new SLLens(2.0f, 2.0f, 1.8f, 0.1f, 32, 32, "lens", matLens));
    lens->rotate(90, 1, 0, 0, TS_Local);
    lens->translate(0, 1.0, 0, TS_Local);
    SLNode* cornea = new SLNode(new SLLens(2.0f, -2.0f, 2.2f, 0.0f, 32, 32, "cornea", matLens));
    cornea->rotate(90, 1, 0, 0, TS_Local);
    cornea->translate(0, 0.3, 0, TS_Local);

    addChild(eye);
    addChild(lens);
    addChild(cornea);

    cout << "gullstrandcameraKonstruktor" << endl;

    prepareImage();
    
}



void SLGullstrandCamera::eyeToPixelRay(SLfloat x, SLfloat y, SLRay* ray)
{
    cout << "eyeToPicelRay 1" << endl;
    SLCamera::eyeToPixelRay(x, y, ray);
    cout << "eyeToPicelRay 2" << endl;

   
}

void SLGullstrandCamera::prepareImage()
{
    cout << "preapare image gullstrand" << endl;
    
}

SLbool SLGullstrandCamera::onMouseWheel(const SLint delta, const SLKey mod)
{
    cout << "Mouse wheel" << endl;
    return SLCamera::onMouseWheel(delta, mod);
}