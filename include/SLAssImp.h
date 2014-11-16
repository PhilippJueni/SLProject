//#############################################################################
//  File:      SL/SLAssImp.h
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: 2002-2014 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>
#include <SLNode.h>
#include <SLGLTexture.h>
#include <SLSkeleton.h>

// @todo    dont make this class static
// @todo    rename SLAssImp to SLImporter
//          make SLImporter a base class and put the implementation of the assimp importer in a derived class
 //         find the best way to make this nice and clean and to provide an interface for others to
//          write their own importers (for custom file formats for example)

#ifndef SLASSIMP_H
#define SLASSIMP_H

// copy of the aiPostProcessStep enum for usage in the wrapper load function
enum slPostProcessSteps
{
    SLProcess_CalcTangentSpace = 0x1,
    SLProcess_JoinIdenticalVertices = 0x2,
    SLProcess_MakeLeftHanded = 0x4,
    SLProcess_Triangulate = 0x8,
    SLProcess_RemoveComponent = 0x10,
    SLProcess_GenNormals = 0x20,
    SLProcess_GenSmoothNormals = 0x40,
    SLProcess_SplitLargeMeshes = 0x80,
    SLProcess_PreTransformVertices = 0x100,
    SLProcess_LimitBoneWeights = 0x200,
    SLProcess_ValidateDataStructure = 0x400,
    SLProcess_ImproveCacheLocality = 0x800,
    SLProcess_RemoveRedundantMaterials = 0x1000,
    SLProcess_FixInfacingNormals = 0x2000,
    SLProcess_SortByPType = 0x8000,
    SLProcess_FindDegenerates = 0x10000,
    SLProcess_FindInvalidData = 0x20000,
    SLProcess_GenUVCoords = 0x40000,
    SLProcess_TransformUVCoords = 0x80000,
    SLProcess_FindInstances = 0x100000,
    SLProcess_OptimizeMeshes = 0x200000,
    SLProcess_OptimizeGraph = 0x400000,
    SLProcess_FlipUVs = 0x800000,
    SLProcess_FlipWindingOrder = 0x1000000,
    SLProcess_SplitByBoneCount = 0x2000000,
    SLProcess_Debone = 0x4000000
};

// forward declarations of assimp types
// @todo    Is it good practice to not include the assimp headers here and just use forward declaration?
//          Do some research on best practices.
struct aiScene;
struct aiNode;
struct aiBone;
struct aiMaterial;
struct aiAnimation;
struct aiMesh;
struct aiVectorKey;
struct aiQuatKey;

//-----------------------------------------------------------------------------
typedef std::map<int, SLMesh*> SLMeshMap;
//-----------------------------------------------------------------------------
//! Small class interface into the AssImp library for importing 3D assets. 
/*! See AssImp library (http://assimp.sourceforge.net/) documentation for 
supported file formats and the import processing options.
*/
class SLAssImp
{  
public:
    enum LogVerbosity {
        Quiet = 0,
        Minimal = 1,
        Normal = 2,
        Detailed = 3,
        Diagnostic = 4
    };

protected:
    ofstream        _log;                   //!< log stream
    SLstring        _logFile;               //!< name of the log file
    LogVerbosity    _logConsoleVerbosity;   //!< verbosity level of log output to the console
    LogVerbosity    _logFileVerbosity;      //!< verbosity level of log output to the file

    // @todo get the bone information struct out of here
    struct BoneInformation
    {
        SLstring name;
        SLuint id;
        SLMat4f offsetMat;
    };
    

    std::map<SLstring, BoneInformation>   bones; // bone node to bone id mapping

    // old global vars just put in the class for now
    // @todo clean up names, remove unneded, etc.
    
    SLSkeleton* skel = NULL;
    std::map<SLstring, SLNode*>  nameToNodeMapping;   // node name to SLNode instance mapping

    // list of meshes utilising a skeleton
    std::vector<SLMesh*> skinnedMeshes;

