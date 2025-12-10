// Copyright (c) Microsoft Corporation and Contributors. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "stdafx.h"
#include <memory>

namespace Microsoft::Resources
{
    HRESULT MrmProfile::GetTargetPlatformVersionForFileMagic(
        _In_ const DEFFILE_MAGIC& fileMagicNumber,
        _Out_ MrmPlatformVersionInternal* pPlatformVersionOut)
    {
        if (fileMagicNumber.ullMagic == gWin8PriFileMagic.ullMagic)
        {
            *pPlatformVersionOut = WindowsClient8;
            return S_OK;
        }
        else if (fileMagicNumber.ullMagic == gWinBluePriFileMagic.ullMagic)
        {
            *pPlatformVersionOut = WindowsClientBlue;
            return S_OK;
        }
        else if (fileMagicNumber.ullMagic == gWindowsPhoneBluePriFileMagic.ullMagic)
        {
            *pPlatformVersionOut = WindowsPhoneBlue;
            return S_OK;
        }
        else if (fileMagicNumber.ullMagic == gUniversalPriFileMagic.ullMagic)
        {
            *pPlatformVersionOut = WindowsCore;
            return S_OK;
        }
        else if (fileMagicNumber.ullMagic == gUniversalRS4PriFileMagic.ullMagic)
        {
            *pPlatformVersionOut = WindowsCoreRS4;
            return S_OK;
        }
        else if (fileMagicNumber.ullMagic == gUniversalVNextPriFileMagic.ullMagic)
        {
            *pPlatformVersionOut = WindowsCoreVNext;
            return S_OK;
        }

        RETURN_HR(HRESULT_FROM_WIN32(ERROR_MRM_INVALID_PRI_FILE));
    }

    HRESULT MrmProfile::GetTargetPlatformAndVersionForFileMagic(
        _In_ const DEFFILE_MAGIC& fileMagicNumber,
        _Inout_opt_ StringResult* correspondingPlatformName,
        _Inout_opt_ StringResult* correspondingPlatformVersion)
    {
        if (!IsSupportedFileMagicNumber(fileMagicNumber))
        {
            RETURN_HR(HRESULT_FROM_WIN32(ERROR_MRM_INVALID_PRI_FILE));
        }

        auto platformName = CoreEnvironment::PlatformName;
        auto platformVersion = CoreEnvironment::DefaultPlatformVersion;

        if (fileMagicNumber.ullMagic == gWin8PriFileMagic.ullMagic)
        {
            platformName = WindowsClientEnvironment::PlatformName;
            platformVersion = WindowsClientEnvironment::LegacyPlatformVersion;
        }
        else if (fileMagicNumber.ullMagic == gWinBluePriFileMagic.ullMagic)
        {
            platformName = WindowsClientEnvironment::PlatformName;
            platformVersion = WindowsClientEnvironment::DefaultPlatformVersion;
        }
        else if (fileMagicNumber.ullMagic == gWindowsPhoneBluePriFileMagic.ullMagic)
        {
            platformName = WindowsPhoneEnvironment::PlatformName;
            platformVersion = WindowsPhoneEnvironment::DefaultPlatformVersion;
        }
        else if (fileMagicNumber.ullMagic == gUniversalPriFileMagic.ullMagic)
        {
            platformName = CoreEnvironment::PlatformName;
            platformVersion = CoreEnvironment::UniversalPlatformVersion;
        }

        if (correspondingPlatformName != nullptr)
        {
            RETURN_IF_FAILED(correspondingPlatformName->SetRef(platformName));
        }

        if (correspondingPlatformVersion != nullptr)
        {
            RETURN_IF_FAILED(correspondingPlatformVersion->SetRef(platformVersion));
        }

        return S_OK;
    }

