#pragma once
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

class GUI_Manager;
class GUI_Interface;
class EventManager;
struct EventDetails;
class TextureManager;
class TileMap;

class GUI_MapTileSelector {
public:
	GUI_MapTileSelector(EventManager* l_eventManager, GUI_Manager* l_guiManager, TextureManager* l_textureManager);
	~GUI_MapTileSelector();

	void Show();
	void Hide();

	bool IsActive() const;

	void SetSheetTexture(const std::string& l_texture);

	void UpdateInterface();

	bool CopySelection(TileMap& l_tileMap) const;

	void TileSelect(EventDetails* l_details);
	void Close(EventDetails* l_details);
private:
	EventManager* m_eventManager;
	GUI_Manager* m_guiManager;
	TextureManager* m_textureManager;

	GUI_Interface* m_interface;

	sf::RenderTexture m_selectorTexture;
	sf::Sprite m_tileMapSprite;
	sf::RectangleShape m_shape;

	std::string m_sheetTexture;

	sf::Vector2u m_startCoords;
	sf::Vector2u m_endCoords;
	bool m_selected;
};