    // helper functions
    SLMaterial*   loadMaterial(SLint index, aiMaterial* material, SLstring modelPath);
    SLGLTexture*  loadTexture(SLstring &path, SLTexType texType);
    SLMesh*       loadMesh(aiMesh *mesh);
    SLNode*       loadNodesRec(SLNode *curNode, aiNode *aiNode, SLMeshMap& meshes, SLbool loadMeshesOnly = true);
    void          findAndLoadSkeleton(aiNode* node);
    void          loadSkeleton(aiNode* node);
    SLBone*       loadSkeletonRec(aiNode* node, SLBone* parent);
    SLAnimation*  loadAnimation(aiAnimation* anim);
    SLstring      checkFilePath(SLstring modelPath, SLstring texFile);
    SLbool        aiNodeHasMesh(aiNode* node);
    SLbool        isBone(const SLstring& name);
    BoneInformation* getBoneInformation(const SLstring& name);
    SLNode* findLoadedNodeByName(const SLstring& name);

    // new intermediate containers
    std::map<SLstring, aiNode*>             _nameToNode;    //!< map containing name to aiNode releationships
    std::map<SLstring, aiBone*>             _nameToBone;    //!< map containing name to aiBone releationships
    std::map<int, std::vector<SLstring>>    _boneGroups;    //!< map containing bone names combined under a group, used to find skeletons
    std::vector<aiNode*>                    _skeletonRoots; //!< list containing the root nodes for all the skeletons found
    std::vector<aiMesh*>                    _skinnedMeshes; //!< list containing all of the skinned meshes, used to assign the skinned materials

    // new import helpers
    /*aiNode*         getNodeByName(const SLstring& name);    // return an aiNode ptr if name exists, or null if it doesn't
    aiBone*         getBoneByName(const SLstring& name);    // return an aiBone ptr if name exists, or null if it doesn't

    void            performInitialScan(aiScene* scene);     // populates nameToNode, nameToBone, boneGroups, skinnedMeshes,
    void            findAllNodes(aiScene* scene);           // scans the assimp scene graph structure and populates nameToNode
    void            findAllBones(aiScene* scene);           // scans all meshes in the assimp scene and populates nameToBone and boneGroups
    void            findDistinctSkeletons();                // goes over the boneGroups data to find non overlapping data
    void            findSkeletonRoots();                    // finds the common ancestor for each remaining group in boneGroups, these are our final skeleton roots
    
    SLGLTexture*    loadTexture(SLstring &path, SLTexType texType);
    SLMaterial*     loadMaterial(SLint index, aiMaterial* material, SLstring modelPath);
    SLMesh*         loadMesh(aiMesh *mesh);
    SLNode*         loadSceneNodes(SLNode* parent, aiNode* node);
    SLSkeleton*     loadSkeleton(SLBone* parent, aiNode* node);
    SLAnimation*    loadAnimation(aiAnimation* anim);*/

    // new smaller helper
    void logMessage(const SLstring& message, LogVerbosity verbosity);
    void clear();

public:
    SLAssImp();
    SLAssImp(LogVerbosity consoleVerb);
    SLAssImp(const SLstring& logFile, LogVerbosity logConsoleVerb = Normal, LogVerbosity logFileVerb = Diagnostic);
    ~SLAssImp();

      SLNode*       load           (SLstring pathFilename,
                                           SLbool loadMeshesOnly = true,
                                           SLuint flags = 
                                             SLProcess_Triangulate
                                            |SLProcess_JoinIdenticalVertices
                                            |SLProcess_SplitLargeMeshes
                                            |SLProcess_RemoveRedundantMaterials
                                            |SLProcess_SortByPType
                                            |SLProcess_FindDegenerates
                                            |SLProcess_FindInvalidData
                                            //|SLProcess_OptimizeMeshes
                                            //|SLProcess_OptimizeGraph
                                            //|SLProcess_CalcTangentSpace
                                            //|SLProcess_MakeLeftHanded
                                            //|SLProcess_RemoveComponent
                                            //|SLProcess_GenNormals
                                            //|SLProcess_GenSmoothNormals
                                            //|SLProcess_PreTransformVertices
                                            //|SLProcess_LimitBoneWeights
                                            //|SLProcess_ValidateDataStructure
                                            //|SLProcess_ImproveCacheLocality
                                            //|SLProcess_FixInfacingNormals
                                            //|SLProcess_GenUVCoords
                                            //|SLProcess_TransformUVCoords
                                            //|SLProcess_FindInstances
                                            //|SLProcess_FlipUVs
                                            //|SLProcess_FlipWindingOrder
                                            //|SLProcess_SplitByBoneCount
                                            //|SLProcess_Debone
                                          );
      static SLstring      defaultPath;
};
//-----------------------------------------------------------------------------
#endif // SLASSIMP_H
