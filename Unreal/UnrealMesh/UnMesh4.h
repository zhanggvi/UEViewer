#ifndef __UNMESH4_H__
#define __UNMESH4_H__

#if UNREAL4

#include "UnMesh.h"			// common types
#include "UnMesh3.h"		// animation enums

// forwards
class UMaterialInterface;
class UAnimSequence4;

#define NUM_INFLUENCES_UE4				4
#define MAX_TOTAL_INFLUENCES_UE4		8
#define MAX_SKELETAL_UV_SETS_UE4		4
#define MAX_STATIC_UV_SETS_UE4			8
#define MAX_STATIC_LODS_UE4				8		// 4 before 4.9, 8 starting with 4.9

//!! These classes should be explicitly defined
typedef UObject UStaticMeshSocket;


/*-----------------------------------------------------------------------------
	USkeletalMesh
-----------------------------------------------------------------------------*/

struct FSkeletalMaterial;
struct FStaticLODModel4;
struct FSkeletalMeshLODInfo;
struct FMeshUVChannelInfo;

struct FMeshBoneInfo
{
	FName					Name;
	int						ParentIndex;

	friend FArchive& operator<<(FArchive& Ar, FMeshBoneInfo& B);
};

struct FReferenceSkeleton
{
	TArray<FMeshBoneInfo>	RefBoneInfo;
	TArray<FTransform>		RefBonePose;
	TMap<FName, int32>		NameToIndexMap;

	friend FArchive& operator<<(FArchive& Ar, FReferenceSkeleton& S);
};


struct FReferencePose
{
	FName					PoseName;
	TArray<FTransform>		ReferencePose;

	friend FArchive& operator<<(FArchive& Ar, FReferencePose& P);
};

struct FBoneReference
{
	FName					BoneName;

	friend FArchive& operator<<(FArchive& Ar, FBoneReference& B)
	{
		return Ar << B.BoneName;
	}
};

struct FCurveMetaData
{
	bool					bMaterial;
	bool					bMorphtarget;
	TArray<FBoneReference>	LinkedBones;
	uint8					MaxLOD;

	friend FArchive& operator<<(FArchive& Ar, FCurveMetaData& D)
	{
		Ar << D.bMaterial << D.bMorphtarget << D.LinkedBones;
		if (FAnimPhysObjectVersion::Get(Ar) >= FAnimPhysObjectVersion::AddLODToCurveMetaData)
		{
			Ar << D.MaxLOD;
#if KH3
			if (Ar.Game == GAME_KH3) Ar.Seek(Ar.Tell() + 3); // MaxLOD is int32 here
#endif
		}
		return Ar;
	}
};

struct FSmartNameMapping
{
	//!! TODO: no data fields here
	int32 Dummy;

	friend FArchive& operator<<(FArchive& Ar, FSmartNameMapping& N);
};


struct FSmartNameContainer
{
	TMap<FName, FSmartNameMapping> NameMappings;

	friend FArchive& operator<<(FArchive& Ar, FSmartNameContainer& C)
	{
		return Ar << C.NameMappings;
	}
};


struct FVirtualBone
{
	DECLARE_STRUCT(FVirtualBone);

	FName					SourceBoneName;
	FName					TargetBoneName;
	FName					VirtualBoneName;

	BEGIN_PROP_TABLE
		PROP_NAME(SourceBoneName)
		PROP_NAME(TargetBoneName)
		PROP_NAME(VirtualBoneName)
	END_PROP_TABLE
};

enum EBoneTranslationRetargetingMode
{
	Animation,				// use translation from animation
	Skeleton,				// use translation from skeleton
	AnimationScaled,
	AnimationRelative,
	OrientAndScale,
};

_ENUM(EBoneTranslationRetargetingMode)
{
	_E(Animation),
	_E(Skeleton),
	_E(AnimationScaled),
	_E(AnimationRelative),
	_E(OrientAndScale),
};

struct FBoneNode
{
	DECLARE_STRUCT(FBoneNode)

	EBoneTranslationRetargetingMode TranslationRetargetingMode;

	BEGIN_PROP_TABLE
		PROP_ENUM2(TranslationRetargetingMode, EBoneTranslationRetargetingMode)
	END_PROP_TABLE
};

class USkeleton : public UObject
{
	DECLARE_CLASS(USkeleton, UObject);
public:
	FReferenceSkeleton		ReferenceSkeleton;
	TMap<FName, FReferencePose> AnimRetargetSources;
	FGuid					Guid;
	FSmartNameContainer		SmartNames;
	TArray<FVirtualBone>	VirtualBones;
	TArray<FBoneNode>		BoneTree;
	//!! TODO: sockets

