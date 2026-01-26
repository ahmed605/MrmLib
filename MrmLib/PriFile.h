#pragma once
#include "PriFile.g.h"

#include <common/Base.h>
#include <DefObject.h>
#include <Results.h>
#include <Atoms.h>
#include <readers/MrmManagers.h>
#include <readers/MrmReaders.h>
#include <build/MrmBuilders.h>
#include <Platform/Base.h>

#include <namespaces.h>

namespace winrt::MrmLib::implementation
{
    using namespace ::winrt::Windows::Foundation::Collections;

    struct PriFile : PriFileT<PriFile>
    {
    private:
        static std::unique_ptr<mrm::CoreProfile> s_coreProfile;

        com_array<uint8_t> m_priFileBytes;
        std::unique_ptr<mrm::StandalonePriFile> m_priFile;
        IVector<winrt::MrmLib::ResourceCandidate> m_resourceCandidates { nullptr };

        hstring m_simpleId;
		hstring m_uniqueId;
        uint32_t m_checksum;
		const DEFFILE_HEADER* m_header;
		mrm::MrmPlatformVersionInternal m_version;
		bool m_idsChanged { false };

    public:
        PriFile() = default;
        PriFile(mrm::StandalonePriFile* priFile, com_array<uint8_t>&& priBytes = { });

        static winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::PriFile> LoadAsync(array_view<uint8_t const> priBytes);
        static winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::PriFile> LoadAsync(hstring priFilePath);
        static winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::PriFile> LoadAsync(winrt::Windows::Storage::StorageFile priFile);
        static winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::PriFile> LoadAsync(winrt::Windows::Storage::Streams::IBuffer priBytesBuffer);

        winrt::Windows::Foundation::Collections::IVector<winrt::MrmLib::ResourceCandidate> ResourceCandidates();

        winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::ReplacePathCandidatesWithEmbeddedDataResult> ReplacePathCandidatesWithEmbeddedDataAsync(winrt::Windows::Storage::StorageFolder sourceFolderToEmbed);
        winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::ReplacePathCandidatesWithEmbeddedDataResult> ReplacePathCandidatesWithEmbeddedDataAsync(hstring sourceFolderPathToEmbed);

        void WriteInternal(void (*pCallback)(array_view<uint8_t const>&& priBytes, void* context), void* context);

        com_array<uint8_t> Write();
        winrt::Windows::Storage::Streams::IBuffer WriteAsBuffer();
        winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::Streams::IBuffer> WriteAsBufferAsync();
        winrt::Windows::Foundation::IAsyncAction WriteAsync(hstring destinationFilePath);
        winrt::Windows::Foundation::IAsyncAction WriteAsync(winrt::Windows::Storage::StorageFile destinationFile);
        winrt::Windows::Foundation::IAsyncAction WriteAsync(winrt::Windows::Storage::Streams::IOutputStream destinationStream);
        winrt::Windows::Foundation::IAsyncAction WriteAsync(winrt::Windows::Storage::Streams::IBuffer destinationBuffer);

        winrt::MrmLib::PriType Type();
        uint64_t Magic();
        hstring MagicString();

        winrt::MrmLib::PriVersion Version();
        uint32_t Checksum();

        hstring SimpleId();
        void SimpleId(hstring const& value);

        hstring UniqueId();
        void UniqueId(hstring const& value);
    };
}

namespace winrt::MrmLib::factory_implementation
{
    struct PriFile : PriFileT<PriFile, implementation::PriFile>
    {
    };
}

namespace winrt::MrmLib
{
#if ENABLE_SCHEMA_VERSION_WRAPPER
    class HierarchicalSchemaVersionInfoWrapper : public mrm::IHierarchicalSchemaVersionInfo
    {
    private:
        const mrm::IHierarchicalSchemaVersionInfo* m_innerVersionInfo = nullptr;

        uint16_t m_majorVersion;
        uint16_t m_minorVersion;

