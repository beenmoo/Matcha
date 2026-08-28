#pragma once

#include "Core/Application.h"
#include "Core/Assert.h"
#include "Core/EngineContext.h"
#include "Core/Event.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Core/Logger.h"
#include "Core/Time.h"
#include "Core/Window.h"

#include "Graphics/FrameBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderHandles.h"
#include "Graphics/Renderer.h"
#include "Graphics/RendererAPI.h"
#include "Graphics/ResourceManager.h"
#include "Graphics/Shader.h"
#include "Graphics/ShaderDataType.h"
#include "Graphics/Texture.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/VertexArray.h"
#include "Graphics/VertexBuffer.h"

#include "Loader/ModelLoader.h"

#include "Math/Matrix.h"
#include "Math/Quaternion.h"
#include "Math/Transform.h"
#include "Math/Vector.h"

#include "Scene/Entity.h"
#include "Scene/ScriptableEntity.h"
#include "Scene/Scene.h"
#include "Scene/Component/CameraComponent.h"
#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Component/MaterialComponent.h"
#include "Scene/Component/MeshComponent.h"
#include "Scene/Component/TransformComponent.h"
#include "Scene/Component/NativeScriptComponent.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/System/RenderSystem.h"
#include "Scene/System/TransformSystem.h"
#include "Scene/System/ScriptSystem.h"

using namespace Matcha;

// Not included here: Core/EntryPoint.h defines main() and must be included exactly once, only
// by the application's own entry-point translation unit (see Sandbox/Code/main.cpp). Graphics/GL
// backend headers are intentionally excluded too - user code should only ever touch the abstract
// Shader/Texture/VertexArray/etc. interfaces and resource handles, never a concrete GL* type.
