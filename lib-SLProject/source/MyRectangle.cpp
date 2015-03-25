//#############################################################################
//  File:      SLRectangle.cpp
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: 2002-2014 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>           // precompiled headers
#ifdef SL_MEMLEAKDETECT       // set in SL.h for debug config only
#include <debug_new.h>        // memory leak detector
#endif


#include "stdafx.h"
#include "MyRectangle.h"


MyRectangle::MyRectangle(SLVec2f min, SLVec2f max,
    SLuint resX, SLuint resY,
    SLstring name,
    SLMaterial* mat) : SLMesh(name)
{
    assert(min != max);
    assert(resX>0);
    assert(resY>0);
    assert(name != "");
    _min = min;
    _max = max;
    _tmin.set(0, 0);
    _tmax.set(1, 1);
    _resX = resX;
    _resY = resY;
    _isVolume = true;
    buildMesh(mat);
}

void MyRectangle::buildMesh(SLMaterial* material)
{
    deleteData(); // löscht die Daten in SLMesh->deleteData();

    // Check max. allowed no. of verts
    SLuint uIntNumV64 = (_resX + 1) * (_resY + 1);
    if (uIntNumV64 > UINT_MAX) // prüft ob die auflösung kleiner max unsigned int value ist
        SL_EXIT_MSG("SLMesh supports max. 2^32 vertices.");

    // allocate new arrays of SLMesh
    numV = (_resX + 1) * (_resY + 1); //!< Number of elements in P, N, C, T & Tc  (hängt von der Auflösung ab)
    numI = _resX * _resY * 2 * 3;     //!< Number of elements in I16 or I32
    P = new SLVec3f[numV];            //!< Array of vertex positions
    N = new SLVec3f[numV];            //!< Array of vertex normals (opt.)
    Tc = new SLVec2f[numV];           //!< Array of vertex tex. coords. (opt.)
    

    cout << "intNum: " << uIntNumV64 << endl;
    if (uIntNumV64 < 65535)           // wenn auflösung grösser 65535
        I16 = new SLushort[numI];     //!< Array of vertex indexes 16 bit
    else I32 = new SLuint[numI];      //!< Array of vertex indexes 32 bit

    // Calculate normal from the first 3 corners
    // _min = min corner -1 -1 0
    // _max = max corner 1 1 0
    SLVec3f maxmin(_max.x, _min.y, 0); // 1 -1 0
    SLVec3f minmax(_min.x, _max.y, 0); // -1 1 0
    SLVec3f e1(maxmin - _min); // ( 1 -1 0 ) - ( -1 -1 0 ) = ( 2 0 0 )   // eigenvektoren
    SLVec3f e2(minmax - _min); // ( -1 1 0 ) - ( -1 -1 0 ) = ( 0 2 0 )
    SLVec3f curN(e1^e2); // ( 2 0 0 ) ^ ( 0 2 0 ) = ( 0 0 4 )
    curN.normalize(); // 0 0 4 -> 0 0 1 
    

    //Set one default material index
    mat = material;

    // define delta vectors dX & dY and deltas for texCoord dS,dT
    // _tmin = min corner texCoord
    // _tmax = max corner texCoord
    SLVec3f dX = e1 / (SLfloat)_resX; // 2 0 0 / 1 = 2 0 0
    SLVec3f dY = e2 / (SLfloat)_resY; // 0 2 0 / 1 = 0 2 0
    SLfloat dS = (_tmax.x - _tmin.x) / (SLfloat)_resX; // 1 / 1 = 1
    SLfloat dT = (_tmax.y - _tmin.y) / (SLfloat)_resY; // 1 / 1 = 1

    // Build vertex data
    SLuint i = 0;
    for (SLuint y = 0; y <= _resY; ++y) // für jede zeile
    {
        SLVec3f curV = _min; // -1 -1 0
        SLVec2f curT = _tmin; // 0 0 0
        curV += (SLfloat)y*dY; // -1 -1 0 += 0 * 2 
        curT.y += (SLfloat)y*dT; // 0 0 += 0*1

        for (SLuint x = 0; x <= _resX; ++x, ++i) // für jede spalte
        {
            P[i] = curV;
            Tc[i] = curT;
            N[i] = curN;
            curV += dX;
            curT.x += dS;
        }
    }

    

    // Build face vertex indexes
    if (I16) // 16 bit array of indexes
    {
        cout << "haha 16" << endl;
        SLuint v = 0, i = 0; //index for vertices and indexes
        for (SLuint y = 0; y<_resY; ++y)
        {
            //cout << "y for: " << v << endl;
            for (SLuint x = 0; x<_resX; ++x, ++v)
            {  // triangle 1
                //cout << "x for _ t1: " << v << endl;
                I16[i++] = v;
                I16[i++] = v + _resX + 2;
                I16[i++] = v + _resX + 1;

                //cout << "t2: " << v <<  endl;
                
                // triangle 2
                I16[i++] = v;
                I16[i++] = v + 1;
                I16[i++] = v + _resX + 2;

               
            }
            //cout << "v++: " << v << endl;
            v++;
        }
    }
    else // 32 bit array of indexes
    {
        cout << "haha 32" << endl;
        SLuint v = 0, i = 0; //index for vertices and indexes
        for (SLuint y = 0; y<_resY; ++y)
        {
            for (SLuint x = 0; x<_resX; ++x, ++v)
            {  // triangle 1
                I32[i++] = v;
                I32[i++] = v + _resX + 2;
                I32[i++] = v + _resX + 1;

                // triangle 2
                I32[i++] = v;
                I32[i++] = v + 1;
                I32[i++] = v + _resX + 2;
            }
            v++;
        }
    }
}
//-----------------------------------------------------------------------------
