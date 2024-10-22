#include "Precompile.h"
#include "RenderObject.h"
#include "Model.h"

using namespace WinterEngine;
using namespace WinterEngine::Graphics;

void RenderObject::Terminate()
{
	meshBuffer.Terminate();

}

void RenderGroup::Initialize(const Model& model)
{
	for (const Model::MeshData& meshData : model.meshData)
	{
		RenderObject& renderObject = renderObjects.emplace_back();
		renderObject.meshBuffer.Initialize(meshData.mesh);
	}
}

void RenderGroup::Terminate()
{
	for (RenderObject& renderObject : renderObjects)
	{
		renderObject.Terminate();
	}
}