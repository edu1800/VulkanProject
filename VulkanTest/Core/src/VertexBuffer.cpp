#include "../include/VertexBuffer.h"
#include <algorithm>
#include "../include/CommonUtil.h"
#include "../include/HardwareBufferManager.h"

namespace Vulkan
{
	VertexElement::VertexElement(unsigned short u32VertexBufferIndex, unsigned int offset, VertexElementType eType, VertexElementSemantic eSemantic, unsigned int u32TextureCoordIndex)
		:m_u32VertexBufferIndex(u32VertexBufferIndex)
		,m_u32Offset(offset)
		,m_eType(eType)
		,m_eSemantic(eSemantic)
		,m_u32TextureCoordIndex(u32TextureCoordIndex)
	{
	}

	VertexElement::~VertexElement()
	{
	}

	unsigned int VertexElement::GetSize()
	{
		return GetTypeSize(m_eType);
	}

	unsigned int VertexElement::GetTypeSize(VertexElementType eType)
	{
		switch (eType)
		{
		case VET_COLOUR:
		case VET_COLOUR_ABGR:
		case VET_COLOUR_ARGB:
			return sizeof(unsigned int);
		case VET_FLOAT1:
			return sizeof(float);
		case VET_FLOAT2:
			return sizeof(float) * 2;
		case VET_FLOAT3:
			return sizeof(float) * 3;
		case VET_FLOAT4:
			return sizeof(float) * 4;
		case VET_DOUBLE1:
			return sizeof(double);
		case VET_DOUBLE2:
			return sizeof(double) * 2;
		case VET_DOUBLE3:
			return sizeof(double) * 3;
		case VET_DOUBLE4:
			return sizeof(double) * 4;
		case VET_SHORT1:
			return sizeof(short);
		case VET_SHORT2:
			return sizeof(short) * 2;
		case VET_SHORT3:
			return sizeof(short) * 3;
		case VET_SHORT4:
			return sizeof(short) * 4;
		case VET_USHORT1:
			return sizeof(unsigned short);
		case VET_USHORT2:
			return sizeof(unsigned short) * 2;
		case VET_USHORT3:
			return sizeof(unsigned short) * 3;
		case VET_USHORT4:
			return sizeof(unsigned short) * 4;
		case VET_INT1:
			return sizeof(int);
		case VET_INT2:
			return sizeof(int) * 2;
		case VET_INT3:
			return sizeof(int) * 3;
		case VET_INT4:
			return sizeof(int) * 4;
		case VET_UINT1:
			return sizeof(unsigned int);
		case VET_UINT2:
			return sizeof(unsigned int) * 2;
		case VET_UINT3:
			return sizeof(unsigned int) * 3;
		case VET_UINT4:
			return sizeof(unsigned int) * 4;
		case VET_UBYTE4:
			return sizeof(unsigned char) * 4;
		}
		return 0;
	}
	unsigned short VertexElement::GetTypeCount(VertexElementType eType)
	{
		switch (eType)
		{
		case VET_COLOUR:
		case VET_COLOUR_ABGR:
		case VET_COLOUR_ARGB:
		case VET_FLOAT1:
		case VET_SHORT1:
		case VET_USHORT1:
		case VET_UINT1:
		case VET_INT1:
		case VET_DOUBLE1:
			return 1;
		case VET_FLOAT2:
		case VET_SHORT2:
		case VET_USHORT2:
		case VET_UINT2:
		case VET_INT2:
		case VET_DOUBLE2:
			return 2;
		case VET_FLOAT3:
		case VET_SHORT3:
		case VET_USHORT3:
		case VET_UINT3:
		case VET_INT3:
		case VET_DOUBLE3:
			return 3;
		case VET_FLOAT4:
		case VET_SHORT4:
		case VET_USHORT4:
		case VET_UINT4:
		case VET_INT4:
		case VET_DOUBLE4:
		case VET_UBYTE4:
			return 4;
		}

		return 4;
	}

	VertexDeclaration::VertexDeclaration()
		: m_u32BindingIndex(0)
	{
	}

	VertexDeclaration::~VertexDeclaration()
	{
	}

	const VertexDeclaration::VertexElementList & VertexDeclaration::GetElements() const
	{
		return m_listElement;
	}

	const VertexElement * VertexDeclaration::GetElement(unsigned short index) const
	{
		if (index >= m_listElement.size())
		{
			return NULL;
		}

		VertexElementList::const_iterator itr = m_listElement.begin();
		for (size_t i = 0; i < index; i++)
		{
			++itr;
		}

		return &(*itr);
	}

	unsigned short VertexDeclaration::GetMaxVertexBufferIndex()
	{
		unsigned short ret = 0;
		VertexElementList::const_iterator itr = m_listElement.begin();
		VertexElementList::const_iterator itrEnd = m_listElement.end();
		for (;  itr != itrEnd; ++itr)
		{
			if (itr->GetVertexBufferIndex() > ret)
			{
				ret = itr->GetVertexBufferIndex();
			}
		}
		return ret;
	}

	const VertexElement & VertexDeclaration::AddElement(unsigned short vertexBufferIndex, unsigned int offset, VertexElementType eType, VertexElementSemantic eSemantic, int texturecoordIndex)
	{
		m_listElement.push_back(VertexElement(vertexBufferIndex, offset, eType, eSemantic, texturecoordIndex));

		return m_listElement.back();
	}

