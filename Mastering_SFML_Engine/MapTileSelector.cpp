#include "MapTileSelector.h"
#include "GUI_Manager.h"
#include "GUI_Sprite.h"
#include "EventManager.h"
#include "StateTypes.h"
#include "TextureManager.h"
#include "MapDefinitions.h"
#include "TileMap.h"

GUI_MapTileSelector::GUI_MapTileSelector(EventManager* l_eventManager, GUI_Manager* l_guiManager, TextureManager* l_textureManager)
	: m_eventManager(l_eventManager), m_guiManager(l_guiManager), m_textureManager(l_textureManager), m_selected(false)
{
	m_eventManager->AddCallback(StateType::MapEditor, "MapEditor_TileSelectClick", &GUI_MapTileSelector::TileSelect, this);
	m_eventManager->AddCallback(StateType::MapEditor, "MapEditor_TileSelectRelease", &GUI_MapTileSelector::TileSelect, this);
	m_eventManager->AddCallback(StateType::MapEditor, "MapEditor_TileSelectClose", &GUI_MapTileSelector::Close, this);

	m_guiManager->LoadInterface("MapEditorTileSelect.interface", "MapEditorTileSelect");
	m_interface = m_guiManager->GetInterface("MapEditorTileSelect");
	m_interface->SetContentRectSize(sf::Vector2i(m_interface->GetSize() - sf::Vector2f(32.f, 32.f)));
	m_interface->SetContentOffset({ 16.f, 16.f });
	m_interface->PositionCenterScreen();
	m_interface->SetActive(false);

	m_shape.setFillColor({ 0, 0, 150, 150 });
	m_shape.setSize({ Sheet::Tile_Size, Sheet::Tile_Size });
	m_shape.setPosition(0.f, 0.f);
}

GUI_MapTileSelector::~GUI_MapTileSelector() {
	m_eventManager->RemoveCallback(StateType::MapEditor, "MapEditor_TileSelectClick");
	m_eventManager->RemoveCallback(StateType::MapEditor, "MapEditor_TileSelectRelease");
	m_eventManager->RemoveCallback(StateType::MapEditor, "MapEditor_TileSelectClose");
	m_guiManager->RemoveInterface(StateType::MapEditor, "MapEditorTileSelect");
	if (!m_sheetTexture.empty()) { m_textureManager->ReleaseResource(m_sheetTexture); }
}

void GUI_MapTileSelector::SetSheetTexture(const std::string& l_texture) {
	if (!m_sheetTexture.empty()) { m_textureManager->ReleaseResource(m_sheetTexture); }
	m_sheetTexture = l_texture;
	m_textureManager->RequireResource(m_sheetTexture); 
	m_tileMapSprite.setTexture(*m_textureManager->GetResource(m_sheetTexture));
	m_tileMapSprite.setPosition({ 0.f, 0.f });
	auto size = m_tileMapSprite.getTexture()->getSize();
	m_selectorTexture.create(size.x, size.y);
	m_selectorTexture.clear({ 0,0,0,0 });
	m_selectorTexture.draw(m_tileMapSprite);
	m_selectorTexture.display();

	auto element = static_cast<GUI_Sprite*>(m_interface->GetElement("TileSprite"));
	element->SetTexture(m_selectorTexture);
}

void GUI_MapTileSelector::UpdateInterface() {
	m_selectorTexture.clear({ 0,0,0,0 });
	m_selectorTexture.draw(m_tileMapSprite);
	m_selectorTexture.draw(m_shape);
	m_selectorTexture.display();

	m_interface->RequestContentRedraw();
}

bool GUI_MapTileSelector::CopySelection(TileMap& l_tileMap) const {
	if (!m_selected) { return false; }
	l_tileMap.Purge();
	auto TileCoordsStart = m_startCoords / static_cast<unsigned int>(Sheet::Tile_Size);
	auto TileCoordsEnd = m_endCoords / static_cast<unsigned int>(Sheet::Tile_Size);
	auto size = TileCoordsEnd - TileCoordsStart;
	l_tileMap.SetMapSize(size + sf::Vector2u(1,1));

	auto sheetSize = m_textureManager->GetResource(l_tileMap.GetTileSet().GetTextureName())->getSize();
	auto nPerRow = sheetSize.x / Sheet::Tile_Size;

	auto t_x = 0, t_y = 0;
	for (auto x = TileCoordsStart.x; x <= TileCoordsEnd.x; ++x) {
		for (auto y = TileCoordsStart.y; y <= TileCoordsEnd.y; ++y) {
			auto coordinate = (y * nPerRow) + x;
			auto tile = l_tileMap.SetTile(t_x, t_y, 0, coordinate); // Always layer 0.
			if (!tile) { std::cout << "Failed creating tile..." << std::endl; ++t_y; continue; }
			tile->m_solid = false;
			++t_y;
		}
		t_y = 0;
		++t_x;
	}
	return true;
}

void GUI_MapTileSelector::TileSelect(EventDetails* l_details) {
	if (l_details->m_name == "MapEditor_TileSelectClick") {
		m_startCoords = sf::Vector2u(l_details->m_mouse);
		m_endCoords = sf::Vector2u(l_details->m_mouse);
		m_selected = false;
	} else {
		if (l_details->m_mouse.x < 0 || l_details->m_mouse.y < 0) {
			m_endCoords = sf::Vector2u(0, 0);
			return;
		}
		m_endCoords = sf::Vector2u(l_details->m_mouse);
		m_selected = true;
	}

	if (m_startCoords.x > m_endCoords.x) { std::swap(m_startCoords.x, m_endCoords.x); }
	if (m_startCoords.y > m_endCoords.y) { std::swap(m_startCoords.y, m_endCoords.y); }

	auto start = sf::Vector2i(m_startCoords.x / Sheet::Tile_Size, m_startCoords.y / Sheet::Tile_Size);
	start *= static_cast<int>(Sheet::Tile_Size);
	auto end = sf::Vector2i(m_endCoords.x / Sheet::Tile_Size, m_endCoords.y / Sheet::Tile_Size);
	end *= static_cast<int>(Sheet::Tile_Size);

	m_shape.setPosition(sf::Vector2f(start));
	m_shape.setSize(sf::Vector2f(end - start) + sf::Vector2f(Sheet::Tile_Size, Sheet::Tile_Size));
	UpdateInterface();
}

void GUI_MapTileSelector::Close(EventDetails* l_details) { Hide(); }
void GUI_MapTileSelector::Show() {
	m_interface->SetActive(true);
	m_interface->Focus();
}
void GUI_MapTileSelector::Hide() { m_interface->SetActive(false); }
bool GUI_MapTileSelector::IsActive() const { return m_interface->IsActive(); }