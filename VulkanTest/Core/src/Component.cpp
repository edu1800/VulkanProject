#include "../include/Component.h"
#include "../include/CommonUtil.h"
namespace Vulkan
{
	Component::Component()
		:m_pGameObject(NULL)
		,m_HashCode(0)
	{

	}

	Component::~Component()
	{
	}

	unsigned Component::GetHashCode()
	{
		if (m_HashCode == 0)
		{
			m_HashCode = CommonUtil::GetHashCode(typeid(*this).name());
		}

		return m_HashCode;
	}
}