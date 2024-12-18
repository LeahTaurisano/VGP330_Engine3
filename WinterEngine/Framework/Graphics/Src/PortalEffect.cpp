#include "Precompile.h"
#include "PortalEffect.h"

#include "VertexTypes.h"
#include "StandardEffect.h"
#include "RenderObject.h"
#include "GraphicsSystem.h"

using namespace WinterEngine;
using namespace WinterEngine::Graphics;

void PortalEffect::Initialize(const MeshPX& portalMesh)
{
	mPortalMesh = portalMesh;
	std::filesystem::path shaderFile = L"../../Assets/Shaders/Portal.fx";
	mVertexShader.Initialize<VertexPX>(shaderFile);
	mPixelShader.Initialize(shaderFile);
	mTransformBuffer.Initialize();
	mPortalCamera.SetNearPlane(0.1f);
	mPortalCamera.SetFarPlane(1000.0f);
	//mPortalCamera.SetAspectRatio(1.0f);
	mPortalCamera.SetMode(Camera::ProjectionMode::Perspective);
	mSampler.Initialize(Sampler::Filter::Linear, Sampler::AddressMode::Wrap);
	constexpr uint32_t depthMapResolution = 4096;
	mPortalRenderTarget.Initialize(depthMapResolution, depthMapResolution, Texture::Format::RGBA_U32);
}

void PortalEffect::Terminate()
{
	mSampler.Terminate();
	mPortalRenderTarget.Terminate();
	mTransformBuffer.Terminate();
	mPixelShader.Terminate();
	mVertexShader.Terminate();
}

void PortalEffect::Begin()
{
	mVertexShader.Bind();
	mPixelShader.Bind();
	mTransformBuffer.BindVS(0);
	mPortalRenderTarget.BindPS(0);
	mSampler.BindPS(0);
}

void PortalEffect::Render(const RenderObject& renderObject)
{
	//Screen space vs world space

	const Math::Matrix4 matWorld = renderObject.transform.GetMatrix4();
	const Math::Matrix4 matView = mGameCamera->GetViewMatrix();
	const Math::Matrix4 matProj = mGameCamera->GetProjectionMatrix();

	TransformData data;
	data.wvp = Math::Transpose(matWorld * matView * matProj);
	data.portalPos = mPortalObject->transform.position;
	mTransformBuffer.Update(data);
	renderObject.meshBuffer.Render();
}

void PortalEffect::End()
{
}

void PortalEffect::BeginPortalImageRender()
{
	UpdatePortalCamera();

	mPortalRenderTarget.BeginRender();
	//mStandardEffect->SetCamera(*mLinkedPortalCamera);
	mStandardEffect->SetCamera(mPortalCamera);
	mStandardEffect->Begin();
}

void PortalEffect::PortalImageRender(const RenderObject& renderObject)
{
	mStandardEffect->Render(renderObject);
}

void PortalEffect::PortalImageRender(const RenderGroup& renderGroup)
{
	mStandardEffect->Render(renderGroup);
}

void PortalEffect::EndPortalImageRender()
{
	mStandardEffect->End();
	mPortalRenderTarget.EndRender();
	mStandardEffect->SetCamera(*mGameCamera);
}