    HRESULT MrmProfile::GetQualifierInfoForEnvironment(
        _In_ PCWSTR sourceName,
        _In_ const IEnvironmentVersionInfo* sourceVersion,
        _In_ const IEnvironment* targetEnvironment,
        _Out_ int* numMappedQualifiers,
        _Outptr_result_buffer_maybenull_(*numMappedQualifiers) const PCWSTR** mappedQualifierNames,
        _Outptr_result_buffer_maybenull_(*numMappedQualifiers) const Atom::SmallIndex** qualifierMappings) const
    {
        *numMappedQualifiers = 0;
        *mappedQualifierNames = nullptr;
        *qualifierMappings = nullptr;

        if (DefString_ICompare(targetEnvironment->GetUniqueName(), CoreEnvironment::EnvironmentUniqueName) == Def_Equal)
        {
            if (DefString_ICompare(sourceName, CoreEnvironment::EnvironmentSimpleName) == Def_Equal)
            {
				auto targetVersion = targetEnvironment->GetVersionInfo();
				auto sourceMajor = sourceVersion->GetMajorVersion();
				auto sourceMinor = sourceVersion->GetMinorVersion();
				auto targetMajor = targetVersion->GetMajorVersion();
				auto targetMinor = targetVersion->GetMinorVersion();

                if (sourceMajor == targetMajor == 1 && sourceMinor != 0 && targetMinor != 0)
                {
					static_assert(CoreEnvironment::MajorVersion == 1 && CoreEnvironment::MinorVersion == 2, "Update this when new Core versions are added.");
                    return S_OK;
                }

                return CoreProfile::GetQualifierInfoForEnvironment(
                    sourceName,
                    sourceVersion,
                    targetEnvironment,
                    numMappedQualifiers,
                    mappedQualifierNames,
                    qualifierMappings);
            }
            else if (DefString_ICompare(sourceName, WindowsClientEnvironment::EnvironmentSimpleName) == Def_Equal)
            {
                *numMappedQualifiers = WindowsClientEnvironment::version1_4_NumQualifiers;
                *mappedQualifierNames = WindowsClientEnvironment::QualifierNames;
                *qualifierMappings = WindowsClientEnvironment::ClientToCoreQualifierMapping;
            }
            else
            {
                *numMappedQualifiers = WindowsPhoneEnvironment::version1_1_NumQualifiers;
                *mappedQualifierNames = WindowsPhoneEnvironment::QualifierNames;
                *qualifierMappings = WindowsPhoneEnvironment::PhoneToCoreQualifierMapping;
            }
        }
        else if (DefString_ICompare(targetEnvironment->GetUniqueName(), WindowsClientEnvironment::EnvironmentUniqueName) == Def_Equal)
        {
            if (DefString_ICompare(sourceName, CoreEnvironment::EnvironmentSimpleName) == Def_Equal)
            {
                *numMappedQualifiers = CoreEnvironment::version1_1_NumQualifiers;
                *mappedQualifierNames = CoreEnvironment::QualifierNames;
                *qualifierMappings = WindowsClientEnvironment::CoreToClientQualifierMapping;
            }
            else if (DefString_ICompare(sourceName, WindowsPhoneEnvironment::EnvironmentSimpleName) == Def_Equal)
            {
                *numMappedQualifiers = WindowsPhoneEnvironment::version1_1_NumQualifiers;
                *mappedQualifierNames = WindowsPhoneEnvironment::QualifierNames;
                *qualifierMappings = WindowsPhoneEnvironment::PhoneToClientQualifierMapping;
            }
        }
        else if (DefString_ICompare(targetEnvironment->GetUniqueName(), WindowsPhoneEnvironment::EnvironmentUniqueName) == Def_Equal)
        {
            if (DefString_ICompare(sourceName, CoreEnvironment::EnvironmentSimpleName) == Def_Equal)
            {
                *numMappedQualifiers = CoreEnvironment::version1_1_NumQualifiers;
                *mappedQualifierNames = CoreEnvironment::QualifierNames;
                *qualifierMappings = WindowsPhoneEnvironment::CoreToPhoneQualifierMapping;
            }
            else if (DefString_ICompare(sourceName, WindowsClientEnvironment::EnvironmentSimpleName) == Def_Equal)
            {
                *numMappedQualifiers = WindowsClientEnvironment::version1_4_NumQualifiers;
                *mappedQualifierNames = WindowsClientEnvironment::QualifierNames;
                *qualifierMappings = WindowsClientEnvironment::ClientToPhoneQualifierMapping;
            }
        }
        else
        {
            RETURN_HR(E_MRM_UNSUPPORTED_ENVIRONMENT);
        }

        return S_OK;
    }

