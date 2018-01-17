#include "SelectionOptions.h"
#include "EventManager.h"
#include "MapControls.h"
#include "Map.h"
#include "StateTypes.h"
#include "GUI_Manager.h"
#include "Emitter.h"
#include "ParticleSystem.h"
#include "C_Position.h"
#include "C_Collidable.h"
#include "C_SpriteSheet.h"

GUI_SelectionOptions::GUI_SelectionOptions(EventManager* l_eventManager, GUI_Manager* l_guiManager,
	MapControls* l_controls, GUI_MapTileSelector* l_selector, Map* l_map, TileMap* l_brush,
	EntityManager* l_entityMgr, ParticleSystem* l_particles) :
	/* Processing arguments. */
	m_eventManager(l_eventManager), m_guiManager(l_guiManager), m_mapControls(l_controls),
	m_tileSelector(l_selector), m_map(l_map), m_brush(l_brush), m_entityManager(l_entityMgr), m_particleSystem(l_particles),
	/* Initializing default values of data members. */
	m_selectRangeX(-1, -1), m_selectRangeY(-1, -1), m_layerSelectLow(0), m_layerSelectHigh(0),
	m_selectMode(SelectMode::Tiles), m_entityId(-1), m_entity(nullptr), m_emitter(nullptr), m_selectUpdate(true)
{
	auto state = StateType::MapEditor;
	m_eventManager->AddCallback(state, "MapEditor_SwitchSelectMode", &GUI_SelectionOptions::SelectModeSwitch, this);
	m_eventManager->AddCallback(state, "MapEditor_OpenTileSelect", &GUI_SelectionOptions::OpenTileSelection, this);
	m_eventManager->AddCallback(state, "MapEditor_SolidToggle", &GUI_SelectionOptions::SolidToggle, this);
	m_eventManager->AddCallback(state, "MapEditor_CopySelection", &GUI_SelectionOptions::CopySelection, this);
	m_eventManager->AddCallback(state, "MapEditor_PlaceSelection", &GUI_SelectionOptions::PlaceSelection, this);
	m_eventManager->AddCallback(state, "MapEditor_RemoveSelection", &GUI_SelectionOptions::RemoveSelection, this);
	m_eventManager->AddCallback(state, "MapEditor_ToggleLayers", &GUI_SelectionOptions::ToggleLayers, this);
	m_eventManager->AddCallback(state, "MapEditor_SelectOptionsPlus", &GUI_SelectionOptions::SelectionOptionsElevation, this);
	m_eventManager->AddCallback(state, "MapEditor_SelectOptionsMinus", &GUI_SelectionOptions::SelectionOptionsElevation, this);

	m_guiManager->LoadInterface("MapEditorSelectionOptions.interface", "MapEditorSelectionOptions");
	m_selectionOptions = m_guiManager->GetInterface("MapEditorSelectionOptions");
	m_selectionOptions->SetPosition({ 0.f, 164.f });
	m_selectionOptions->SetActive(false);

	m_selectStartColor = sf::Color(0, 0, 150, 120);
	m_selectEndColor = sf::Color(0, 0, 255, 150);
	m_entityColor = sf::Color(255, 0, 0, 150);
	m_emitterColor = sf::Color(0, 255, 0, 150);

	m_entityNames = Utils::GetFileList(Utils::GetWorkingDirectory() + "media/Entities/", "*.entity");
	m_emitterNames = Utils::GetFileList(Utils::GetWorkingDirectory() + "media/Particles/", "*.particle");
}

GUI_SelectionOptions::~GUI_SelectionOptions() {
	auto state = StateType::MapEditor;
	m_eventManager->RemoveCallback(state, "MapEditor_SwitchSelectMode");
	m_eventManager->RemoveCallback(state, "MapEditor_OpenTileSelect");
	m_eventManager->RemoveCallback(state, "MapEditor_SolidToggle");
	m_eventManager->RemoveCallback(state, "MapEditor_ToggleLayers");
	m_eventManager->RemoveCallback(state, "MapEditor_CopySelection");
	m_eventManager->RemoveCallback(state, "MapEditor_PlaceSelection");
	m_eventManager->RemoveCallback(state, "MapEditor_RemoveSelection");
	m_eventManager->RemoveCallback(state, "MapEditor_SelectOptionsPlus");
	m_eventManager->RemoveCallback(state, "MapEditor_SelectOptionsMinus");

	m_guiManager->RemoveInterface(state, "MapEditorSelectionOptions");
}

