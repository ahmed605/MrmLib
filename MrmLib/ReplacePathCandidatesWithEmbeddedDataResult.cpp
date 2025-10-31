#include "pch.h"
#include "ReplacePathCandidatesWithEmbeddedDataResult.h"
#include "ReplacePathCandidatesWithEmbeddedDataResult.g.cpp"

namespace winrt::MrmLib::implementation
{
    ReplacePathCandidatesWithEmbeddedDataResult::ReplacePathCandidatesWithEmbeddedDataResult(IVectorView<winrt::MrmLib::ResourceCandidate>&& candidatesReplaced)
        : m_candidatesReplaced(std::forward<IVectorView<winrt::MrmLib::ResourceCandidate>>(candidatesReplaced))
    {

    }

    IVectorView<winrt::MrmLib::ResourceCandidate> ReplacePathCandidatesWithEmbeddedDataResult::CandidatesReplaced()
    {
		return m_candidatesReplaced;
    }
}
