#pragma once
#include "Textures.h"

typedef arrayWithLength<_TEXTURE_NAME> textureNameArray;
class ResourceLocation
{
private:
	textureNameArray m_texture;
	wstring m_path;
	bool m_preloaded;

public:
	ResourceLocation()
	{
		m_preloaded = false;
		m_path = L"";
	}

	ResourceLocation(_TEXTURE_NAME texture)
	{
		m_texture = textureNameArray(1);
		m_texture[0] = texture;
		m_preloaded = true;
	}

	ResourceLocation(wstring path)
	{
		m_path = path;
		m_preloaded = false;
	}

	ResourceLocation(intArray textures)
	{
		m_texture = textureNameArray(textures.length);
		for(unsigned int i = 0; i < textures.length; ++i)
		{
			m_texture[i] = static_cast<_TEXTURE_NAME>(textures[i]);
		}
		m_preloaded = true;
	}

	// Deep-copying copy ctor and copy assignment (rule of three). arrayWithLength is
	// a shallow wrapper by design (no dtor, no copy ctor), so the implicit copies
	// shared m_texture.data and every copy's destructor deleted it -- a double-free
	// that corrupted the heap. Each ResourceLocation now owns its own texture array.
	// (m_texture.data is always either nullptr or a new[] allocation made by one of
	// the ctors above, so delete[] is the matching free.)
	ResourceLocation(const ResourceLocation &other)
		: m_path(other.m_path), m_preloaded(other.m_preloaded)
	{
		m_texture = textureNameArray(other.m_texture.length);
		for (unsigned int i = 0; i < other.m_texture.length; ++i)
		{
			m_texture[i] = other.m_texture[i];
		}
	}

	ResourceLocation &operator=(const ResourceLocation &other)
	{
		if (this != &other)
		{
			delete[] m_texture.data;
			m_path = other.m_path;
			m_preloaded = other.m_preloaded;
			m_texture = textureNameArray(other.m_texture.length);
			for (unsigned int i = 0; i < other.m_texture.length; ++i)
			{
				m_texture[i] = other.m_texture[i];
			}
		}
		return *this;
	}

	~ResourceLocation()
	{
		delete[] m_texture.data;
	}

	_TEXTURE_NAME getTexture()
	{
		return m_texture[0];
	}

	_TEXTURE_NAME getTexture(int idx)
	{
		return m_texture[idx];
	}

	int getTextureCount()
	{
		return m_texture.length;
	}

	wstring getPath()
	{
		return m_path;
	}

	bool isPreloaded()
	{
		return m_preloaded;
	}
};