void GUI_SelectionOptions::Show() { m_selectionOptions->SetActive(true); m_guiManager->BringToFront(m_selectionOptions); }
void GUI_SelectionOptions::Hide() { m_selectionOptions->SetActive(false); }

void GUI_SelectionOptions::SetControlMode(ControlMode l_mode) {
	if (l_mode != ControlMode::Brush && l_mode != ControlMode::Select) { return; }
	SetSelectMode(SelectMode::Tiles);
	if (l_mode == ControlMode::Brush) {
		m_selectionOptions->SetActive(true);
		m_selectionOptions->Focus();
		m_selectionOptions->GetElement("TileSelect")->SetActive(true);
	} else if (l_mode == ControlMode::Select) {
		m_selectionOptions->SetActive(true);
		m_selectionOptions->Focus();
		m_selectionOptions->GetElement("SolidToggle")->SetActive(true);
		m_selectionOptions->GetElement("CopySelection")->SetActive(true);
	}
}

void GUI_SelectionOptions::SetSelectMode(SelectMode l_mode) {
	Reset();
	m_selectMode = l_mode;
	m_selectionOptions->SetActive(true);
	m_selectionOptions->Focus();

	if (l_mode == SelectMode::Tiles) {
		m_selectionOptions->GetElement("SelectMode")->SetText("Tiles");
		m_selectionOptions->GetElement("InfoLabel")->SetText("Tile(s):");
		m_selectionOptions->GetElement("InfoText")->SetText("0");
		m_selectionOptions->GetElement("TileSelect")->SetActive(true);
		m_selectionOptions->GetElement("ElevationLabel")->SetActive(true);
		m_selectionOptions->GetElement("ElevationMinus")->SetActive(true);
		m_selectionOptions->GetElement("Elevation")->SetActive(true);
		m_selectionOptions->GetElement("ElevationPlus")->SetActive(true);
		m_selectionOptions->GetElement("SolidLabel")->SetActive(true);
		m_selectionOptions->GetElement("Solidity")->SetActive(true);
		m_selectionOptions->GetElement("SolidToggle")->SetActive(true);
	} else if(l_mode == SelectMode::Entities) {
		m_selectionOptions->GetElement("SelectMode")->SetText("Entities");
		m_selectionOptions->GetElement("InfoLabel")->SetText("Entity:");
		m_selectionOptions->GetElement("InfoText")->SetText("");
		m_selectionOptions->GetElement("SelectDropdown")->SetActive(true);
		m_selectionOptions->GetElement("PositionLabel")->SetActive(true);
		m_selectionOptions->GetElement("Pos_X")->SetActive(true);
		m_selectionOptions->GetElement("Pos_Y")->SetActive(true);
		m_selectionOptions->GetElement("ElevationLabel")->SetActive(true);
		m_selectionOptions->GetElement("ElevationMinus")->SetActive(true);
		m_selectionOptions->GetElement("Elevation")->SetActive(true);
		m_selectionOptions->GetElement("ElevationPlus")->SetActive(true);
		m_selectionOptions->GetElement("PlaceSelection")->SetActive(true);
		m_selectionOptions->GetElement("Pos_X")->SetText("0");
		m_selectionOptions->GetElement("Pos_Y")->SetText("0");
		
		auto dropdown = static_cast<GUI_DropDownMenu*>(m_selectionOptions->GetElement("SelectDropdown"))->GetMenu();
		dropdown->PurgeEntries();
		for (auto& entity : m_entityNames) { dropdown->AddEntry(entity.first.substr(0, entity.first.find(".entity"))); }
		dropdown->Redraw();
	} else if (l_mode == SelectMode::Emitters) {
		m_selectionOptions->GetElement("SelectMode")->SetText("Emitters");
		m_selectionOptions->GetElement("InfoLabel")->SetText("Emitter:");
		m_selectionOptions->GetElement("InfoText")->SetText("");
		m_selectionOptions->GetElement("SelectDropdown")->SetActive(true);
		m_selectionOptions->GetElement("PositionLabel")->SetActive(true);
		m_selectionOptions->GetElement("Pos_X")->SetActive(true);
		m_selectionOptions->GetElement("Pos_Y")->SetActive(true);
		m_selectionOptions->GetElement("Pos_Z")->SetActive(true);
		m_selectionOptions->GetElement("EmitRateLabel")->SetActive(true);
		m_selectionOptions->GetElement("EmitRate")->SetActive(true);
		m_selectionOptions->GetElement("PlaceSelection")->SetActive(true);
		m_selectionOptions->GetElement("Pos_X")->SetText("0");
		m_selectionOptions->GetElement("Pos_Y")->SetText("0");
		m_selectionOptions->GetElement("Pos_Z")->SetText("0");

		auto dropdown = static_cast<GUI_DropDownMenu*>(m_selectionOptions->GetElement("SelectDropdown"))->GetMenu();
		dropdown->PurgeEntries();
		for (auto& emitter : m_emitterNames) { dropdown->AddEntry(emitter.first.substr(0, emitter.first.find(".particle"))); }
		dropdown->Redraw();
	}
}

