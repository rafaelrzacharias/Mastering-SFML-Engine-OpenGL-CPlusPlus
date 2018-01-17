#include "Game.h"
#include "State_Intro.h"
#include "State_MainMenu.h"
#include "State_MapEditor.h"
#include "State_Game.h"
#include "C_Position.h"
#include "C_SpriteSheet.h"
#include "C_State.h"
#include "C_Movable.h"
#include "C_Controller.h"
#include "C_Collidable.h"
#include "C_SoundEmitter.h"
#include "C_SoundListener.h"
#include "S_Renderer.h"
#include "S_Movement.h"
#include "S_Collision.h"
#include "S_Control.h"
#include "S_State.h"
#include "S_SheetAnimation.h"
#include "S_Sound.h"

Game::Game() : m_window("Chapter 6", sf::Vector2u(800, 600)),
	m_entityManager(&m_systemManager, &m_textureManager),
	m_guiManager(m_window.GetEventManager(), &m_context),
	m_soundManager(&m_audioManager), m_gameMap(&m_window, &m_entityManager, &m_textureManager)
{
	SetUpClasses();
	SetUpECS();
	SetUpStates();

	m_fontManager.RequireResource("Main");
	m_stateManager->SwitchTo(StateType::Intro);
}

Game::~Game() { 
	m_fontManager.ReleaseResource("Main");
}

sf::Time Game::GetElapsed() { return m_clock.getElapsedTime(); }
void Game::RestartClock() { m_elapsed = m_clock.restart(); }
Window* Game::GetWindow() { return &m_window; }

void Game::Update() {
	m_window.Update();
	m_stateManager->Update(m_elapsed);
	m_guiManager.Update(m_elapsed.asSeconds());
	m_soundManager.Update(m_elapsed.asSeconds());

	GUI_Event guiEvent;
	while (m_context.m_guiManager->PollEvent(guiEvent)) {
		m_window.GetEventManager()->HandleEvent(guiEvent);
	}
}

void Game::Render() {
	m_window.BeginDraw();
	// Render here.
	m_stateManager->Draw();
	m_guiManager.Render(m_window.GetRenderWindow());
	m_window.EndDraw();
}

void Game::LateUpdate() {
	m_stateManager->ProcessRequests();
	RestartClock();
}

void Game::SetUpClasses() {
	m_clock.restart();
	m_context.m_rand = &m_rand;
	srand(static_cast<unsigned int>(time(nullptr)));
	m_systemManager.SetEntityManager(&m_entityManager);

	m_context.m_wind = &m_window;
	m_context.m_eventManager = m_window.GetEventManager();
	m_context.m_textureManager = &m_textureManager;
	m_context.m_fontManager = &m_fontManager;
	m_context.m_audioManager = &m_audioManager;
	m_context.m_soundManager = &m_soundManager;
	m_context.m_gameMap = &m_gameMap;
	m_context.m_systemManager = &m_systemManager;
	m_context.m_entityManager = &m_entityManager;
	m_context.m_guiManager = &m_guiManager;

	m_stateManager = std::make_unique<StateManager>(&m_context);
	m_gameMap.SetStateManager(m_stateManager.get());
	m_particles = std::make_unique<ParticleSystem>(m_stateManager.get(), &m_textureManager, &m_rand, &m_gameMap);
	m_context.m_particles = m_particles.get();
	m_gameMap.AddLoadee(m_particles.get());
}

void Game::SetUpECS() {
	m_entityManager.AddComponentType<C_Position>(Component::Position);
	m_entityManager.AddComponentType<C_SpriteSheet>(Component::SpriteSheet);
	m_entityManager.AddComponentType<C_State>(Component::State);
	m_entityManager.AddComponentType<C_Movable>(Component::Movable);
	m_entityManager.AddComponentType<C_Controller>(Component::Controller);
	m_entityManager.AddComponentType<C_Collidable>(Component::Collidable);
	m_entityManager.AddComponentType<C_SoundEmitter>(Component::SoundEmitter);
	m_entityManager.AddComponentType<C_SoundListener>(Component::SoundListener);

	m_systemManager.AddSystem<S_State>(System::State);
	m_systemManager.AddSystem<S_Control>(System::Control);
	m_systemManager.AddSystem<S_Movement>(System::Movement);
	m_systemManager.AddSystem<S_Collision>(System::Collision);
	m_systemManager.AddSystem<S_SheetAnimation>(System::SheetAnimation);
	m_systemManager.AddSystem<S_Sound>(System::Sound);
	m_systemManager.AddSystem<S_Renderer>(System::Renderer);

	m_systemManager.GetSystem<S_Collision>(System::Collision)->SetMap(&m_gameMap);
	m_systemManager.GetSystem<S_Movement>(System::Movement)->SetMap(&m_gameMap);
	m_systemManager.GetSystem<S_Sound>(System::Sound)->SetUp(&m_audioManager, &m_soundManager);
}

void Game::SetUpStates() {
	m_stateManager->AddDependent(m_context.m_eventManager);
	m_stateManager->AddDependent(&m_guiManager);
	m_stateManager->AddDependent(&m_soundManager);
	m_stateManager->AddDependent(m_particles.get());
	m_stateManager->RegisterState<State_Intro>(StateType::Intro);
	m_stateManager->RegisterState<State_MainMenu>(StateType::MainMenu);
	m_stateManager->RegisterState<State_MapEditor>(StateType::MapEditor);
	m_stateManager->RegisterState<State_Game>(StateType::Game);
}