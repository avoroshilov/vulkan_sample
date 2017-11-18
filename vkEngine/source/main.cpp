#include "app.h"

#include "windows/timer.h"
#include "windows/window.h"
#include "vulkan/manager.h"
#include "camera.h"

#define MOUSE_USE_RAWINPUT	1

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char * layerPrefix,
	const char * msg,
	void * userData
	)
{
	printf("[VAL] %s\n", msg);
	return VK_FALSE;
}

void resizeCallback(void * pUserData, int width, int height)
{
	if (pUserData)
	{
		CallbackData * callbackData = reinterpret_cast<CallbackData *>(pUserData);
		callbackData->app->onWindowResize(width, height);
	}
}
void chageFocusCallback(void * pUserData, bool isInFocus)
{
}
void chageActiveCallback(void * pUserData, bool isActive)
{
	printf("Active: %s\n", isActive ? "true" : "false");
	if (pUserData)
	{
		CallbackData * callbackData = reinterpret_cast<CallbackData *>(pUserData);
		if (isActive)
		{
			windows::hideMouse();
			windows::setMouseCoordinates(callbackData->mx, callbackData->my);
		}
		else
		{
			windows::showMouse();
		}
		callbackData->isActive = isActive;
	}
}
void keyStateCallback(void * pUserData, windows::KeyCode keyCode, windows::KeyState keyState)
{
	using namespace windows;

#define DBG_KEY_TESTING 0

#if (DBG_KEY_TESTING == 1)
	char * keyStatus = "UNKNOWN";
	if (keyState == KeyState::ePressed)
	{
		keyStatus = "pressed";
	}
	else if (keyState == KeyState::eHeldDown)
	{
		keyStatus = "held down";
	}
	else if (keyState == KeyState::eReleased)
	{
		keyStatus = "released";
	}
#endif

	if (!pUserData)
		return;

	CallbackData * callbackData = reinterpret_cast<CallbackData *>(pUserData);
	App * app = callbackData->app;

	if (!app)
		return;

	switch (keyCode)
	{
		case KeyCode::eEscape:
		{
			app->setIsExitting(true);
			break;
		}
		case KeyCode::eF12:
		{
			if (keyState == KeyState::ePressed)
			{
				app->requestCapture(true);
			}
			break;
		}
		case (KeyCode)'Q':
		{
			if (keyState == KeyState::ePressed)
			{
				callbackData->isPaused = !callbackData->isPaused;
			}
			break;
		}
		case (KeyCode)'W':
		{
			if (keyState == KeyState::ePressed || keyState == KeyState::eHeldDown)
				callbackData->movementFlags |= (uint32_t)CallbackData::MovementKindBits::eForward;
			else
				callbackData->movementFlags &= ~(uint32_t)CallbackData::MovementKindBits::eForward;

			break;
		}
		case (KeyCode)'S':
		{
			if (keyState == KeyState::ePressed || keyState == KeyState::eHeldDown)
				callbackData->movementFlags |= (uint32_t)CallbackData::MovementKindBits::eBackward;
			else
				callbackData->movementFlags &= ~(uint32_t)CallbackData::MovementKindBits::eBackward;

			break;
		}
		case (KeyCode)'A':
		{
			if (keyState == KeyState::ePressed || keyState == KeyState::eHeldDown)
				callbackData->movementFlags |= (uint32_t)CallbackData::MovementKindBits::eLeft;
			else
				callbackData->movementFlags &= ~(uint32_t)CallbackData::MovementKindBits::eLeft;

			break;
		}
		case (KeyCode)'D':
		{
			if (keyState == KeyState::ePressed || keyState == KeyState::eHeldDown)
				callbackData->movementFlags |= (uint32_t)CallbackData::MovementKindBits::eRight;
			else
				callbackData->movementFlags &= ~(uint32_t)CallbackData::MovementKindBits::eRight;

			break;
		}
		case KeyCode::ePgDown:
		{
			if (keyState == KeyState::ePressed || keyState == KeyState::eHeldDown)
				callbackData->movementFlags |= (uint32_t)CallbackData::MovementKindBits::eDown;
			else
				callbackData->movementFlags &= ~(uint32_t)CallbackData::MovementKindBits::eDown;

			break;
		}
		case KeyCode::ePgUp:
		{
			if (keyState == KeyState::ePressed || keyState == KeyState::eHeldDown)
				callbackData->movementFlags |= (uint32_t)CallbackData::MovementKindBits::eUp;
			else
				callbackData->movementFlags &= ~(uint32_t)CallbackData::MovementKindBits::eUp;

			break;
		}
		case KeyCode::eLShift:
		{
			if (keyState == KeyState::ePressed || keyState == KeyState::eHeldDown)
				callbackData->movementFlags |= (uint32_t)CallbackData::MovementKindBits::eAccel;
			else
				callbackData->movementFlags &= ~(uint32_t)CallbackData::MovementKindBits::eAccel;

			break;
		}
		case KeyCode::eLCtrl:
		{
			if (keyState == KeyState::ePressed || keyState == KeyState::eHeldDown)
				callbackData->movementFlags |= (uint32_t)CallbackData::MovementKindBits::eDeccel;
			else
				callbackData->movementFlags &= ~(uint32_t)CallbackData::MovementKindBits::eDeccel;

			break;
		}
#if (DBG_KEY_TESTING == 1)
		case KeyCode::eEnter:
		{
			printf("Enter %s\n", keyStatus);
			break;
		}
		case KeyCode::eRCtrl:
		{
			printf("RCtrl %s\n", keyStatus);
			break;
		}
		case KeyCode::eRShift:
		{
			printf("RShift %s\n", keyStatus);
			break;
		}
		case KeyCode::eLAlt:
		{
			printf("LAlt %s\n", keyStatus);
			break;
		}
		case KeyCode::eRAlt:
		{
			printf("RAlt %s\n", keyStatus);
			break;
		}
		default:
		{
			printf("Key %d %s\n", (int)keyCode, keyStatus);
			break;
		}
#endif
	}
}

