#include "EngineContext.h"
#include "Application.h"
#include "Input.h"
#include "Time.h"
#include "Window.h"

namespace Matcha
{
EngineContext::EngineContext(Application& application,
                             Input& input,
                             Time& time,
                             Window& window,
                             Renderer& renderer,
                             ResourceManager& resourceManager,
                             Scene& scene)
    : m_Application(application),
      m_Input(input),
      m_Time(time),
      m_Window(window),
      m_Renderer(renderer),
      m_ResourceManager(resourceManager),
      m_Scene(scene)
{
}
}  // namespace Matcha