void GUI_SelectionOptions::SelectEntity(int l_id) {
	if (l_id == -1) {
		m_entityId = -1;
		m_selectionOptions->GetElement("CopySelection")->SetActive(false);
		m_selectionOptions->GetElement("PlaceSelection")->SetText("Place");
		m_selectionOptions->GetElement("RemoveSelection")->SetActive(false);
		m_entity = nullptr;
		return;
	}

	auto pos = m_entityManager->GetComponent<C_Position>(l_id, Component::Position);
	if (!pos) {
		m_entityId = -1;
		m_selectionOptions->GetElement("CopySelection")->SetActive(false);
		m_selectionOptions->GetElement("PlaceSelection")->SetText("Place");
		m_selectionOptions->GetElement("RemoveSelection")->SetActive(false);
		m_entity = nullptr;
		return;
	}

	m_selectionOptions->GetElement("CopySelection")->SetActive(true);
	m_selectionOptions->GetElement("PlaceSelection")->SetText("Edit");
	m_selectionOptions->GetElement("RemoveSelection")->SetActive(true);
	m_entityId = l_id;
	m_entity = pos;
	m_selectionOptions->GetElement("InfoText")->SetText(std::to_string(m_entityId));
	m_selectUpdate = true;
}

void GUI_SelectionOptions::SelectEmitter(Emitter* l_emitter) {
	m_emitter = l_emitter;
	if (!l_emitter) {
		m_selectionOptions->GetElement("CopySelection")->SetActive(false);
		m_selectionOptions->GetElement("PlaceSelection")->SetText("Place");
		m_selectionOptions->GetElement("RemoveSelection")->SetActive(false);
		return;
	}
	m_selectionOptions->GetElement("CopySelection")->SetActive(true);
	m_selectionOptions->GetElement("PlaceSelection")->SetText("Edit");
	m_selectionOptions->GetElement("RemoveSelection")->SetActive(true);
	m_selectionOptions->GetElement("InfoText")->SetText(m_emitter->GetGenerators());
	m_selectionOptions->GetElement("EmitRate")->SetText(std::to_string(m_emitter->GetEmitRate()));
	m_selectUpdate = true;
}

void GUI_SelectionOptions::SelectModeSwitch(EventDetails* l_details) {
	if (m_selectMode == SelectMode::Tiles) {
		if (m_mapControls->GetMode() != ControlMode::Select) { m_mapControls->SelectMode(ControlMode::Select); }
		SetSelectMode(SelectMode::Entities);
	} else if (m_selectMode == SelectMode::Entities) { SetSelectMode(SelectMode::Emitters); }
	else { SetSelectMode(SelectMode::Tiles); }
}

void GUI_SelectionOptions::OpenTileSelection(EventDetails* l_details) {
	if (!m_tileSelector->IsActive()) { m_tileSelector->Show(); return; }
	m_mapControls->SelectMode(ControlMode::Brush);
	if (m_tileSelector->CopySelection(*m_brush)) { m_selectionOptions->GetElement("Solidity")->SetText("False"); m_mapControls->RedrawBrush(); }
	m_selectionOptions->GetElement("InfoText")->SetText(std::to_string(m_brush->GetTileCount()));
}