	BEGIN_PROP_TABLE
		PROP_ARRAY(VirtualBones, FVirtualBone)
		PROP_ARRAY(BoneTree, FBoneNode)
		PROP_DROP(Notifies)			// not working with notifies in our program
	END_PROP_TABLE

	USkeleton()
	:	ConvertedAnim(NULL)
	{}

	virtual ~USkeleton();

	// generated stuff
	TArray<UAnimSequence4*>	OriginalAnims;
	CAnimSet*				ConvertedAnim;

	virtual void Serialize(FArchive &Ar);
	virtual void PostLoad();

	void ConvertAnims(UAnimSequence4* Seq);
};


class USkeletalMesh4 : public UObject
{
	DECLARE_CLASS(USkeletalMesh4, UObject);
public:
	FBoxSphereBounds		Bounds;
	TArray<FSkeletalMaterial> Materials;
	FReferenceSkeleton		RefSkeleton;
	USkeleton				*Skeleton;
	TArray<FStaticLODModel4> LODModels;
	TArray<FSkeletalMeshLODInfo> LODInfo;
	TArray<UMorphTarget*>	MorphTargets;
#if BORDERLANDS3
	byte					NumVertexColorChannels;
#endif

	// properties
	bool					bHasVertexColors;

	CSkeletalMesh			*ConvertedMesh;

	BEGIN_PROP_TABLE
		PROP_OBJ(Skeleton)
		PROP_BOOL(bHasVertexColors)
		PROP_ARRAY(LODInfo, FSkeletalMeshLODInfo)
		PROP_ARRAY(MorphTargets, UObject*)
#if BORDERLANDS3
		PROP_BYTE(NumVertexColorChannels)
#endif
		PROP_DROP(bHasBeenSimplified)
		PROP_DROP(SamplingInfo)
		PROP_DROP(MinLod)
		PROP_DROP(PhysicsAsset)
		PROP_DROP(ShadowPhysicsAsset)
	END_PROP_TABLE

	USkeletalMesh4();
	virtual ~USkeletalMesh4();

	virtual void Serialize(FArchive &Ar);
	virtual void PostLoad();

protected:
	void ConvertMesh();
};


/*-----------------------------------------------------------------------------
	UDestructibleMesh
-----------------------------------------------------------------------------*/

class UDestructibleMesh : public USkeletalMesh4
{
	DECLARE_CLASS(UDestructibleMesh, USkeletalMesh4);

	virtual void Serialize(FArchive& Ar)
	{
		Super::Serialize(Ar);
		// This class has lots of PhysX data
		DROP_REMAINING_DATA(Ar);
	}
};

/*-----------------------------------------------------------------------------
	UStaticMesh
-----------------------------------------------------------------------------*/

// forwards (structures are declared in cpp)
struct FStaticMeshLODModel4;

struct FMeshBuildSettings
{
	DECLARE_STRUCT(FMeshBuildSettings);

	bool					bRecomputeNormals;
	bool					bRecomputeTangents;

	BEGIN_PROP_TABLE
		PROP_BOOL(bRecomputeNormals)
		PROP_BOOL(bRecomputeTangents)
		PROP_DROP(bUseMikkTSpace)
		PROP_DROP(bRemoveDegenerates)
		PROP_DROP(bBuildAdjacencyBuffer)
		PROP_DROP(bBuildReversedIndexBuffer)
		PROP_DROP(bUseFullPrecisionUVs)
		PROP_DROP(bGenerateLightmapUVs)
		PROP_DROP(MinLightmapResolution)
		PROP_DROP(SrcLightmapIndex)
		PROP_DROP(DstLightmapIndex)
		PROP_DROP(BuildScale3D)						//?? FVector, use?
		PROP_DROP(DistanceFieldResolutionScale)
		PROP_DROP(bGenerateDistanceFieldAsIfTwoSided)
		PROP_DROP(DistanceFieldReplacementMesh)
	END_PROP_TABLE
};

struct FStaticMeshSourceModel
{
	DECLARE_STRUCT(FStaticMeshSourceModel);
	FByteBulkData			BulkData;
	FMeshBuildSettings		BuildSettings;

	BEGIN_PROP_TABLE
		PROP_STRUC(BuildSettings, FMeshBuildSettings)
		PROP_DROP(ReductionSettings)
		PROP_DROP(ScreenSize)
	END_PROP_TABLE
};

