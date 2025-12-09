// Copyright (c) Microsoft Corporation and Contributors. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "stdafx.h"
#include "mrm/platform/WindowsClient.h"

namespace Microsoft::Resources::WindowsClientEnvironment
{

static const QUALIFIER_DESCRIPTION ClientQualifierDescriptions[] = {
    {
        // CoreEnvironment::Qualifier_Language
        L"lang",
        L"en-US",
        0,
        static_cast<UINT32>(QualifierTypeIndex::LanguageList),
        900,
        700,
    },
    {
        // CoreEnvironment::Qualifier_Contrast
        L"contrast",
        ContrastValue_Standard,
        0,
        static_cast<UINT32>(QualifierTypeIndex::Contrast),
        700,
        400,
    },
    {
        // CoreEnvironment::Qualifier_Scale
        L"scale",
        ScaleValue_100,
        fQualifier_AlwaysMatches | fQualifier_IsThreadAware,
        static_cast<UINT32>(QualifierTypeIndex::Scale),
        500,
        200,
    },
    {
        // CoreEnvironment::Qualifier_HomeRegion
        L"region",
        HomeRegionValue_001,
        0,
        static_cast<UINT32>(QualifierTypeIndex::RegionId),
        950,
        800,
    },
    {
        // CoreEnvironment::Qualifier_TargetSize
        L"target",
        TargetSizeValue_256,
        fQualifier_ValueProvidedbyCaller,
        static_cast<UINT32>(QualifierTypeIndex::TargetSize),
        400,
        300,
    },
    {
        // CoreEnvironment::Qualifier_LayoutDirection
        L"layoutdir",
        LayoutDirectionValue_LTR,
        0,
        static_cast<UINT32>(QualifierTypeIndex::LayoutDirection),
        450,
        600,
    },
    {
        // CoreEnvironment::Qualifier_Configuration
        L"config",
        L"",
        fQualifier_ReadOnlyForApp,
        static_cast<UINT32>(QualifierTypeIndex::Configuration),
        1000,
        900,
    },
    {
        // WindowsCoreConstDefs::Qualifier_AlternateForm,
        L"altform",
        L"",
        fQualifier_ValueProvidedbyCaller,
        static_cast<UINT32>(QualifierTypeIndex::AlternateForm),
        10,
        100,
    },
    {
        // WindowsCoreConstDefs::Qualifier_DXFeatureLevel,
        L"dxfl",
        DXFeatureLevelValue_9,
        fQualifier_ValueProvidedbyCaller,
        static_cast<UINT32>(QualifierTypeIndex::DXFeatureLevel),
        150,
        150,
    }};

static const int NumQualifierDescriptions = ARRAYSIZE(ClientQualifierDescriptions);

// NOTE: We store the pre-computed value for efficiency, but it must
// be updated whenever the contents of the environment change in any
// way.  _Always_ run all Environment unit tests to verify that the
// stored and computed version checksums match.

static const QUALIFIER_INFO ClientQualifierInfo[] = {
    {
     version1_4_Checksum, // evToken
     version1_4_NumQualifiers,
     QualifierNames,
     ClientQualifierDescriptions,
     version1_4_NumQualifierTypes,
     QualifierTypeNames,
     QualifierTypeDescriptions},
    {
     version1_3_Checksum, // evToken
     version1_3_NumQualifiers,
     QualifierNames,
     ClientQualifierDescriptions,
     version1_3_NumQualifierTypes,
     QualifierTypeNames,
     QualifierTypeDescriptions},
    {
     version1_2_Checksum, // evToken
     version1_2_NumQualifiers,
     QualifierNames,
     ClientQualifierDescriptions,
     version1_2_NumQualifierTypes,
     QualifierTypeNames,
     QualifierTypeDescriptions},
    {
     version1_1_Checksum, // evToken
     version1_1_NumQualifiers,
     QualifierNames,
     ClientQualifierDescriptions,
     version1_1_NumQualifierTypes,
     QualifierTypeNames,
     QualifierTypeDescriptions},
    {
     version1_0_Checksum, // evToken
     version1_0_NumQualifiers,
     QualifierNames,
     ClientQualifierDescriptions,
     version1_0_NumQualifierTypes,
     QualifierTypeNames,
     QualifierTypeDescriptions},
};

// Each time a new minor version of the environment is to be created, add an entry as pVersions[1] (ie, the
// most recent previous verions) with the entries as constants.  Then update majorVersion, minorVersion, etc
// When rev'ing the major version, delete all the "previous version" array entries (ie, delete pVersions[1+])
static const MRMFILE_ENVIRONMENT_VERSION_INFO ClientVersions[] = {
    // current version
    {MajorVersion,
     MinorVersion,
     version1_4_Checksum,
     version1_4_NumQualifierTypes,
     version1_4_NumQualifiers,
     version1_4_NumResourceValueTypes,
     version1_4_NumResourceValueTypes,
     MrmEnvironment::NumResourceValueLocators,
     0},
     {MajorVersion,
     3,
     version1_3_Checksum,
     version1_3_NumQualifierTypes,
     version1_3_NumQualifiers,
     version1_3_NumResourceValueTypes,
     version1_3_NumResourceValueTypes,
     MrmEnvironment::NumResourceValueLocators,
     0},
     {MajorVersion,
     2,
     version1_2_Checksum,
     version1_2_NumQualifierTypes,
     version1_2_NumQualifiers,
     version1_2_NumResourceValueTypes,
     version1_2_NumResourceValueTypes,
     MrmEnvironment::NumResourceValueLocators,
     0},
     {MajorVersion,
     1,
     version1_1_Checksum,
     version1_1_NumQualifierTypes,
     version1_1_NumQualifiers,
     version1_1_NumResourceValueTypes,
     version1_1_NumResourceValueTypes,
     MrmEnvironment::NumResourceValueLocators,
     0},
     {MajorVersion,
     0,
     version1_0_Checksum,
     version1_0_NumQualifierTypes,
     version1_0_NumQualifiers,
     version1_0_NumResourceValueTypes,
     version1_0_NumResourceValueTypes,
     MrmEnvironment::NumResourceValueLocators,
     0},
};

static const int NumVersions = ARRAYSIZE(ClientVersions);

static const ENVIRONMENT_DESCRIPTION ClientEnvironmentDescription = {
    EnvironmentSimpleName,
    EnvironmentUniqueName,
    NumVersions,
    ClientVersions,
};

static const ENVIRONMENT_INITIALIZER ClientEnvironmentInitializer =
    {&ClientEnvironmentDescription, NoBaseEnvironment, ARRAYSIZE(ClientQualifierInfo), &ClientQualifierInfo[0]};

const ENVIRONMENT_INITIALIZER* GetEnvironmentInitializer() { return &ClientEnvironmentInitializer; }

const HRESULT GetInitializers(
    MrmPlatformVersionInternal version,
    _Out_ const ENVIRONMENT_INITIALIZER** ppEnvironmentInitializerOut,
    _Out_ const QUALIFIER_INFO** ppDefaultQualifierInfoOut)
{
    if (version == WindowsClient8 || version == MrmPlatformVersionInternal::DefaultPlatformVersion)
    {
        *ppEnvironmentInitializerOut = &ClientEnvironmentInitializer;
        *ppDefaultQualifierInfoOut = &ClientEnvironmentInitializer.pQualifierInfos[1];
    }
    else if (version == WindowsClientBlue)
    {
        *ppEnvironmentInitializerOut = &ClientEnvironmentInitializer;
        *ppDefaultQualifierInfoOut = &ClientEnvironmentInitializer.pQualifierInfos[0];
    }
    else
    {
        RETURN_HR(E_DEF_UNSUPPORTED_VERSION);
    }

    return S_OK;
}

HRESULT CreateInstance(_In_ AtomPoolGroup* pAtoms, _Outptr_ MrmEnvironment** environment)
{
    return MrmEnvironment::CreateInstance(pAtoms, &ClientEnvironmentInitializer, MajorVersion, MinorVersion, environment);
}

HRESULT CreateInstance(_In_ AtomPoolGroup* pAtoms, _In_ int major, _In_ int minor, _Outptr_ MrmEnvironment** environment)
{
    return MrmEnvironment::CreateInstance(pAtoms, &ClientEnvironmentInitializer, major, minor, environment);
}

HRESULT FindInfoForVersion(
    int major,
    int minor,
    _Out_opt_ const MRMFILE_ENVIRONMENT_VERSION_INFO** ppInfoOut,
    _Inout_opt_ StringResult* pUniqueNameOut,
    _Inout_opt_ StringResult* pSimpleNameOut)
{
    return MrmEnvironment::FindInfoForVersion(&ClientEnvironmentDescription, major, minor, ppInfoOut, pUniqueNameOut, pSimpleNameOut);
}

HRESULT FindInfoForCurrentVersion(
    _Out_opt_ const MRMFILE_ENVIRONMENT_VERSION_INFO** ppInfoOut,
    _Inout_opt_ StringResult* pUniqueNameOut,
    _Inout_opt_ StringResult* pSimpleNameOut)
{
    return MrmEnvironment::FindInfoForCurrentVersion(&ClientEnvironmentDescription, ppInfoOut, pUniqueNameOut, pSimpleNameOut);
}

} // namespace Microsoft::Resources::CoreEnvironment