void mouseEventCallback(void * pUserData, windows::MouseEvent mouseEvent)
{
	if (!pUserData)
		return;

	CallbackData * callbackData = reinterpret_cast<CallbackData *>(pUserData);
	App * app = callbackData->app;

	if (!app)
		return;

	callbackData->dmx += mouseEvent.dX;
	callbackData->dmy += mouseEvent.dY;
}

class SphericalMesh
{

	uint32_t m_detI = 64;
	uint32_t m_detJ = 64;

	std::vector<vulkan::Vertex> m_vertices;

	vulkan::Mesh * m_mesh = nullptr;

	math::Vec3 m_obbMin, m_obbMax;

public:

	void getOBB(math::Vec3 * pOBBMin, math::Vec3 * pOBBMax) const
	{
		*pOBBMin = m_obbMin;
		*pOBBMax = m_obbMax;
	}

	const math::Mat44 & getModelMatrix() const
	{
		return m_mesh->getModelMatrix();
	}

	void init(vulkan::Mesh * mesh, uint32_t detI, uint32_t detJ)
	{
		m_detI = detI;
		m_detJ = detJ;
		m_mesh = mesh;

		const size_t numVertices = m_detI*m_detJ;
		m_vertices.resize(numVertices);

		const uint32_t numQuadsI = m_detI - 1;
		const uint32_t numQuadsJ = m_detJ - 1;
		const uint32_t numTrisPerQuad = 2;
		const uint32_t numTris = numQuadsI*numQuadsJ*numTrisPerQuad;
		const uint32_t numIdxPerTri = 3;
		const size_t numIndices = numTris * numIdxPerTri;

		m_mesh->initBuffers(
			vulkan::BufferUsage::eDynamic, (uint32_t)numVertices, sizeof(vulkan::Vertex),
			vulkan::BufferUsage::eStatic, (uint32_t)numIndices
			);

		std::vector<uint16_t> indices;
		indices.resize(numIndices);
		for (uint32_t qj = 0; qj < numQuadsJ; ++qj)
		{
			for (uint32_t qi = 0; qi < numQuadsI; ++qi)
			{
				uint32_t triOffset;

				triOffset = ((qi+qj*numQuadsI)*numTrisPerQuad+0)*numIdxPerTri;
				indices[triOffset+0] = qi+qj*m_detI;
				indices[triOffset+1] = (qi+1)+qj*m_detI;
				indices[triOffset+2] = qi+(qj+1)*m_detI;

				triOffset = ((qi+qj*numQuadsI)*numTrisPerQuad+1)*numIdxPerTri;
				indices[triOffset+0] = (qi+1)+qj*m_detI;
				indices[triOffset+1] = (qi+1)+(qj+1)*m_detI;
				indices[triOffset+2] = qi+(qj+1)*m_detI;
			}
		}
		m_mesh->updateIndexBuffer(indices.data());
	}