void GUI_SelectionOptions::SolidToggle(EventDetails* l_details) {
	auto mode = m_mapControls->GetMode();
	if ((m_mapControls->GetMode() != ControlMode::Brush && mode != ControlMode::Select) || m_selectRangeX.x == -1) { return; }
	auto element = m_selectionOptions->GetElement("Solidity");
	auto state = element->GetText();
	bool solid = false;
	std::string newText;
	if (state == "True") { newText = "False"; } else { solid = true; newText = "True"; }
	element->SetText(newText);
	sf::Vector2u start;
	sf::Vector2u finish;
	TileMap* map = nullptr;
	if (mode == ControlMode::Brush) {
		map = m_brush;
		start = sf::Vector2u(0, 0);
		finish = map->GetMapSize() - sf::Vector2u(1, 1);
	} else if (mode == ControlMode::Select) {
		map = m_map->GetTileMap();
		start = sf::Vector2u(m_selectRangeX.x, m_selectRangeY.x);
		finish = sf::Vector2u(m_selectRangeX.y, m_selectRangeY.y);
	}

	for (auto x = start.x; x <= finish.x; ++x) {
		for (auto y = start.y; y <= finish.y; ++y) {
			for (auto layer = m_layerSelectLow; layer <= m_layerSelectHigh; ++layer) {
				auto tile = map->GetTile(x, y, layer);
				if (!tile) { continue; }
				tile->m_solid = solid;
			}
		}
	}
}

void GUI_SelectionOptions::CopySelection(EventDetails* l_details) {
	if (m_selectRangeX.x == -1) { return; }
	auto size = sf::Vector2u(m_selectRangeX.y - m_selectRangeX.x, m_selectRangeY.y - m_selectRangeY.x);
	size.x += 1;
	size.y += 1;
	m_brush->Purge();
	m_brush->SetMapSize(size);
	unsigned int b_x = 0, b_y = 0, b_l = 0;
	bool solid = false, mixed = false;
	unsigned short changes = 0;
	for (auto x = m_selectRangeX.x; x <= m_selectRangeX.y; ++x) {
		for (auto y = m_selectRangeY.x; y <= m_selectRangeY.y; ++y) {
			for (auto layer = m_layerSelectLow; layer <= m_layerSelectHigh; ++layer) {
				auto tile = m_map->GetTile(x, y, layer);
				if (!tile) { ++b_l; continue; }
				auto newTile = m_brush->SetTile(b_x, b_y, b_l, tile->m_properties->m_id);
				if (!newTile) { std::cout << "Failed copying a tile..." << std::endl; ++b_l; continue; }
				if (!mixed) {
					if (tile->m_solid && !solid) { solid = true; ++changes; } else if (solid) { solid = false; ++changes; }
					if (changes >= 2) { mixed = true; }
				}
				*newTile = *tile;
				++b_l;
			}
			b_l = 0;
			++b_y;
		}
		b_y = 0;
		++b_x;
	}
	m_layerSelectHigh = m_layerSelectLow + m_brush->GetHighestElevation();
	if (m_layerSelectHigh >= Sheet::Num_Layers) {
		auto difference = (m_layerSelectHigh - Sheet::Num_Layers) + 1;
		m_layerSelectHigh = Sheet::Num_Layers - 1;
		m_layerSelectLow -= difference;
	}
	SelectionElevationUpdate();
	m_mapControls->SelectMode(ControlMode::Brush);
	m_selectionOptions->GetElement("InfoText")->SetText(std::to_string(m_brush->GetTileCount()));
	m_selectionOptions->GetElement("Solidity")->SetText((mixed ? "Mixed" : (solid ? "True" : "False")));
}

