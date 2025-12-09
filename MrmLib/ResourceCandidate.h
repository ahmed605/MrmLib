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
	using namespace ::winrt::Windows::Foundation::Collections;

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

        IVectorView<winrt::MrmLib::Qualifier> m_qualifiers { nullptr };

    public:
        mrm::ResourceCandidateResult Candidate;
		bool HasCustomQualifiers = false;

        ResourceCandidate() = default;
        ResourceCandidate(hstring&& resourceName, mrm::ResourceCandidateResult&& candidate, mrm::AtomPoolGroup* pPoolGroup, mrm::MrmPlatformVersionInternal version);
        ResourceCandidate(hstring const& resourceName, ResourceValueType const& valueType, hstring const& value, IVectorView<winrt::MrmLib::Qualifier> const& qualifiers);
        ResourceCandidate(hstring const& resourceName, array_view<uint8_t const>& value, IVectorView<winrt::MrmLib::Qualifier> const& qualifiers);

        static winrt::MrmLib::ResourceCandidate Create(hstring const& resourceName, winrt::MrmLib::ResourceValueType const& valueType, hstring const& stringValue);
        static winrt::MrmLib::ResourceCandidate Create(hstring const& resourceName, winrt::MrmLib::ResourceValueType const& valueType, hstring const& stringValue, array_view<winrt::MrmLib::Qualifier const> qualifiers);
        static winrt::MrmLib::ResourceCandidate Create(hstring const& resourceName, array_view<uint8_t const> dataValue);
        static winrt::MrmLib::ResourceCandidate Create(hstring const& resourceName, array_view<uint8_t const> dataValue, array_view<winrt::MrmLib::Qualifier const> qualifiers);

        hstring ResourceName();
        void ResourceName(hstring const& value);

        winrt::MrmLib::ResourceValueType ValueType();
        void ValueType(winrt::MrmLib::ResourceValueType const& value);

        winrt::Windows::Foundation::IInspectable Value();

        hstring StringValue();
        void StringValue(hstring const& value);

        com_array<uint8_t> DataValue();
        void DataValue(array_view<uint8_t const> value);

        IVectorView<winrt::MrmLib::Qualifier> Qualifiers();
        void Qualifiers(winrt::Windows::Foundation::Collections::IVectorView<winrt::MrmLib::Qualifier> const& value);

        void SetValue(hstring const& value);
        void SetValue(winrt::MrmLib::ResourceValueType const& valueType, hstring const& value);
        void SetValue(array_view<uint8_t const> value);

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

namespace winrt::MrmLib::factory_implementation
{
    struct ResourceCandidate : ResourceCandidateT<ResourceCandidate, implementation::ResourceCandidate>
    {
    };
}