    HRESULT MrmProfile::GetDefaultEnvironmentForFileMagic(
        _In_ const DEFFILE_MAGIC& fileMagicNumber,
        _Inout_opt_ StringResult* nameOut,
        _Inout_opt_ EnvironmentVersionInfo* versionInfoOut) const
    {
        RETURN_HR_IF(HRESULT_FROM_WIN32(ERROR_MRM_INVALID_PRI_FILE), !IsSupportedFileMagicNumber(fileMagicNumber));

        const MRMFILE_ENVIRONMENT_VERSION_INFO* version = nullptr;
        if (fileMagicNumber.ullMagic == gWin8PriFileMagic.ullMagic)
        {
            RETURN_IF_FAILED(MrmEnvironment::FindInfoForVersion(WindowsClientEnvironment::GetEnvironmentInitializer()->pEnvironmentDescription, 1, 3, &version, nullptr, nameOut));
        }
        else if (fileMagicNumber.ullMagic == gWinBluePriFileMagic.ullMagic)
        {
            RETURN_IF_FAILED(MrmEnvironment::FindInfoForVersion(WindowsClientEnvironment::GetEnvironmentInitializer()->pEnvironmentDescription, 1, 4, &version, nullptr, nameOut));
        }
        else if (fileMagicNumber.ullMagic == gWindowsPhoneBluePriFileMagic.ullMagic)
        {
            RETURN_IF_FAILED(MrmEnvironment::FindInfoForVersion(WindowsPhoneEnvironment::GetEnvironmentInitializer()->pEnvironmentDescription, 1, 1, &version, nullptr, nameOut));
        }
        else if (fileMagicNumber.ullMagic == gUniversalPriFileMagic.ullMagic)
        {
            RETURN_IF_FAILED(MrmEnvironment::FindInfoForVersion(CoreEnvironment::GetEnvironmentInitializer()->pEnvironmentDescription, 1, 1, &version, nullptr, nameOut));
        }
        else if (fileMagicNumber.ullMagic == gUniversalRS4PriFileMagic.ullMagic)
        {
            RETURN_IF_FAILED(MrmEnvironment::FindInfoForVersion(CoreEnvironment::GetEnvironmentInitializer()->pEnvironmentDescription, 1, 2, &version, nullptr, nameOut));
        }
        else
        {
            RETURN_IF_FAILED(MrmEnvironment::FindInfoForCurrentVersion(CoreEnvironment::GetEnvironmentInitializer()->pEnvironmentDescription, &version, nullptr, nameOut));
        }

        if (versionInfoOut != nullptr)
        {
            versionInfoOut->SetVersionInfo(version);
        }
        return S_OK;
    }

    WindowsClientProfileBase::~WindowsClientProfileBase()
    {
        // TODO
    }

    WindowsClientProfileBase::WindowsClientProfileBase(_In_ MrmPlatformVersionInternal platform)
		: m_isDesignMode(false),
        m_platformVersion(platform),
        m_pEnvironmentInitializer(nullptr),
		m_pDefaultQualifierInfo(nullptr)
    {

    }