void GUI_SelectionOptions::PlaceSelection(EventDetails* l_details) {
	if (m_selectMode == SelectMode::Tiles) { return; }
	auto dropdownValue = static_cast<GUI_DropDownMenu*>(m_selectionOptions->GetElement("SelectDropdown"))->GetMenu()->GetSelected();
	if (dropdownValue.empty()) { return; }
	if (m_selectMode == SelectMode::Entities) {
		if (!m_entity || m_entityId == -1) {
			// New entity.
			auto id = m_entityManager->AddEntity(dropdownValue);
			if (id == -1) { return; }
			SelectEntity(id);
		}
		SaveOptions(nullptr);
	} else if (m_selectMode == SelectMode::Emitters) {
		if (!m_emitter) {
			// New emitter.
			auto text = m_selectionOptions->GetElement("EmitRate")->GetText();
			auto rate = std::stoi(text);
			auto emitter = m_particleSystem->AddEmitter(sf::Vector3f(0.f, 0.f, 0.f), dropdownValue, rate, StateType::MapEditor);
			SelectEmitter(emitter);
		}
		SaveOptions(nullptr);
	}
}

void GUI_SelectionOptions::RemoveSelection(EventDetails* l_details) {
	DeleteSelection(l_details->m_shiftPressed);
}

void GUI_SelectionOptions::ToggleLayers(EventDetails* l_details) { m_mapControls->ToggleDrawingSelectedLayers(); }

void GUI_SelectionOptions::SelectionOptionsElevation(EventDetails* l_details) {
	int low = 0, high = 0;
	bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
	if (l_details->m_name == "MapEditor_SelectOptionsPlus") {
		if (shift) { high = 1; } else { low = 1; }
	} else if (l_details->m_name == "MapEditor_SelectOptionsMinus") {
		if (shift) { high = -1; } else { low = -1; }
	}

	auto mode = m_mapControls->GetMode();

	if (mode == ControlMode::Brush) {
		if (high != 0) { return; } // only working with low values.
		int l = m_layerSelectLow + low;
		if (l < 0 || l >= Sheet::Num_Layers) { return; }
		if (l + m_brush->GetHighestElevation() >= Sheet::Num_Layers) { return; }
		m_layerSelectLow = l;
		m_layerSelectHigh = l + m_brush->GetHighestElevation();
		SelectionElevationUpdate();
	} else if (mode == ControlMode::Select) {
		int l = m_layerSelectLow + low;
		int h = m_layerSelectHigh + high;
		if (l < 0 || l >= Sheet::Num_Layers) { return; }
		if (h < 0 || h >= Sheet::Num_Layers) { return; }
		if (m_layerSelectLow == m_layerSelectHigh && !shift) { m_layerSelectLow += low; m_layerSelectLow += high; m_layerSelectHigh = m_layerSelectLow; } else { m_layerSelectLow = l; m_layerSelectHigh = h; }
		if (m_layerSelectLow > m_layerSelectHigh) { std::swap(m_layerSelectLow, m_layerSelectHigh); }
		SelectionElevationUpdate();
	}
}

void GUI_SelectionOptions::SaveOptions(EventDetails* l_details) {
	if (m_selectMode == SelectMode::Tiles) { return; }

	auto x = m_selectionOptions->GetElement("Pos_X")->GetText();
	auto y = m_selectionOptions->GetElement("Pos_Y")->GetText();
	auto z = m_selectionOptions->GetElement("Pos_Z")->GetText();

	auto c_x = std::stoi(x);
	auto c_y = std::stoi(y);
	auto c_z = std::stoi(z);

	if (m_selectMode == SelectMode::Entities) {
		if (!m_entity || m_entityId == -1) { return; }
		m_entity->SetPosition(sf::Vector2f(c_x, c_y));
		m_entity->SetElevation(m_layerSelectLow);
	} else if (m_selectMode == SelectMode::Emitters) {
		if (!m_emitter) { return; }
		auto emitRate = m_selectionOptions->GetElement("EmitRate")->GetText();
		auto c_rate = std::stoi(emitRate);
		m_emitter->SetPosition(sf::Vector3f(c_x, c_y, c_z));
		m_emitter->SetEmitRate(c_rate);
	}
}

void GUI_SelectionOptions::SelectionElevationUpdate() {
	if (!m_selectionOptions->IsActive()) { return; }
	m_selectionOptions->GetElement("Elevation")->SetText(std::to_string(m_layerSelectLow) +
		(m_layerSelectLow != m_layerSelectHigh ? " - " + std::to_string(m_layerSelectHigh) : "")
	);
	SaveOptions(nullptr);
}

