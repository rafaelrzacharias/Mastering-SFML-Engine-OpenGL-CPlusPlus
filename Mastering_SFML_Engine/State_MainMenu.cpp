#include "State_MainMenu.h"
#include "StateManager.h"
#include "Window.h"
#include "GUI_Manager.h"
#include "ParticleSystem.h"
#include "MapDefinitions.h"
#include "State_Loading.h"

State_MainMenu::State_MainMenu(StateManager* l_stateManager)
	: BaseState(l_stateManager) {}

State_MainMenu::~State_MainMenu() {}

void State_MainMenu::OnCreate() {
	auto context = m_stateMgr->GetContext();
	GUI_Manager* gui = context->m_guiManager;
	gui->LoadInterface("MainMenu.interface", "MainMenu");
	gui->GetInterface("MainMenu")->SetPosition(sf::Vector2f(250.f, 168.f));
	EventManager* eMgr = context->m_eventManager;
	eMgr->AddCallback("MainMenu_Play", &State_MainMenu::Play, this);
	eMgr->AddCallback("MainMenu_MapEditor", &State_MainMenu::MapEdit, this);
	eMgr->AddCallback("MainMenu_Quit", &State_MainMenu::Quit, this);

	auto w_size = context->m_wind->GetWindowSize();
	context->m_particles->CreateState(StateType::MainMenu);
	auto emitter = std::make_unique<Emitter>(sf::Vector3f(
		static_cast<float>(w_size.x) / 3.f,
		static_cast<float>(w_size.y) - 64.f,
		33.f));
	emitter->SetEmitRate(25);
	emitter->SetGenerators("MenuFlame");
	context->m_particles->AddEmitter(std::move(emitter));
	emitter = std::make_unique<Emitter>(sf::Vector3f(
		(static_cast<float>(w_size.x) / 3.f) * 2.f,
		static_cast<float>(w_size.y) - 64.f,
		33.f));
	emitter->SetEmitRate(25);
	emitter->SetGenerators("MenuFlame");
	context->m_particles->AddEmitter(std::move(emitter));
	emitter = std::make_unique<Emitter>(sf::Vector3f(
		0.f,
		static_cast<float>(w_size.y) / 2.f,
		0.f));
	emitter->SetEmitRate(60);
	emitter->SetGenerators("MenuSmoke");
	context->m_particles->AddEmitter(std::move(emitter));
	auto f = ForceApplicator(
		sf::Vector3f(static_cast<float>(w_size.x) / 2.f, static_cast<float>(w_size.y) / 2.f, 64.f),
		sf::Vector3f(500.f, 500.f, 500.f), 256.f);
	context->m_particles->AddForce(f);
}

void State_MainMenu::OnDestroy() {
	m_stateMgr->GetContext()->m_guiManager->RemoveInterface(StateType::MainMenu, "MainMenu");
	EventManager* eMgr = m_stateMgr->GetContext()->m_eventManager;
	eMgr->RemoveCallback(StateType::MainMenu, "MainMenu_Play");
	eMgr->RemoveCallback(StateType::MainMenu, "MainMenu_MapEditor");
	eMgr->RemoveCallback(StateType::MainMenu, "MainMenu_Quit");
}

void State_MainMenu::Activate() {
	auto& play = *m_stateMgr->GetContext()->m_guiManager->GetInterface("MainMenu")->GetElement("Play");
	if (m_stateMgr->HasState(StateType::Game)) {
		// Resume
		play.SetText("Resume");
	} else {
		// Play
		play.SetText("Play");
	}
}

void State_MainMenu::Play(EventDetails* l_details) {
		m_stateMgr->SwitchTo(StateType::Game);
}

void State_MainMenu::MapEdit(EventDetails* l_details) {
	m_stateMgr->SwitchTo(StateType::MapEditor);
	m_stateMgr->Remove(StateType::Game);
}

void State_MainMenu::Quit(EventDetails* l_details) { m_stateMgr->GetContext()->m_wind->Close(); }

void State_MainMenu::Draw() {
	auto context = m_stateMgr->GetContext();
	auto particles = m_stateMgr->GetContext()->m_particles;
	for (unsigned int i = 0; i < Sheet::Num_Layers; ++i) {
		particles->Draw(m_stateMgr->GetContext()->m_wind, i);
	}
	particles->Draw(m_stateMgr->GetContext()->m_wind, -1);
}
void State_MainMenu::Update(const sf::Time& l_dT) { m_stateMgr->GetContext()->m_particles->Update(l_dT.asSeconds()); }
void State_MainMenu::Deactivate() {}