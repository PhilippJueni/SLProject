//#############################################################################
//  File:      SLGullstrandRay.cpp
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Copyright (c): 2002-2014 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>           // precompiled headers
#ifdef SL_MEMLEAKDETECT
#include <nvwa/debug_new.h>   // memory leak detector
#endif

#include <SLGullstrandRay.h>

/*
// init static variables
SLint   SLGullstrandRay::maxDepth = 0;
SLfloat SLGullstrandRay::minContrib = 1.0 / 256.0;
SLuint  SLGullstrandRay::reflectedRays = 0;
SLuint  SLGullstrandRay::refractedRays = 0;
SLuint  SLGullstrandRay::shadowRays = 0;
SLuint  SLGullstrandRay::subsampledRays = 0;
SLuint  SLGullstrandRay::subsampledPixels = 0;
SLuint  SLGullstrandRay::tirRays = 0;
SLuint  SLGullstrandRay::tests = 0;
SLuint  SLGullstrandRay::intersections = 0;
SLint   SLGullstrandRay::depthReached = 1;
SLint   SLGullstrandRay::maxDepthReached = 0;
SLfloat SLGullstrandRay::avgDepth = 0;

//-----------------------------------------------------------------------------
/*! Global uniform random number generator for numbers between 0 and 1 that are
used in SLGullstrandRay, SLLightRect and SLPathtracer. So far they work perfectly with
CPP11 multithreading.
*/
#ifdef SL_CPP11
auto random01 = bind(uniform_real_distribution<SLfloat>(0.0, 1.0),
    mt19937((SLuint)time(NULL)));