void GUI_SelectionOptions::UpdateSelectDrawable() {
	if (m_selectMode == SelectMode::Entities) {
		if (m_entityId == -1) { return; }
		if (!m_entity) { return; }
		if (m_entityManager->HasComponent(m_entityId, Component::Collidable)) {
			auto col = m_entityManager->GetComponent<C_Collidable>(m_entityId, Component::Collidable);
			auto primitive = col->GetCollidable();
			m_selectDrawable.setPosition(primitive.left, primitive.top);
			m_selectDrawable.setSize(sf::Vector2f(primitive.width, primitive.height));
		} else if (m_entityManager->HasComponent(m_entityId, Component::SpriteSheet)) {
			auto drawable = m_entityManager->GetComponent<C_SpriteSheet>(m_entityId, Component::SpriteSheet);
			auto pos = drawable->GetSpriteSheet()->GetSpritePosition();
			auto size = drawable->GetSpriteSheet()->GetSpriteSize();
			m_selectDrawable.setPosition(pos);
			m_selectDrawable.setSize(sf::Vector2f(size));
		} else {
			m_selectDrawable.setPosition(m_entity->GetPosition() - sf::Vector2f(16.f, 16.f));
			m_selectDrawable.setSize(sf::Vector2f(32.f, 32.f));
		}
	} else if (m_selectMode == SelectMode::Emitters) {
		if (!m_emitter) { return; }
		auto pos = sf::Vector2f(m_emitter->GetPosition().x, m_emitter->GetPosition().y);
		m_selectDrawable.setPosition(pos - sf::Vector2f(16.f, 16.f));
		m_selectDrawable.setSize(sf::Vector2f(32.f, 32.f));
	}
}

void GUI_SelectionOptions::UpdateTileSelection() {
	auto& tileStart = m_mapControls->GetMouseTileStart();
	auto& mouseTile = m_mapControls->GetMouseTile();

	auto start = sf::Vector2f(
		(tileStart.x + (tileStart.x > mouseTile.x ? 1 : 0)) * Sheet::Tile_Size,
		(tileStart.y + (tileStart.y > mouseTile.y ? 1 : 0)) * Sheet::Tile_Size
	);

	auto end = sf::Vector2f(
		(mouseTile.x + (tileStart.x <= mouseTile.x ? 1 : 0)) * Sheet::Tile_Size,
		(mouseTile.y + (tileStart.y <= mouseTile.y ? 1 : 0)) * Sheet::Tile_Size
	);

	m_selectDrawable.setPosition(
		(start.x <= end.x ? start.x : end.x),
		(start.y <= end.y ? start.y : end.y)
	);

	m_selectDrawable.setFillColor(m_selectStartColor);
	m_selectDrawable.setSize({
		std::abs(end.x - start.x),
		std::abs(end.y - start.y)
	});

	m_selectRangeX = sf::Vector2i(
		std::min(tileStart.x, mouseTile.x),
		std::max(tileStart.x, mouseTile.x)
	);

	m_selectRangeY = sf::Vector2i(
		std::min(tileStart.y, mouseTile.y),
		std::max(tileStart.y, mouseTile.y)
	);
}

void GUI_SelectionOptions::UpdateEntitySelection() {
	if (!m_mapControls->IsInAction()) { return; }
	if (!m_entity) { return; }
	m_entity->MoveBy(m_mapControls->GetMouseDifference());
	auto elevation = m_entity->GetElevation();
	m_selectionOptions->GetElement("Pos_X")->SetText(std::to_string(static_cast<int>(m_entity->GetPosition().x)));
	m_selectionOptions->GetElement("Pos_Y")->SetText(std::to_string(static_cast<int>(m_entity->GetPosition().y)));
	m_selectionOptions->GetElement("Elevation")->SetText(std::to_string(elevation));
	m_layerSelectLow = elevation;
	m_layerSelectHigh = elevation;
	m_selectUpdate = true;
}

