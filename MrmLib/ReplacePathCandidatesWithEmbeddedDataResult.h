#pragma once
#include "ReplacePathCandidatesWithEmbeddedDataResult.g.h"
#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::MrmLib::implementation
{
	using namespace winrt::Windows::Foundation::Collections;

    struct ReplacePathCandidatesWithEmbeddedDataResult : ReplacePathCandidatesWithEmbeddedDataResultT<ReplacePathCandidatesWithEmbeddedDataResult>
    {
    private:
        IVectorView<winrt::MrmLib::ResourceCandidate> m_candidatesReplaced;

    public:
        ReplacePathCandidatesWithEmbeddedDataResult() = default;
        ReplacePathCandidatesWithEmbeddedDataResult(IVectorView<winrt::MrmLib::ResourceCandidate>&& candidatesReplaced);

        IVectorView<winrt::MrmLib::ResourceCandidate> CandidatesReplaced();
    };
}
