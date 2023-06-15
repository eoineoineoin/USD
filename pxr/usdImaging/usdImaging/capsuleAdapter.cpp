//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "pxr/usdImaging/usdImaging/capsuleAdapter.h"

#include "pxr/usdImaging/usdImaging/dataSourceImplicits-Impl.h"
#include "pxr/usdImaging/usdImaging/delegate.h"
#include "pxr/usdImaging/usdImaging/indexProxy.h"
#include "pxr/usdImaging/usdImaging/tokens.h"

#include "pxr/imaging/geomUtil/capsuleMeshGenerator.h"
#include "pxr/imaging/hd/capsuleSchema.h"
#include "pxr/imaging/hd/mesh.h"
#include "pxr/imaging/hd/meshTopology.h"
#include "pxr/imaging/hd/perfLog.h"
#include "pxr/imaging/hd/tokens.h"

#include "pxr/usd/usdGeom/capsule.h"
#include "pxr/usd/usdGeom/capsule_1.h"
#include "pxr/usd/usdGeom/xformCache.h"

#include "pxr/base/tf/type.h"

#include <cmath>

PXR_NAMESPACE_OPEN_SCOPE

namespace {
using _PrimSource0 = UsdImagingDataSourceImplicitsPrim<UsdGeomCapsule, HdCapsuleSchema>;
using _PrimSource1 = UsdImagingDataSourceImplicitsPrim<UsdGeomCapsule_1, HdCapsuleSchema>;
}


class UsdImagingCapsuleAdapter_0 : public UsdImagingCapsuleAdapterBase<_PrimSource0> {
public:

    // Override the implemetation in GprimAdapter since we don't fetch the
    // points attribute for implicit primitives.
    USDIMAGING_API
    VtValue GetPoints(
        UsdPrim const& prim,
        UsdTimeCode time) const override;
};

class UsdImagingCapsuleAdapter_1 : public UsdImagingCapsuleAdapterBase<_PrimSource1> {
public:

    // Override the implemetation in GprimAdapter since we don't fetch the
    // points attribute for implicit primitives.
    USDIMAGING_API
    VtValue GetPoints(
        UsdPrim const& prim,
        UsdTimeCode time) const override;
};

TF_REGISTRY_FUNCTION(TfType)
{
    typedef UsdImagingCapsuleAdapter_0 Adapter_0;
    TfType t0 = TfType::Define<Adapter_0, TfType::Bases<Adapter_0::BaseAdapter> >();
    t0.SetFactory< UsdImagingPrimAdapterFactory<Adapter_0> >();
    typedef UsdImagingCapsuleAdapter_1 Adapter_1;
    TfType t1 = TfType::Define<Adapter_1, TfType::Bases<Adapter_1::BaseAdapter> >();
    t1.SetFactory< UsdImagingPrimAdapterFactory<Adapter_1> >();
}

template <typename PrimSource>
UsdImagingCapsuleAdapterBase<PrimSource>::~UsdImagingCapsuleAdapterBase() 
{
}

template <typename PrimSource>
TfTokenVector
UsdImagingCapsuleAdapterBase<PrimSource>::GetImagingSubprims(UsdPrim const& prim)
{
    return { TfToken() };
}

template <typename PrimSource>
TfToken
UsdImagingCapsuleAdapterBase<PrimSource>::GetImagingSubprimType(
        UsdPrim const& prim,
        TfToken const& subprim)
{
    if (subprim.IsEmpty()) {
        return HdPrimTypeTokens->capsule;
    }
    return TfToken();
}

template <typename PrimSource>
HdContainerDataSourceHandle
UsdImagingCapsuleAdapterBase<PrimSource>::GetImagingSubprimData(
        UsdPrim const& prim,
        TfToken const& subprim,
        const UsdImagingDataSourceStageGlobals &stageGlobals)
{
    if (subprim.IsEmpty()) {
        return PrimSource::New(
            prim.GetPath(),
            prim,
            stageGlobals);
    }
    return nullptr;
}

