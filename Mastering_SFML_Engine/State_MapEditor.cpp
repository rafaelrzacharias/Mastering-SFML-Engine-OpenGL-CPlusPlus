#include "State_MapEditor.h"
#include "StateManager.h"
#include "Window.h"
#include "State_Loading.h"
#include "GUI_Manager.h"
#include "ParticleSystem.h"
#include "System_Manager.h"

State_MapEditor::State_MapEditor(StateManager* l_stateManager)
	: BaseState(l_stateManager), m_files("FileManager", m_stateMgr->GetContext()->m_guiManager, m_stateMgr),
	m_mapControls(m_stateMgr->GetContext()->m_wind, m_stateMgr->GetContext()->m_eventManager, m_stateMgr,
		m_stateMgr->GetContext()->m_gameMap, m_stateMgr->GetContext()->m_guiManager,
		m_stateMgr->GetContext()->m_entityManager, m_stateMgr->GetContext()->m_particles, m_view),
	m_mapRedraw(false)
{}

State_MapEditor::~State_MapEditor() {}

void State_MapEditor::OnCreate() {
	auto context = m_stateMgr->GetContext();
	auto evMgr = context->m_eventManager;
	evMgr->AddCallback("Key_Escape", &State_MapEditor::MainMenu, this);
	evMgr->AddCallback("MapEditor_New", &State_MapEditor::MapEditorNew, this);
	evMgr->AddCallback("MapEditor_Load", &State_MapEditor::MapEditorLoad, this);
	evMgr->AddCallback("MapEditor_Save", &State_MapEditor::MapEditorSave, this);
	evMgr->AddCallback("MapEditor_SaveAs", &State_MapEditor::MapEditorSaveAs, this);
	evMgr->AddCallback("MapEditor_Exit", &State_MapEditor::MapEditorExit, this);

	m_files.SetActionCallback(&State_MapEditor::MapAction, this);
	m_files.SetDirectory(Utils::GetWorkingDirectory() + "media/maps/");
	m_files.Hide();
	context->m_guiManager->LoadInterface("MapEditorTop.interface", "MapEditorTop");
	context->m_guiManager->GetInterface("MapEditorTop")->SetPosition({ 200, 0 });

	context->m_particles->CreateState(StateType::MapEditor);
}

void State_MapEditor::OnDestroy() {
	auto context = m_stateMgr->GetContext();
	auto textureMgr = context->m_textureManager;
	auto evMgr = context->m_eventManager;
	evMgr->RemoveCallback(StateType::MapEditor, "Key_Escape");
	evMgr->RemoveCallback(StateType::MapEditor, "MapEditor_New");
	evMgr->RemoveCallback(StateType::MapEditor, "MapEditor_Load");
	evMgr->RemoveCallback(StateType::MapEditor, "MapEditor_Save");
	evMgr->RemoveCallback(StateType::MapEditor, "MapEditor_SaveAs");
	evMgr->RemoveCallback(StateType::MapEditor, "MapEditor_Exit");
	context->m_gameMap->PurgeMap();
	context->m_gameMap->GetTileMap()->SetMapSize({ 0,0 });
}

void State_MapEditor::Draw() {
	auto context = m_stateMgr->GetContext();
	auto window = context->m_wind->GetRenderWindow();
	auto from = (m_mapControls.DrawSelectedLayers() ? m_mapControls.GetSelectionOptions()->GetLowestLayer() : 0);
	auto to = (m_mapControls.DrawSelectedLayers() ? m_mapControls.GetSelectionOptions()->GetHighestLayer() : Sheet::Num_Layers - 1);
	for (auto i = from; i <= to; ++i) {
		context->m_gameMap->Draw(i);
		context->m_systemManager->Draw(context->m_wind, i);
		context->m_particles->Draw(context->m_wind, i);
	}
	if (!m_mapControls.DrawSelectedLayers()) {
		context->m_particles->Draw(context->m_wind, -1);
	}
	m_mapControls.Draw(window);
}

void State_MapEditor::ResetSavePath() { m_mapSavePath = ""; }
void State_MapEditor::SetMapRedraw(bool l_redraw) { m_mapRedraw = l_redraw; }

void State_MapEditor::MainMenu(EventDetails* l_details) {
	m_stateMgr->SwitchTo(StateType::MainMenu);
}

void State_MapEditor::MapEditorNew(EventDetails* l_details) {
	m_mapControls.NewMap();
}

void State_MapEditor::MapEditorLoad(EventDetails* l_details) {
	m_files.LoadMode();
	m_files.Show();
}

void State_MapEditor::MapEditorSave(EventDetails* l_details) {
	// Just save the file out here, unless it hasn't been saved yet.
	if (m_mapSavePath.empty()) { MapEditorSaveAs(nullptr); return; }
	SaveMap(m_mapSavePath);
}

void State_MapEditor::MapEditorSaveAs(EventDetails* l_details) {
	m_files.SaveMode();
	m_files.Show();
}

void State_MapEditor::MapEditorExit(EventDetails* l_details) {
	m_stateMgr->SwitchTo(StateType::MainMenu);
	m_stateMgr->Remove(StateType::MapEditor);
}

void State_MapEditor::SaveMap(const std::string& l_path) {
	m_stateMgr->GetContext()->m_gameMap->SaveToFile(l_path);
	m_mapSavePath = l_path;
}

void State_MapEditor::LoadMap(const std::string& l_path) {
	auto context = m_stateMgr->GetContext();
	auto loading = m_stateMgr->GetState<State_Loading>(StateType::Loading);
	context->m_particles->PurgeCurrentState();
	context->m_gameMap->PurgeMap();
	context->m_gameMap->ResetWorker();
	context->m_gameMap->GetTileMap()->GetTileSet().ResetWorker();
	context->m_gameMap->AddFile(l_path);
	loading->AddLoader(context->m_gameMap);
	loading->SetManualContinue(false);
	m_mapRedraw = true;
	m_mapSavePath = l_path;
	m_stateMgr->SwitchTo(StateType::Loading);
}

void State_MapEditor::MapAction(const std::string& l_path) {
	if(m_files.IsInSaveMode()) { SaveMap(l_path); }
	else { LoadMap(l_path); }
	m_files.Hide();
}

void State_MapEditor::Update(const sf::Time& l_time) {
	auto context = m_stateMgr->GetContext();
	m_mapControls.Update(l_time.asSeconds());
	context->m_gameMap->Update(l_time.asSeconds());
	context->m_systemManager->Update(l_time.asSeconds());
	context->m_particles->Update(l_time.asSeconds());
}

void State_MapEditor::Activate() {
	if (!m_mapRedraw) { return; }
	auto map = m_stateMgr->GetContext()->m_gameMap;
	map->Redraw();
	m_mapControls.SetTileSheetTexture(map->GetTileSet()->GetTextureName());
	m_mapRedraw = false;
}

void State_MapEditor::Deactivate() {}