SLfloat rnd01();
SLfloat rnd01(){ return random01(); }
#else
SLfloat rnd01(){ return (SLfloat)rand() / (SLfloat)RAND_MAX; }
#endif
//-----------------------------------------------------------------------------
/*!
SLGullstrandRay::SLGullstrandRay default constructor
*/
SLGullstrandRay::SLGullstrandRay()
{
    origin = SLVec3f::ZERO;
    setDir(SLVec3f::ZERO);
//    type = PRIMARY;
    length = FLT_MAX;
    depth = 1;
    hitTriangle = -1;
    hitPoint = SLVec3f::ZERO;
    hitNormal = SLVec3f::ZERO;
    hitTexCol = SLCol4f::BLACK;
    hitNode = 0;
    hitMesh = 0;
    hitMat = 0;
    originNode = 0;
    originMesh = 0;
    originTria = -1;
    originMat = 0;
    x = -1;
    y = -1;
    contrib = 1.0f;
    isOutside = true;
    isInsideVolume = false;
}
//-----------------------------------------------------------------------------
/*!
SLGullstrandRay::SLGullstrandRay constructor for primary rays
*/
SLGullstrandRay::SLGullstrandRay(SLVec3f Origin, SLVec3f Dir, SLfloat X, SLfloat Y)
{
    origin = Origin;
    setDir(Dir);
//    type = PRIMARY;
    length = FLT_MAX;
    depth = 1;
    hitTriangle = -1;
    hitPoint = SLVec3f::ZERO;
    hitNormal = SLVec3f::ZERO;
    hitTexCol = SLCol4f::BLACK;
    hitNode = 0;
    hitMesh = 0;
    hitMat = 0;
    originNode = 0;
    originMesh = 0;
    originTria = -1;
    originMat = 0;
    x = (SLfloat)X;
    y = (SLfloat)Y;
    contrib = 1.0f;
    isOutside = true;
    isInsideVolume = false;
}
//-----------------------------------------------------------------------------
/*!
SLGullstrandRay::SLGullstrandRay constructor for shadow rays
*/
SLGullstrandRay::SLGullstrandRay(SLfloat distToLight,
    SLVec3f dirToLight,
    SLGullstrandRay*  rayFromHitPoint)
{
    origin = rayFromHitPoint->hitPoint;
    setDir(dirToLight);
 //   type = SHADOW;
    length = distToLight;
    lightDist = distToLight;
    depth = rayFromHitPoint->depth;
    hitPoint = SLVec3f::ZERO;
    hitNormal = SLVec3f::ZERO;
    hitTexCol = SLCol4f::BLACK;
    hitTriangle = -1;
    hitNode = 0;
    hitMesh = 0;
    hitMat = 0;
    originNode = rayFromHitPoint->hitNode;
    originMesh = rayFromHitPoint->hitMesh;
    originTria = rayFromHitPoint->hitTriangle;
    originMat = rayFromHitPoint->hitMat;
    x = rayFromHitPoint->x;
    y = rayFromHitPoint->y;
    contrib = 0.0f;
    isOutside = rayFromHitPoint->isOutside;
    shadowRays++;
}
//-----------------------------------------------------------------------------
/*!
SLGullstrandRay::prints prints the rays origin (O), direction (D) and the length to the
intersection (L)
*/
void SLGullstrandRay::print()
{
    SL_LOG("Ray: O(%.2f, %.2f, %.2f), D(%.2f, %.2f, %.2f), L: %.2f\n",
        origin.x, origin.y, origin.z, dir.x, dir.y, dir.z, length);
}
//-----------------------------------------------------------------------------
/*!
SLGullstrandRay::normalizeNormal does a careful normalization of the normal only when the
squared length is > 1.0+FLT_EPSILON or < 1.0-FLT_EPSILON.
*/
void SLGullstrandRay::normalizeNormal()
{
    SLfloat nLenSqr = hitNormal.lengthSqr();
    if (nLenSqr > 1.0f + FLT_EPSILON || nLenSqr < 1.0f - FLT_EPSILON)
    {
        SLfloat len = sqrt(nLenSqr);
        hitNormal /= len;
    }
}
//-----------------------------------------------------------------------------
/*!
SLGullstrandRay::reflect calculates a secondary ray reflected at the normal, starting at
the intersection point. All vectors must be normalized vectors.
R = 2(-I�N) N + I
*/
void SLGullstrandRay::reflect(SLGullstrandRay* reflected)
{
    SLVec3f R(dir - 2.0f*(dir*hitNormal)*hitNormal);

    reflected->setDir(R);
    reflected->origin.set(hitPoint);
    reflected->depth = depth + 1;
    reflected->length = FLT_MAX;
    reflected->contrib = contrib * hitMat->kr();
    reflected->originMat = hitMat;
    reflected->originNode = hitNode;
    reflected->originMesh = hitMesh;
    reflected->originTria = hitTriangle;
//    reflected->type = REFLECTED;
    reflected->isOutside = isOutside;
    reflected->x = x;
    reflected->y = y;
    depthReached = reflected->depth;
    ++reflectedRays;
}
//-----------------------------------------------------------------------------
/*!
SLGullstrandRay::refract calculates a secondary refracted ray, starting at the
intersection point. All vectors must be normalized vectors, so the refracted
vector T will be a unit vector too. If total internal refraction occurs a
reflected ray is calculated instead.
Index of refraction eta = Kn_Source/Kn_Destination (Kn_Air = 1.0)
*/
void SLGullstrandRay::refract(SLGullstrandRay* refracted)
{
    SLVec3f T;   // refracted direction
    SLfloat eta; // refraction coefficient

    // Calculate index of refraction eta = Kn_Source/Kn_Destination
    if (isOutside)
    {
        if (originMat == 0) // from air (outside) into a material
            eta = 1 / hitMat->kn();
        else // from another material into another one
            eta = originMat->kn() / hitMat->kn();
    }
    else
    {
        if (originMat == hitMat) // from the inside a material into air
            eta = hitMat->kn(); // hitMat / 1
        else // from inside a material into another material
            eta = originMat->kn() / hitMat->kn();
    }

    // Bec's formula is a little faster (from Ray Tracing News) 
    SLfloat c1 = hitNormal * -dir;
    SLfloat w = eta * c1;
    SLfloat c2 = 1.0f + (w - eta) * (w + eta);

    if (c2 >= 0.0f)
    {
        T = eta * dir + (w - sqrt(c2)) * hitNormal;
        refracted->contrib = contrib * hitMat->kt();
 //       refracted->type = TRANSMITTED;
        refracted->isOutside = !isOutside;
        ++refractedRays;
    }
    else // total internal refraction results in a internal reflected ray
    {
        T = 2.0f * (-dir*hitNormal) * hitNormal + dir;
        refracted->contrib = 1.0f;
  //      refracted->type = REFLECTED;
        refracted->isOutside = isOutside;
        ++tirRays;
    }

    refracted->setDir(T);
    refracted->origin.set(hitPoint);
    refracted->originMat = hitMat;
    refracted->length = FLT_MAX;
    refracted->originNode = hitNode;
    refracted->originMesh = hitMesh;
    refracted->originTria = hitTriangle;
    refracted->depth = depth + 1;
    refracted->x = x;
    refracted->y = y;
    depthReached = refracted->depth;
}
//-----------------------------------------------------------------------------
/*!
SLGullstrandRay::reflectMC scatters a ray around perfect specular direction according to
shininess (for higher shininess the ray is less scattered). This is used for
path tracing and distributed ray tracing as well as for photon scattering.
The direction is calculated according to MCCABE. The created direction is
along z-axis and then transformed to lie along specular direction with
rotationMatrix rotMat. The rotation matrix must be precalculated (stays the
same for each ray sample, needs to be be calculated only once)
*/
bool SLGullstrandRay::reflectMC(SLGullstrandRay* reflected, SLMat3f rotMat)
{
    SLfloat eta1, eta2;
    SLVec3f randVec;
    SLfloat shininess = hitMat->shininess();

    //scatter within specular lobe
    eta1 = rnd01();
    eta2 = SL_2PI*rnd01();
    SLfloat f1 = sqrt(1.0f - pow(eta1, 2.0f / (shininess + 1.0f)));

    //tranform to cartesian
    randVec.set(f1 * cos(eta2),
        f1 * sin(eta2),
        pow(eta1, 1.0f / (shininess + 1.0f)));

    //ray needs to be reset if already hit a scene node
    if (reflected->hitNode)
    {
        reflected->length = FLT_MAX;
        reflected->hitNode = 0;
        reflected->hitMesh = 0;
        reflected->hitPoint = SLVec3f::ZERO;
        reflected->hitNormal = SLVec3f::ZERO;
    }

    //apply rotation
    reflected->setDir(rotMat*randVec);

    //true if in direction of normal
    return (hitNormal * reflected->dir >= 0.0f);
}
//-----------------------------------------------------------------------------
/*!
SLGullstrandRay::refractMC scatters a ray around perfect transmissive direction according
to translucency (for higher translucency the ray is less scattered).
This is used for path tracing and distributed ray tracing as well as for photon
scattering. The direction is calculated the same as with specular scattering
(see reflectMC). The created direction is along z-axis and then transformed to
lie along transmissive direction with rotationMatrix rotMat. The rotation
matrix must be precalculated (stays the same for each ray sample, needs to be
be calculated only once)
*/
void SLGullstrandRay::refractMC(SLGullstrandRay* refracted, SLMat3f rotMat)
{
    SLfloat eta1, eta2;
    SLVec3f randVec;
    SLfloat translucency = hitMat->translucency();

    //scatter within transmissive lobe
    eta1 = rnd01();
    eta2 = SL_2PI*rnd01();
    SLfloat f1 = sqrt(1.0f - pow(eta1, 2.0f / (translucency + 1.0f)));

    //transform to cartesian
    randVec.set(f1*cos(eta2),
        f1*sin(eta2),
        pow(eta1, 1.0f / (translucency + 1.0f)));

    //ray needs to be reset if already hit a scene node
    if (refracted->hitNode)
    {
        refracted->length = FLT_MAX;
        refracted->hitNode = 0;
        refracted->hitMesh = 0;
        refracted->hitPoint = SLVec3f::ZERO;
        refracted->hitNormal = SLVec3f::ZERO;
    }

    refracted->setDir(rotMat*randVec);
}
//-----------------------------------------------------------------------------
/*!
SLGullstrandRay::diffuseMC scatters a ray around hit normal (cosine distribution).
This is only used for photonmapping(russian roulette).
The random direction lies around z-Axis and is then transformed by a rotation
matrix to lie along the normal. The direction is calculated according to MCCABE
*/
void SLGullstrandRay::diffuseMC(SLGullstrandRay* scattered)
{
    SLVec3f randVec;
    SLfloat eta1, eta2, eta1sqrt;

    scattered->setDir(hitNormal);
    scattered->origin = hitPoint;
    scattered->depth = depth + 1;
    depthReached = scattered->depth;

    // for reflectance the start material stays the same
    scattered->originMat = hitMat;
    scattered->originNode = hitNode;
    scattered->originMesh = hitMesh;
  //  scattered->type = REFLECTED;

    //calculate rotation matrix
    SLMat3f rotMat;
    SLVec3f rotAxis((SLVec3f(0.0, 0.0, 1.0) ^ scattered->dir).normalize());
    SLfloat rotAngle = acos(scattered->dir.z); //z*scattered.dir()
    rotMat.rotation(rotAngle*180.0f / SL_PI, rotAxis);

    //cosine distribution
    eta1 = rnd01();
    eta2 = SL_2PI*rnd01();
    eta1sqrt = sqrt(1 - eta1);
    //transform to cartesian
    randVec.set(eta1sqrt * cos(eta2),
        eta1sqrt * sin(eta2),
        sqrt(eta1));

    scattered->setDir(rotMat*randVec);
}
//-----------------------------------------------------------------------------
