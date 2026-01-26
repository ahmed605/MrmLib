#include "pch.h"
#include "ResourceCandidate.h"
#include "ResourceCandidate.g.cpp"
#include <Qualifier.h>
#include "BufferView.h"

namespace winrt::MrmLib::implementation
{
    using namespace ::winrt::Windows::Foundation;

    ResourceCandidate::ResourceCandidate(hstring&& resourceName, mrm::ResourceCandidateResult&& candidate, mrm::AtomPoolGroup* pPoolGroup) :
        m_resourceName(std::move(resourceName)),
        Candidate(std::move(candidate))
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

            qualifiers.push_back(make<implementation::Qualifier>(std::move(qualifierResult), pPoolGroup));
        }

        m_qualifiers = winrt::single_threaded_vector<winrt::MrmLib::Qualifier>(std::move(qualifiers)).GetView();
    }

    ResourceCandidate::ResourceCandidate(hstring const& resourceName, ResourceValueType const& valueType, hstring const& value, IVectorView<winrt::MrmLib::Qualifier> const& qualifiers)
    {
        m_resourceName = resourceName;
        SetValue(valueType, value);
        Qualifiers(qualifiers);
    }

    ResourceCandidate::ResourceCandidate(hstring const& resourceName, array_view<uint8_t const>& value, IVectorView<winrt::MrmLib::Qualifier> const& qualifiers)
    {
        m_resourceName = resourceName;
        SetValue(value);
        Qualifiers(qualifiers);
    }

    winrt::MrmLib::ResourceCandidate ResourceCandidate::Create(hstring const& resourceName, winrt::MrmLib::ResourceValueType const& valueType, hstring const& stringValue)
    {
        return winrt::make<implementation::ResourceCandidate>(resourceName, valueType, stringValue, nullptr);
    }

    winrt::MrmLib::ResourceCandidate ResourceCandidate::Create(hstring const& resourceName, winrt::MrmLib::ResourceValueType const& valueType, hstring const& stringValue, array_view<winrt::MrmLib::Qualifier const> qualifiers)
    {
        return winrt::make<implementation::ResourceCandidate>(resourceName, valueType, stringValue, single_threaded_vector(std::move(std::vector<MrmLib::Qualifier>(qualifiers.begin(), qualifiers.end()))).GetView());
    }

    winrt::MrmLib::ResourceCandidate ResourceCandidate::Create(hstring const& resourceName, array_view<uint8_t const> dataValue)
    {
        return winrt::make<implementation::ResourceCandidate>(resourceName, dataValue, nullptr);
    }

    winrt::MrmLib::ResourceCandidate ResourceCandidate::Create(hstring const& resourceName, array_view<uint8_t const> dataValue, array_view<winrt::MrmLib::Qualifier const> qualifiers)
    {
        return winrt::make<implementation::ResourceCandidate>(resourceName, dataValue, single_threaded_vector(std::move(std::vector<MrmLib::Qualifier>(qualifiers.begin(), qualifiers.end()))).GetView());
    }

    winrt::MrmLib::ResourceCandidate ResourceCandidate::Create(hstring const& resourceName, winrt::MrmLib::ResourceValueType const& valueType, hstring const& stringValue, winrt::Windows::Foundation::Collections::IVectorView<winrt::MrmLib::Qualifier> const& qualifiers)
    {
        return winrt::make<implementation::ResourceCandidate>(resourceName, valueType, stringValue, qualifiers);
    }

    winrt::MrmLib::ResourceCandidate ResourceCandidate::Create(hstring const& resourceName, array_view<uint8_t const> dataValue, winrt::Windows::Foundation::Collections::IVectorView<winrt::MrmLib::Qualifier> const& qualifiers)
    {
        return winrt::make<implementation::ResourceCandidate>(resourceName, dataValue, qualifiers);
    }

    winrt::MrmLib::ResourceCandidate ResourceCandidate::Create(hstring const& resourceName, winrt::Windows::Storage::Streams::IBuffer const& dataValue)
    {
        auto array = array_view<uint8_t const> { dataValue.data(), dataValue.Length() };
        return winrt::make<implementation::ResourceCandidate>(resourceName, array, nullptr);
    }

    winrt::MrmLib::ResourceCandidate ResourceCandidate::Create(hstring const& resourceName, winrt::Windows::Storage::Streams::IBuffer const& dataValue, array_view<winrt::MrmLib::Qualifier const> qualifiers)
    {
        auto array = array_view<uint8_t const> { dataValue.data(), dataValue.Length() };
        return winrt::make<implementation::ResourceCandidate>(resourceName, array, single_threaded_vector(std::move(std::vector<MrmLib::Qualifier>(qualifiers.begin(), qualifiers.end()))).GetView());
    }

    winrt::MrmLib::ResourceCandidate ResourceCandidate::Create(hstring const& resourceName, winrt::Windows::Storage::Streams::IBuffer const& dataValue, winrt::Windows::Foundation::Collections::IVectorView<winrt::MrmLib::Qualifier> const& qualifiers)
    {
        auto array = array_view<uint8_t const> { dataValue.data(), dataValue.Length() };
        return winrt::make<implementation::ResourceCandidate>(resourceName, array, qualifiers);
    }

    hstring ResourceCandidate::ResourceName()
    {
        return m_resourceName;
    }

    void ResourceCandidate::ResourceName(hstring const& value)
    {
        m_resourceName = value;
    }

    winrt::MrmLib::ResourceValueType ResourceCandidate::ValueType()
    {
        if (m_replacementValueType != NullValueType)
        {
            return m_replacementValueType;
        }

        return m_valueType;
    }

    void ResourceCandidate::ValueType(winrt::MrmLib::ResourceValueType const& value)
    {
        if (value == ResourceValueType::EmbeddedData && !m_replacementDataValue.size()) [[unlikely]]
        {
            throw hresult_invalid_argument(L"ValueType cannot be set to EmbeddedData without setting DataValue first.");
        }

        m_replacementValueType = value;
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

    void ResourceCandidate::StringValue(hstring const& value)
    {
        SetValue(value);
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

    void ResourceCandidate::DataValue(array_view<uint8_t const> value)
    {
        SetValue(value);
    }

    IBuffer ResourceCandidate::DataValueBuffer()
    {
        if (ValueType() == ResourceValueType::EmbeddedData)
        {
            auto size = HasReplacementValue() ? m_replacementDataValue.size() : m_dataValue.size();

            Buffer buffer { size };
            CopyMemory(
                buffer.data(),
                HasReplacementValue() ? m_replacementDataValue.begin() : m_dataValue.begin(),
                HasReplacementValue() ? m_replacementDataValue.size() : m_dataValue.size()
            );

            return buffer;
        }

        return nullptr;
    }

    void ResourceCandidate::DataValueBuffer(IBuffer const& value)
    {
        SetValue(value);
    }

    IBuffer ResourceCandidate::DataValueReference()
    {
        if (ValueType() == ResourceValueType::EmbeddedData)
        {
            return HasReplacementValue() ?
                make<::MrmLib::BufferView>(m_replacementDataValue.begin(), m_replacementDataValue.size()) :
                make<::MrmLib::BufferView>(m_dataValue.begin(), m_dataValue.size());
        }

        return nullptr;
    }

    IVectorView<winrt::MrmLib::Qualifier> ResourceCandidate::Qualifiers()
    {
        return m_qualifiers;
    }

    void ResourceCandidate::Qualifiers(winrt::Windows::Foundation::Collections::IVectorView<winrt::MrmLib::Qualifier> const& value)
    {
        m_qualifiers = value ? value : single_threaded_vector<MrmLib::Qualifier>().GetView();
        HasCustomQualifiers = true;
    }

    void ResourceCandidate::SetValue(ResourceValueType const& valueType, hstring const& stringValue)
    {
        m_replacementStringValue = stringValue;
        m_replacementValueType = valueType;
    }

    void ResourceCandidate::SetValue(hstring const& stringValue)
    {
        auto originalType = ValueType();
        m_replacementStringValue = stringValue;
        m_replacementValueType = originalType != ResourceValueType::EmbeddedData ? originalType : ResourceValueType::String;
    }

    void ResourceCandidate::SetValue(array_view<uint8_t const> dataValue)
    {
        m_replacementDataValue = { dataValue.begin(), dataValue.end() };
        m_replacementValueType = ResourceValueType::EmbeddedData;
    }

    void ResourceCandidate::SetValue(IBuffer const& value)
    {
        m_replacementDataValue = { value.data(), value.data() + value.Length() };
        m_replacementValueType = ResourceValueType::EmbeddedData;
    }
}
