//#############################################################################
//  File:      SLGullstrandCamera.h
//  Author:    Philipp Jüni
//  Date:      October 2014
//  Copyright: 2002-2014 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLGULLSTRANDRAY_H
#define SLGULLSTRANDRAY_H

#include <stdafx.h>
#include <SLMaterial.h>

struct SLFace16;
class  SLNode;
class  SLMesh;

//-----------------------------------------------------------------------------
//! SLRayType enumeration for specifying ray type in ray tracing
//enum SLGullstrandRayType { PRIMARY = 0, REFLECTED = 1, TRANSMITTED = 2, SHADOW = 3 };

//! Ray tracing constant for max. allowed recursion depth
#define SL_MAXTRACE    15
//-----------------------------------------------------------------------------
//! Ray class with ray and intersection properties
class SLGullstrandRay
{
public:

    SLGullstrandRay();

    //! ctor for primary rays
    SLGullstrandRay(SLVec3f Origin,
        SLVec3f Dir,
        SLfloat X,
        SLfloat Y);

    //! ctor for shadow rays
    SLGullstrandRay(SLfloat distToLight,
        SLVec3f dirToLight,
        SLGullstrandRay*  rayFromHitPoint);

    void        reflect(SLGullstrandRay* reflected);
    void        refract(SLGullstrandRay* refracted);
    bool        reflectMC(SLGullstrandRay* reflected, SLMat3f rotMat);
    void        refractMC(SLGullstrandRay* refracted, SLMat3f rotMat);
    void        diffuseMC(SLGullstrandRay* scattered);
    void        volumeRay(SLGullstrandRay* volumeRay);

    // Helper methods
    inline  void        setDir(SLVec3f Dir)
    {
        dir = Dir;
        invDir.x = (SLfloat)(1 / dir.x);
        invDir.y = (SLfloat)(1 / dir.y);
        invDir.z = (SLfloat)(1 / dir.z);
        sign[0] = (invDir.x<0);
        sign[1] = (invDir.y<0);
        sign[2] = (invDir.z<0);
    }
    inline  void        setDirOS(SLVec3f Dir)
    {
        dirOS = Dir;
        invDirOS.x = (SLfloat)(1 / dirOS.x);
        invDirOS.y = (SLfloat)(1 / dirOS.y);
        invDirOS.z = (SLfloat)(1 / dirOS.z);
        signOS[0] = (invDirOS.x<0);
        signOS[1] = (invDirOS.y<0);
        signOS[2] = (invDirOS.z<0);
    }
    //SLbool      isShaded() { return type == SHADOW && length<lightDist; }
    void        print();
    void        normalizeNormal();

    // Classic ray members
    SLVec3f     origin;        //!< Vector to the origin of ray in WS
    SLVec3f     dir;           //!< Direction vector of ray in WS
    SLVec3f     originOS;      //!< Vector to the origin of ray in OS
    SLVec3f     dirOS;         //!< Direction vector of ray in OS
    SLfloat     length;        //!< length from origin to an intersection
    SLint       depth;         //!< Recursion depth for ray tracing
    SLfloat     contrib;       //!< Current contibution of ray to color

    // Additional info for intersection 
    //SLRayType   type;          //!< PRIMARY, REFLECTED, TRANSMITTED, SHADOW
    SLfloat     lightDist;     //!< Distance to light for shadow rays
    SLfloat     x, y;          //!< Pixel position for primary rays
    SLbool      isOutside;     //!< Flag if ray is inside of a material
    SLbool      isInsideVolume;//!< Flag if ray is in Volume
    SLNode*     originNode;    //!< Points to the node at ray origin
    SLMesh*     originMesh;    //!< Points to the mesh at ray origin
    SLint       originTria;    //!< Points to the triangle at ray origin
    SLMaterial* originMat;     //!< Points to appearance at ray origin

    // Members set after at intersection
    SLfloat     hitU, hitV;    //!< barycentric coords in hit triangle
    SLNode*     hitNode;       //!< Points to the intersected node
    SLMesh*     hitMesh;       //!< Points to the intersected mesh
    SLint       hitTriangle;   //!< Points to the intersected triangle
    SLMaterial* hitMat;        //!< Points to material of intersected node

    // Members set before shading
    SLVec3f     hitPoint;      //!< Point of intersection
    SLVec3f     hitNormal;     //!< Surface normal at intersection point
    SLCol4f     hitTexCol;     //!< Texture color at intersection point

    // Helpers for fast AABB intersection
    SLVec3f     invDir;        //!< Inverse ray dir for fast AABB hit in WS
    SLVec3f     invDirOS;      //!< Inverse ray dir for fast AABB hit in OS
    SLint       sign[3];       //!< Sign of invDir for fast AABB hit in WS
    SLint       signOS[3];     //!< Sign of invDir for fast AABB hit in OS
    SLfloat     tmin;          //!< min. dist. of last AABB intersection
    SLfloat     tmax;          //!< max. dist. of last AABB intersection

    // static variables for statistics
    static SLint       maxDepth;         //!< Max. recursion depth
    static SLfloat     minContrib;       //!< Min. contibution to color (1/256)
    static SLuint      reflectedRays;    //!< NO. of reflected rays
    static SLuint      refractedRays;    //!< NO. of transmitted rays
    static SLuint      shadowRays;       //!< NO. of shadow rays
    static SLuint      tirRays;          //!< NO. of TIR refraction rays
    static SLuint      tests;            //!< NO. of intersection tests
    static SLuint      intersections;    //!< NO. of intersection
    static SLint       depthReached;     //!< depth reached for a primary ray
    static SLint       maxDepthReached;  //!< max. depth reached for all rays
    static SLfloat     avgDepth;         //!< average depth reached
    static SLuint      subsampledRays;   //!< NO. of of subsampled rays
    static SLuint      subsampledPixels; //!< NO. of of subsampled pixels

    SLbool      nodeReflectance() {
        return ((hitMat->specular().r >0.0f) ||
            (hitMat->specular().g >0.0f) ||
            (hitMat->specular().b >0.0f));
    }
    SLbool      nodeTransparency() {
        return ((hitMat->transmission().r >0.0f) ||
            (hitMat->transmission().g >0.0f) ||
            (hitMat->transmission().b >0.0f));
    }
    SLbool      nodeDiffuse() {
        return ((hitMat->diffuse().r >0.0f) ||
            (hitMat->diffuse().g >0.0f) ||
            (hitMat->diffuse().b >0.0f));
    }

    
};

//-----------------------------------------------------------------------------
#endif //SLGULLSTRANDRAY_H