void PortalEffect::DebugUI()
{
	if (ImGui::CollapsingHeader("PortalEffect", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Image(
			mPortalRenderTarget.GetRawData(),
			{ 144, 144 },
			{ 0, 0 },
			{ 1, 1 },
			{ 1, 1, 1, 1 },
			{ 1, 1, 1, 1 });
		//ImGui::DragFloat("Size##Portal", &mSize, 1.0f, 1.0f, 1000.0f);
	}
}

void PortalEffect::SetStandardEffect(StandardEffect& standardEffect)
{
	mStandardEffect = &standardEffect;
}

void PortalEffect::LinkPortal(const PortalEffect& linkPortal)
{
	mLinkedPortalCamera = &linkPortal.GetPortalCamera();
	mLinkedPortalObject = &linkPortal.GetPortalObject();
}
void PortalEffect::LinkPortalMesh(const MeshPX& meshPX)
{
	mLinkedPortalMesh = &meshPX;
}

void PortalEffect::SetGameCamera(const Camera& gameCamera)
{
	mGameCamera = &gameCamera;
}

void PortalEffect::SetPortalObject(RenderObject& renderObject)
{
	mPortalObject = &renderObject;
}

void PortalEffect::UpdatePortalCamera()
{
	//ASSERT(mDirectionalLight != nullptr, "ShadowEffect: no light set");
	Math::Matrix4 portalMat = mLinkedPortalObject->transform.GetMatrix4();
	//Math::Vector3 direction = { -portalMat._31, -portalMat._32, -portalMat._33 }; //static portal image
	//Math::Vector3 direction = (mGameCamera->GetPosition() - mLinkedPortalCamera->GetPosition()); //Adjusts based on position, needs improving
	Math::Vector3 dirToPortal = mGameCamera->GetDirection(); //testing based on the portal object, puts both cameras in the same position
	dirToPortal.x *= -1;
	dirToPortal.z *= -1;
	Math::Vector3 direction = Math::Normalize(dirToPortal);
	//Math::Vector3 position = Math::Vector3(portalMat._41, portalMat._42, portalMat._43) - direction; //static portal image
	//Math::Vector3 position = mPortalObject->transform.position - direction * 0.3; //adjust based on position, needs improving
	Math::Vector3 distToPortal = mGameCamera->GetPosition() - mPortalObject->transform.position; //testing based on the portal object, puts both cameras in the same position
	distToPortal.x *= -1;
	distToPortal.z *= -1;
	Math::Vector3 position = mLinkedPortalObject->transform.position + distToPortal;
	position.y = mGameCamera->GetPosition().y;
	mPortalCamera.SetPosition(position);
	mPortalCamera.SetDirection(direction);
	//.SetPosition(mFocusPoint - (direction * 100.0f));
	mPortalCamera.SetSize(mSize, mSize);

	UpdatePortalUV();
}
void PortalEffect::UpdatePortalUV()
{
	float width = GraphicsSystem::Get()->GetBackBufferWidth();
	float height = GraphicsSystem::Get()->GetBackBufferHeight();
	float hw = width * 0.5f;
	float hh = height * 0.5f;
	Math::Matrix4 matScreenSpace = {
		  hw, 0.0f, 0.0f, 0.0f,
		0.0f,  -hh, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		  hw,   hh, 0.0f, 1.0f
	};
	Math::Matrix4 matWorld = mPortalObject->transform.GetMatrix4();
	const Math::Matrix4 matView = mGameCamera->GetViewMatrix();
	const Math::Matrix4 matProj = mGameCamera->GetProjectionMatrix();
	Math::Matrix4 matFinal = matWorld * matView * matProj * matScreenSpace;

	for (size_t i = 0; i < mPortalMesh.vertices.size(); ++i)
	{
		Math::Matrix4 ndcSpacePos = Math::Matrix4::Translation(mLinkedPortalMesh->vertices[i].position);// = Math::TransformCoord(mLinkedPortalMesh->vertices[i].position, matFinal);
		ndcSpacePos = ndcSpacePos * matFinal;
		Math::Vector2 newUV;
		newUV.x = (ndcSpacePos._41 / ndcSpacePos._44) / width;
		newUV.y = (ndcSpacePos._42 / ndcSpacePos._44) / height;
		//newUV.x = ndcSpacePos.x / width;
		//newUV.y = ndcSpacePos.y / height;
		mPortalMesh.vertices[i].uvCoord = newUV;
	}
	mPortalObject->meshBuffer.Update(mPortalMesh.vertices.data(), mPortalMesh.vertices.size());
	// portalVertices
	// wvp
	// -1, 1
	// (vector.x + 1 / 2) update uv

}

const Camera& PortalEffect::GetPortalCamera() const
{
	return mPortalCamera;
}

const RenderObject& PortalEffect::GetPortalObject() const
{
	return *mPortalObject;
}
