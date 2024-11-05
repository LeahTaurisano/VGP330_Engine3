#pragma once

#include "MeshBuffer.h"
#include "TextureCache.h"
#include "ModelCache.h"
#include "Transform.h"
#include "Material.h"

namespace WinterEngine::Graphics
{
	struct Model;

	class RenderObject
	{
	public:
		void Terminate();

		Transform transform;
		MeshBuffer meshBuffer;

		Material material;
		TextureId diffuseMapId;
		TextureId normalMapId;
		TextureId specMapId;
		TextureId bumpMapId;
	};

	class RenderGroup
	{
	public:
		void Initialize(const std::filesystem::path& modelFilePath);
		void Initialize(const Model& model);
		void Terminate();

		ModelId modelId;
		Transform transform;
		std::vector<RenderObject> renderObjects;
	};
}