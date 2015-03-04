//#############################################################################
//  File:      SLGullstrandCamera.cpp
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
#include <SLSceneView.h>
//-----------------------------------------------------------------------------


SLGullstrandCamera::SLGullstrandCamera()
{

}

void SLGullstrandCamera::addLens(SLNode* node, SLfloat position)
{
    node->rotate(90, 1, 0, 0, TS_Local);
    node->translate(0, position, 0, TS_Local);
    addChild(node);
}


void SLGullstrandCamera::eyeToPixelRay(SLfloat x, SLfloat y, SLRay* ray)
{
    cout << "eyeToPixelRay SLGullstrandCamera" << endl;
    SLCamera::eyeToPixelRay(x, y, ray);
}



void SLGullstrandCamera::drawMeshes(SLSceneView* sv)
{
    if (sv->camera() != this)
    {
        if (_projection == monoOrthographic)
        {
            const SLMat4f& vm = updateAndGetWMI();
            SLVec3f P[17 * 2];
            SLuint  i = 0;
            SLVec3f pos(vm.translation());
            SLfloat t = tan(SL_DEG2RAD*_fov*0.5f) * pos.length();
            SLfloat b = -t;
            SLfloat l = -sv->scrWdivH() * t;
            SLfloat r = -l;

            P[i++].set(0, -10, 0); P[i++].set(0, 10, 0);

            // small line in view direction
            P[i++].set(0, 0, 0); P[i++].set(0, 0, _clipNear * 4);

            // frustum pyramid lines
            P[i++].set(r, t, _clipNear); P[i++].set(r, t, -_clipFar);
            P[i++].set(l, t, _clipNear); P[i++].set(l, t, -_clipFar);
            P[i++].set(l, b, _clipNear); P[i++].set(l, b, -_clipFar);
            P[i++].set(r, b, _clipNear); P[i++].set(r, b, -_clipFar);

            // around far clipping plane
            P[i++].set(r, t, -_clipFar); P[i++].set(r, b, -_clipFar);
            P[i++].set(r, b, -_clipFar); P[i++].set(l, b, -_clipFar);
            P[i++].set(l, b, -_clipFar); P[i++].set(l, t, -_clipFar);
            P[i++].set(l, t, -_clipFar); P[i++].set(r, t, -_clipFar);

            // around projection plane at focal distance
            P[i++].set(r, t, -_focalDist); P[i++].set(r, b, -_focalDist);
            P[i++].set(r, b, -_focalDist); P[i++].set(l, b, -_focalDist);
            P[i++].set(l, b, -_focalDist); P[i++].set(l, t, -_focalDist);
            P[i++].set(l, t, -_focalDist); P[i++].set(r, t, -_focalDist);

            // around near clipping plane
            P[i++].set(r, t, _clipNear); P[i++].set(r, b, _clipNear);
            P[i++].set(r, b, _clipNear); P[i++].set(l, b, _clipNear);
            P[i++].set(l, b, _clipNear); P[i++].set(l, t, _clipNear);
            P[i++].set(l, t, _clipNear); P[i++].set(r, t, _clipNear);

            SLCamera::_bufP.generate(P, i, 3);
        }
        else
        {
            SLVec3f P[17 * 2];
            SLuint  i = 0;
            SLfloat aspect = sv->scrWdivH();
            SLfloat tanFov = tan(_fov*SL_DEG2RAD*0.5f);
            SLfloat tF = tanFov * _clipFar;    //top far
            SLfloat rF = tF * aspect;          //right far
            SLfloat lF = -rF;                   //left far
            SLfloat tP = tanFov * _focalDist;  //top projection at focal distance
            SLfloat rP = tP * aspect;          //right projection at focal distance
            SLfloat lP = -tP * aspect;          //left projection at focal distance
            SLfloat tN = tanFov * _clipNear;   //top near
            SLfloat rN = tN * aspect;          //right near
            SLfloat lN = -tN * aspect;          //left near

            // small line in view direction
            P[i++].set(0, 0, -5); P[i++].set(0, 0, 7);

            // frustum pyramid lines
            P[i++].set(0, 0, 0); P[i++].set(rF, tF, -_clipFar);
            P[i++].set(0, 0, 0); P[i++].set(lF, tF, -_clipFar);
            P[i++].set(0, 0, 0); P[i++].set(lF, -tF, -_clipFar);
            P[i++].set(0, 0, 0); P[i++].set(rF, -tF, -_clipFar);

            // around far clipping plane
            P[i++].set(rF, tF, -_clipFar); P[i++].set(rF, -tF, -_clipFar);
            P[i++].set(rF, -tF, -_clipFar); P[i++].set(lF, -tF, -_clipFar);
            P[i++].set(lF, -tF, -_clipFar); P[i++].set(lF, tF, -_clipFar);
            P[i++].set(lF, tF, -_clipFar); P[i++].set(rF, tF, -_clipFar);

            // around projection plane at focal distance
            P[i++].set(rP, tP, -_focalDist); P[i++].set(rP, -tP, -_focalDist);
            P[i++].set(rP, -tP, -_focalDist); P[i++].set(lP, -tP, -_focalDist);
            P[i++].set(lP, -tP, -_focalDist); P[i++].set(lP, tP, -_focalDist);
            P[i++].set(lP, tP, -_focalDist); P[i++].set(rP, tP, -_focalDist);

            // around near clipping plane
            P[i++].set(rN, tN, -_clipNear); P[i++].set(rN, -tN, -_clipNear);
            P[i++].set(rN, -tN, -_clipNear); P[i++].set(lN, -tN, -_clipNear);
            P[i++].set(lN, -tN, -_clipNear); P[i++].set(lN, tN, -_clipNear);
            P[i++].set(lN, tN, -_clipNear); P[i++].set(rN, tN, -_clipNear);

            SLCamera::_bufP.generate(P, i, 3);
        }

        SLCamera::_bufP.drawArrayAsConstantColorLines(SLCol3f::WHITE*0.7f);
    }
}
//-----------------------------------------------------------------------------