#define TEXSTREAM_MAX_NUM_UVCHANNELS	4

struct FMeshUVChannelInfo
{
	bool					bInitialized;
	bool					bOverrideDensities;
	float					LocalUVDensities[TEXSTREAM_MAX_NUM_UVCHANNELS];

	friend FArchive& operator<<(FArchive& Ar, FMeshUVChannelInfo& V)
	{
		Ar << V.bInitialized << V.bOverrideDensities;
		for (int i = 0; i < TEXSTREAM_MAX_NUM_UVCHANNELS; i++)
		{
			Ar << V.LocalUVDensities[i];
		}
		return Ar;
	}
};

struct FStaticMaterial
{
	DECLARE_STRUCT(FStaticMaterial);
	UMaterialInterface* MaterialInterface;
	FName				MaterialSlotName;
	FMeshUVChannelInfo	UVChannelData;

	BEGIN_PROP_TABLE
		PROP_OBJ(MaterialInterface)
		PROP_NAME(MaterialSlotName)
		PROP_DROP(ImportedMaterialSlotName)
		PROP_DROP(UVChannelData)
	END_PROP_TABLE
};

class UStaticMesh4 : public UObject
{
	DECLARE_CLASS(UStaticMesh4, UObject);
public:
	CStaticMesh				*ConvertedMesh;

	UObject					*BodySetup;			// UBodySetup
	UObject*				NavCollision;		// UNavCollision
	FGuid					LightingGuid;
	bool					bUseHighPrecisionTangentBasis;

	TArray<UStaticMeshSocket*> Sockets;
	TArray<UMaterialInterface*> Materials;		// pre-UE4.14
	TArray<FStaticMaterial> StaticMaterials;	// UE4.14+

	// FStaticMeshRenderData fields
	FBoxSphereBounds		Bounds;
	float					ScreenSize[MAX_STATIC_LODS_UE4];
	bool					bLODsShareStaticLighting;
	TArray<FStaticMeshLODModel4> Lods;
	TArray<FStaticMeshSourceModel> SourceModels;

	BEGIN_PROP_TABLE
		PROP_ARRAY(Materials, UObject*)
		PROP_ARRAY(SourceModels, FStaticMeshSourceModel)
		PROP_ARRAY(StaticMaterials, FStaticMaterial)
		PROP_DROP(LightmapUVDensity)
		PROP_DROP(LightMapResolution)
		PROP_DROP(ExtendedBounds)
		PROP_DROP(LightMapCoordinateIndex)
	END_PROP_TABLE

	UStaticMesh4();
	virtual ~UStaticMesh4();

	virtual void Serialize(FArchive &Ar);

protected:
	void ConvertMesh();
	void ConvertSourceModels();
};


/*-----------------------------------------------------------------------------
	UAnimSequence
-----------------------------------------------------------------------------*/

//!! References:
//!! CompressAnimationsFunctor() - Source/Editor/UnrealEd/Private/Commandlets/PackageUtilities.cpp

struct FCompressedOffsetData
{
	DECLARE_STRUCT(FCompressedOffsetData);
public:
	TArray<int32>			OffsetData;
	int32					StripSize;

	BEGIN_PROP_TABLE
		PROP_ARRAY(OffsetData, int)
		PROP_INT(StripSize)
	END_PROP_TABLE

	int GetOffsetData(int Index, int Offset = 0)
	{
		return OffsetData[Index * StripSize + Offset];
	}

	friend FArchive& operator<<(FArchive& Ar, FCompressedOffsetData& D)
	{
		return Ar << D.OffsetData << D.StripSize;
	}

	bool IsValid() const
	{
		return StripSize > 0 && OffsetData.Num() != 0;
	}
};


struct FTrackToSkeletonMap
{
	DECLARE_STRUCT(FTrackToSkeletonMap);

	int32					BoneTreeIndex;

	BEGIN_PROP_TABLE
		PROP_INT(BoneTreeIndex)
		PROP_DROP(SkeletonIndex)	// this field was deprecated even before 4.0 release, however it appears in some old assets
	END_PROP_TABLE

	friend FArchive& operator<<(FArchive& Ar, FTrackToSkeletonMap& M)
	{
		return Ar << M.BoneTreeIndex;
	}
};


struct FRichCurve // : public FIndexedCurve
{
	DECLARE_STRUCT(FRichCurve)

	int						Dummy;	// just to make non-empty structure

