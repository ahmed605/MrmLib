#pragma once
#include "Qualifier.g.h"

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
    struct Qualifier : QualifierT<Qualifier>
    {
    private:
		//mrm::QualifierResult m_qualifier;
		QualifierAttribute m_attribute = (QualifierAttribute)-1;
        hstring m_attributeName;
		QualifierOperator m_operator = (QualifierOperator)-1;
		//QualifierValueType m_valueType = (QualifierValueType)-1;
		hstring m_stringValue;
        //QualifierAttribute m_attributeValue = (QualifierAttribute)-1;
        hstring m_customOperator;
		int32_t m_priority = 0;
		double m_fallbackScore = 0.0;

    public:
        Qualifier() = default;
		Qualifier(mrm::QualifierResult&& qualifier, mrm::AtomPoolGroup* pPoolGroup, mrm::MrmPlatformVersionInternal version);
		Qualifier(QualifierAttribute const& attribute, QualifierOperator const& _operator, hstring const& qualifierValue, int32_t priority, double fallbackScore);

        static winrt::MrmLib::Qualifier Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, winrt::MrmLib::QualifierOperator const& _operator, hstring const& qualifierValue, int32_t priority, double fallbackScore);
        static winrt::MrmLib::Qualifier Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, winrt::MrmLib::QualifierOperator const& _operator, hstring const& qualifierValue, double fallbackScore);
        static winrt::MrmLib::Qualifier Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, winrt::MrmLib::QualifierOperator const& _operator, hstring const& qualifierValue);
        static winrt::MrmLib::Qualifier Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, hstring const& qualifierValue, int32_t priority, double fallbackScore);
        static winrt::MrmLib::Qualifier Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, hstring const& qualifierValue, double fallbackScore);
        static winrt::MrmLib::Qualifier Create(winrt::MrmLib::QualifierAttribute const& qualifierAttribute, hstring const& qualifierValue);

        winrt::MrmLib::QualifierAttribute Attribute();
        hstring AttributeName();
        winrt::MrmLib::QualifierOperator Operator();
        hstring CustomOperator();
        hstring Value();
        int32_t Priority();
        double FallbackScore();
    };
}

namespace winrt::MrmLib::factory_implementation
{
    struct Qualifier : QualifierT<Qualifier, implementation::Qualifier>
    {
    };
}