	void update(double elapsedTime)
	{
		using namespace math;

		Mat44 modelMat;
		modelMat.identity();
		modelMat.fillRotation(Vec3C(0.1f, 0.2f, 0.0f).normalize(), (float)elapsedTime * 0.001f);
		Mat44 rot;
		rot.identity();
		rot.fillRotation(Vec3C(0.0f, 0.1f, 0.2f).normalize(), (float)elapsedTime * 0.0005f);
		m_mesh->setModelMatrix(rot * modelMat);

		const float rad = 0.7f;
		const Vec3 offset = Vec3C(0.0f, 0.0f, 0.0f);
#define SIMPLE_DISTORT 0

		m_obbMin = Vec3C( FLT_MAX,  FLT_MAX,  FLT_MAX);
		m_obbMax = Vec3C(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		auto checkDim = [](float vertexCoord, float * obbMinCoord, float * obbMaxCoord)
		{
			if (*obbMinCoord > vertexCoord)
				*obbMinCoord = vertexCoord;
			if (*obbMaxCoord < vertexCoord)
				*obbMaxCoord = vertexCoord;
		};

		vulkan::Vertex * vertices = m_vertices.data();

#if (SIMPLE_DISTORT == 1)
		float amp = 0.2f, shift = (float)elapsedTime / 500.0f;
		Vec3 sines = Vec3C(2, 2, 3);
		for (uint32_t vj = 0; vj < m_detJ; ++vj)
		{
			const float nrm_vj = vj/(float)(m_detJ - 1);
			const float angJ = PI*nrm_vj - _PI2;

			for (uint32_t vi = 0; vi < m_detI; ++vi)
			{
				const float nrm_vi = vi/(float)(m_detI - 1);
				const float angI = _2PI*nrm_vi;

				vulkan::Vertex & curVertex = vertices[vi+vj*m_detI];
				curVertex.pos = offset + Vec3C(rad * cosf(angJ)*cosf(angI), rad * sinf(angJ), rad * cosf(angJ)*sinf(angI));

				Vec3 distort;
				distort.x = sinf(curVertex.pos.y*curVertex.pos.z * PI * sines.x + shift) * amp;
				distort.y = cosf(curVertex.pos.x*curVertex.pos.z * PI * sines.y + shift) * amp;
				distort.z = sinf(curVertex.pos.x*curVertex.pos.y * PI * sines.z + shift) * amp;

				curVertex.pos += distort;

				for (int posC = 0; posC < 3; ++posC)
					checkDim(curVertex.pos.v[posC], m_obbMin.v+posC, m_obbMax.v+posC);

				curVertex.tc = Vec2C(nrm_vi, nrm_vj);
				curVertex.col = Vec4C(1.0f, 1.0f, 1.0f, 1.0f);

				curVertex.nrm = Vec3C(0.0f, 0.0f, 0.0f);
			}
		}
#else
		float varying0 = sinf((float)elapsedTime * 0.0009f) * 0.5f + 0.5f;
		float varying1 = cosf((float)elapsedTime * 0.0015f) * 0.5f + 0.5f;
		float angMomentumEigenMat[8] =
			{
				varying0 * 4.0f + 1.0f, 3, varying1 * 4.0f + 1.0f, 4,
				1, 2, 4, 4
			};
		for (uint32_t vj = 0; vj < m_detJ; ++vj)
		{
			const float nrm_vj = vj/(float)(m_detJ - 1);
			const float angJ = PI*nrm_vj - _PI2;

			for (uint32_t vi = 0; vi < m_detI; ++vi)
			{
				const float nrm_vi = vi/(float)(m_detI - 1);
				const float angI = _2PI*nrm_vi;

				vulkan::Vertex & curVertex = vertices[vi+vj*m_detI];

				float * m = angMomentumEigenMat;
				float shAngMomentumEigen =
					powf(sinf(m[0] * angJ), m[1]) +
					powf(cosf(m[2] * angJ), m[3]) +
					powf(sinf(m[4] * angI), m[5]) +
					powf(cosf(m[6] * angI), m[7]);
				float curRad = rad * (0.5f * shAngMomentumEigen);

				curVertex.pos = offset + Vec3C(curRad * cosf(angJ)*cosf(angI), curRad * sinf(angJ), curRad * cosf(angJ)*sinf(angI));

				for (int posC = 0; posC < 3; ++posC)
					checkDim(curVertex.pos.v[posC], m_obbMin.v+posC, m_obbMax.v+posC);

				curVertex.tc = Vec2C(nrm_vi, nrm_vj);
				curVertex.col = Vec4C(1.0f, curRad, curRad, 1.0f);

				curVertex.nrm = Vec3C(0.0f, 0.0f, 0.0f);
			}
		}
#endif
		for (uint j = 0; j < m_detJ; j++)
		{
			uint jAdd0 = j * m_detI;
			uint jAdd1 = ((j + 1) % m_detJ) * m_detI;

			for (int i0 = 0; i0 < (int)m_detI; ++i0)
			{
				uint i1 = i0 + 1;
				if (i1 == m_detI)
				{
					// detI-1 and 0 vertices exactly match, so diff would be 0 and normal incorrect, need to use slice#1
					i1 = 1;
				}
				Vec3 normal = (vertices[i0+jAdd1].pos - vertices[i0+jAdd0].pos).cross(vertices[i1+jAdd0].pos - vertices[i0+jAdd0].pos).getNormalized();
				m_vertices[i0 + jAdd0].nrm += normal;
			}
		}
		for (uint cnt = 0; cnt < m_detI*m_detJ; cnt++)
		{
			vertices[cnt].nrm.normalize();
		}

		m_mesh->updateVertexBuffer(vertices);
	}
};

class CubeMesh
{
	vulkan::Mesh * m_mesh = nullptr;

public:

	void init(vulkan::Mesh * mesh, const math::Vec3 & offset, const math::Vec3 & cubeSize, const math::Vec4 & color)
	{
		using namespace math;

		m_mesh = mesh;

		const size_t numVertices = 24;
		std::vector<vulkan::Vertex> vertices;
		vertices.resize(numVertices);

		const size_t numIndices = 36;
		std::vector<uint16_t> indices;
		indices.resize(numIndices);

		m_mesh->initBuffers(
			vulkan::BufferUsage::eStatic, (uint32_t)numVertices, sizeof(vulkan::Vertex),
			vulkan::BufferUsage::eStatic, (uint32_t)numIndices
			);

		Mat44 modelMatrix;
		modelMatrix.identity();
		modelMatrix.fillTranslation(offset);
		m_mesh->setModelMatrix(modelMatrix);

		auto finalizeFace = [&vertices](int vtxOffset, const Vec3 & offset, const Vec3 & normal, const Vec4 & faceColor)
		{
			vertices[vtxOffset+0].pos += offset;
			vertices[vtxOffset+1].pos += offset;
			vertices[vtxOffset+2].pos += offset;
			vertices[vtxOffset+3].pos += offset;

			vertices[vtxOffset+0].nrm = normal;
			vertices[vtxOffset+1].nrm = normal;
			vertices[vtxOffset+2].nrm = normal;
			vertices[vtxOffset+3].nrm = normal;

			vertices[vtxOffset+0].col = faceColor;
			vertices[vtxOffset+1].col = faceColor;
			vertices[vtxOffset+2].col = faceColor;
			vertices[vtxOffset+3].col = faceColor;

			vertices[vtxOffset+0].tc = Vec2C( 0.0f,  0.0f);
			vertices[vtxOffset+1].tc = Vec2C( 1.0f,  0.0f);
			vertices[vtxOffset+2].tc = Vec2C( 1.0f,  1.0f);
			vertices[vtxOffset+3].tc = Vec2C( 0.0f,  1.0f);
		};

		Vec3 vecOffset = Vec3C(0.0f, 0.0f, 0.0f);

		// Vertices
		// Front
		vertices[ 0].pos = Vec3C(-cubeSize.x, -cubeSize.y,  cubeSize.z);
		vertices[ 1].pos = Vec3C(-cubeSize.x,  cubeSize.y,  cubeSize.z);
		vertices[ 2].pos = Vec3C( cubeSize.x,  cubeSize.y,  cubeSize.z);
		vertices[ 3].pos = Vec3C( cubeSize.x, -cubeSize.y,  cubeSize.z);
		finalizeFace( 0, vecOffset, Vec3C( 0.0f,  0.0f,  1.0f), color);

		// Back
		vertices[ 4].pos = Vec3C(-cubeSize.x, -cubeSize.y, -cubeSize.z);
		vertices[ 5].pos = Vec3C(-cubeSize.x,  cubeSize.y, -cubeSize.z);
		vertices[ 6].pos = Vec3C( cubeSize.x,  cubeSize.y, -cubeSize.z);
		vertices[ 7].pos = Vec3C( cubeSize.x, -cubeSize.y, -cubeSize.z);
		finalizeFace( 4, vecOffset, Vec3C( 0.0f,  0.0f, -1.0f), color);

		// Left
		vertices[ 8].pos = Vec3C(-cubeSize.x, -cubeSize.y, -cubeSize.z);
		vertices[ 9].pos = Vec3C(-cubeSize.x, -cubeSize.y,  cubeSize.z);
		vertices[10].pos = Vec3C(-cubeSize.x,  cubeSize.y,  cubeSize.z);
		vertices[11].pos = Vec3C(-cubeSize.x,  cubeSize.y, -cubeSize.z);
		finalizeFace( 8, vecOffset, Vec3C(-1.0f,  0.0f,  0.0f), color);

		// Right
		vertices[12].pos = Vec3C( cubeSize.x, -cubeSize.y, -cubeSize.z);
		vertices[13].pos = Vec3C( cubeSize.x, -cubeSize.y,  cubeSize.z);
		vertices[14].pos = Vec3C( cubeSize.x,  cubeSize.y,  cubeSize.z);
		vertices[15].pos = Vec3C( cubeSize.x,  cubeSize.y, -cubeSize.z);
		finalizeFace(12, vecOffset, Vec3C( 1.0f,  0.0f,  0.0f), color);

		// Top
		vertices[16].pos = Vec3C(-cubeSize.x,  cubeSize.y, -cubeSize.z);
		vertices[17].pos = Vec3C(-cubeSize.x,  cubeSize.y,  cubeSize.z);
		vertices[18].pos = Vec3C( cubeSize.x,  cubeSize.y,  cubeSize.z);
		vertices[19].pos = Vec3C( cubeSize.x,  cubeSize.y, -cubeSize.z);
		finalizeFace(16, vecOffset, Vec3C( 0.0f,  1.0f,  0.0f), color);

		// Bottom
		vertices[20].pos = Vec3C(-cubeSize.x, -cubeSize.y, -cubeSize.z);
		vertices[21].pos = Vec3C(-cubeSize.x, -cubeSize.y,  cubeSize.z);
		vertices[22].pos = Vec3C( cubeSize.x, -cubeSize.y,  cubeSize.z);
		vertices[23].pos = Vec3C( cubeSize.x, -cubeSize.y, -cubeSize.z);
		finalizeFace(20, vecOffset, Vec3C( 0.0f, -1.0f,  0.0f), color);

		// Indices
		int vtxOffset;
		int idxOffset;

		// Front
		vtxOffset =  0;
		idxOffset =  0;
		indices[idxOffset+0] = vtxOffset + 0; indices[idxOffset+1] = vtxOffset + 1; indices[idxOffset+2] = vtxOffset + 2;
		indices[idxOffset+3] = vtxOffset + 0; indices[idxOffset+4] = vtxOffset + 2; indices[idxOffset+5] = vtxOffset + 3;

		// Back
		vtxOffset =  4;
		idxOffset =  6;
		indices[idxOffset+0] = vtxOffset + 0; indices[idxOffset+1] = vtxOffset + 2; indices[idxOffset+2] = vtxOffset + 1;
		indices[idxOffset+3] = vtxOffset + 0; indices[idxOffset+4] = vtxOffset + 3; indices[idxOffset+5] = vtxOffset + 2;

		// Left
		vtxOffset =  8;
		idxOffset = 12;
		indices[idxOffset+0] = vtxOffset + 0; indices[idxOffset+1] = vtxOffset + 2; indices[idxOffset+2] = vtxOffset + 1;
		indices[idxOffset+3] = vtxOffset + 0; indices[idxOffset+4] = vtxOffset + 3; indices[idxOffset+5] = vtxOffset + 2;

		// Right
		vtxOffset = 12;
		idxOffset = 18;
		indices[idxOffset+0] = vtxOffset + 0; indices[idxOffset+1] = vtxOffset + 1; indices[idxOffset+2] = vtxOffset + 2;
		indices[idxOffset+3] = vtxOffset + 0; indices[idxOffset+4] = vtxOffset + 2; indices[idxOffset+5] = vtxOffset + 3;

		// Top
		vtxOffset = 16;
		idxOffset = 24;
		indices[idxOffset+0] = vtxOffset + 0; indices[idxOffset+1] = vtxOffset + 2; indices[idxOffset+2] = vtxOffset + 1;
		indices[idxOffset+3] = vtxOffset + 0; indices[idxOffset+4] = vtxOffset + 3; indices[idxOffset+5] = vtxOffset + 2;

		// Bottom
		vtxOffset = 20;
		idxOffset = 30;
		indices[idxOffset+0] = vtxOffset + 0; indices[idxOffset+1] = vtxOffset + 1; indices[idxOffset+2] = vtxOffset + 2;
		indices[idxOffset+3] = vtxOffset + 0; indices[idxOffset+4] = vtxOffset + 2; indices[idxOffset+5] = vtxOffset + 3;

		m_mesh->updateVertexBuffer(vertices.data());
		m_mesh->updateIndexBuffer(indices.data());
	}
};

int main()
{
	using namespace windows;
	using namespace math;

	Timer perfTimer;

	Window window;
	window.setParameters(800, 600, Window::Kind::eWindowed);
	window.init();

	MSG msg;

	vulkan::Wrapper renderManager;
	renderManager.setDebugCallback(debugCallback);

	App sampleApp;
	sampleApp.setRenderManager(&renderManager);
	sampleApp.init(window.getHWnd(), window.getWidth(), window.getHeight());
	sampleApp.setWindowTitle(L"Vulkan app");

	CallbackData callbackData = {};
	callbackData.app = &sampleApp;
	callbackData.movementFlags = 0;
	callbackData.isActive = true;
	callbackData.isPaused = false;
	getMouseCoordinates(&callbackData.mx, &callbackData.my);
	hideMouse();

	setUserDataPointer(&callbackData);
	setResizeCallback(resizeCallback);
	setChangeFocusCallback(chageFocusCallback);
	setChangeActiveCallback(chageActiveCallback);
	setKeyStateCallback(keyStateCallback);
	setMouseEventCallback(mouseEventCallback);

	Camera mainCamera;
	mainCamera.setPosition(Vec3C(0.0f, 0.0f, 1.7f));

	SphericalMesh sphMesh;
	sphMesh.init(renderManager.createMesh(), 64, 64);

	const Vec3 basisOffset = Vec3C(1.0f, 0.0f, 0.0f);
	const Vec2 basisHalfSize = Vec2C(0.5f, 0.02f);
	CubeMesh basisMesh[3];
	basisMesh[0].init(renderManager.createMesh(), basisOffset+Vec3C(0.5f-basisHalfSize.y, 0.0f, 0.0f), Vec3C(basisHalfSize.x, basisHalfSize.y, basisHalfSize.y), Vec4C(1.0f, 0.0f, 0.0f, 1.0f));
	basisMesh[1].init(renderManager.createMesh(), basisOffset+Vec3C(0.0f, 0.5f-basisHalfSize.y, 0.0f), Vec3C(basisHalfSize.y, basisHalfSize.x, basisHalfSize.y), Vec4C(0.0f, 1.0f, 0.0f, 1.0f));
	basisMesh[2].init(renderManager.createMesh(), basisOffset+Vec3C(0.0f, 0.0f, 0.5f-basisHalfSize.y), Vec3C(basisHalfSize.y, basisHalfSize.y, basisHalfSize.x), Vec4C(0.0f, 0.0f, 1.0f, 1.0f));

	const bool renderUnitCubeMesh = false;
	if (renderUnitCubeMesh)
	{
		CubeMesh unitCubeMesh;
		unitCubeMesh.init(renderManager.createMesh(), Vec3C(.0f, .0f, .0f), Vec3C(.4f, .4f, .4f), Vec4C(1.0f, 1.0f, 1.0f, 1.0f));
	}

	const bool renderShadowMapBiasTestingMeshes = true;
	if (renderShadowMapBiasTestingMeshes)
	{
		CubeMesh shadowTestMesh01;
		shadowTestMesh01.init(renderManager.createMesh(), Vec3C(-2.5f, -0.5f, 0.0f), Vec3C(0.1f, 1.0f, 1.0f), Vec4C(1.0f, 1.0f, 1.0f, 1.0f));
		CubeMesh shadowTestMesh02;
		shadowTestMesh02.init(renderManager.createMesh(), Vec3C( 0.0f, -0.5f, 2.0f), Vec3C(0.05f, 1.0f, 0.05f), Vec4C(1.0f, 1.0f, 1.0f, 1.0f));
	}

	const Vec3 floorSize = Vec3C(10.0f, 0.5f, 10.0f);
	CubeMesh floorMesh;
	floorMesh.init(renderManager.createMesh(), Vec3C(0.0f, -2.0f, 0.0f), floorSize, Vec4C(1.0f, 1.0f, 1.0f, 1.0f));

	// Light setup
	renderManager.setLightPerspProjParams(PI * 0.5f, 1.0f, 0.1f, 20.0f, 1.0f, 1.0f);

	double accumTime = 0.0;
	int accumFrames = 0;
	perfTimer.start();
	double dtMS = 0.0;

	bool isRunning = true;
	while (isRunning)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				isRunning = false;
				break;
			}
		}

		if (sampleApp.getIsExitting())
		{
			isRunning = false;
		}

		if (!isRunning)
		{
			break;
		}

		if (callbackData.isActive)
		{
			int nmx, nmy;
			getMouseCoordinates(&nmx, &nmy);
			setMouseCoordinates(callbackData.mx, callbackData.my);

			float sens = 0.01f;
			float movementSpeed = 0.005f;
			Vec3 posOffset = Vec3C(0.0f, 0.0f, 0.0f);

			const float movementAccelFactor = 5.0f;
			const float movementDeccelFactor = 0.2f;
			if (callbackData.movementFlags & (uint32_t)CallbackData::MovementKindBits::eAccel)
			{
				movementSpeed *= movementAccelFactor;
			}
			if (callbackData.movementFlags & (uint32_t)CallbackData::MovementKindBits::eDeccel)
			{
				movementSpeed *= movementDeccelFactor;
			}
		
			if (callbackData.movementFlags & (uint32_t)CallbackData::MovementKindBits::eForward)
			{
				posOffset.z -= movementSpeed;
			}
			if (callbackData.movementFlags & (uint32_t)CallbackData::MovementKindBits::eBackward)
			{
				posOffset.z += movementSpeed;
			}
			if (callbackData.movementFlags & (uint32_t)CallbackData::MovementKindBits::eLeft)
			{
				posOffset.x -= movementSpeed;
			}
			if (callbackData.movementFlags & (uint32_t)CallbackData::MovementKindBits::eRight)
			{
				posOffset.x += movementSpeed;
			}
			if (callbackData.movementFlags & (uint32_t)CallbackData::MovementKindBits::eDown)
			{
				posOffset.y -= movementSpeed;
			}
			if (callbackData.movementFlags & (uint32_t)CallbackData::MovementKindBits::eUp)
			{
				posOffset.y += movementSpeed;
			}

#if (MOUSE_USE_RAWINPUT == 1)
			int dMouseX = callbackData.dmx;
			int dMouseY = callbackData.dmy;

			callbackData.dmx = 0;
			callbackData.dmy = 0;
#else
			int dMouseX = nmx - callbackData.mx;
			int dMouseY = nmy - callbackData.my;
#endif
			mainCamera.update((float)dtMS * posOffset, sens*dMouseX, sens*dMouseY);
			mainCamera.fillMatrix(&renderManager.getViewMatrix());
		}

		{
			Vec3 meshOBBMin, meshOBBMax;
			sphMesh.getOBB(&meshOBBMin, &meshOBBMax);

			Mat44 meshModelMatrix = sphMesh.getModelMatrix();
			Vec3 meshOBB[8];
			meshOBB[0] = meshModelMatrix * Vec3C(meshOBBMin.x, meshOBBMin.y, meshOBBMin.z);
			meshOBB[1] = meshModelMatrix * Vec3C(meshOBBMax.x, meshOBBMin.y, meshOBBMin.z);
			meshOBB[2] = meshModelMatrix * Vec3C(meshOBBMax.x, meshOBBMax.y, meshOBBMin.z);
			meshOBB[3] = meshModelMatrix * Vec3C(meshOBBMin.x, meshOBBMax.y, meshOBBMin.z);
			meshOBB[4] = meshModelMatrix * Vec3C(meshOBBMin.x, meshOBBMin.y, meshOBBMax.z);
			meshOBB[5] = meshModelMatrix * Vec3C(meshOBBMax.x, meshOBBMin.y, meshOBBMax.z);
			meshOBB[6] = meshModelMatrix * Vec3C(meshOBBMax.x, meshOBBMax.y, meshOBBMax.z);
			meshOBB[7] = meshModelMatrix * Vec3C(meshOBBMin.x, meshOBBMax.y, meshOBBMax.z);

			vulkan::LinePoint lp0, lp1;
			lp0.col = Vec4C(1.0f, 1.0f, 1.0f, 1.0f);
			lp1.col = Vec4C(1.0f, 1.0f, 1.0f, 1.0f);

			auto addLine = [&renderManager, &lp0, &lp1](const Vec3 & pos0, const Vec3 & pos1)
			{
				lp0.pos = pos0;
				lp1.pos = pos1;
				renderManager.m_debugLines.push_back(lp0);
				renderManager.m_debugLines.push_back(lp1);
			};

			{
				addLine(meshOBB[0], meshOBB[1]);
				addLine(meshOBB[1], meshOBB[2]);
				addLine(meshOBB[2], meshOBB[3]);
				addLine(meshOBB[3], meshOBB[0]);

				addLine(meshOBB[4], meshOBB[5]);
				addLine(meshOBB[5], meshOBB[6]);
				addLine(meshOBB[6], meshOBB[7]);
				addLine(meshOBB[7], meshOBB[4]);

				addLine(meshOBB[0], meshOBB[4]);
				addLine(meshOBB[1], meshOBB[5]);
				addLine(meshOBB[2], meshOBB[6]);
				addLine(meshOBB[3], meshOBB[7]);
			}

			int etInt = (int)(2 * sampleApp.getElapsedTime() / 1000.0);
			int etIntMod10 = etInt % 20;
			Vec3 etOffset = Vec3C(0.0f, 2.0f, 0.0f);
			for (int ec = 0; ec < etIntMod10; ++ ec)
			{
				float ang = ec / (float)etIntMod10 * _2PI;
				float inRad = 0.3f;
				float outRad = 0.8f;

				float cosAng = cosf(ang);
				float sinAng = sinf(ang);

				if (ec & 1)
				{
					lp0.col = Vec4C(1.0f, 0.0f, 0.0f, 1.0f);
					lp1.col = Vec4C(0.0f, 1.0f, 0.0f, 1.0f);
				}
				else
				{
					lp0.col = Vec4C(1.0f, 1.0f, 0.0f, 1.0f);
					lp1.col = Vec4C(0.0f, 0.0f, 1.0f, 1.0f);
				}

				addLine(etOffset + inRad * Vec3C(cosAng, sinAng, 0.0f), etOffset + outRad * Vec3C(cosAng, sinAng, 0.0f));
			}

			lp0.col = Vec4C(1.0f, 1.0f, 1.0f, 1.0f);
			lp1.col = Vec4C(1.0f, 1.0f, 1.0f, 1.0f);
		}

		{
			Mat44 lightView, lightProj;

			const Vec3 lightPos = Vec3C(10.0f * sinf((float)sampleApp.getElapsedTime() * 0.001f), 3.0f, 3.0f);
			const Vec3 lightDir = Vec3C(0.0f, 0.0f, 0.0f);

			Vec3 lightRight, lightUp, lightViewDir;
			lightViewDir = -(lightDir - lightPos).normalize();
			lightRight = Vec3C(0.0f, 1.0f, 0.0f).cross(lightViewDir).normalize();
			lightUp = lightViewDir.cross(lightRight).normalize();
			lightView.setBasis0(lightRight);
			lightView.setBasis1(lightUp);
			lightView.setBasis2(lightViewDir);
			lightView.setBasis3(lightPos);
			lightView = lightView.invertRTCopy();

			renderManager.setLightViewMatrix(lightView);

			auto projPerspective = [](float fovRad, float aspect, float zNear, float zFar, float width, float height, Mat44 * projection)
			{
				projection->zero();

				Vec2 scale;
				scale.x = width * 1.0f / tanf(fovRad / 2);
				scale.y = height * aspect * scale.x;

				float zDiff = zNear - zFar;
				float m[] = {
					scale.x, 0, 0, 0,
					0, scale.y, 0, 0,
					0, 0, (zFar + zNear) / zDiff, 2*zFar*zNear / zDiff,
					0, 0, -1, 0 
				};    
				memcpy(projection->m, m, sizeof(float)*16);
			};

			const bool renderLightFrustum = true;
			if (renderLightFrustum)
			{
				float fovRad, aspect, zNear, zFar, width, height;
				renderManager.getLightPerspProjParams(&fovRad, &aspect, &zNear, &zFar, &width, &height);
				projPerspective(fovRad, aspect, zNear, zFar, width, height, &lightProj);

				Vec3 meshOBBMin = Vec3C(-1.0f, -1.0f, -1.0f), meshOBBMax = Vec3C(1.0f, 1.0f, 1.0f);

				Mat44 lightMatrixInv = lightProj * lightView;
				lightMatrixInv.invert();

				Vec3 meshOBB[8];
				meshOBB[0] = lightMatrixInv.homogTransformCopy(Vec3C(meshOBBMin.x, meshOBBMin.y, meshOBBMin.z));
				meshOBB[1] = lightMatrixInv.homogTransformCopy(Vec3C(meshOBBMax.x, meshOBBMin.y, meshOBBMin.z));
				meshOBB[2] = lightMatrixInv.homogTransformCopy(Vec3C(meshOBBMax.x, meshOBBMax.y, meshOBBMin.z));
				meshOBB[3] = lightMatrixInv.homogTransformCopy(Vec3C(meshOBBMin.x, meshOBBMax.y, meshOBBMin.z));
				meshOBB[4] = lightMatrixInv.homogTransformCopy(Vec3C(meshOBBMin.x, meshOBBMin.y, meshOBBMax.z));
				meshOBB[5] = lightMatrixInv.homogTransformCopy(Vec3C(meshOBBMax.x, meshOBBMin.y, meshOBBMax.z));
				meshOBB[6] = lightMatrixInv.homogTransformCopy(Vec3C(meshOBBMax.x, meshOBBMax.y, meshOBBMax.z));
				meshOBB[7] = lightMatrixInv.homogTransformCopy(Vec3C(meshOBBMin.x, meshOBBMax.y, meshOBBMax.z));

				vulkan::LinePoint lp0, lp1;
				lp0.col = Vec4C(1.0f, 1.0f, 0.0f, 1.0f);
				lp1.col = Vec4C(1.0f, 1.0f, 0.0f, 1.0f);

				auto addLine = [&renderManager, &lp0, &lp1](const Vec3 & pos0, const Vec3 & pos1)
				{
					lp0.pos = pos0;
					lp1.pos = pos1;
					renderManager.m_debugLines.push_back(lp0);
					renderManager.m_debugLines.push_back(lp1);
				};

				{
					addLine(meshOBB[0], meshOBB[1]);
					addLine(meshOBB[1], meshOBB[2]);
					addLine(meshOBB[2], meshOBB[3]);
					addLine(meshOBB[3], meshOBB[0]);

					addLine(meshOBB[4], meshOBB[5]);
					addLine(meshOBB[5], meshOBB[6]);
					addLine(meshOBB[6], meshOBB[7]);
					addLine(meshOBB[7], meshOBB[4]);

					addLine(meshOBB[0], meshOBB[4]);
					addLine(meshOBB[1], meshOBB[5]);
					addLine(meshOBB[2], meshOBB[6]);
					addLine(meshOBB[3], meshOBB[7]);
				}
			}
		}

		sampleApp.update(callbackData.isPaused ? 0.0 : dtMS);

		renderManager.beginFrame();

		sphMesh.update(sampleApp.getElapsedTime());
		renderManager.update();

		renderManager.render();

		dtMS = perfTimer.time();
		accumTime += dtMS;
		++accumFrames;
		if (accumTime > 500.0)
		{
			sampleApp.setDTime(accumTime / (float)accumFrames);
			accumTime = 0.0;
			accumFrames = 0;
		}
		perfTimer.start();
	}

	sampleApp.deinit();

	window.deinit();

	return 0;
}