namespace Microsoft::Resources::WindowsPhoneEnvironment
{

    static const QUALIFIER_DESCRIPTION PhoneQualifierDescriptions[] = {
        {
            // CoreEnvironment::Qualifier_Language
            L"lang",
            L"en-US",
            0,
            static_cast<UINT32>(QualifierTypeIndex::LanguageList),
            900,
            700,
        },
        {
            // CoreEnvironment::Qualifier_Contrast
            L"contrast",
            ContrastValue_Standard,
            0,
            static_cast<UINT32>(QualifierTypeIndex::Contrast),
            700,
            400,
        },
        {
            // CoreEnvironment::Qualifier_Scale
            L"scale",
            ScaleValue_100,
            fQualifier_AlwaysMatches | fQualifier_IsThreadAware,
            static_cast<UINT32>(QualifierTypeIndex::Scale),
            500,
            200,
        },
        {
            // CoreEnvironment::Qualifier_HomeRegion
            L"region",
            HomeRegionValue_001,
            0,
            static_cast<UINT32>(QualifierTypeIndex::RegionId),
            950,
            800,
        },
        {
            // CoreEnvironment::Qualifier_TargetSize
            L"target",
            TargetSizeValue_256,
            fQualifier_ValueProvidedbyCaller,
            static_cast<UINT32>(QualifierTypeIndex::TargetSize),
            400,
            300,
        },
        {
            // CoreEnvironment::Qualifier_LayoutDirection
            L"layoutdir",
            LayoutDirectionValue_LTR,
            0,
            static_cast<UINT32>(QualifierTypeIndex::LayoutDirection),
            450,
            600,
        },
        {
            // CoreEnvironment::Qualifier_Configuration
            L"config",
            L"",
            fQualifier_ReadOnlyForApp,
            static_cast<UINT32>(QualifierTypeIndex::Configuration),
            1000,
            900,
        },
        {
            // WindowsCoreConstDefs::Qualifier_AlternateForm,
            L"altform",
            L"",
            fQualifier_ValueProvidedbyCaller,
            static_cast<UINT32>(QualifierTypeIndex::AlternateForm),
            10,
            100,
        },
        {
            // WindowsCoreConstDefs::Qualifier_DXFeatureLevel,
            L"dxfl",
            DXFeatureLevelValue_9,
            fQualifier_ValueProvidedbyCaller,
            static_cast<UINT32>(QualifierTypeIndex::DXFeatureLevel),
            150,
            150,
        } };

