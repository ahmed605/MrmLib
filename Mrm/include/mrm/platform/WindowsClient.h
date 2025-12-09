// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include "mrm/platform/base.h"
#include "mrm/platform/MrmConstants.h"

namespace Microsoft::Resources
{

    // forward declarations
    class ExtensibilityAdapterBase;

    namespace WindowsClientEnvironment
    {

        __declspec(selectany) extern const DefChecksum::Checksum version1_0_Checksum = 0x0C5872444;
        __declspec(selectany) extern const DefChecksum::Checksum version1_1_Checksum = 0x8AE57D17;
        __declspec(selectany) extern const DefChecksum::Checksum version1_2_Checksum = 0x29CB5A06;
        __declspec(selectany) extern const DefChecksum::Checksum version1_3_Checksum = 0x2AEB3101;
        __declspec(selectany) extern const DefChecksum::Checksum version1_4_Checksum = 0x64D4872B;
        __declspec(selectany) extern const DefChecksum::Checksum currentChecksum = version1_4_Checksum;

        __declspec(selectany) extern const QUALIFIER_TYPE_DESCRIPTION QualifierTypeDescriptions[] = {
            fQualifierType_NotUsingExtensionEvaluator, // Language List
            fQualifierType_NotUsingExtensionEvaluator, // Contrast
            fQualifierType_NotUsingExtensionEvaluator, // Scale
            fQualifierType_NotUsingExtensionEvaluator, // Region ID
            fQualifierType_NotUsingExtensionEvaluator, // Target Size
            fQualifierType_NotUsingExtensionEvaluator, // Layout Direction
            fQualifierType_NotUsingExtensionEvaluator, // Configuration
            fQualifierType_NotUsingExtensionEvaluator, // Alternate Form
            fQualifierType_NotUsingExtensionEvaluator, // DX Feature Level
        };

        const ENVIRONMENT_INITIALIZER* GetEnvironmentInitializer();

        const HRESULT GetInitializers(
            _In_ MrmPlatformVersionInternal version,
            _Out_ const ENVIRONMENT_INITIALIZER** ppEnvironmentInitializerOut,
            _Out_ const QUALIFIER_INFO** ppDefaultQualifierInfoOut);

        HRESULT CreateInstance(_In_ AtomPoolGroup* pAtoms, _Outptr_ MrmEnvironment** environment);

        HRESULT CreateInstance(_In_ AtomPoolGroup* pAtoms, _In_ int major, _In_ int minor, _Outptr_ MrmEnvironment** environment);

        HRESULT FindInfoForVersion(
            _In_ int major,
            _In_ int minor,
            _Out_opt_ const MRMFILE_ENVIRONMENT_VERSION_INFO** ppInfoOut,
            _Inout_opt_ StringResult* pUniqueNameOut,
            _Inout_opt_ StringResult* pSimpleNameOut);

        HRESULT FindInfoForCurrentVersion(
            _Out_opt_ const MRMFILE_ENVIRONMENT_VERSION_INFO** ppInfoOut,
            _Inout_opt_ StringResult* pUniqueNameOut,
            _Inout_opt_ StringResult* pSimpleNameOut);

    } // namespace WindowsClientEnvironment

    namespace WindowsPhoneEnvironment
    {

        __declspec(selectany) extern const DefChecksum::Checksum version1_0_Checksum = 0x7103FD70;
        __declspec(selectany) extern const DefChecksum::Checksum version1_1_Checksum = 0x0BADC0FFE;
        __declspec(selectany) extern const DefChecksum::Checksum currentChecksum = version1_1_Checksum;

        __declspec(selectany) extern const QUALIFIER_TYPE_DESCRIPTION QualifierTypeDescriptions[] = {
            fQualifierType_NotUsingExtensionEvaluator, // Language List
            fQualifierType_NotUsingExtensionEvaluator, // Contrast
            fQualifierType_NotUsingExtensionEvaluator, // Scale
            fQualifierType_NotUsingExtensionEvaluator, // Region ID
            fQualifierType_NotUsingExtensionEvaluator, // Target Size
            fQualifierType_NotUsingExtensionEvaluator, // Layout Direction
            fQualifierType_NotUsingExtensionEvaluator, // Configuration
            fQualifierType_NotUsingExtensionEvaluator, // Alternate Form
            fQualifierType_NotUsingExtensionEvaluator, // DX Feature Level
        };

        const ENVIRONMENT_INITIALIZER* GetEnvironmentInitializer();

        const HRESULT GetInitializers(
            _In_ MrmPlatformVersionInternal version,
            _Out_ const ENVIRONMENT_INITIALIZER** ppEnvironmentInitializerOut,
            _Out_ const QUALIFIER_INFO** ppDefaultQualifierInfoOut);

        HRESULT CreateInstance(_In_ AtomPoolGroup* pAtoms, _Outptr_ MrmEnvironment** environment);

        HRESULT CreateInstance(_In_ AtomPoolGroup* pAtoms, _In_ int major, _In_ int minor, _Outptr_ MrmEnvironment** environment);

        HRESULT FindInfoForVersion(
            _In_ int major,
            _In_ int minor,
            _Out_opt_ const MRMFILE_ENVIRONMENT_VERSION_INFO** ppInfoOut,
            _Inout_opt_ StringResult* pUniqueNameOut,
            _Inout_opt_ StringResult* pSimpleNameOut);

        HRESULT FindInfoForCurrentVersion(
            _Out_opt_ const MRMFILE_ENVIRONMENT_VERSION_INFO** ppInfoOut,
            _Inout_opt_ StringResult* pUniqueNameOut,
            _Inout_opt_ StringResult* pSimpleNameOut);

    } // namespace WindowsPhoneEnvironment
} // namespace Microsoft::Resources