template <typename PrimSource>
HdDataSourceLocatorSet
UsdImagingCapsuleAdapterBase<PrimSource>::InvalidateImagingSubprim(
        UsdPrim const& prim,
        TfToken const& subprim,
        TfTokenVector const& properties)
{
    if (subprim.IsEmpty()) {
        return PrimSource::Invalidate(prim, subprim, properties);
    }
    
    return HdDataSourceLocatorSet();
}

template <typename PrimSource>
bool
UsdImagingCapsuleAdapterBase<PrimSource>::IsSupported(UsdImagingIndexProxy const* index) const
{
    return index->IsRprimTypeSupported(HdPrimTypeTokens->mesh);
}

template <typename PrimSource>
SdfPath
UsdImagingCapsuleAdapterBase<PrimSource>::Populate(UsdPrim const& prim, 
                            UsdImagingIndexProxy* index,
                            UsdImagingInstancerContext const* instancerContext)

{
    return _AddRprim(HdPrimTypeTokens->mesh,
                     prim, index, GetMaterialUsdPath(prim), instancerContext);
}

template <typename PrimSource>
void 
UsdImagingCapsuleAdapterBase<PrimSource>::TrackVariability(UsdPrim const& prim,
                                          SdfPath const& cachePath,
                                          HdDirtyBits* timeVaryingBits,
                                          UsdImagingInstancerContext const* 
                                              instancerContext) const
{
    BaseAdapter::TrackVariability(
        prim, cachePath, timeVaryingBits, instancerContext);

    // Check DirtyPoints before doing variability checks, in case we can skip
    // any of them...
    if ((*timeVaryingBits & HdChangeTracker::DirtyPoints) == 0) {
        _IsVarying(prim, UsdGeomTokens->height,
                   HdChangeTracker::DirtyPoints,
                   UsdImagingTokens->usdVaryingPrimvar,
                   timeVaryingBits, /*inherited*/false);
    }
    if ((*timeVaryingBits & HdChangeTracker::DirtyPoints) == 0) {
        _IsVarying(prim, UsdGeomTokens->radius,
                   HdChangeTracker::DirtyPoints,
                   UsdImagingTokens->usdVaryingPrimvar,
                   timeVaryingBits, /*inherited*/false);
    }
    if ((*timeVaryingBits & HdChangeTracker::DirtyPoints) == 0) {
        _IsVarying(prim, UsdGeomTokens->axis,
                   HdChangeTracker::DirtyPoints,
                   UsdImagingTokens->usdVaryingPrimvar,
                   timeVaryingBits, /*inherited*/false);
    }
}

template <typename PrimSource>
HdDirtyBits
UsdImagingCapsuleAdapterBase<PrimSource>::ProcessPropertyChange(UsdPrim const& prim,
                                                SdfPath const& cachePath,
                                                TfToken const& propertyName)
{
    if (propertyName == UsdGeomTokens->height ||
        propertyName == UsdGeomTokens->radius ||
        propertyName == UsdGeomTokens->axis) {
        return HdChangeTracker::DirtyPoints;
    }

    // Allow base class to handle change processing.
    return BaseAdapter::ProcessPropertyChange(prim, cachePath, propertyName);
}

/*virtual*/ 
template <typename PrimSource>
VtValue
UsdImagingCapsuleAdapterBase<PrimSource>::GetTopology(UsdPrim const& prim,
                                      SdfPath const& cachePath,
                                      UsdTimeCode time) const
{
    TRACE_FUNCTION();
    HF_MALLOC_TAG_FUNCTION();

    // All capsules share the same topology.
    static const HdMeshTopology topology =
        HdMeshTopology(GeomUtilCapsuleMeshGenerator::GenerateTopology(
                            numRadial, numCapAxial));

    return VtValue(topology);
}

