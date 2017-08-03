#ifndef _VERTEX_BUFFER_H_
#define _VERTEX_BUFFER_H_

#pragma once

#define NOMINMAX 1

#include <list>
#include <map>
#include <vector>
#include "HardwareBufferBase.h"
#include "VertexIndexDefine.h"

namespace Vulkan
{
	class VertexElement
	{
	public:
		VertexElement() {};
		VertexElement(unsigned short u32VertexBufferIndex, unsigned int offset, VertexElementType eType, VertexElementSemantic eSemantic, unsigned int u32TextureCoordIndex = 0);
		~VertexElement();

		unsigned int GetVertexBufferIndex() const { return m_u32VertexBufferIndex; }
		unsigned int GetOffset() const { return m_u32Offset; }
		VertexElementType GetVertexElementType() const { return m_eType; }
		VertexElementSemantic GetVertexElementSemantic() const { return m_eSemantic; }
		unsigned int GetTxtureCoordIndex() const { return m_u32TextureCoordIndex; }

		unsigned int GetSize();

		static unsigned int GetTypeSize(VertexElementType eType);
		static unsigned short GetTypeCount(VertexElementType eType);

		inline bool operator== (const VertexElement& rhs) const
		{
			if (m_eType != rhs.m_eType ||
				m_eSemantic != rhs.m_eSemantic ||
				m_u32VertexBufferIndex != rhs.m_u32VertexBufferIndex ||
				m_u32Offset != rhs.m_u32Offset ||
				m_u32TextureCoordIndex != rhs.m_u32TextureCoordIndex)
				return false;
			return true;
		}

		inline void BaseVertexPointerToElement(void* pBase, void** pElement)
		{
			*pElement = static_cast<void*>(static_cast<unsigned char*>(pBase) + m_u32Offset);
		}
	private:
		unsigned short m_u32VertexBufferIndex;
		//The offset in the buffer that this element starts at
		unsigned int m_u32Offset;
		VertexElementType m_eType;
		VertexElementSemantic m_eSemantic;
		unsigned int m_u32TextureCoordIndex;
	};

	class VertexDeclaration
	{
	public:
		typedef std::list<VertexElement> VertexElementList;

		VertexDeclaration();
		~VertexDeclaration();

		unsigned int GetElementCount() { return m_listElement.size(); }
		const VertexElementList& GetElements() const;
		const VertexElement* GetElement(unsigned short index) const;

		unsigned short GetMaxVertexBufferIndex();
		const VertexElement& AddElement(unsigned short vertexBufferIndex, unsigned int offset, VertexElementType eType, VertexElementSemantic eSemantic, int texturecoordIndex = 0);
		const VertexElement& InsertElement(unsigned short pos, unsigned short vertexBufferIndex, unsigned int offset, VertexElementType eType, VertexElementSemantic eSemantic, int texturecoordIndex = 0);
		void RemoveElement(unsigned int elementIndex);
		void RemoveElement(VertexElementSemantic eSemantic);
		void RemoveAllElements();
		void ModifyElement(unsigned int elementIndex, unsigned short vertexBufferIndex, unsigned int offset, VertexElementType eType, VertexElementSemantic eSemantic, int texturecoordIndex = 0);
		const VertexElement* FindElementBySemantic(VertexElementSemantic eSemantic) const;
		VertexElementList FindElementsByVertexBufferIndex(unsigned short index) const;
		unsigned int GetVertexSize(unsigned short vertexBufferIndex);

		void SetBindingIndex(unsigned short index) { m_u32BindingIndex = index; }
		unsigned short GetBindingIndex() const { return m_u32BindingIndex; }

		inline bool operator== (const VertexDeclaration& rhs) const
		{
			if (m_listElement.size() != rhs.m_listElement.size())
				return false;

			VertexElementList::const_iterator itr = m_listElement.begin();
			VertexElementList::const_iterator itrEnd = m_listElement.end();
			VertexElementList::const_iterator ritr = rhs.m_listElement.begin();
			VertexElementList::const_iterator ritrEnd = rhs.m_listElement.end();
			for (;  itr != itrEnd && ritr != ritrEnd; ++itr, ++ritr)
			{
				if (!(*itr == *ritr))
					return false;
			}

			return true;
		}
	private:
		VertexElementList m_listElement;
		unsigned short m_u32BindingIndex;
	};

	class VertexBuffer : public HardwareBuffer
	{
	public:
		VertexBuffer(unsigned int vertexSize, unsigned int numVertices);
		~VertexBuffer();

		unsigned int GetVertexSize() const { return m_u32VertexSize; }
		unsigned int GetNumVertices() const { return m_u32NumVertices; }

		virtual void ReadData(unsigned int offset, unsigned int length, void* pDest);
		virtual void WriteData(unsigned int offset, unsigned int length, void* pSrc);

	protected:
		unsigned int m_u32VertexSize;
		unsigned int m_u32NumVertices;
	};

	class VertexBufferBinding
	{
	public:
		typedef std::map<unsigned short, VertexBuffer*> VertexBufferBindingMap;

		VertexBufferBinding();
		~VertexBufferBinding();

		void SetBinding(unsigned short index, VertexBuffer* buffer);
		//Remove an existing binding
		void UnsetBinding(unsigned short index);
		void UnsetAllBindings();
		const VertexBufferBindingMap& GetBindings() const { return m_mapBinding; }
		VertexBuffer* GetBuffer(unsigned short index);
		bool IsBufferBound(unsigned short index);
		unsigned int GetBufferCount() const { return m_mapBinding.size(); }
		unsigned short GetNextIndex() { return m_u32HighIndex++; }
		unsigned short GetLastboundIndex();

	private:
		VertexBufferBindingMap m_mapBinding;
		unsigned short m_u32HighIndex;
	};
}

#endif