    static const int NumQualifierDescriptions = ARRAYSIZE(PhoneQualifierDescriptions);

    // NOTE: We store the pre-computed value for efficiency, but it must
    // be updated whenever the contents of the environment change in any
    // way.  _Always_ run all Environment unit tests to verify that the
    // stored and computed version checksums match.

    static const QUALIFIER_INFO PhoneQualifierInfo[] = {
        {
         version1_1_Checksum, // evToken
         version1_1_NumQualifiers,
         QualifierNames,
         PhoneQualifierDescriptions,
         version1_1_NumQualifierTypes,
         QualifierTypeNames,
         QualifierTypeDescriptions},
        {
         version1_0_Checksum, // evToken
         version1_0_NumQualifiers,
         QualifierNames,
         PhoneQualifierDescriptions,
         version1_0_NumQualifierTypes,
         QualifierTypeNames,
         QualifierTypeDescriptions},
    };

    // Each time a new minor version of the environment is to be created, add an entry as pVersions[1] (ie, the
    // most recent previous verions) with the entries as constants.  Then update majorVersion, minorVersion, etc
    // When rev'ing the major version, delete all the "previous version" array entries (ie, delete pVersions[1+])
    static const MRMFILE_ENVIRONMENT_VERSION_INFO PhoneVersions[] = {
         {MajorVersion,
         MinorVersion,
         version1_1_Checksum,
         version1_1_NumQualifierTypes,
         version1_1_NumQualifiers,
         version1_1_NumResourceValueTypes,
         version1_1_NumResourceValueTypes,
         MrmEnvironment::NumResourceValueLocators,
         0},
         {MajorVersion,
         0,
         version1_0_Checksum,
         version1_0_NumQualifierTypes,
         version1_0_NumQualifiers,
         version1_0_NumResourceValueTypes,
         version1_0_NumResourceValueTypes,
         MrmEnvironment::NumResourceValueLocators,
         0},
    };

