#pragma once
#include "Window.h"
#include "EventManager.h"
#include "StateManager.h"
#include "TextureManager.h"
#include "RandomGenerator.h"
#include "FontManager.h"
#include "AudioManager.h"
#include "SoundManager.h"
#include "Map.h"
#include "System_Manager.h"
#include "Entity_Manager.h"
#include "GUI_Manager.h"
#include "ParticleSystem.h"

class Game{
public:
	Game();
	~Game();

	void Update();
	void Render();
	void LateUpdate();

	sf::Time GetElapsed();

	Window* GetWindow();
private:
	void SetUpClasses();
	void SetUpECS();
	void SetUpStates();
	void RestartClock();
	sf::Clock m_clock;
	sf::Time m_elapsed;
	SharedContext m_context;
	RandomGenerator m_rand;
	Window m_window;
	TextureManager m_textureManager;
	FontManager m_fontManager;
	AudioManager m_audioManager;
	SoundManager m_soundManager;
	GUI_Manager m_guiManager;
	SystemManager m_systemManager;
	EntityManager m_entityManager;
	Map m_gameMap;
	std::unique_ptr<ParticleSystem> m_particles;
	std::unique_ptr<StateManager> m_stateManager;
};