	BEGIN_PROP_TABLE
		PROP_DROP(PreInfinityExtrap)
		PROP_DROP(PostInfinityExtrap)
		PROP_DROP(DefaultValue)
		PROP_DROP(Keys)
	END_PROP_TABLE
};


struct FAnimCurveBase
{
	DECLARE_STRUCT(FAnimCurveBase);

	int						CurveTypeFlags;

	BEGIN_PROP_TABLE
		PROP_DROP(Name)			// FSmartName
		PROP_DROP(LastObservedName)
		PROP_INT(CurveTypeFlags)
	END_PROP_TABLE

	void PostSerialize(FArchive& Ar);
};



struct FFloatCurve : public FAnimCurveBase
{
	DECLARE_STRUCT2(FFloatCurve, FAnimCurveBase)

	FRichCurve				FloatCurve;

	BEGIN_PROP_TABLE
		PROP_STRUC(FloatCurve, FRichCurve)
	END_PROP_TABLE
};


struct FRawCurveTracks
{
	DECLARE_STRUCT(FRawCurveTracks);

	TArray<FFloatCurve>		FloatCurves;

	BEGIN_PROP_TABLE
		PROP_ARRAY(FloatCurves, FFloatCurve)
	END_PROP_TABLE

	void PostSerialize(FArchive& Ar);

	friend FArchive& operator<<(FArchive& Ar, FRawCurveTracks& T);
};


class UAnimationAsset : public UObject
{
	DECLARE_CLASS(UAnimationAsset, UObject);
public:
	USkeleton*				Skeleton;
	FGuid					SkeletonGuid;

	BEGIN_PROP_TABLE
		PROP_OBJ(Skeleton)
	END_PROP_TABLE

	UAnimationAsset()
	:	Skeleton(NULL)
	{}

	virtual void Serialize(FArchive& Ar)
	{
		Super::Serialize(Ar);
		if (Ar.ArVer >= VER_UE4_SKELETON_GUID_SERIALIZATION)
			Ar << SkeletonGuid;
	}
};


class UAnimSequenceBase : public UAnimationAsset
{
	DECLARE_CLASS(UAnimSequenceBase, UAnimationAsset);
public:
	FRawCurveTracks			RawCurveData;

	BEGIN_PROP_TABLE
		PROP_STRUC(RawCurveData, FRawCurveTracks)
		PROP_DROP(Notifies)			// not working with notifies in our program
	END_PROP_TABLE

	virtual void Serialize(FArchive& Ar);
};

enum EAnimInterpolationType
{
	Linear,
	Step,
};

_ENUM(EAnimInterpolationType)
{
	_E(Linear),
	_E(Step),
};

enum EAdditiveAnimationType
{
	AAT_None,
	AAT_LocalSpaceBase,
	AAT_RotationOffsetMeshSpace,
};

_ENUM(EAdditiveAnimationType)
{
	_E(AAT_None),
	_E(AAT_LocalSpaceBase),
	_E(AAT_RotationOffsetMeshSpace),
};

struct FCompressedSegment
{
	int32					StartFrame;
	int32					NumFrames;
	int32					ByteStreamOffset;
	AnimationCompressionFormat TranslationCompressionFormat;
	AnimationCompressionFormat RotationCompressionFormat;
	AnimationCompressionFormat ScaleCompressionFormat;

	friend FArchive& operator<<(FArchive& Ar, FCompressedSegment& S)
	{
		Ar << S.StartFrame << S.NumFrames << S.ByteStreamOffset;
		Ar << (byte&)S.TranslationCompressionFormat << (byte&)S.RotationCompressionFormat << (byte&)S.ScaleCompressionFormat;
		return Ar;
	}
};

class UAnimSequence4 : public UAnimSequenceBase
{
	DECLARE_CLASS(UAnimSequence4, UAnimSequenceBase);
public:
	int32					NumFrames;
	float					RateScale;
	float					SequenceLength;
	TArray<FRawAnimSequenceTrack> RawAnimationData;
	TArray<uint8>			CompressedByteStream;
	TArray<FCompressedSegment> CompressedSegments;
	bool					bUseRawDataOnly;

	AnimationKeyFormat		KeyEncodingFormat;
	AnimationCompressionFormat TranslationCompressionFormat;
	AnimationCompressionFormat RotationCompressionFormat;
	AnimationCompressionFormat ScaleCompressionFormat;
	TArray<int32>			CompressedTrackOffsets;
	FCompressedOffsetData	CompressedScaleOffsets;
	TArray<FTrackToSkeletonMap> TrackToSkeletonMapTable;			// used for raw data
	TArray<FTrackToSkeletonMap> CompressedTrackToSkeletonMapTable;	// used for compressed data, missing before 4.12
	FRawCurveTracks			CompressedCurveData;
	EAnimInterpolationType	Interpolation;
	EAdditiveAnimationType	AdditiveAnimType;