    HRESULT WindowsClientProfileBase::Initialize(_In_ MrmPlatformVersionInternal platform)
    {
        switch (platform)
        {
            case WindowsCore:
            case WindowsCoreRS4:
            case WindowsCoreVNext:
                m_pEnvironmentInitializer = CoreEnvironment::GetEnvironmentInitializer();
                m_pDefaultQualifierInfo = m_pEnvironmentInitializer->pQualifierInfos;
                break;
            case WindowsClientBlue:
            case WindowsClient8:
                WindowsClientEnvironment::GetInitializers(platform, &m_pEnvironmentInitializer, &m_pDefaultQualifierInfo);
                break;
            case WindowsPhoneBlue:
                WindowsPhoneEnvironment::GetInitializers(platform, &m_pEnvironmentInitializer, &m_pDefaultQualifierInfo);
                break;
            default:
                RETURN_HR(E_MRM_UNSUPPORTED_PLATFORM);
        }

		return S_OK;
    }

    HRESULT WindowsClientProfileBase::CreateInstance(_In_ MrmPlatformVersionInternal platform, _Outptr_ WindowsClientProfileBase** ppProfile)
    {
        std::unique_ptr<WindowsClientProfileBase> profile { new WindowsClientProfileBase(platform) };
        RETURN_IF_FAILED(profile->Initialize(platform));
        *ppProfile = profile.release();

        return S_OK;
    }

    HRESULT WindowsClientProfileBase::RemapQualifierTypeIndexToCore(
        MrmPlatformVersionInternal platform,
        Microsoft::Resources::Atom qualifierName,
        CoreEnvironment::QualifierTypeIndex* coreQualifierTypeIndex,
        bool* bucketedScale)
    {
        return RemapQualifierIndexToCore(
            platform,
            qualifierName,
            reinterpret_cast<CoreEnvironment::QualifierIndex*>(coreQualifierTypeIndex),
            bucketedScale);
    }

    HRESULT WindowsClientProfileBase::RemapQualifierIndexToCore(
        Microsoft::Resources::MrmPlatformVersionInternal platform,
        Microsoft::Resources::Atom qualifierName,
        CoreEnvironment::QualifierIndex* coreQualifierIndex,
        bool* bucketedScale)
    {   
        switch (platform)
        {
        case WindowsCore:
        case WindowsCoreRS4:
        case WindowsCoreVNext:
            *coreQualifierIndex = (CoreEnvironment::QualifierIndex)qualifierName.GetIndex();
            return S_OK;
        case WindowsClientBlue:
        case WindowsClient8:
            *coreQualifierIndex = (CoreEnvironment::QualifierIndex)(WindowsClientEnvironment::ClientToCoreQualifierMapping[qualifierName.GetIndex()]);
            return S_OK;
        case WindowsPhoneBlue:
            *coreQualifierIndex = (CoreEnvironment::QualifierIndex)(WindowsPhoneEnvironment::PhoneToCoreQualifierMapping[qualifierName.GetIndex()]);
            return S_OK;
        default:
            RETURN_HR(E_MRM_UNSUPPORTED_ENVIRONMENT);
        }
    }

    bool WindowsClientProfileBase::IsSupportedFileMagicNumber(_In_ const DEFFILE_MAGIC& fileMagicNumber) const
    {
        return fileMagicNumber.ullMagic == gWin8PriFileMagic.ullMagic ||
               fileMagicNumber.ullMagic == gWinBluePriFileMagic.ullMagic ||
               fileMagicNumber.ullMagic == gWindowsPhoneBluePriFileMagic.ullMagic ||
               fileMagicNumber.ullMagic == gUniversalPriFileMagic.ullMagic ||
               fileMagicNumber.ullMagic == gUniversalRS4PriFileMagic.ullMagic ||
            fileMagicNumber.ullMagic == gUniversalVNextPriFileMagic.ullMagic;
    }