	const VertexElement & VertexDeclaration::InsertElement(unsigned short pos, unsigned short vertexBufferIndex, unsigned int offset, VertexElementType eType, VertexElementSemantic eSemantic, int texturecoordIndex)
	{
		if (pos >= m_listElement.size())
		{
			return AddElement(vertexBufferIndex, offset, eType, eSemantic, texturecoordIndex);
		}

		VertexElementList::iterator itr = m_listElement.begin();
		for (size_t i = 0; i < pos; i++)
		{
			++itr;
		}

		itr = m_listElement.insert(itr, VertexElement(vertexBufferIndex, offset, eType, eSemantic));
		return *itr;
	}

	void VertexDeclaration::RemoveElement(unsigned int elementIndex)
	{
		if (elementIndex >= m_listElement.size())
		{
			return;
		}

		VertexElementList::iterator itr = m_listElement.begin();
		for (size_t i = 0; i < elementIndex; i++)
		{
			itr++;
		}

		m_listElement.erase(itr);
	}

	void VertexDeclaration::RemoveElement(VertexElementSemantic eSemantic)
	{
		VertexElementList::iterator itr = m_listElement.begin();
		VertexElementList::iterator itrEnd = m_listElement.end();
		for (;  itr != itrEnd; ++itr)
		{
			if (itr->GetVertexElementSemantic() == eSemantic)
			{
				m_listElement.erase(itr);
				break;
			}
		}
	}

	void VertexDeclaration::RemoveAllElements()
	{
		m_listElement.clear();
	}

	void VertexDeclaration::ModifyElement(unsigned int elementIndex, unsigned short vertexBufferIndex, unsigned int offset, VertexElementType eType, VertexElementSemantic eSemantic, int texturecoordIndex)
	{
		if (elementIndex >= m_listElement.size())
		{
			return;
		}

		VertexElementList::iterator itr = m_listElement.begin();
		std::advance(itr, elementIndex);
		(*itr) = VertexElement(vertexBufferIndex, offset, eType, eSemantic, texturecoordIndex);
	}

	const VertexElement * VertexDeclaration::FindElementBySemantic(VertexElementSemantic eSemantic) const
	{
		VertexElementList::const_iterator itr = m_listElement.begin();
		VertexElementList::const_iterator itrEnd = m_listElement.end();
		for (; itr != itrEnd; ++itr)
		{
			if (itr->GetVertexElementSemantic() == eSemantic)
			{
				return &(*itr);
			}
		}

		return NULL;
	}

	VertexDeclaration::VertexElementList VertexDeclaration::FindElementsByVertexBufferIndex(unsigned short index) const
	{
		VertexElementList list;
		VertexElementList::const_iterator itr = m_listElement.begin();
		VertexElementList::const_iterator itrEnd = m_listElement.end();
		for (; itr != itrEnd; ++itr)
		{
			if (itr->GetVertexBufferIndex() == index)
			{
				list.push_back(*itr);
			}
		}

		return list;
	}

	unsigned int VertexDeclaration::GetVertexSize(unsigned short vertexBufferIndex) 
	{
		unsigned int sz = 0;
		VertexElementList::iterator itr = m_listElement.begin();
		VertexElementList::iterator itrEnd = m_listElement.end();
		for (; itr != itrEnd; ++itr)
		{
			if (itr->GetVertexBufferIndex() == vertexBufferIndex)
			{
				sz += itr->GetSize();
			}
		}

		return sz;
	}

	VertexBufferBinding::VertexBufferBinding()
		:m_u32HighIndex(0)
	{
	}

	VertexBufferBinding::~VertexBufferBinding()
	{
		UnsetAllBindings();
	}

	void VertexBufferBinding::SetBinding(unsigned short index, VertexBuffer* buffer)
	{
		if (buffer == NULL)
		{
			return;
		}

		m_mapBinding[index] = buffer;
		m_u32HighIndex = std::max(m_u32HighIndex, (unsigned short)(index + 1));
	}

	void VertexBufferBinding::UnsetBinding(unsigned short index)
	{
		VertexBufferBindingMap::iterator itr = m_mapBinding.find(index);
		if (itr != m_mapBinding.end())
		{
			HardwareBufferManager::GetSingleton().DestroyVertexBuffer(itr->second);
			m_mapBinding.erase(itr);
		}
	}

	void VertexBufferBinding::UnsetAllBindings()
	{
		VertexBufferBindingMap::iterator itr = m_mapBinding.begin();
		VertexBufferBindingMap::iterator itrEnd = m_mapBinding.end();
		for ( ; itr != itrEnd; itr++)
		{
			HardwareBufferManager::GetSingleton().DestroyVertexBuffer(itr->second);
		}

		m_mapBinding.clear();
		m_u32HighIndex = 0;
	}

	VertexBuffer * VertexBufferBinding::GetBuffer(unsigned short index)
	{
		VertexBufferBindingMap::iterator itr = m_mapBinding.find(index);
		if (itr != m_mapBinding.end())
		{
			return itr->second;
		}

		return NULL;
	}

	bool VertexBufferBinding::IsBufferBound(unsigned short index)
	{
		return m_mapBinding.find(index) != m_mapBinding.end();
	}

	unsigned short VertexBufferBinding::GetLastboundIndex()
	{
		return m_mapBinding.empty() ? 0 : m_mapBinding.rbegin()->first + 1;
	}

	VertexBuffer::VertexBuffer(unsigned int vertexSize, unsigned int numVertices)
		:HardwareBuffer()
		,m_u32VertexSize(vertexSize)
		,m_u32NumVertices(numVertices)
	{
		m_u32SizeInBytes = m_u32VertexSize * m_u32NumVertices;
	}

	VertexBuffer::~VertexBuffer()
	{
		
	}

	void VertexBuffer::ReadData(unsigned int offset, unsigned int length, void * pDest)
	{
	}

	void VertexBuffer::WriteData(unsigned int offset, unsigned int length, void * pSrc)
	{
	}
}