void GUI_SelectionOptions::UpdateEmitterSelection() {
	if (!m_mapControls->IsInAction()) { return; }
	if (!m_emitter) { return; }
	auto emitPos = m_emitter->GetPosition();
	auto position = sf::Vector2f(emitPos.x, emitPos.y);
	position += m_mapControls->GetMouseDifference();
	m_emitter->SetPosition({ position.x, position.y, m_emitter->GetPosition().z });
	m_selectionOptions->GetElement("Pos_X")->SetText(std::to_string(static_cast<int>(emitPos.x)));
	m_selectionOptions->GetElement("Pos_Y")->SetText(std::to_string(static_cast<int>(emitPos.y)));
	m_selectionOptions->GetElement("Pos_Z")->SetText(std::to_string(static_cast<int>(emitPos.z)));
	m_selectUpdate = true;
}

void GUI_SelectionOptions::DeleteSelection(bool l_deleteAll) {
	if (m_selectMode == SelectMode::Tiles) {
		if (m_selectRangeX.x == -1) { return; }
		auto layerRange = (l_deleteAll ?
			sf::Vector2u(0, Sheet::Num_Layers - 1) :
			sf::Vector2u(m_layerSelectLow, m_layerSelectHigh));

		m_map->GetTileMap()->RemoveTiles(sf::Vector2u(m_selectRangeX), sf::Vector2u(m_selectRangeY), layerRange);
		m_map->ClearMapTexture(
			sf::Vector3i(m_selectRangeX.x, m_selectRangeY.x, layerRange.x),
			sf::Vector3i(m_selectRangeX.y, m_selectRangeY.y, layerRange.y));
	} else if (m_selectMode == SelectMode::Entities) {
		if (!m_entity || m_entityId == -1) { return; }
		m_entityManager->RemoveEntity(m_entityId);
		SelectEntity(-1);
	} else if (m_selectMode == SelectMode::Emitters) {
		if (!m_emitter) { return; }
		m_particleSystem->RemoveEmitter(m_emitter);
		SelectEmitter(nullptr);
	}
}

void GUI_SelectionOptions::Update() {
	if (m_selectUpdate) { UpdateSelectDrawable(); }
	if (!m_mapControls->IsInAction()) { return; }
	if (m_selectMode == SelectMode::Tiles) { UpdateTileSelection(); }
	else if (m_selectMode == SelectMode::Entities) { UpdateEntitySelection(); }
	else if (m_selectMode == SelectMode::Emitters) { UpdateEmitterSelection(); }
}

void GUI_SelectionOptions::Draw(sf::RenderWindow* l_window) {
	auto mode = m_mapControls->GetMode();
	if (mode == ControlMode::Select) {
		if (m_selectMode == SelectMode::Tiles && m_selectRangeX.x == -1) { return; }
		if (m_selectMode == SelectMode::Entities && !m_entity) { return; }
		if (m_selectMode == SelectMode::Emitters && !m_emitter) { return; }
		l_window->draw(m_selectDrawable);
	}
}

bool GUI_SelectionOptions::MouseClick(const sf::Vector2f& l_pos) {
	if (m_selectMode == SelectMode::Tiles) { return true; }
	bool madeSelection = false;
	if (m_selectMode == SelectMode::Entities) {
		int entity = -1;
		if (m_mapControls->DrawSelectedLayers()) {
			entity = m_entityManager->FindEntityAtPoint(l_pos,
				m_layerSelectLow, m_layerSelectHigh);
		} else {
			entity = m_entityManager->FindEntityAtPoint(l_pos);
		}
		SelectEntity(entity);
		madeSelection = entity != -1;
	} else if (m_selectMode == SelectMode::Emitters) {
		Emitter* emitter = nullptr;
		if (m_mapControls->DrawSelectedLayers()) {
			emitter = m_particleSystem->FindEmitter(l_pos,
				sf::Vector2f(32.f, 32.f), m_layerSelectLow, m_layerSelectHigh);
		} else {
			emitter = m_particleSystem->FindEmitter(l_pos, sf::Vector2f(32.f, 32.f));
		}
		SelectEmitter(emitter);
		madeSelection = emitter != nullptr;
	}
	if (!madeSelection) {
		m_selectionOptions->GetElement("Pos_X")->SetText(std::to_string(static_cast<int>(l_pos.x)));
		m_selectionOptions->GetElement("Pos_Y")->SetText(std::to_string(static_cast<int>(l_pos.y)));
	}
	return madeSelection;
}

