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

    public:
        PriFile() = default;
        PriFile(mrm::StandalonePriFile* priFile, com_array<uint8_t>&& priBytes = { });

        static winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::PriFile> LoadAsync(array_view<uint8_t const> priBytes);
        static winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::PriFile> LoadAsync(hstring priFilePath);
        static winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::PriFile> LoadAsync(winrt::Windows::Storage::StorageFile priFile);

        winrt::Windows::Foundation::Collections::IVector<winrt::MrmLib::ResourceCandidate> ResourceCandidates();

        winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::ReplacePathCandidatesWithEmbeddedDataResult> ReplacePathCandidatesWithEmbeddedDataAsync(winrt::Windows::Storage::StorageFolder sourceFolderToEmbed);
        winrt::Windows::Foundation::IAsyncOperation<winrt::MrmLib::ReplacePathCandidatesWithEmbeddedDataResult> ReplacePathCandidatesWithEmbeddedDataAsync(hstring sourceFolderPathToEmbed);

        com_array<uint8_t> Write();
        winrt::Windows::Foundation::IAsyncAction WriteAsync(hstring destinationFilePath);
        winrt::Windows::Foundation::IAsyncAction WriteAsync(winrt::Windows::Storage::StorageFile destinationFile);
        winrt::Windows::Foundation::IAsyncAction WriteAsync(winrt::Windows::Storage::Streams::IOutputStream destinationStream);
    };
}

namespace winrt::MrmLib::factory_implementation
{
    struct PriFile : PriFileT<PriFile, implementation::PriFile>
    {
    };
}
