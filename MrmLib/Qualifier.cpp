#include "pch.h"
#include "Qualifier.h"
#include "Qualifier.g.cpp"

namespace winrt::MrmLib::implementation
{
    Qualifier::Qualifier(mrm::QualifierResult&& qualifier)
    {
        mrm::Atom attribute;
        check_hresult(qualifier.GetOperand1Attribute(&attribute));
        m_attribute = static_cast<QualifierAttribute>(attribute.GetUInt64());

        if (!qualifier.OperatorIsCustom()) [[likely]]
        {
            mrm::ICondition::ConditionOperator op = { };
            check_hresult(qualifier.GetOperator(&op));
            m_operator = static_cast<QualifierOperator>(op);
        }
        else [[unlikely]]
        {
            m_operator = QualifierOperator::Custom;

            mrm::Atom op;
            check_hresult(qualifier.GetCustomOperator(&op));
            m_customOperator = op.GetUInt64();
        }

        bool isLiteral = false;
        check_hresult(qualifier.Operand2IsLiteral(&isLiteral));

        if (isLiteral) [[likely]]
        {
            m_valueType = QualifierValueType::String;

            mrm::StringResult strResult;
            check_hresult(qualifier.GetOperand2Literal(&strResult));
            auto result = strResult.GetStringResult();

            m_stringValue = result->cchBuf ? hstring(result->pRef, result->cchBuf - 1) : hstring(result->pRef);
        }
        else [[unlikely]]
        {
            m_valueType = QualifierValueType::Attribute;

            mrm::Atom attribute2;
            check_hresult(qualifier.GetOperand2Attribute(&attribute2));
            m_attributeValue = static_cast<QualifierAttribute>(attribute2.GetUInt64());
        }

        m_priority = qualifier.GetPriority();

        double score = 0.0;
        check_hresult(qualifier.GetFallbackScore(&score));
        m_fallbackScore = score;
    }

    Qualifier::Qualifier(QualifierAttribute const& attribute, QualifierOperator const& _operator, hstring const& qualifierValue, int32_t priority, double fallbackScore)
    {
        m_attribute = attribute;
        m_operator = _operator;
        m_stringValue = qualifierValue;
        m_valueType = QualifierValueType::String;
        m_priority = priority;
        m_fallbackScore = fallbackScore;
    }

    winrt::MrmLib::Qualifier Qualifier::Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, winrt::MrmLib::QualifierOperator const& _operator, hstring const& qualifierValue, int32_t priority, double fallbackScore)
    {
        return winrt::make<implementation::Qualifier>(qualifierAttribute, _operator, qualifierValue, priority, fallbackScore);
    }

    winrt::MrmLib::Qualifier Qualifier::Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, winrt::MrmLib::QualifierOperator const& _operator, hstring const& qualifierValue, double fallbackScore)
    {
        return winrt::make<implementation::Qualifier>(qualifierAttribute, _operator, qualifierValue, 0, fallbackScore);
    }

    winrt::MrmLib::Qualifier Qualifier::Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, winrt::MrmLib::QualifierOperator const& _operator, hstring const& qualifierValue)
    {
        return winrt::make<implementation::Qualifier>(qualifierAttribute, _operator, qualifierValue, 0, 0.0);
    }

    winrt::MrmLib::Qualifier Qualifier::Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, hstring const& qualifierValue, int32_t priority, double fallbackScore)
    {
        return winrt::make<implementation::Qualifier>(qualifierAttribute, QualifierOperator::Match, qualifierValue, priority, fallbackScore);
    }

    winrt::MrmLib::Qualifier Qualifier::Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, hstring const& qualifierValue, double fallbackScore)
    {
        return winrt::make<implementation::Qualifier>(qualifierAttribute, QualifierOperator::Match, qualifierValue, 0, fallbackScore);
    }

    winrt::MrmLib::Qualifier Qualifier::Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, hstring const& qualifierValue)
    {
        return winrt::make<implementation::Qualifier>(qualifierAttribute, QualifierOperator::Match, qualifierValue, 0, 0.0);
    }

    winrt::MrmLib::QualifierAttribute Qualifier::Attribute()
    {
        /*mrm::Atom attribute;
        check_hresult(m_qualifier.GetOperand1Attribute(&attribute));

        return static_cast<winrt::MrmLib::QualifierAttribute>(attribute.GetUInt64());*/

        return m_attribute;
    }

    hstring Qualifier::AttributeName()
    {
		// TODO: Ensure index is in bounds
		return mrm::CoreEnvironment::QualifierNames[static_cast<uint32_t>(m_attribute)];
    }

    hstring Qualifier::AttributeTypeName()
    {
        // TODO: Ensure index is in bounds
		return mrm::CoreEnvironment::QualifierTypeNames[static_cast<uint32_t>(m_attribute)];
    }

    winrt::MrmLib::QualifierOperator Qualifier::Operator()
    {
        /*if (m_qualifier.OperatorIsCustom())
            return QualifierOperator::Custom;

        mrm::ICondition::ConditionOperator op = { };
        check_hresult(m_qualifier.GetOperator(&op));
        return static_cast<winrt::MrmLib::QualifierOperator>(op);*/

        return m_operator;
    }

    uint64_t Qualifier::CustomOperator()
    {
        /*if (!m_qualifier.OperatorIsCustom())
            return 0;

        mrm::Atom op;
        check_hresult(m_qualifier.GetCustomOperator(&op));
        return static_cast<uint32_t>(op.GetUInt64());*/

        return m_customOperator;
    }

    winrt::MrmLib::QualifierValueType Qualifier::ValueType()
    {
        /*bool isLiteral = false;
        check_hresult(m_qualifier.Operand2IsLiteral(&isLiteral));
        return isLiteral ? QualifierValueType::String : QualifierValueType::Attribute;*/

        return m_valueType;
    }

    hstring Qualifier::StringValue()
    {
        /*if (ValueType() == QualifierValueType::String)
        {
            mrm::StringResult strResult;
            check_hresult(m_qualifier.GetOperand2Literal(&strResult));
            auto result = strResult.GetStringResult();
            return result->cchBuf ? hstring(result->pRef, result->cchBuf - 1) : hstring(result->pRef);
        }*/

        if (m_valueType == QualifierValueType::String) [[likely]]
        {
            return m_stringValue;
        }

        return L"";
    }

    QualifierAttribute Qualifier::AttributeValue()
    {
        /*if (ValueType() == QualifierValueType::Attribute)
        {
            mrm::Atom attribute;
            check_hresult(m_qualifier.GetOperand2Attribute(&attribute));
            return static_cast<uint32_t>(attribute.GetUInt64());
        }*/

        if (m_valueType == QualifierValueType::Attribute) [[unlikely]]
        {
            return m_attributeValue;
        }

        return (QualifierAttribute)-1;
    }

    winrt::Windows::Foundation::IInspectable Qualifier::Value()
    {
        /*if (ValueType() == QualifierValueType::String)
            return winrt::box_value(StringValue());

        return winrt::box_value(AttributeValue());*/

        if (m_valueType == QualifierValueType::String) [[likely]]
            return winrt::box_value(m_stringValue);

        return winrt::box_value(m_attributeValue);
    }

    int32_t Qualifier::Priority()
    {
        /* 
        return m_qualifier.GetPriority();
        */

        return m_priority;
    }

    double Qualifier::FallbackScore()
    {
        /*double score = 0.0;
        check_hresult(m_qualifier.GetFallbackScore(&score));
        return score;*/

        return m_fallbackScore;
    }
}
