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
        mrm::CoreProfile* profile = nullptr;
        check_hresult(mrm::CoreProfile::ChooseDefaultProfile(&profile));
        return std::unique_ptr<mrm::CoreProfile> { profile };
    }();

    PriFile::PriFile(mrm::StandalonePriFile* priFile, com_array<uint8_t>&& priBytes)
        : m_priFile(priFile)
    {
        if (priBytes.size())
            m_priFileBytes = std::forward<com_array<uint8_t>>(priBytes);

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

    com_array<uint8_t> PriFile::Write()
    {
        std::unique_ptr<mrm::PriFileBuilder> priFileBuilder;
        check_hresult(mrm::PriFileBuilder::CreateInstance(s_coreProfile.get(), std::out_ptr(priFileBuilder)));

        const mrm::IResourceMapBase* map = nullptr;
        check_hresult(m_priFile->GetResourceMap(0, &map));

        mrm::ResourceMapSectionBuilder* mapBuilder = nullptr;
        mrm::PriSectionBuilder* priSectionBuilder = priFileBuilder->GetDescriptor();
        const mrm::IHierarchicalSchema* schema = map->GetSchema();

        mapBuilder = priSectionBuilder->GetResourceMapBuilder(schema->GetSimpleId());
        if (mapBuilder == nullptr)
        {
            int mapBuilderIndex = -1;
            check_hresult(priSectionBuilder->AddResourceMapBuilder(schema, true, mrm::PriBuildType::PriBuildFromScratch, &mapBuilderIndex));
            mapBuilder = priSectionBuilder->GetResourceMapBuilder(mapBuilderIndex);

            check_pointer(mapBuilder);
        }

        mrm::RemapUInt16 qualifierMap;
        mrm::RemapUInt16 qualifierSetMap;
        mrm::RemapUInt16 decisionMap;

        //auto environment = mapBuilder->GetEnvironment();
        auto decisions = mapBuilder->GetDecisionInfo();
        auto decisionInfo = map->GetDecisionInfo();
        check_hresult(decisions->Merge(decisionInfo, &qualifierMap, &qualifierSetMap, &decisionMap));

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
                        check_hresult(customQualifierSet->AddQualifier(
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

        return { (uint8_t*)priFileContents, (uint8_t*)priFileContents + priFileContentsSize };
    }

    winrt::Windows::Foundation::IAsyncAction PriFile::WriteAsync(winrt::Windows::Storage::Streams::IOutputStream destinationStream)
    {
        co_await winrt::resume_background();

        auto bytes = Write();
        DataWriter writer(destinationStream);
        writer.WriteBytes(bytes);
        co_await writer.StoreAsync();

        co_return;
    }

    winrt::Windows::Foundation::IAsyncAction PriFile::WriteAsync(winrt::Windows::Storage::StorageFile destinationFile)
    {
        co_await WriteAsync(co_await destinationFile.OpenAsync(FileAccessMode::ReadWrite));
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
}
