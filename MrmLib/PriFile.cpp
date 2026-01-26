#include "pch.h"
#include "PriFile.h"
#include "PriFile.g.cpp"

#include <filesystem>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <ResourceCandidate.h>
#include <ReplacePathCandidatesWithEmbeddedDataResult.h>

namespace winrt::MrmLib::implementation
{
    using namespace ::winrt::Windows::Storage;
    using namespace ::winrt::Windows::Storage::Streams;

    std::unique_ptr<mrm::CoreProfile> PriFile::s_coreProfile = []()
    {
        mrm::WindowsClientProfileBase* profile = nullptr;
        check_hresult(mrm::WindowsClientProfileBase::CreateInstance(mrm::WindowsCoreRS4, &profile));
        return std::unique_ptr<mrm::CoreProfile> { profile };
    }();

    PriFile::PriFile(mrm::StandalonePriFile* priFile, com_array<uint8_t>&& priBytes)
        : m_priFile(priFile)
    {
        if (priBytes.size())
            m_priFileBytes = std::move(priBytes);

        const mrm::BaseFile* baseFile = nullptr;
        winrt::check_hresult(static_cast<mrm::PriFile*>(m_priFile.get())->GetBaseFile(&baseFile));

        m_header = baseFile->GetFileHeader();
        check_hresult(m_priFile->GetProfile()->GetTargetPlatformVersionForFileMagic(m_header->magic, &m_version));

        const mrm::IResourceMapBase* map = nullptr;
        check_hresult(m_priFile->GetResourceMap(0, &map));

        mrm::NamedResourceResult namedResource;
        mrm::ResourceCandidateResult resCandidate;

        std::vector<winrt::MrmLib::ResourceCandidate> candidates;
        for (int resIdx = 0; resIdx < map->GetNumResources(); resIdx++)
        {
            check_hresult(map->GetResourceByIndex(resIdx, &namedResource));
            for (int candidateIdx = 0; candidateIdx < namedResource.GetNumCandidates(); candidateIdx++)
            {
                check_hresult(namedResource.GetCandidate(candidateIdx, &resCandidate));

                mrm::StringResult str;
                check_hresult(namedResource.GetResourceName(&str));

                auto result = str.GetStringResult();
                candidates.push_back(winrt::make<implementation::ResourceCandidate>(std::move(hstring(result->pRef, result->cchBuf - 1)), std::move(resCandidate), m_priFile->GetAtoms()));
            }
        }

        m_resourceCandidates = winrt::multi_threaded_vector<winrt::MrmLib::ResourceCandidate>(std::move(candidates));

        auto schema = map->GetSchema();
        m_simpleId = schema->GetSimpleId();
        m_uniqueId = schema->GetUniqueId();
        m_checksum = schema->GetVersionInfo()->GetVersionChecksum();
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::PriFile> PriFile::LoadAsync(array_view<uint8_t const> priBytes)
    {
        co_await winrt::resume_background();

        auto bytes = com_array<uint8_t>(priBytes.begin(), priBytes.end());

        mrm::StandalonePriFile* priFile = nullptr;
        check_hresult(mrm::StandalonePriFile::CreateInstance(0, bytes.data(), bytes.size(), s_coreProfile.get(), &priFile));

        co_return winrt::make<PriFile>(priFile, std::move(bytes));
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::PriFile> PriFile::LoadAsync(hstring priFilePath)
    {
        co_await winrt::resume_background();

        mrm::StandalonePriFile* priFile = nullptr;
        check_hresult(mrm::StandalonePriFile::CreateInstance(0, priFilePath.c_str(), s_coreProfile.get(), &priFile));

        co_return winrt::make<PriFile>(priFile);
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::PriFile> PriFile::LoadAsync(winrt::Windows::Storage::StorageFile priFile)
    {
        auto const& buffer = co_await FileIO::ReadBufferAsync(priFile);
        co_return co_await LoadAsync({ buffer.data(), buffer.Length() });
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::PriFile> PriFile::LoadAsync(IBuffer priBytesBuffer)
    {
        co_return co_await LoadAsync({ priBytesBuffer.data(), priBytesBuffer.Length() });
    }

    winrt::Windows::Foundation::Collections::IVector<winrt::MrmLib::ResourceCandidate> PriFile::ResourceCandidates()
    {
        return m_resourceCandidates;
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::ReplacePathCandidatesWithEmbeddedDataResult> PriFile::ReplacePathCandidatesWithEmbeddedDataAsync(winrt::Windows::Storage::StorageFolder sourceFolderToEmbed)
    {
        co_await winrt::resume_background();

        auto folderName = sourceFolderToEmbed.Name() + L"\\";
        auto folderNameSize = folderName.size();

        std::vector<winrt::MrmLib::ResourceCandidate> replacedCandidates;
        for (auto candidate : m_resourceCandidates)
        {
            if (candidate.ValueType() == ResourceValueType::Path)
            {
                auto path = candidate.StringValue();

                if (path.starts_with(folderName))
                {
                    std::wstring_view name_view = path.data() + folderNameSize;

                    if (auto const& item = co_await sourceFolderToEmbed.TryGetItemAsync(name_view))
                    {
                        if (auto const& file = item.try_as<StorageFile>())
                        {
                            auto const& buffer = co_await FileIO::ReadBufferAsync(file);
                            array_view<uint8_t const> view = { buffer.data(), buffer.Length() };

                            auto self = winrt::get_self<implementation::ResourceCandidate>(candidate);
                            self->SetValue(view);

                            replacedCandidates.push_back(candidate);
                        }
                    }
                }
            }
        }

        co_return make<implementation::ReplacePathCandidatesWithEmbeddedDataResult>(std::move(winrt::single_threaded_vector<winrt::MrmLib::ResourceCandidate>(std::move(replacedCandidates)).GetView()));
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::ReplacePathCandidatesWithEmbeddedDataResult> PriFile::ReplacePathCandidatesWithEmbeddedDataAsync(hstring sourceFolderPathToEmbed)
    {
        auto const& folder = co_await StorageFolder::GetFolderFromPathAsync(sourceFolderPathToEmbed);
        co_return co_await ReplacePathCandidatesWithEmbeddedDataAsync(folder);
    }

    void PriFile::WriteInternal(void (*pCallback)(array_view<uint8_t const>&& priBytes, void* context), void* context)
    {
        mrm::CoreProfile* profile = s_coreProfile.get();

        std::unique_ptr<mrm::WindowsClientProfileBase> customProfile;
        if (SUCCEEDED(mrm::WindowsClientProfileBase::CreateInstance(m_version, std::out_ptr(customProfile))))
        {
            profile = customProfile.get();
        }

        std::unique_ptr<mrm::PriFileBuilder> priFileBuilder;
        check_hresult(mrm::PriFileBuilder::CreateInstance(profile, std::out_ptr(priFileBuilder)));

        const mrm::IResourceMapBase* map = nullptr;
        check_hresult(m_priFile->GetResourceMap(0, &map));

        mrm::ResourceMapSectionBuilder* mapBuilder = nullptr;
        mrm::PriSectionBuilder* priSectionBuilder = priFileBuilder->GetDescriptor();

        std::unique_ptr<HierarchicalSchemaWrapper> schemaWrapper;
        const mrm::IHierarchicalSchema* schema = map->GetSchema();

        if (m_idsChanged)
        {
#if ENABLE_SCHEMA_VERSION_WRAPPER
            auto version = Version();
            schemaWrapper = std::make_unique<HierarchicalSchemaWrapper>(schema, m_simpleId, m_uniqueId, version.Major, version.Minor);
            #warning "Schema version wrapper is enabled but no version override is passed."
#else
            schemaWrapper = std::make_unique<HierarchicalSchemaWrapper>(schema, m_simpleId, m_uniqueId);
#endif

            schema = schemaWrapper.get();
        }

        mapBuilder = priSectionBuilder->GetResourceMapBuilder(schema->GetSimpleId());
        if (mapBuilder == nullptr)
        {
            int mapBuilderIndex = -1;
            check_hresult(priSectionBuilder->AddResourceMapBuilder(schema, true, mrm::PriBuildType::PriBuildFromScratch, &mapBuilderIndex));
            mapBuilder = priSectionBuilder->GetResourceMapBuilder(mapBuilderIndex);

            check_pointer(mapBuilder);
        }

        auto sourceEnvironment = m_priFile->GetUnifiedEnvironment()->GetDefaultEnvironment();

        int numMappedQualifiers = 0;
        const PCWSTR* mappedQualifierNames = nullptr;
        const mrm::Atom::SmallIndex* qualiferMappings = nullptr;
        LOG_IF_FAILED(s_coreProfile->GetQualifierInfoForEnvironment(
            sourceEnvironment->GetDisplayName(),
            sourceEnvironment->GetVersionInfo(),
            priSectionBuilder->GetEnvironment()->GetDefaultEnvironment(),
            &numMappedQualifiers,
            &mappedQualifierNames,
            &qualiferMappings));

        mrm::RemapUInt16 qualifierMap;
        mrm::RemapUInt16 qualifierSetMap;
        mrm::RemapUInt16 decisionMap;

        //auto environment = mapBuilder->GetEnvironment();
        auto decisions = mapBuilder->GetDecisionInfo();
        auto decisionInfo = map->GetDecisionInfo();
        check_hresult(decisions->Merge(decisionInfo, &qualifierMap, &qualifierSetMap, &decisionMap, qualiferMappings));

        int qualifierSetIndex = 0;
        uint16_t remappedQualifierSetIndex = 0;
        mrm::QualifierSetResult qualifierSet;

        int emptyQualifierSetIndex = -1;

        for (auto candidate : m_resourceCandidates)
        {
            auto self = winrt::get_self<implementation::ResourceCandidate>(candidate);
            auto resCandidate = &self->Candidate;
            auto resName = self->ResourceName();

            bool mappingSucceeded = true;
            if (self->HasCustomQualifiers) [[unlikely]]
            {
                auto customQualifiers = self->Qualifiers();
                if (!customQualifiers.Size()) [[unlikely]]
                {
                    if (emptyQualifierSetIndex == -1) [[unlikely]]
                    {
                        std::unique_ptr<mrm::DecisionInfoQualifierSetBuilder> emptyQualifierSet;
                        check_hresult(priSectionBuilder->GetQualifierSetBuilder(std::out_ptr(emptyQualifierSet)));
                        check_hresult(decisions->GetOrAddQualifierSet(emptyQualifierSet.get(), &emptyQualifierSetIndex));
                    }

                    remappedQualifierSetIndex = emptyQualifierSetIndex;
                }
                else [[likely]]
                {
                    std::unique_ptr<mrm::DecisionInfoQualifierSetBuilder> customQualifierSet;
                    check_hresult(priSectionBuilder->GetQualifierSetBuilder(std::out_ptr(customQualifierSet)));

                    for (auto qualifier : customQualifiers)
                    {
                        auto stringValue = qualifier.Value();
                        auto attributeName = qualifier.AttributeName();
                        LOG_IF_FAILED(customQualifierSet->AddQualifier(
                            attributeName.c_str(),
                            stringValue.c_str(),
                            qualifier.Priority(),
                            qualifier.FallbackScore()));
                    }

                    int customQualifierSetIndex = 0;
                    check_hresult(decisions->GetOrAddQualifierSet(customQualifierSet.get(), &customQualifierSetIndex));
                    remappedQualifierSetIndex = static_cast<uint16_t>(customQualifierSetIndex);
                }
            }
            else [[likely]]
            {
                check_hresult(resCandidate->GetQualifiers(&qualifierSet));
                check_hresult(qualifierSet.GetIndex(&qualifierSetIndex));
                mappingSucceeded = qualifierSetMap.TryGetMapping(static_cast<uint16_t>(qualifierSetIndex), &remappedQualifierSetIndex);
            }

            if (mappingSucceeded)
            {
                auto type = self->ValueType();
                if (type == ResourceValueType::EmbeddedData)
                {
                    size_t blobSize = 0;
                    const uint8_t* blob = nullptr;

                    if (self->HasReplacementValue())
                    {
                        auto ref = self->GetReplacementDataValueRef();
                        blobSize = ref->size();
                        blob = ref->data();
                    }
                    else
                    {
                        mrm::BlobResult brCandidateValue;
                        check_bool(resCandidate->TryGetBlobValue(&brCandidateValue));
                        blob = static_cast<const uint8_t*>(brCandidateValue.GetRef(&blobSize));
                    }

                    auto dataItems = priSectionBuilder->GetDataItemOrchestrator();

                    mrm::IBuildInstanceReference* buildInstanceReference = nullptr;
                    check_hresult(dataItems->AddDataAndCreateInstanceReference(blob,
                                                                               static_cast<uint32_t>(blobSize), 
                                                                               static_cast<int>(remappedQualifierSetIndex),
                                                                               &buildInstanceReference));

                    auto hr = mapBuilder->AddCandidate(resName.c_str(),
                                                       mrm::MrmEnvironment::ResourceValueType_EmbeddedData,
                                                       buildInstanceReference,
                                                       static_cast<int>(remappedQualifierSetIndex));

                    if (!((hr == E_DEF_ALREADY_INITIALIZED) || (hr == HRESULT_FROM_WIN32(ERROR_MRM_DUPLICATE_ENTRY))))
                        check_hresult(hr);
                }
                else
                {
                    auto stringValue = self->StringValue();

                    mrm::StringResult strNewCandidateValue;
                    check_hresult(strNewCandidateValue.SetRef(stringValue.c_str()));

                    HRESULT hr = mapBuilder->AddCandidateWithInternalString(resName.c_str(),
                                                                            type == ResourceValueType::Path ?
                                                                                mrm::MrmEnvironment::ResourceValueType_Utf16Path :
                                                                                mrm::MrmEnvironment::ResourceValueType_Utf16String,
                                                                            strNewCandidateValue.GetRef(),
                                                                            static_cast<int>(remappedQualifierSetIndex));

                    if (!((hr == E_DEF_ALREADY_INITIALIZED) || (hr == HRESULT_FROM_WIN32(ERROR_MRM_DUPLICATE_ENTRY))))
                        check_hresult(hr);
                }
            }
        }

        void* priFileContents = nullptr;
        uint32_t priFileContentsSize = 0;
        check_hresult(priFileBuilder->GetFileContentsRef(&priFileContents, &priFileContentsSize));
        m_checksum = mapBuilder->GetSchema()->GetVersionInfo()->GetVersionChecksum();

        pCallback({ static_cast<uint8_t const*>(priFileContents), priFileContentsSize }, context);
    }

    com_array<uint8_t> PriFile::Write()
    {
        com_array<uint8_t> priBytes;
        WriteInternal(
            [](array_view<uint8_t const>&& priBytesView, void* context)
            {
                auto pPriBytes = static_cast<com_array<uint8_t>*>(context);
                *pPriBytes = { priBytesView.begin(), priBytesView.end() };
            }, &priBytes);

        return priBytes;
    }

    IBuffer PriFile::WriteAsBuffer()
    {
        IBuffer outBuffer { nullptr };
        WriteInternal(
            [](array_view<uint8_t const>&& priBytesView, void* context)
            {
                auto size = priBytesView.size();

                Buffer buffer { size };
                CopyMemory(
                    buffer.data(),
                    priBytesView.begin(),
                    size);

                auto ppBufferABI = static_cast<void**>(context);
                *ppBufferABI = winrt::detach_abi(buffer);
            }, winrt::put_abi(outBuffer));

        return outBuffer;
    }

    winrt::Windows::Foundation::IAsyncOperation<IBuffer> PriFile::WriteAsBufferAsync()
    {
        co_await winrt::resume_background();
        co_return WriteAsBuffer();
    }

    winrt::Windows::Foundation::IAsyncAction PriFile::WriteAsync(winrt::Windows::Storage::Streams::IOutputStream destinationStream)
    {
        co_await winrt::resume_background();

        DataWriter writer(destinationStream);
        WriteInternal(
            [](array_view<uint8_t const>&& priBytesView, void* context)
            {
                auto pWriter = static_cast<DataWriter*>(context);
                pWriter->WriteBytes(priBytesView);
            }, &writer);

        co_await writer.StoreAsync();
        co_await destinationStream.FlushAsync();
        writer.Close();

        co_return;
    }

    winrt::Windows::Foundation::IAsyncAction PriFile::WriteAsync(winrt::Windows::Storage::StorageFile destinationFile)
    {
        auto const& stream = co_await destinationFile.OpenAsync(FileAccessMode::ReadWrite);
        co_await WriteAsync(stream);
        stream.Close();
        co_return;
    }

    winrt::Windows::Foundation::IAsyncAction PriFile::WriteAsync(hstring destinationFilePath)
    {
        std::wstring_view file_view = destinationFilePath;
        auto folder_path = std::filesystem::path(file_view).parent_path().wstring();
        auto const& folder = co_await StorageFolder::GetFolderFromPathAsync(folder_path);
        auto const& file = co_await folder.CreateFileAsync(std::filesystem::path(file_view).filename().wstring(), CreationCollisionOption::ReplaceExisting);
        co_await WriteAsync(file);
        co_return;
    }

    winrt::Windows::Foundation::IAsyncAction PriFile::WriteAsync(IBuffer destinationBuffer)
    {
        co_await winrt::resume_background();

        WriteInternal(
            [](array_view<uint8_t const>&& priBytesView, void* context)
            {
                auto pBuffer = static_cast<IBuffer*>(context);
                auto size = priBytesView.size();

                if (pBuffer->Capacity() < size)
                {
                    throw winrt::hresult_invalid_argument(L"Destination buffer is too small to hold the PRI file data.");
				}

                CopyMemory(
                    pBuffer->data(),
                    priBytesView.begin(),
                    size);

				pBuffer->Length(size);
            }, &destinationBuffer);

        co_return;
    }

    uint64_t PriFile::Magic()
    {
        return m_header->magic.ullMagic;
    }

    #define PriFile_Magic(...) std::bit_cast<uint64_t>(std::array<BYTE, sizeof(UINT64) / sizeof(BYTE)> { __VA_ARGS__ })
    inline static constexpr const auto PriFile_gWin8PriFileMagic = PriFile_Magic('m', 'r', 'm', '_', 'p', 'r', 'i', '0');
    inline static constexpr const auto PriFile_gWinBluePriFileMagic = PriFile_Magic('m', 'r', 'm', '_', 'p', 'r', 'i', '1');
    inline static constexpr const auto PriFile_gWindowsPhoneBluePriFileMagic = PriFile_Magic('m', 'r', 'm', '_', 'p', 'r', 'i', 'f');
    inline static constexpr const auto PriFile_gUniversalPriFileMagic = PriFile_Magic('m', 'r', 'm', '_', 'p', 'r', 'i', '2');
    inline static constexpr const auto PriFile_gUniversalRS4PriFileMagic = PriFile_Magic('m', 'r', 'm', '_', 'p', 'r', 'i', '3');
    inline static constexpr const auto PriFile_gUniversalVNextPriFileMagic = PriFile_Magic('m', 'r', 'm', '_', 'v', 'n', 'x', 't');
    inline static constexpr const auto PriFile_gTestPriFileMagic = PriFile_Magic('m', 'r', 'm', '_', 't', 'e', 's', 't');

    winrt::MrmLib::PriType PriFile::Type()
    {
        switch (Magic())
        {
            case PriFile_gWin8PriFileMagic:
                return PriType::WindowsEight;
            case PriFile_gWinBluePriFileMagic:
                return PriType::WindowsBlue;
            case PriFile_gWindowsPhoneBluePriFileMagic:
                return PriType::WindowsPhoneBlue;
            case PriFile_gUniversalPriFileMagic:
                return PriType::Universal;
            case PriFile_gUniversalRS4PriFileMagic:
                return PriType::UniversalRS4;
            case PriFile_gUniversalVNextPriFileMagic:
                return PriType::UniversalVNext;
            case PriFile_gTestPriFileMagic:
                return PriType::Test;
            default:
                return PriType::Unknown;
        }
    }

    hstring PriFile::MagicString()
    {
        auto magic = Magic();
        wchar_t magicStr[sizeof(magic) / sizeof(char)] = { };
        MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<const char*>(&magic), _countof(magicStr), magicStr, _countof(magicStr));

        return { magicStr, _countof(magicStr) };
    }

    winrt::MrmLib::PriVersion PriFile::Version()
    {
        return { m_header->majorVersion, m_header->minorVersion };
    }

    uint32_t PriFile::Checksum()
    {
        return m_checksum;
    }

    hstring PriFile::SimpleId()
    {
        return m_simpleId;
    }

    void PriFile::SimpleId(hstring const& value)
    {
        m_simpleId = value;
        m_idsChanged = true;
    }

    hstring PriFile::UniqueId()
    {
        return m_uniqueId;
    }

    void PriFile::UniqueId(hstring const& value)
    {
        m_uniqueId = value;
        m_idsChanged = true;
    }
}
