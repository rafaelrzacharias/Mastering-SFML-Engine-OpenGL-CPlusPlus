#include "MapControls.h"
#include "SharedContext.h"
#include "Map.h"
#include "EventManager.h"
#include "StateTypes.h"
#include "Window.h"
#include "GUI_Manager.h"
#include "Entity_Manager.h"
#include "ParticleSystem.h"
#include "StateManager.h"
#include "State_Loading.h"
#include "State_MapEditor.h"
#include "VerticalDropDown.h"

MapControls::MapControls(Window* l_window, EventManager* l_eventManager, StateManager* l_stateManager,
	Map* l_map, GUI_Manager* l_gui, EntityManager* l_entityMgr, ParticleSystem* l_particles, sf::View& l_view):
	/* Storing arguments first. */
	m_window(l_window), m_eventManager(l_eventManager), m_stateManager(l_stateManager), m_map(l_map),
	m_guiManager(l_gui), m_entityManager(l_entityMgr), m_particleSystem(l_particles), m_view(l_view),
	/* Setting up initial data member values. */
	m_mode(ControlMode::Pan), m_action(false), m_secondaryAction(false), m_rightClickPan(false),
	m_zoom(1.f), m_brush(sf::Vector2u(1, 1), *l_map->GetTileSet()), m_drawSelectedLayers(false),
	/* Initializing other interface classes. */
	m_tileSelector(l_eventManager, l_gui, l_gui->GetContext()->m_textureManager), m_mapSettings(nullptr),
	m_selectionOptions(l_eventManager, l_gui, this, &m_tileSelector, l_map, &m_brush, l_entityMgr, l_particles)
{
	auto state = StateType::MapEditor;
	m_eventManager->AddCallback(state, "Mouse_Left", &MapControls::MouseClick, this);
	m_eventManager->AddCallback(state, "Mouse_Left_Release", &MapControls::MouseRelease, this);
	m_eventManager->AddCallback(state, "Mouse_Right", &MapControls::MouseClick, this);
	m_eventManager->AddCallback(state, "Mouse_Right_Release", &MapControls::MouseRelease, this);
	m_eventManager->AddCallback(state, "Mouse_Wheel", &MapControls::MouseWheel, this);
	m_eventManager->AddCallback(state, "MapEditor_NewMapCreate", &MapControls::NewMapCreate, this);
	m_eventManager->AddCallback(state, "MapEditor_NewMapClose", &MapControls::NewMapClose, this);
	m_eventManager->AddCallback(state, "MapEditor_PanTool", &MapControls::ToolSelect, this);
	m_eventManager->AddCallback(state, "MapEditor_BrushTool", &MapControls::ToolSelect, this);
	m_eventManager->AddCallback(state, "MapEditor_PaintTool", &MapControls::ToolSelect, this);
	m_eventManager->AddCallback(state, "MapEditor_EraserTool", &MapControls::ToolSelect, this);
	m_eventManager->AddCallback(state, "MapEditor_SelectTool", &MapControls::ToolSelect, this);
	m_eventManager->AddCallback(state, "MapEditor_Delete", &MapControls::DeleteTiles, this);

	m_guiManager->LoadInterface("MapEditorTools.interface", "MapEditorTools");
	m_guiManager->GetInterface("MapEditorTools")->SetPosition({ 0.f, 16.f });
	m_guiManager->LoadInterface("MapEditorNewMap.interface", "MapEditorNewMap");
	m_mapSettings = m_guiManager->GetInterface("MapEditorNewMap");
	m_mapSettings->PositionCenterScreen();
	m_mapSettings->SetActive(false);

	m_brush.SetTile(0, 0, 0, 0);
	m_brushDrawable.setFillColor({ 255, 255, 255, 200 });
	m_brushDrawable.setOutlineColor({ 255, 0, 0, 255 });
	m_brushDrawable.setOutlineThickness(-1.f);
	m_mapBoundaries.setPosition({ 0.f, 0.f });
	m_mapBoundaries.setFillColor({ 0,0,0,0 });
	m_mapBoundaries.setOutlineColor({255, 50, 50, 255});
	m_mapBoundaries.setOutlineThickness(-1.f);

	auto dropdown = static_cast<GUI_DropDownMenu*>(m_mapSettings->GetElement("SheetDropdown"))->GetMenu();
	dropdown->PurgeEntries();
	auto names = Utils::GetFileList(Utils::GetWorkingDirectory() + "media/Tilesheets/", "*.tilesheet");
	for (auto& entity : names) { dropdown->AddEntry(entity.first.substr(0, entity.first.find(".tilesheet"))); }
	dropdown->Redraw();
}