    HRESULT WindowsClientProfileBase::CreateEnvironment(_In_ int index, _In_ AtomPoolGroup* pAtoms, _Outptr_ IEnvironment** environment) const
    {
        if (!index)
        {
            switch (m_platformVersion)
            {
            case WindowsCoreRS4:
                return Microsoft::Resources::MrmEnvironment::CreateInstance(pAtoms, CoreEnvironment::GetEnvironmentInitializer(), CoreEnvironment::MajorVersion, CoreEnvironment::MinorVersion, (MrmEnvironment**)environment);
            case WindowsCore:
                return Microsoft::Resources::MrmEnvironment::CreateInstance(pAtoms, CoreEnvironment::GetEnvironmentInitializer(), CoreEnvironment::MajorVersion, 1, (MrmEnvironment**)environment);
            case WindowsClientBlue:
                return Microsoft::Resources::MrmEnvironment::CreateInstance(pAtoms, WindowsClientEnvironment::GetEnvironmentInitializer(), WindowsClientEnvironment::MajorVersion, WindowsClientEnvironment::MinorVersion, (MrmEnvironment**)environment);
			case WindowsClient8:
				return Microsoft::Resources::MrmEnvironment::CreateInstance(pAtoms, WindowsClientEnvironment::GetEnvironmentInitializer(), WindowsClientEnvironment::MajorVersion, 3, (MrmEnvironment**)environment);
            case WindowsPhoneBlue:
				return Microsoft::Resources::MrmEnvironment::CreateInstance(pAtoms, WindowsPhoneEnvironment::GetEnvironmentInitializer(), WindowsPhoneEnvironment::MajorVersion, WindowsPhoneEnvironment::MinorVersion, (MrmEnvironment**)environment);
            default:
                break;
            }
        }

        return Microsoft::Resources::MrmEnvironment::CreateInstance(pAtoms, this->m_initializer, index, (MrmEnvironment**)environment);
    }

    MrmBuildConfiguration* WindowsClientProfileBase::GetBuildConfiguration()
    {
        if (!m_pBuildConfiguration)
            LOG_IF_FAILED(MrmBuildConfiguration::CreateInstance(this->m_platformVersion, &m_pBuildConfiguration));

		return m_pBuildConfiguration;
    }

    HRESULT WindowsClientProfileBase::GetQualifierBuildInfoByName(
        Microsoft::Resources::Atom qualifierName,
        Microsoft::Resources::UnifiedEnvironment* pUnifiedEnvironment,
        Microsoft::Resources::QualifierBuildInfo* pQualifierInfoOut) const
    {
        ResourceQualifier qualifier = { Atom::NullAtom };
        auto environment = pUnifiedEnvironment->GetDefaultEnvironment();
        auto info = environment->GetQualifierInfo();

		RETURN_IF_FAILED(environment->GetQualifier(qualifierName, &qualifier));

		auto names = environment->GetQualifierNames();
        if (names->GetPoolIndex() != qualifierName.GetPoolIndex())
        {
            RETURN_HR(E_DEF_ATOM_POOL_MISMATCH);
		}

        if (qualifierName.GetIndex() > info->numQualifiers - 1 || qualifierName.GetIndex() < 0)
        {
			RETURN_HR(E_DEF_OUT_OF_RANGE);
        }

        if (!pQualifierInfoOut)
            return S_OK;

		auto qualifierInfo = &info->pQualifiers[qualifierName.GetIndex()];
        pQualifierInfoOut->qualifier = qualifier;
        pQualifierInfoOut->pToken = qualifierInfo->pQualifierToken;
        pQualifierInfoOut->pDefaultValue = qualifierInfo->pDefaultValue;
        pQualifierInfoOut->alwaysMatches = qualifierInfo->flags & fQualifier_AlwaysMatches;
        pQualifierInfoOut->readOnlyForApp = qualifierInfo->flags & fQualifier_ReadOnlyForApp;

        return pUnifiedEnvironment->GetTypeOfQualifier(
            qualifier.name,
            &pQualifierInfoOut->pQualifierType);
    }