/*virtual*/
VtValue
UsdImagingCapsuleAdapter_0::GetPoints(UsdPrim const& prim,
                                    UsdTimeCode time) const
{
    TRACE_FUNCTION();
    HF_MALLOC_TAG_FUNCTION();

    UsdGeomCapsule capsule(prim);
    double height = 1.0;
    double radius = 0.5;
    TfToken axis = UsdGeomTokens->z;
    TF_VERIFY(capsule.GetHeightAttr().Get(&height, time));
    TF_VERIFY(capsule.GetRadiusAttr().Get(&radius, time));
    TF_VERIFY(capsule.GetAxisAttr().Get(&axis, time));

    // The capsule point generator computes points such that the "rings" of the
    // capsule lie on a plane parallel to the XY plane, with the Z-axis being
    // the "spine" of the capsule. These need to be transformed to the right
    // basis when a different spine axis is used.
    const GfMatrix4d basis = UsdImagingGprimAdapter::GetImplicitBasis(axis);

    const size_t numPoints =
        GeomUtilCapsuleMeshGenerator::ComputeNumPoints(numRadial, numCapAxial);

    VtVec3fArray points(numPoints);
        
    GeomUtilCapsuleMeshGenerator::GeneratePoints(
        points.begin(),
        numRadial,
        numCapAxial,
        radius,
        height,
        &basis
    );

    return VtValue(points);
}

/*virtual*/
VtValue
UsdImagingCapsuleAdapter_1::GetPoints(UsdPrim const& prim,
                                    UsdTimeCode time) const
{
    TRACE_FUNCTION();
    HF_MALLOC_TAG_FUNCTION();

    UsdGeomCapsule_1 capsule(prim);
    double height = 1.0;
    double radiusTop = 0.5;
    double radiusBottom = 0.5;
    TfToken axis = UsdGeomTokens->z;
    TF_VERIFY(capsule.GetHeightAttr().Get(&height, time));
    TF_VERIFY(capsule.GetRadiusTopAttr().Get(&radiusTop, time));
    TF_VERIFY(capsule.GetRadiusBottomAttr().Get(&radiusBottom, time));
    TF_VERIFY(capsule.GetAxisAttr().Get(&axis, time));

    // The capsule point generator computes points such that the "rings" of the
    // capsule lie on a plane parallel to the XY plane, with the Z-axis being
    // the "spine" of the capsule. These need to be transformed to the right
    // basis when a different spine axis is used.
    const GfMatrix4d basis = UsdImagingGprimAdapter::GetImplicitBasis(axis);

    const size_t numPoints =
        GeomUtilCapsuleMeshGenerator::ComputeNumPoints(numRadial, numCapAxial);

    double latitudeRange = 0.0;
    if (radiusBottom != radiusTop)
    {
        // USD describes that the height excludes the sphere radii, so we have two spheres
        // located at +height/2 and -height/2. We need to find a plane tangent to both spheres
        // to generate a smooth smooth interface between the different radii. The angle of this
        // tangent from the axis which will become the latitudeRange for the two spheres.

        // First, construct two circles:
        // * One at (0,0), of radius height * 0.5 (i.e. the centers of the caps are on this surface)
        // * One at (-height,0) of radius rBottom - rTop
        // Then, find the intersection between those two circles = q.
        // The vector |q - (-height, 0)| is perpendicular to the tangent
        double rA = radiusBottom - radiusTop;
        double rB = height * 0.5;
        double a = height * -0.5;
        GfVec2d q(0, 0);
        q[0] = (rB * rB - rA * rA + a * a) / (2 * a);
        //<todo.eoin If this value is negative, we have a degenerate capsule; should we just draw a sphere?
        q[1] = GfSqrt(rA * rA - (q[0] - a) * (q[0] - a));
        GfVec2d perpTangent = (q - GfVec2d(a, 0)).GetNormalized();
        latitudeRange = acos(perpTangent[1]);

        if (radiusTop > radiusBottom)
        {
            latitudeRange *= -1;
        }
    }
        
    VtVec3fArray points(numPoints);
    const double sweep = 360;
    GeomUtilCapsuleMeshGenerator::GeneratePoints(
        points.begin(),
        numRadial,
        numCapAxial,
        radiusBottom,
        radiusTop,
        height,
        radiusBottom,
        latitudeRange,
        radiusTop,
        latitudeRange,
        sweep,
        &basis
    );

    return VtValue(points);
}



PXR_NAMESPACE_CLOSE_SCOPE