    static const int NumVersions = ARRAYSIZE(PhoneVersions);

    static const ENVIRONMENT_DESCRIPTION PhoneEnvironmentDescription = {
        EnvironmentSimpleName,
        EnvironmentUniqueName,
        NumVersions,
        PhoneVersions,
    };

    static const ENVIRONMENT_INITIALIZER PhoneEnvironmentInitializer =
    { &PhoneEnvironmentDescription, NoBaseEnvironment, ARRAYSIZE(PhoneQualifierInfo), &PhoneQualifierInfo[0] };

    const ENVIRONMENT_INITIALIZER* GetEnvironmentInitializer() { return &PhoneEnvironmentInitializer; }

    const HRESULT GetInitializers(
        MrmPlatformVersionInternal version,
        _Out_ const ENVIRONMENT_INITIALIZER** ppEnvironmentInitializerOut,
        _Out_ const QUALIFIER_INFO** ppDefaultQualifierInfoOut)
    {
        if (version == WindowsPhoneBlue || version == MrmPlatformVersionInternal::DefaultPlatformVersion)
        {
            *ppEnvironmentInitializerOut = &PhoneEnvironmentInitializer;
            *ppDefaultQualifierInfoOut = &PhoneEnvironmentInitializer.pQualifierInfos[0];
        }
        else
        {
            RETURN_HR(E_DEF_UNSUPPORTED_VERSION);
        }

        return S_OK;
    }

    HRESULT CreateInstance(_In_ AtomPoolGroup* pAtoms, _Outptr_ MrmEnvironment** environment)
    {
        return MrmEnvironment::CreateInstance(pAtoms, &PhoneEnvironmentInitializer, MajorVersion, MinorVersion, environment);
    }

    HRESULT CreateInstance(_In_ AtomPoolGroup* pAtoms, _In_ int major, _In_ int minor, _Outptr_ MrmEnvironment** environment)
    {
        return MrmEnvironment::CreateInstance(pAtoms, &PhoneEnvironmentInitializer, major, minor, environment);
    }

    HRESULT FindInfoForVersion(
        int major,
        int minor,
        _Out_opt_ const MRMFILE_ENVIRONMENT_VERSION_INFO** ppInfoOut,
        _Inout_opt_ StringResult* pUniqueNameOut,
        _Inout_opt_ StringResult* pSimpleNameOut)
    {
        return MrmEnvironment::FindInfoForVersion(&PhoneEnvironmentDescription, major, minor, ppInfoOut, pUniqueNameOut, pSimpleNameOut);
    }

    HRESULT FindInfoForCurrentVersion(
        _Out_opt_ const MRMFILE_ENVIRONMENT_VERSION_INFO** ppInfoOut,
        _Inout_opt_ StringResult* pUniqueNameOut,
        _Inout_opt_ StringResult* pSimpleNameOut)
    {
        return MrmEnvironment::FindInfoForCurrentVersion(&PhoneEnvironmentDescription, ppInfoOut, pUniqueNameOut, pSimpleNameOut);
    }

} // namespace Microsoft::Resources::WindowsPhoneEnvironment
