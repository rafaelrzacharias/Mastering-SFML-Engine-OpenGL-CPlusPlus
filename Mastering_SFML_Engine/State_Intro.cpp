#include "State_Intro.h"
#include "StateManager.h"
#include "Window.h"
#include "TextureManager.h"
#include "ParticleSystem.h"
#include "State_Loading.h"
#include "SoundManager.h"
#include "FontManager.h"

State_Intro::State_Intro(StateManager* l_stateManager)
	: BaseState(l_stateManager) {}

State_Intro::~State_Intro() {}

void State_Intro::OnCreate() {
	auto context = m_stateMgr->GetContext();
	sf::Vector2u windowSize = context->m_wind->GetRenderWindow()->getSize();

	TextureManager* textureMgr = context->m_textureManager;
	textureMgr->RequireResource("Intro");
	m_introSprite.setTexture(*textureMgr->GetResource("Intro"));
	m_introSprite.setOrigin(textureMgr->GetResource("Intro")->getSize().x / 2.0f,
							textureMgr->GetResource("Intro")->getSize().y / 2.0f);

	m_introSprite.setPosition(windowSize.x / 2.0f, windowSize.y / 2.0f);
 
	m_text.setString(sf::String("Press SPACE to continue"));
	m_text.setCharacterSize(15);
	m_text.setFont(*context->m_fontManager->GetResource("Main"));
	sf::FloatRect textRect = m_text.getLocalBounds();
	m_text.setOrigin(textRect.left + textRect.width / 2.0f,textRect.top + textRect.height / 2.0f);
	m_text.setPosition(m_introSprite.getPosition().x, m_introSprite.getPosition().y + textureMgr->GetResource("Intro")->getSize().y / 1.5f);

	context->m_soundManager->PlayMusic("Electrix", 50.f, true);

	EventManager* evMgr = context->m_eventManager;
	evMgr->AddCallback("Intro_Continue",&State_Intro::Continue,this);
	LoadFiles();
}

void State_Intro::LoadFiles() {
	auto context = m_stateMgr->GetContext();
	std::string dir = "media/Particles/";
	auto fileList = Utils::GetFileList(Utils::GetWorkingDirectory() + dir, "*.particle");
	for (auto& file : fileList) { context->m_particles->AddFile(Utils::GetWorkingDirectory() + (dir + file.first)); }
	auto loading = m_stateMgr->GetState<State_Loading>(StateType::Loading);
	loading->AddLoader(context->m_particles);
	loading->SetManualContinue(false);
}

void State_Intro::OnDestroy() {
	TextureManager* textureMgr = m_stateMgr->GetContext()->m_textureManager;
	textureMgr->ReleaseResource("Intro");

	EventManager* evMgr = m_stateMgr->GetContext()->m_eventManager;
	evMgr->RemoveCallback(StateType::Intro,"Intro_Continue");
}

void State_Intro::Draw() {
	sf::RenderWindow* window = m_stateMgr->GetContext()->m_wind->GetRenderWindow();

	window->draw(m_introSprite);
	window->draw(m_text);
}

void State_Intro::Continue(EventDetails* l_details) {
	m_stateMgr->GetContext()->m_soundManager->StopMusic(StateType::Intro);

	m_stateMgr->SwitchTo(StateType::MainMenu);
	m_stateMgr->Remove(StateType::Intro);
}

void State_Intro::Update(const sf::Time& l_time) {}
void State_Intro::Activate() {}
void State_Intro::Deactivate() {}
