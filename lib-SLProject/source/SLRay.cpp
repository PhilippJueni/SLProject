//#############################################################################
//  File:      SLRay.cpp
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

#include <SLRay.h>

// init static variables
SLint   SLRay::maxDepth = 0;
SLfloat SLRay::minContrib = 1.0 / 256.0;     
SLuint  SLRay::reflectedRays = 0;
SLuint  SLRay::refractedRays = 0;
SLuint  SLRay::shadowRays = 0;
SLuint  SLRay::subsampledRays = 0;
SLuint  SLRay::subsampledPixels = 0;
SLuint  SLRay::tirRays = 0;
SLuint  SLRay::tests = 0;
SLuint  SLRay::intersections = 0;
SLint   SLRay::depthReached = 1;
SLint   SLRay::maxDepthReached = 0;
SLfloat SLRay::avgDepth = 0;

//-----------------------------------------------------------------------------
/*! Global uniform random number generator for numbers between 0 and 1 that are
used in SLRay, SLLightRect and SLPathtracer. So far they work perfectly with 
CPP11 multithreading.
*/
#ifdef SL_CPP11
    auto random01 = bind(uniform_real_distribution<SLfloat>(0.0, 1.0),
                        mt19937((SLuint)time(NULL)));
    SLfloat rnd01();
    SLfloat rnd01(){return random01();}
#else
    SLfloat rnd01(){return (SLfloat)rand() / (SLfloat)RAND_MAX;}