MapControls::~MapControls() {
	auto state = StateType::MapEditor;
	m_eventManager->RemoveCallback(state, "Mouse_Left");
	m_eventManager->RemoveCallback(state, "Mouse_Left_Release");
	m_eventManager->RemoveCallback(state, "Mouse_Right");
	m_eventManager->RemoveCallback(state, "Mouse_Right_Release");
	m_eventManager->RemoveCallback(state, "Mouse_Wheel");
	m_eventManager->RemoveCallback(state, "MapEditor_NewMapCreate");
	m_eventManager->RemoveCallback(state, "MapEditor_NewMapClose");
	m_eventManager->RemoveCallback(state, "MapEditor_PanTool");
	m_eventManager->RemoveCallback(state, "MapEditor_BrushTool");
	m_eventManager->RemoveCallback(state, "MapEditor_PaintTool");
	m_eventManager->RemoveCallback(state, "MapEditor_EraserTool");
	m_eventManager->RemoveCallback(state, "MapEditor_SelectTool");
	m_eventManager->RemoveCallback(state, "MapEditor_Delete");

	m_guiManager->RemoveInterface(state, "MapEditorTools");
	m_guiManager->RemoveInterface(state, "MapEditorNewMap");
}

void MapControls::Update(float l_dT) {
	m_mapBoundaries.setSize(sf::Vector2f(m_map->GetTileMap()->GetMapSize() * static_cast<unsigned int>(Sheet::Tile_Size)));
	UpdateMouse();
	if (m_mode == ControlMode::Pan || m_rightClickPan) { PanUpdate(); }
	else if (m_mode == ControlMode::Brush) { BrushUpdate(); }
	else if (m_mode == ControlMode::Bucket) { BucketUpdate(); }
	else if (m_mode == ControlMode::Eraser) { EraserUpdate(); }
	else if (m_mode == ControlMode::Select) { SelectionUpdate(); }
}

void MapControls::Draw(sf::RenderWindow* l_window) {
	l_window->draw(m_mapBoundaries);
	if (m_mode == ControlMode::Brush) {
		l_window->draw(m_brushDrawable);
	}
	m_selectionOptions.Draw(l_window);
}

void MapControls::PanUpdate() {
	if (!m_action && !m_rightClickPan) { return; }
	if (m_mouseDifference == sf::Vector2f(0.f, 0.f)) { return; }
	m_view.setCenter(m_view.getCenter() + (sf::Vector2f(0.f, 0.f) - sf::Vector2f(m_mouseDifference)));
}

void MapControls::BrushUpdate() {
	auto tilePos = sf::Vector2f(
		static_cast<float>(m_mouseTilePosition.x * Sheet::Tile_Size),
		static_cast<float>(m_mouseTilePosition.y * Sheet::Tile_Size)
	);
	m_brushDrawable.setPosition(tilePos);
	PlaceBrushTiles();
}

void MapControls::BucketUpdate() { /* IMPLEMENT */ }
void MapControls::EraserUpdate() { /* IMPLEMENT */ }

void MapControls::SelectionUpdate() { m_selectionOptions.Update(); }