    HRESULT WindowsClientProfileBase::GetProviderForQualifier(
        _In_ const IEnvironment* pResolver,
        _In_ Atom qualifierAtom,
        _Out_ IQualifierValueProvider** ppProviderOut) const
    {
        /*return GetDefaultProviderForQualifier(
            pResolver,
            m_platformVersion,
			qualifierAtom,
            ppProviderOut);*/

        // TODO: RE rest of providers

        CoreEnvironment::QualifierIndex index = { };
        RETURN_IF_FAILED(RemapQualifierIndexToCore(
            m_platformVersion,
            qualifierAtom,
            &index,
			nullptr));

        if (index == CoreEnvironment::QualifierIndex::Language)
        {
            RETURN_IF_FAILED(WindowsDisplayLanguageProvider::CreateInstance(this, (WindowsDisplayLanguageProvider**)ppProviderOut));
        }
        else
        {
            RETURN_IF_FAILED(GenericQVProvider::CreateInstance((GenericQVProvider**)ppProviderOut));
        }

        return S_OK;
    }

    /*HRESULT WindowsClientProfileBase::GetDefaultProviderForQualifier(
        const Microsoft::Resources::IEnvironment* pEnvironment,
        Microsoft::Resources::MrmPlatformVersionInternal platform,
        Microsoft::Resources::Atom qualifierName,
        _Out_ IQualifierValueProvider** ppProviderOut)
    {

    }*/

    /*HRESULT WindowsClientProfileBase::GetDefaultTypeForQualifier(
        const Microsoft::Resources::IEnvironment* pEnvironment,
        Microsoft::Resources::MrmPlatformVersionInternal platform,
        Microsoft::Resources::Atom qualifierName)
    {

    }*/

    HRESULT WindowsClientProfileBase::GetTypeForQualifier(
        _In_ const IEnvironment* pEnvironment,
        _In_ Atom qualifierAtom,
        _Out_ IBuildQualifierType** ppTypeOut) const
    {
        CoreEnvironment::QualifierTypeIndex index = { };
        RETURN_IF_FAILED(RemapQualifierTypeIndexToCore(
            m_platformVersion,
            qualifierAtom,
            &index,
            nullptr));

        ResourceQualifier qualifier;

        *ppTypeOut = nullptr;

        RETURN_IF_FAILED(pEnvironment->GetQualifier(qualifierAtom, &qualifier));
        switch (index)
        {
        case CoreEnvironment::QualifierTypeIndex::LanguageList:
            RETURN_IF_FAILED(RtlLanguageListQualifierType::CreateInstance((RtlLanguageListQualifierType**)ppTypeOut));
            break;
        case CoreEnvironment::QualifierTypeIndex::Contrast:
        case CoreEnvironment::QualifierTypeIndex::RegionId:
        case CoreEnvironment::QualifierTypeIndex::LayoutDirection:
        case CoreEnvironment::QualifierTypeIndex::Theme:
            RETURN_IF_FAILED(StringIdentifierQualifierType::CreateInstance(
                12, QualifierTypeBase::RequiredValueQualifierTypeFlags, (StringIdentifierQualifierType**)ppTypeOut));
            break;
        case CoreEnvironment::QualifierTypeIndex::DXFeatureLevel:
            RETURN_IF_FAILED(StringIdentifierQualifierType::CreateInstance(
                5, QualifierTypeBase::OptionalValueQualifierTypeFlags, (StringIdentifierQualifierType**)ppTypeOut));
            break;
        case CoreEnvironment::QualifierTypeIndex::AlternateForm:
            RETURN_IF_FAILED(StringIdentifierQualifierType::CreateInstance(
                CoreEnvironment::Qualifier_AlternateForm_Max_Length,
                QualifierTypeBase::OptionalValueQualifierTypeFlags,
                (StringIdentifierQualifierType**)ppTypeOut));
            break;
        case CoreEnvironment::QualifierTypeIndex::Configuration:
            RETURN_IF_FAILED(StringIdentifierQualifierType::CreateInstance(
                CoreEnvironment::Qualifier_Configuration_Max_Length,
                QualifierTypeBase::OptionalValueQualifierTypeFlags,
                (StringIdentifierQualifierType**)ppTypeOut));
            break;
        case CoreEnvironment::QualifierTypeIndex::Scale:
            RETURN_IF_FAILED(IntegerQualifierType::CreateInstance(
                CoreEnvironment::Qualifier_Scale_Min_Value,
                CoreEnvironment::Qualifier_Scale_Max_Value,
                false,
                (IntegerQualifierType**)ppTypeOut));
            break;
        case CoreEnvironment::QualifierTypeIndex::TargetSize:
            RETURN_IF_FAILED(IntegerQualifierType::CreateInstance(
                CoreEnvironment::Qualifier_TargetSize_Min_Value,
                CoreEnvironment::Qualifier_TargetSize_Max_Value,
                true,
                (IntegerQualifierType**)ppTypeOut));
            break;
        case CoreEnvironment::QualifierTypeIndex::DeviceFamily:
            RETURN_IF_FAILED(StringIdentifierQualifierType::CreateInstance(
                CoreEnvironment::Qualifier_DeviceFamily_MaxLength,
                QualifierTypeBase::RequiredValueQualifierTypeFlags,
                (StringIdentifierQualifierType**)ppTypeOut));
            break;
        case CoreEnvironment::QualifierTypeIndex::CustomQualifier:
            RETURN_IF_FAILED(StringIdentifierQualifierType::CreateInstance(
                CoreEnvironment::Qualifier_DeviceFamily_MaxLength,
                QualifierTypeBase::OptionalListQualifierTypeFlags,
                (StringIdentifierQualifierType**)ppTypeOut));
            break;
        default:
            RETURN_HR(HRESULT_FROM_WIN32(ERROR_BAD_FORMAT));
            break;
        }

        return S_OK;
    }