void GUI_SelectionOptions::MouseRelease() {
	if (m_selectMode == SelectMode::Tiles) {
		m_selectDrawable.setFillColor(m_selectEndColor);
		m_selectionOptions->GetElement("CopySelection")->SetActive(true);
		m_selectionOptions->GetElement("RemoveSelection")->SetActive(true);

		bool solid = false, mixed = false;
		unsigned short changes = 0;
		for (auto x = m_selectRangeX.x; x <= m_selectRangeX.y && !mixed; ++x) {
			for (auto y = m_selectRangeY.x; y <= m_selectRangeY.y && !mixed; ++y) {
				for (auto layer = m_layerSelectLow; layer <= m_layerSelectHigh && !mixed; ++layer) {
					auto tile = m_map->GetTile(x, y, layer);
					if (!tile) { continue; }
					if (tile->m_solid && !solid) { solid = true; ++changes; } else if (!tile->m_solid && solid) { solid = false; ++changes; }
					if (changes >= 2) { mixed = true; }
				}
			}
		}
		m_selectionOptions->GetElement("Solidity")->SetText((mixed ? "Mixed" : (solid ? "True" : "False")));
	} else if (m_selectMode == SelectMode::Entities) { m_selectDrawable.setFillColor(m_entityColor); }
	else if (m_selectMode == SelectMode::Emitters) { m_selectDrawable.setFillColor(m_emitterColor); }
}

void GUI_SelectionOptions::Reset() {
	auto defaultVector = sf::Vector2i(-1, -1);
	m_selectRangeX = defaultVector;
	m_selectRangeY = defaultVector;
	m_entityId = -1;
	m_entity = nullptr;
	m_emitter = nullptr;
	static_cast<GUI_DropDownMenu*>(m_selectionOptions->GetElement("SelectDropdown"))->GetMenu()->ResetSelected();
	m_selectionOptions->SetActive(false);
	m_selectionOptions->GetElement("TileSelect")->SetActive(false);
	m_selectionOptions->GetElement("SelectDropdown")->SetActive(false);
	m_selectionOptions->GetElement("PositionLabel")->SetActive(false);
	m_selectionOptions->GetElement("Pos_X")->SetActive(false);
	m_selectionOptions->GetElement("Pos_Y")->SetActive(false);
	m_selectionOptions->GetElement("Pos_Z")->SetActive(false);
	m_selectionOptions->GetElement("ElevationLabel")->SetActive(false);
	m_selectionOptions->GetElement("ElevationMinus")->SetActive(false);
	m_selectionOptions->GetElement("Elevation")->SetActive(false);
	m_selectionOptions->GetElement("ElevationPlus")->SetActive(false);
	m_selectionOptions->GetElement("EmitRateLabel")->SetActive(false);
	m_selectionOptions->GetElement("EmitRate")->SetActive(false);
	m_selectionOptions->GetElement("SolidLabel")->SetActive(false);
	m_selectionOptions->GetElement("Solidity")->SetActive(false);
	m_selectionOptions->GetElement("SolidToggle")->SetActive(false);
	m_selectionOptions->GetElement("CopySelection")->SetActive(false);
	m_selectionOptions->GetElement("PlaceSelection")->SetActive(false);
	m_selectionOptions->GetElement("PlaceSelection")->SetText("Place");
	m_selectionOptions->GetElement("RemoveSelection")->SetActive(false);
}

unsigned int GUI_SelectionOptions::GetLowestLayer() const { return m_layerSelectLow; }
unsigned int GUI_SelectionOptions::GetHighestLayer() const { return m_layerSelectHigh; }

SelectMode GUI_SelectionOptions::GetSelectMode() const { return m_selectMode; }

sf::Vector2i GUI_SelectionOptions::GetSelectXRange() const {
	return sf::Vector2i(std::min(m_selectRangeX.x, m_selectRangeX.y), std::max(m_selectRangeX.x, m_selectRangeX.y));
}

sf::Vector2i GUI_SelectionOptions::GetSelectYRange() const {
	return sf::Vector2i(std::min(m_selectRangeY.x, m_selectRangeY.y), std::max(m_selectRangeY.x, m_selectRangeY.y));
}