void MapControls::RedrawBrush() {
	auto brushSize = m_brush.GetMapSize();
	auto brushRealSize = brushSize * static_cast<unsigned int>(Sheet::Tile_Size);
	auto textureSize = m_brushTexture.getSize();
	if (brushRealSize.x != textureSize.x || brushRealSize.y != textureSize.y) {
		if (!m_brushTexture.create(brushRealSize.x, brushRealSize.y)) { std::cout << "Failed creating brush texture!" << std::endl; }
	}

	m_brushTexture.clear({ 0, 0, 0, 0 });

	for (auto x = 0; x < brushSize.x; ++x) {
		for (auto y = 0; y < brushSize.y; ++y) {
			for (auto layer = 0; layer < Sheet::Num_Layers; ++layer) {
				auto tile = m_brush.GetTile(x, y, layer);
				if (!tile) { continue; }
				auto info = tile->m_properties;
				if (!info) { continue; }
				info->m_sprite.setPosition(sf::Vector2f(x * Sheet::Tile_Size, y * Sheet::Tile_Size));
				m_brushTexture.draw(info->m_sprite);
			}
		}
	}

	m_brushTexture.display();
	m_brushDrawable.setTexture(&m_brushTexture.getTexture());
	m_brushDrawable.setSize(sf::Vector2f(brushRealSize));
	m_brushDrawable.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(brushRealSize)));
}

void MapControls::PlaceBrushTiles() {
	if (!m_action || !m_secondaryAction) { return; }
	m_map->GetTileMap()->PlotTileMap(m_brush, m_mouseTilePosition, m_selectionOptions.GetLowestLayer());
	auto size = m_brush.GetMapSize();
	auto from = sf::Vector3i(m_mouseTilePosition.x, m_mouseTilePosition.y, m_selectionOptions.GetLowestLayer());
	auto to = sf::Vector3i(m_mouseTilePosition.x + size.x - 1, m_mouseTilePosition.y + size.y - 1, m_selectionOptions.GetHighestLayer());
	m_map->Redraw(from, to);
	m_secondaryAction = false; // Set it to false in order to avoid multiple placements.
}

void MapControls::MouseClick(EventDetails* l_details) {
	if (l_details->m_hasBeenProcessed) { return; }
	if (l_details->m_keyCode != static_cast<int>(MouseButtonType::Left)) { m_rightClickPan = true; return; }
	m_mousePosition = m_eventManager->GetMousePos(m_window->GetRenderWindow());
	m_mouseStartPosition = m_mousePosition;

	auto view = m_window->GetRenderWindow()->getView();
	auto viewPos = view.getCenter() - (view.getSize() * 0.5f);
	auto mouseGlobal = viewPos + (sf::Vector2f(m_mousePosition) * m_zoom);
	m_mouseTileStartPosition = sf::Vector2i(
		floor(mouseGlobal.x / Sheet::Tile_Size),
		floor(mouseGlobal.y / Sheet::Tile_Size)
	);

	if (!m_selectionOptions.MouseClick(mouseGlobal)) { return; }

	m_action = true;
	m_secondaryAction = true;
}

void MapControls::MouseRelease(EventDetails* l_details) {
	if (l_details->m_keyCode != static_cast<int>(MouseButtonType::Left)) { m_rightClickPan = false; return; }
	m_action = false;
	m_secondaryAction = false;
	m_selectionOptions.MouseRelease();
}

void MapControls::MouseWheel(EventDetails* l_details) {
	if (l_details->m_hasBeenProcessed) { return; }
	float factor = 0.05f;
	factor *= l_details->m_mouseWheelDelta;
	factor = 1.f - factor;
	m_view.zoom(factor);
	m_zoom *= factor;
}

void MapControls::ToolSelect(EventDetails* l_details) {
	auto mode = ControlMode::None;
	if (l_details->m_name == "MapEditor_PanTool") { mode = ControlMode::Pan; }
	else if (l_details->m_name == "MapEditor_BrushTool") { mode = ControlMode::Brush; }
	else if (l_details->m_name == "MapEditor_PaintTool") { mode = ControlMode::Bucket; }
	else if (l_details->m_name == "MapEditor_EraserTool") { mode = ControlMode::Eraser; }
	else if (l_details->m_name == "MapEditor_SelectTool") { mode = ControlMode::Select; }
	SelectMode(mode);
}

void MapControls::DeleteTiles(EventDetails* l_details) {
	if (m_mode != ControlMode::Select) { return; }
	m_selectionOptions.RemoveSelection(l_details);
}

