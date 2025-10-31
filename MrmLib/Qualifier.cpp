#include "pch.h"
#include "Qualifier.h"
#include "Qualifier.g.cpp"

namespace winrt::MrmLib::implementation
{
    Qualifier::Qualifier(mrm::QualifierResult&& qualifier)
        : m_qualifier(std::forward<mrm::QualifierResult>(qualifier))
    {

    }

    winrt::MrmLib::QualifierAttribute Qualifier::Attribute()
    {
        mrm::Atom attribute;
        check_hresult(m_qualifier.GetOperand1Attribute(&attribute));

        return static_cast<winrt::MrmLib::QualifierAttribute>(attribute.GetUInt64());
    }

    winrt::MrmLib::QualifierOperator Qualifier::Operator()
    {
        if (m_qualifier.OperatorIsCustom())
            return QualifierOperator::Custom;

        mrm::ICondition::ConditionOperator op = { };
        check_hresult(m_qualifier.GetOperator(&op));
        return static_cast<winrt::MrmLib::QualifierOperator>(op);
    }

    uint64_t Qualifier::CustomOperator()
    {
        if (!m_qualifier.OperatorIsCustom())
            return 0;

        mrm::Atom op;
        check_hresult(m_qualifier.GetCustomOperator(&op));
        return static_cast<uint32_t>(op.GetUInt64());
    }

    winrt::MrmLib::QualifierValueType Qualifier::ValueType()
    {
        bool isLiteral = false;
        check_hresult(m_qualifier.Operand2IsLiteral(&isLiteral));
        return isLiteral ? QualifierValueType::String : QualifierValueType::Attribute;
    }

    hstring Qualifier::StringValue()
    {
        if (ValueType() == QualifierValueType::String)
        {
            mrm::StringResult strResult;
            check_hresult(m_qualifier.GetOperand2Literal(&strResult));
            auto result = strResult.GetStringResult();
            return result->cchBuf ? hstring(result->pRef, result->cchBuf - 1) : hstring(result->pRef);
        }

        return L"";
    }

    uint32_t Qualifier::AttributeValue()
    {
        if (ValueType() == QualifierValueType::Attribute)
        {
            mrm::Atom attribute;
            check_hresult(m_qualifier.GetOperand2Attribute(&attribute));
            return static_cast<uint32_t>(attribute.GetUInt64());
        }

        return 0;
    }

    winrt::Windows::Foundation::IInspectable Qualifier::Value()
    {
        if (ValueType() == QualifierValueType::String)
            return winrt::box_value(StringValue());

        return winrt::box_value(AttributeValue());
    }

    int32_t Qualifier::Priority()
    {
        return m_qualifier.GetPriority();
    }

    double Qualifier::FallbackScore()
    {
        double score = 0.0;
        check_hresult(m_qualifier.GetFallbackScore(&score));
        return score;
    }
}