    public:
        HierarchicalSchemaVersionInfoWrapper(const mrm::IHierarchicalSchemaVersionInfo* innerVersionInfo, uint16_t majorVersion, uint16_t minorVersion)
            : m_innerVersionInfo(innerVersionInfo),
              m_majorVersion(majorVersion),
              m_minorVersion(minorVersion)
        {

		}

		UINT16 GetMajorVersion() const { return m_majorVersion; }
		UINT16 GetMinorVersion() const { return m_minorVersion; }
		DEF_CHECKSUM GetVersionChecksum() const { return m_innerVersionInfo->GetVersionChecksum(); }

		int GetNumScopes() const { return m_innerVersionInfo->GetNumScopes(); }
		int GetNumItems() const { return m_innerVersionInfo->GetNumItems(); }

        ~HierarchicalSchemaVersionInfoWrapper()
        {
			m_innerVersionInfo = nullptr;
        }
    };
#endif

    class HierarchicalSchemaWrapper : public mrm::IHierarchicalSchema
    {
    private:
        const mrm::IHierarchicalSchema* m_innerSchema = nullptr;

        hstring m_simpleId;
        hstring m_uniqueId;

#if ENABLE_SCHEMA_VERSION_WRAPPER
        uint16_t m_majorVersion;
        uint16_t m_minorVersion;
        HierarchicalSchemaVersionInfoWrapper m_versionInfoWrapper;
#endif

    public:
        HierarchicalSchemaWrapper(const mrm::IHierarchicalSchema* innerSchema, hstring const& simpleId, hstring const& uniqueId
#if ENABLE_SCHEMA_VERSION_WRAPPER
            ,uint16_t majorVersion, uint16_t minorVersion
#endif
        )
            : m_innerSchema(innerSchema),
              m_simpleId(simpleId),
              m_uniqueId(uniqueId)

#if ENABLE_SCHEMA_VERSION_WRAPPER
             ,m_majorVersion(majorVersion),
              m_minorVersion(minorVersion),
              m_versionInfoWrapper({ innerSchema->GetVersionInfo(), majorVersion, minorVersion })
#endif

        {

        }

        ~HierarchicalSchemaWrapper()
        {
			m_innerSchema = nullptr;
        }

        PCWSTR GetUniqueId() const { return m_uniqueId.c_str(); }
        PCWSTR GetSimpleId() const { return m_simpleId.c_str(); }

		UINT16 GetMajorVersion() const
        {
#if ENABLE_SCHEMA_VERSION_WRAPPER
            return m_majorVersion;
#else
			return m_innerSchema->GetMajorVersion();
#endif
        }

		UINT16 GetMinorVersion() const
        {
#if ENABLE_SCHEMA_VERSION_WRAPPER
            return m_minorVersion;
#else
			return m_innerSchema->GetMinorVersion();
#endif
        }

		int GetNumScopes() const { return m_innerSchema->GetNumScopes(); }
		mrm::IAtomPool* GetScopeNames() const { return m_innerSchema->GetScopeNames(); }

		int GetNumItems() const { return m_innerSchema->GetNumItems(); }
		mrm::IAtomPool* GetItemNames() const { return m_innerSchema->GetItemNames(); }

        int GetNumVersionInfos() const 
        {
#if ENABLE_SCHEMA_VERSION_WRAPPER
            return 1;
#else
			return m_innerSchema->GetNumVersionInfos();
#endif
        }

        const mrm::IHierarchicalSchemaVersionInfo* GetVersionInfo(_In_ int index = 0) const
        {
#if ENABLE_SCHEMA_VERSION_WRAPPER
            return &m_versionInfoWrapper;
#else
			return m_innerSchema->GetVersionInfo(index);
#endif
        }

        int GetNumNames() const { return m_innerSchema->GetNumNames(); }

        bool Contains(_In_ PCWSTR path, _Out_opt_ int* pScopeIndexOut = NULL, _Out_opt_ int* pItemIndexOut = NULL) const { return m_innerSchema->Contains(path, pScopeIndexOut, pItemIndexOut); }

