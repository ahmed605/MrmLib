#include "pch.h"
#include "ResourceCandidate.h"
#include "ResourceCandidate.g.cpp"
#include <Qualifier.h>

namespace winrt::MrmLib::implementation
{
    using namespace ::winrt::Windows::Foundation;

    ResourceCandidate::ResourceCandidate(hstring&& resourceName, mrm::ResourceCandidateResult&& candidate) :
        m_resourceName(std::forward<hstring>(resourceName)),
        Candidate(std::forward<mrm::ResourceCandidateResult>(candidate))
    {
        mrm::MrmEnvironment::ResourceValueType valueType = { };
        check_hresult(Candidate.GetResourceValueType(&valueType));

        if (mrm::MrmEnvironment::IsBinaryResourceValueType(valueType))
        {
            m_valueType = ResourceValueType::EmbeddedData;

            mrm::BlobResult data = { };
            check_bool(Candidate.TryGetBlobValue(&data));

            auto result = data.GetBlobResult();
            m_dataValue = { (uint8_t*)result->pRef, result->cbRef };
        }
        else
        {
            if (mrm::MrmEnvironment::IsPathResourceValueType(valueType))
            {
                m_valueType = ResourceValueType::Path;
            }
            else
            {
                m_valueType = ResourceValueType::String;
            }

            mrm::StringResult data = { };
            check_bool(Candidate.TryGetStringValue(&data));

            auto result = data.GetStringResult();
            m_stringValue = result->cchBuf ? hstring(result->pRef, result->cchBuf - 1) : hstring(result->pRef);
        }

        std::vector<winrt::MrmLib::Qualifier> qualifiers;

        mrm::QualifierSetResult result;
        check_hresult(Candidate.GetQualifiers(&result));

        auto count = result.GetNumQualifiers();
        for (int i = 0; i < count; i++)
        {
			mrm::QualifierResult qualifierResult;
			check_hresult(result.GetQualifier(i, &qualifierResult));

            qualifiers.push_back(make<implementation::Qualifier>(std::move(qualifierResult)));
        }

		m_qualifiers = winrt::single_threaded_vector<winrt::MrmLib::Qualifier>(std::move(qualifiers)).GetView();
    }

    hstring ResourceCandidate::ResourceName()
    {
        return m_resourceName;
    }

    winrt::MrmLib::ResourceValueType ResourceCandidate::ValueType()
    {
        if (m_replacementValueType != NullValueType)
        {
            return m_replacementValueType;
        }

        return m_valueType;
    }

    IInspectable ResourceCandidate::Value()
    {
        if (ValueType() == ResourceValueType::EmbeddedData)
        {
            return winrt::box_value(DataValue());
        }
        else
        {
            return winrt::box_value(StringValue());
        }
    }

    hstring ResourceCandidate::StringValue()
    {
        if (ValueType() != ResourceValueType::EmbeddedData)
        {
            return HasReplacementValue() ? m_replacementStringValue : m_stringValue;
        }

        return L"";
    }

    com_array<uint8_t> ResourceCandidate::DataValue()
    {
        if (ValueType() == ResourceValueType::EmbeddedData)
        {
            return HasReplacementValue() ? com_array<uint8_t> { m_replacementDataValue.begin(), m_replacementDataValue.end() } :
                                           com_array<uint8_t> { m_dataValue.begin(), m_dataValue.end() };
        }

        return { };
    }

    IVectorView<winrt::MrmLib::Qualifier> ResourceCandidate::Qualifiers()
    {
        return m_qualifiers;
    }

    void ResourceCandidate::ReplaceValue(hstring const& stringValue, ResourceValueType valueType)
    {
        m_replacementStringValue = stringValue;
        m_replacementValueType = valueType;
    }

    void ResourceCandidate::ReplaceValue(array_view<uint8_t const>& dataValue)
    {
        m_replacementDataValue = { dataValue.begin(), dataValue.end() };
        m_replacementValueType = ResourceValueType::EmbeddedData;
    }
}
