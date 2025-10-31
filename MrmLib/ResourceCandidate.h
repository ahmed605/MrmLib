#pragma once
#include "ResourceCandidate.g.h"

#include <common/Base.h>
#include <common/file/MrmFiles.h>
#include <common/MrmProfileData.h>
#include <DefObject.h>
#include <Results.h>
#include <Atoms.h>
#include <Checksums.h>
#include <readers/MrmManagers.h>
#include <MrmEnvironment.h>
#include <Platform/Base.h>
#include <readers/MrmReaders.h>

#include <namespaces.h>

namespace winrt::MrmLib::implementation
{
    struct ResourceCandidate : ResourceCandidateT<ResourceCandidate>
    {
    private:
		const constexpr static winrt::MrmLib::ResourceValueType NullValueType = static_cast<winrt::MrmLib::ResourceValueType>(-1);

		hstring m_resourceName;

        ResourceValueType m_replacementValueType = NullValueType;
		hstring m_replacementStringValue;
		com_array<uint8_t> m_replacementDataValue;

		hstring m_stringValue;
        array_view<uint8_t const> m_dataValue;
        ResourceValueType m_valueType = NullValueType;

    public:
        mrm::ResourceCandidateResult Candidate;

        ResourceCandidate() = default;
        ResourceCandidate(hstring&& resourceName, mrm::ResourceCandidateResult&& candidate);

        hstring ResourceName();
        winrt::MrmLib::ResourceValueType ValueType();
        winrt::Windows::Foundation::IInspectable Value();
        hstring StringValue();
        com_array<uint8_t> DataValue();

		void ReplaceValue(hstring const& stringValue, winrt::MrmLib::ResourceValueType valueType);
		void ReplaceValue(array_view<uint8_t const>& dataValue);

        inline bool HasReplacementValue() const
        {
            return m_replacementValueType != NullValueType;
		}

        inline ResourceValueType GetReplacementValueType() const
        {
            return m_replacementValueType;
		}

        inline com_array<uint8_t>* GetReplacementDataValueRef()
        {
            return &m_replacementDataValue;
        }
    };
}