        bool Contains(
            _In_ PCWSTR path,
            _In_ int relativeToScope,
            _Out_opt_ int* pScopeIndexOut = NULL,
            _Out_opt_ int* pItemIndexOut = NULL) const { return m_innerSchema->Contains(path, relativeToScope, pScopeIndexOut, pItemIndexOut); }

        bool TryGetScopeInfo(_In_ int scopeIndex, _Inout_ mrm::StringResult* pNameOut, _Out_opt_ int* pNumChildrenOut = NULL) const { return m_innerSchema->TryGetScopeInfo(scopeIndex, pNameOut, pNumChildrenOut); }

        bool TryGetScopeChild(
            _In_ int scopeIndex,
            _In_ int childIndex,
            _Out_opt_ int* pChildScopeIndexOut,
            _Out_opt_ int* pChildItemIndexOut) const { return m_innerSchema->TryGetScopeChild(scopeIndex, childIndex, pChildScopeIndexOut, pChildItemIndexOut); }

        bool TryGetScopeChildName(_In_ int scopeIndex, _In_ int childIndex, _Inout_ mrm::StringResult* pNameOut) const { return m_innerSchema->TryGetScopeChildName(scopeIndex, childIndex, pNameOut); }

        bool TryGetItemInfo(_In_ int itemIndex, _Inout_ mrm::StringResult* pNameOut) const { return m_innerSchema->TryGetItemInfo(itemIndex, pNameOut); }

        bool TryGetRelativeItemName(_In_ int relativeToScope, _Inout_ int itemIndex, _Inout_ mrm::StringResult* pNameOut) const { return m_innerSchema->TryGetRelativeItemName(relativeToScope, itemIndex, pNameOut); }

        bool TryGetItemLocalName(_Inout_ int itemIndex, _Inout_ mrm::StringResult* pNameOut) const { return m_innerSchema->TryGetItemLocalName(itemIndex, pNameOut); }

        bool TryGetRelativeScopeName(_In_ int relativeToScope, _Inout_ int scopeIndex, _Inout_ mrm::StringResult* pNameOut) const { return m_innerSchema->TryGetRelativeScopeName(relativeToScope, scopeIndex, pNameOut); }

        HRESULT GetNumDescendents(_In_ int scopeIndex, _Out_opt_ int* pNumScopes, _Out_opt_ int* pNumItems) const { return m_innerSchema->GetNumDescendents(scopeIndex, pNumScopes, pNumItems); }

        HRESULT GetDescendents(
            _In_ int scopeIndex,
            _In_ int sizeScopes,
            _Out_writes_to_opt_(sizeScopes, *pNumScopesWritten) int* pScopesOut,
            _Out_opt_ int* pNumScopesWritten,
            _In_ int sizeItems,
            _Out_writes_to_opt_(sizeItems, *pNumItemsWritten) int* pItemsOut,
            _Out_opt_ int* pNumItemsWritten) const { return m_innerSchema->GetDescendents(scopeIndex, sizeScopes, pScopesOut, pNumScopesWritten, sizeItems, pItemsOut, pNumItemsWritten); }

        HRESULT Clone(_Outptr_ IHierarchicalSchema** result) const 
        { 
            IHierarchicalSchema* pResult = nullptr;
            RETURN_IF_FAILED(m_innerSchema->Clone(&pResult));

			*result = new HierarchicalSchemaWrapper(pResult, m_simpleId, m_uniqueId
#if ENABLE_SCHEMA_VERSION_WRAPPER
                ,m_majorVersion, m_minorVersion
#endif
            );
			return S_OK;
        }

        HRESULT GetSchemaBlobFromFileSection(
            _Inout_opt_ DEFFILE_SECTION_TYPEID* pSectionTypeResult,
            _Inout_opt_ mrm::BlobResult* pBlobResult) const { return m_innerSchema->GetSchemaBlobFromFileSection(pSectionTypeResult, pBlobResult); }
    };
}