    HRESULT WindowsClientProfileBase::GetEnvironmentVersionInfo(
        _In_ int index,
        _Inout_ StringResult* environmentName,
        _Outptr_ IEnvironmentVersionInfo** info) const
    {
        const MRMFILE_ENVIRONMENT_VERSION_INFO* pVersionInfo = nullptr;

        RETURN_IF_FAILED(MrmEnvironment::FindInfoForCurrentVersion(
            m_pEnvironmentInitializer->pEnvironmentDescription,
            &pVersionInfo,
            nullptr,
            environmentName));

		RETURN_IF_FAILED(EnvironmentVersionInfo::CreateInstance(pVersionInfo, (EnvironmentVersionInfo**)info));
		return S_OK;
    }

    HRESULT WindowsClientProfileBase::GetTargetPlatformAndVersionForFileMagic(
        _In_ const DEFFILE_MAGIC& fileMagicNumber,
        _Inout_opt_ StringResult* correspondingPlatformName,
        _Inout_opt_ StringResult* correspondingPlatformVersion)
    {
        return MrmProfile::GetTargetPlatformAndVersionForFileMagic(
            fileMagicNumber,
            correspondingPlatformName,
            correspondingPlatformVersion);
    }

    HRESULT WindowsClientProfileBase::GetTargetPlatformAndVersion(
        _Inout_opt_ StringResult* correspondingPlatformName,
        _Inout_opt_ StringResult* correspondingPlatformVersion)
    {
        return MrmProfile::GetTargetPlatformAndVersionForFileMagic(
            GetBuildConfiguration()->GetFileMagicNumber(),
            correspondingPlatformName,
            correspondingPlatformVersion);
    }

    bool WindowsClientProfileBase::IsCompatibleEnvironment(
        _In_ const EnvironmentReference* wantEnvironment,
        _In_ const IEnvironment* haveEnvironment,
        _Outptr_result_maybenull_ const RemapAtomPool** qualifierMapping) const
    {
        return this->m_platformVersion == TestIncompatiblePlatformVersion
            || this->m_platformVersion == TestFutureCoreVersion
            || this->m_platformVersion == TestFutureCoreVersion2
            || Microsoft::Resources::CoreProfile::IsCompatibleEnvironment(
                wantEnvironment,
                haveEnvironment,
                qualifierMapping);
    }
}