	BEGIN_PROP_TABLE
		PROP_INT(NumFrames)
		PROP_FLOAT(RateScale)
		PROP_FLOAT(SequenceLength)
		// Before UE4.12 some fields were serialized as properties
		PROP_ENUM2(KeyEncodingFormat, AnimationKeyFormat)
		PROP_ENUM2(TranslationCompressionFormat, AnimationCompressionFormat)
		PROP_ENUM2(RotationCompressionFormat, AnimationCompressionFormat)
		PROP_ENUM2(ScaleCompressionFormat, AnimationCompressionFormat)
		PROP_ARRAY(CompressedTrackOffsets, int)
		PROP_STRUC(CompressedScaleOffsets, FCompressedOffsetData)
		PROP_ARRAY(TrackToSkeletonMapTable, FTrackToSkeletonMap)
		PROP_ARRAY(CompressedTrackToSkeletonMapTable, FTrackToSkeletonMap)
		PROP_STRUC(CompressedCurveData, FRawCurveTracks)
		PROP_ENUM2(Interpolation, EAnimInterpolationType)
		PROP_ENUM2(AdditiveAnimType, EAdditiveAnimationType)
		PROP_DROP(RetargetSource)
	END_PROP_TABLE

	UAnimSequence4()
	:	RateScale(1.0f)
	,	TranslationCompressionFormat(ACF_None)
	,	RotationCompressionFormat(ACF_None)
	,	ScaleCompressionFormat(ACF_None)
	,	KeyEncodingFormat(AKF_ConstantKeyLerp)
	,	Interpolation(Linear)
	,	AdditiveAnimType(AAT_None)
	{}

	virtual void Serialize(FArchive& Ar);
	virtual void PostLoad();

	// UE4.12-4.22
	void SerializeCompressedData(FArchive& Ar);
	// UE4.23+
	void SerializeCompressedData2(FArchive& Ar);
	// UE4.25+
	void SerializeCompressedData3(FArchive& Ar);

	int GetNumTracks() const;
	int GetTrackBoneIndex(int TrackIndex) const;
	int FindTrackForBoneIndex(int BoneIndex) const;
	void TransferPerTrackData(TArray<uint8>& Dst, const TArray<uint8>& Src);
};


class UAnimStreamable : public UAnimSequenceBase
{
	DECLARE_CLASS(UAnimStreamable, UAnimSequenceBase);
public:
	virtual void Serialize(FArchive& Ar)
	{
		appPrintf("UAnimStreamable: not implemented\n");
		DROP_REMAINING_DATA(Ar);
	}
};

#define REGISTER_MESH_CLASSES_U4 \
	REGISTER_CLASS(USkeleton) \
	REGISTER_CLASS(FSkeletalMeshLODInfo) \
	REGISTER_CLASS_ALIAS(USkeletalMesh4, USkeletalMesh) \
	REGISTER_CLASS(UMorphTarget) \
	REGISTER_CLASS(UDestructibleMesh) \
	REGISTER_CLASS_ALIAS(UStaticMesh4, UStaticMesh) \
	REGISTER_CLASS(FMeshBuildSettings) \
	REGISTER_CLASS(FStaticMeshSourceModel) \
	REGISTER_CLASS(FStaticMaterial) \
	REGISTER_CLASS(FCompressedOffsetData) \
	REGISTER_CLASS(FTrackToSkeletonMap) \
	REGISTER_CLASS(FRichCurve) \
	REGISTER_CLASS(FFloatCurve) \
	REGISTER_CLASS(FRawCurveTracks) \
	REGISTER_CLASS(FVirtualBone) \
	REGISTER_CLASS(FBoneNode) \
	REGISTER_CLASS_ALIAS(UAnimSequence4, UAnimSequence) \
	REGISTER_CLASS(UAnimStreamable)

#define REGISTER_MESH_ENUMS_U4 \
	REGISTER_ENUM(EAnimInterpolationType) \
	REGISTER_ENUM(EBoneTranslationRetargetingMode) \
	REGISTER_ENUM(EAdditiveAnimationType)


#endif // UNREAL4
#endif // __UNMESH4_H__