#endif
//-----------------------------------------------------------------------------
/*! 
SLRay::SLRay default constructor
*/
SLRay::SLRay()
{  
    origin      = SLVec3f::ZERO;
    setDir(SLVec3f::ZERO);
    type        = PRIMARY;
    length      = FLT_MAX;
    depth       = 1;
    hitTriangle = -1;
    hitPoint    = SLVec3f::ZERO;
    hitNormal   = SLVec3f::ZERO;
    hitTexCol   = SLCol4f::BLACK;
    hitNode     = 0;
    hitMesh     = 0;
    hitMat      = 0;
    originNode  = 0;
    originMesh  = 0;
    originTria  = -1;
    originMat   = 0;
    x           = -1;
    y           = -1;
    contrib     = 1.0f;
    isOutside   = true;
    isInsideVolume = false;
}
//-----------------------------------------------------------------------------
/*! 
SLRay::SLRay constructor for primary rays
*/
SLRay::SLRay(SLVec3f Origin, SLVec3f Dir, SLfloat X, SLfloat Y)  
{  
    origin      = Origin;
    setDir(Dir);
    type        = PRIMARY;
    length      = FLT_MAX;
    depth       = 1;
    hitTriangle = -1;
    hitPoint    = SLVec3f::ZERO;
    hitNormal   = SLVec3f::ZERO;
    hitTexCol   = SLCol4f::BLACK;
    hitNode     = 0;
    hitMesh     = 0;
    hitMat      = 0;
    originNode  = 0;
    originMesh  = 0;
    originTria  = -1;
    originMat   = 0;
    x           = (SLfloat)X;
    y           = (SLfloat)Y;
    contrib     = 1.0f;
    isOutside   = true;
    isInsideVolume = false;
}
//-----------------------------------------------------------------------------
/*! 
SLRay::SLRay constructor for shadow rays
*/
SLRay::SLRay(SLfloat distToLight,
             SLVec3f dirToLight,
             SLRay*  rayFromHitPoint)  
{   
    origin      = rayFromHitPoint->hitPoint;
    setDir(dirToLight);
    type        = SHADOW;
    length      = distToLight;
    lightDist   = distToLight;
    depth       = rayFromHitPoint->depth;
    hitPoint    = SLVec3f::ZERO;
    hitNormal   = SLVec3f::ZERO;
    hitTexCol   = SLCol4f::BLACK;
    hitTriangle = -1;
    hitNode     = 0;
    hitMesh     = 0;
    hitMat      = 0;
    originNode  = rayFromHitPoint->hitNode;
    originMesh  = rayFromHitPoint->hitMesh;
    originTria  = rayFromHitPoint->hitTriangle;
    originMat   = rayFromHitPoint->hitMat;
    x           = rayFromHitPoint->x;
    y           = rayFromHitPoint->y;
    contrib     = 0.0f;
    isOutside   = rayFromHitPoint->isOutside;
    shadowRays++;
}
//-----------------------------------------------------------------------------
/*!
SLRay::prints prints the rays origin (O), direction (D) and the length to the 
intersection (L) 
*/
void SLRay::print()
{
    SL_LOG("Ray: O(%.2f, %.2f, %.2f), D(%.2f, %.2f, %.2f), L: %.2f\n",
           origin.x,origin.y,origin.z, dir.x,dir.y,dir.z, length);
}
//-----------------------------------------------------------------------------
/*!
SLRay::normalizeNormal does a careful normalization of the normal only when the
squared length is > 1.0+FLT_EPSILON or < 1.0-FLT_EPSILON.
*/
void SLRay::normalizeNormal()
{
    SLfloat nLenSqr = hitNormal.lengthSqr();
    if (nLenSqr > 1.0f+FLT_EPSILON || nLenSqr < 1.0f-FLT_EPSILON)
    {   SLfloat len = sqrt(nLenSqr);
        hitNormal /= len;
    }
}
//-----------------------------------------------------------------------------
/*!
SLRay::reflect calculates a secondary ray reflected at the normal, starting at 
the intersection point. All vectors must be normalized vectors.
R = 2(-I�N) N + I
*/
void SLRay::reflect(SLRay* reflected)
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
    reflected->type = REFLECTED;
    reflected->isOutside = isOutside;
    reflected->x = x;
    reflected->y = y;
    depthReached = reflected->depth;
    ++reflectedRays;
}
//-----------------------------------------------------------------------------
/*!
SLRay::refract calculates a secondary refracted ray, starting at the 
intersection point. All vectors must be normalized vectors, so the refracted 
vector T will be a unit vector too. If total internal refraction occurs a 
reflected ray is calculated instead.
Index of refraction eta = Kn_Source/Kn_Destination (Kn_Air = 1.0)
*/
void SLRay::refract(SLRay* refracted)
{  
    SLVec3f T;   // refracted direction
    SLfloat eta; // refraction coefficient
      
    // Calculate index of refraction eta = Kn_Source/Kn_Destination
    if (isOutside)
    {   if (originMat==0) // from air (outside) into a material
            eta = 1 / hitMat->knI();
        else // from another material into another one
            eta = originMat->knI() / hitMat->knI();
    } else
    {   if (originMat==hitMat) // from the inside a material into air
            eta = hitMat->knI(); // hitMat / 1
        else // from inside a material into another material
            eta = originMat->knI() / hitMat->knI();
    }

    // Bec's formula is a little faster (from Ray Tracing News) 
    SLfloat c1 = hitNormal * -dir;
    SLfloat w  = eta * c1;
    SLfloat c2 = 1.0f + (w - eta) * (w + eta);

    if (c2 >= 0.0f) 
    {   T = eta * dir + (w - sqrt(c2)) * hitNormal;
        refracted->contrib = contrib * hitMat->kt();
        refracted->type = TRANSMITTED;
        refracted->isOutside = !isOutside;
        ++refractedRays;
    } 
    else // total internal refraction results in a internal reflected ray
    {   T = 2.0f * (-dir*hitNormal) * hitNormal + dir;
        refracted->contrib = 1.0f;
        refracted->type = REFLECTED;
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

/*!
SLRay::refract calculates a secondary refracted ray, starting at the
intersection point. All vectors must be normalized vectors, so the refracted
vector T will be a unit vector too. If total internal refraction occurs a
reflected ray is calculated instead.
Index of refraction eta = Kn_Source/Kn_Destination (Kn_Air = 1.0)
*/
void SLRay::refractHE(SLRay* refracted)
{
    SLVec3f T;   // refracted direction
    SLfloat eta; // refraction coefficient

    // Calculate index of refraction eta = Kn_Source/Kn_Destination
    if (isOutside)
    {
        if (hitMat->knO() != 0.0f)
        {   // refract at a surface ///////////////////////////////////////////////////////////////
            SLfloat knOrigin = (originMat != NULL) ? originMat->knO() : 1.0f;
            if (knOrigin != hitMat->knO())
            {
                assert(knOrigin != hitMat->knO()); 
                cout << "ERROR, not same kn (O): " << knOrigin << "-" << hitMat->knO() << endl;
            }
            eta = hitMat->knO() / hitMat->knI(); // Triangle_1: 1,5 / 1 = 1,5
        }   ///////////////////////////////////////////////////////////////////////////////////////
        else
        {   if (originMat == 0) // from air (outside) into a material
                eta = 1 / hitMat->knI();
            else // from another material into another one
                eta = originMat->knI() / hitMat->knI();
        }
    }
    else
    {   if (hitMat->knO() != 0.0f)
        {   // refract at a surface ///////////////////////////////////////////////////////////////
            if (originMat->knI() != hitMat->knI())
            {
                assert(originMat->knI() != hitMat->knI());
                cout << "ERROR, not same kn (I): " << originMat->knI() << "-" << hitMat->knI() << endl;
            }
            eta = hitMat->knI() / hitMat->knO();
        }   ///////////////////////////////////////////////////////////////////////////////////////
        else
        {   if (originMat == hitMat) // from the inside a material into air
                eta = hitMat->knI(); // hitMat / 1
            else // from inside a material into another material
                eta = originMat->knI() / hitMat->knI();
        }
    }
    
    // Bec's formula is a little faster (from Ray Tracing News) 
    // hitNormal = Surface normal at intersection point
    SLfloat c1 = hitNormal * -dir;              // Triangle_1: 0.961437881 = (0,0,1) * -(-0.231,-0.148,-0.961)
    SLfloat w = eta * c1;                       // Triangle_1: 1.44215679 = 1.5 * 0.961437881
    SLfloat c2 = 1.0f + (w - eta) * (w + eta);  // Triangle_1: 0.829816222

    if (c2 >= 0.0f)
    {   T = eta * dir + (w - sqrt(c2)) * hitNormal;     // Triangle_1: (-0.347, -0.222, -0.910)
        refracted->contrib = contrib * hitMat->kt();    // Triangle_1: 0.5 = 1 * 1.5
        refracted->type = TRANSMITTED;                  // Triangle_1: TRANSMITTED(2)
        refracted->isOutside = !isOutside;              // Triangle_1: false
        ++refractedRays;                                // Triangle_1: 0 --> 1
    }
    else // total internal refraction results in a internal reflected ray
    {   T = 2.0f * (-dir*hitNormal) * hitNormal + dir;
        refracted->contrib = 1.0f; 
        refracted->type = REFLECTED;
        refracted->isOutside = isOutside;
        ++tirRays;
    }
    
    // set refracted ray parameter
    refracted->setDir(T);                       // Triangle_1: (-0.347, -0.222, -0.910)
    refracted->origin.set(hitPoint);            // Triangle_1: (-0.240, -0.154, 3.0 )
    refracted->originMat = hitMat;              // Triangle_1: matTriangle_1
    refracted->length = FLT_MAX;                // Triangle_1: 3.4e+038                     ???
    refracted->originNode = hitNode;            // Triangle_1: Triangle_1-Node
    refracted->originMesh = hitMesh;            // Triangle_1: Triangle_1
    refracted->originTria = hitTriangle;        // Triangle_1: 0
    refracted->depth = depth + 1;               // Triangle_1: 1
    refracted->x = x;                           // Triangle_1: 180
    refracted->y = y;                           // Triangle_1: 150
    depthReached = refracted->depth;            // Triangle_1: 1
}

//-----------------------------------------------------------------------------
/*!
SLRay::reflectMC scatters a ray around perfect specular direction according to 
shininess (for higher shininess the ray is less scattered). This is used for 
path tracing and distributed ray tracing as well as for photon scattering. 
The direction is calculated according to MCCABE. The created direction is 
along z-axis and then transformed to lie along specular direction with 
rotationMatrix rotMat. The rotation matrix must be precalculated (stays the 
same for each ray sample, needs to be be calculated only once)
*/
bool SLRay::reflectMC(SLRay* reflected,SLMat3f rotMat)
{
    SLfloat eta1, eta2;
    SLVec3f randVec;
    SLfloat shininess = hitMat->shininess();

    //scatter within specular lobe
    eta1 = rnd01();
    eta2 = SL_2PI*rnd01();
    SLfloat f1 = sqrt(1.0f-pow(eta1, 2.0f/(shininess+1.0f)));

    //tranform to cartesian
    randVec.set(f1 * cos(eta2),
                f1 * sin(eta2),
                pow(eta1, 1.0f/(shininess+1.0f)));

    //ray needs to be reset if already hit a scene node
    if(reflected->hitNode)
    {  reflected->length = FLT_MAX;
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
SLRay::refractMC scatters a ray around perfect transmissive direction according 
to translucency (for higher translucency the ray is less scattered).
This is used for path tracing and distributed ray tracing as well as for photon 
scattering. The direction is calculated the same as with specular scattering
(see reflectMC). The created direction is along z-axis and then transformed to 
lie along transmissive direction with rotationMatrix rotMat. The rotation 
matrix must be precalculated (stays the same for each ray sample, needs to be 
be calculated only once)
*/
void SLRay::refractMC(SLRay* refracted,SLMat3f rotMat)
{
    SLfloat eta1, eta2;
    SLVec3f randVec;
    SLfloat translucency = hitMat->translucency();

    //scatter within transmissive lobe
    eta1 = rnd01();
    eta2 = SL_2PI*rnd01();
    SLfloat f1=sqrt(1.0f-pow(eta1,2.0f/(translucency+1.0f)));

    //transform to cartesian
    randVec.set(f1*cos(eta2),
                f1*sin(eta2),
                pow(eta1,1.0f/(translucency+1.0f)));

    //ray needs to be reset if already hit a scene node
    if(refracted->hitNode)
    {   refracted->length = FLT_MAX;
        refracted->hitNode = 0;
        refracted->hitMesh = 0;
        refracted->hitPoint = SLVec3f::ZERO;
        refracted->hitNormal = SLVec3f::ZERO;
    }
   
    refracted->setDir(rotMat*randVec);
}
//-----------------------------------------------------------------------------
/*!
SLRay::diffuseMC scatters a ray around hit normal (cosine distribution).
This is only used for photonmapping(russian roulette).
The random direction lies around z-Axis and is then transformed by a rotation 
matrix to lie along the normal. The direction is calculated according to MCCABE
*/
void SLRay::diffuseMC(SLRay* scattered)
{
    SLVec3f randVec;
    SLfloat eta1,eta2,eta1sqrt;

    scattered->setDir(hitNormal);
    scattered->origin = hitPoint;
    scattered->depth = depth+1;
    depthReached = scattered->depth;
   
    // for reflectance the start material stays the same
    scattered->originMat = hitMat;
    scattered->originNode = hitNode;
    scattered->originMesh = hitMesh;
    scattered->type = REFLECTED;

    //calculate rotation matrix
    SLMat3f rotMat;
    SLVec3f rotAxis((SLVec3f(0.0,0.0,1.0) ^ scattered->dir).normalize());
    SLfloat rotAngle=acos(scattered->dir.z); //z*scattered.dir()
    rotMat.rotation(rotAngle*180.0f/SL_PI, rotAxis);

    //cosine distribution
    eta1 = rnd01();
    eta2 = SL_2PI*rnd01();
    eta1sqrt = sqrt(1-eta1);
    //transform to cartesian
    randVec.set(eta1sqrt * cos(eta2),
                eta1sqrt * sin(eta2),
                sqrt(eta1));

    scattered->setDir(rotMat*randVec);
}
//-----------------------------------------------------------------------------
