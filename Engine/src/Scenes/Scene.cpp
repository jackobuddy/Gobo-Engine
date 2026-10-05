#include "Scene.h"
#include "SceneManager.h"


bool Scene::GoToScene(const std::string& id)
{
	return activeContext && activeContext->GetScenes().GoTo(id);
}

bool Scene::RestartScene()
{
	return activeContext && activeContext->GetScenes().Restart();
}