void MapControls::NewMapCreate(EventDetails* l_details) {
	auto s_x = m_mapSettings->GetElement("Size_X")->GetText();
	auto s_y = m_mapSettings->GetElement("Size_Y")->GetText();
	auto friction = m_mapSettings->GetElement("Friction")->GetText();
	auto selection = static_cast<GUI_DropDownMenu*>(m_mapSettings->GetElement("SheetDropdown"))->GetMenu()->GetSelected();
	if (selection.empty()) { return; }

	auto context = m_guiManager->GetContext();
	auto editorState = m_stateManager->GetState<State_MapEditor>(StateType::MapEditor);
	m_particleSystem->PurgeCurrentState();
	m_map->PurgeMap();
	editorState->ResetSavePath();

	m_map->GetTileMap()->SetMapSize(sf::Vector2u(std::stoi(s_x), std::stoi(s_y)));
	m_map->GetDefaultTile()->m_friction = sf::Vector2f(std::stof(friction), std::stof(friction));
	m_map->GetTileSet()->ResetWorker();
	m_map->GetTileSet()->AddFile(Utils::GetWorkingDirectory() + "media/Tilesheets/" + selection + ".tilesheet");
	m_map->GetTileSet()->SetName(selection + ".tilesheet");

	auto loading = m_stateManager->GetState<State_Loading>(StateType::Loading);
	loading->AddLoader(context->m_gameMap->GetTileSet());
	loading->SetManualContinue(false);
	editorState->SetMapRedraw(true);
	m_mapSettings->SetActive(false);
	m_stateManager->SwitchTo(StateType::Loading);
}

void MapControls::NewMapClose(EventDetails* l_details) { m_mapSettings->SetActive(false); }

void MapControls::SelectMode(ControlMode l_mode) {
	ResetTools();
	m_mode = l_mode;
	if (m_mode == ControlMode::Brush) { RedrawBrush(); }
	m_selectionOptions.SetControlMode(m_mode);
}

void MapControls::UpdateMouse() {
	auto mousePos = m_eventManager->GetMousePos(m_window->GetRenderWindow());
	m_mouseDifference = sf::Vector2f(mousePos - m_mousePosition);
	m_mouseDifference *= m_zoom;
	m_mousePosition = mousePos;
	auto view = m_window->GetRenderWindow()->getView();
	auto viewPos = view.getCenter() - (view.getSize() * 0.5f);
	auto mouseGlobal = viewPos + (sf::Vector2f(m_mousePosition) * m_zoom);
	auto newPosition = sf::Vector2i(
		floor(mouseGlobal.x / Sheet::Tile_Size),
		floor(mouseGlobal.y / Sheet::Tile_Size)
	);

	if (m_mouseTilePosition != newPosition && m_action) { m_secondaryAction = true; }
	m_mouseTilePosition = newPosition;
}

void MapControls::ResetTools() {
	auto defaultVector = sf::Vector2i(-1, -1);
	m_mouseTilePosition = defaultVector;
	m_mouseTileStartPosition = defaultVector;
	m_selectionOptions.Reset();
	m_tileSelector.Hide();
}

void MapControls::ResetZoom() {
	m_view.zoom(1.f / m_zoom);
	m_zoom = 1.f;
}

void MapControls::NewMap() { m_mapSettings->SetActive(true); }
void MapControls::SetTileSheetTexture(const std::string& l_sheet) { m_tileSelector.SetSheetTexture(l_sheet); }
ControlMode MapControls::GetMode() const { return m_mode; }
bool MapControls::IsInAction() const { return m_action; }
bool MapControls::IsInSecondary() const { return m_secondaryAction; }
GUI_SelectionOptions* MapControls::GetSelectionOptions() { return &m_selectionOptions; }
sf::Vector2i MapControls::GetMouseTileStart() const { return m_mouseTileStartPosition; }
sf::Vector2i MapControls::GetMouseTile() const { return m_mouseTilePosition; }
sf::Vector2f MapControls::GetMouseDifference() const { return m_mouseDifference; }
bool MapControls::DrawSelectedLayers() const { return m_drawSelectedLayers; }
void MapControls::ToggleDrawingSelectedLayers() { m_drawSelectedLayers = !m_drawSelectedLayers; }