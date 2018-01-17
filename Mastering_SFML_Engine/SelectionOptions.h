#pragma once
#include <string>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "Entity_Manager.h"

class EventManager;
class GUI_MapTileSelector;
class Map;
class TileMap;
class GUI_Interface;
class GUI_Manager;
struct EventDetails;
class MapControls;
enum class ControlMode;
class Emitter;
class EntityManager;
class ParticleSystem;
class C_Position;

enum class SelectMode{ Tiles, Entities, Emitters };
using NameList = std::vector<std::pair<std::string, bool>>;

class GUI_SelectionOptions {
public:
	GUI_SelectionOptions(EventManager* l_eventManager, GUI_Manager* l_guiManager,
		MapControls* l_controls, GUI_MapTileSelector* l_selector, Map* l_map, TileMap* l_brush,
		EntityManager* l_entityMgr, ParticleSystem* l_particles);
	~GUI_SelectionOptions();

	void Show();
	void Hide();

	void SetControlMode(ControlMode l_mode);
	void SetSelectMode(SelectMode l_mode);
	SelectMode GetSelectMode()const;

	void SelectEntity(int l_id);
	void SelectEmitter(Emitter* l_emitter);

	sf::Vector2i GetSelectXRange() const;
	sf::Vector2i GetSelectYRange() const;

	unsigned int GetLowestLayer() const;
	unsigned int GetHighestLayer() const;

	void Update();
	void Draw(sf::RenderWindow* l_window);

	bool MouseClick(const sf::Vector2f& l_pos);
	void MouseRelease();
	void Reset();

	void SelectModeSwitch(EventDetails* l_details);
	void OpenTileSelection(EventDetails* l_details);
	void SolidToggle(EventDetails* l_details);
	void CopySelection(EventDetails* l_details);
	void PlaceSelection(EventDetails* l_details);
	void RemoveSelection(EventDetails* l_details);
	void ToggleLayers(EventDetails* l_details);
	void SelectionOptionsElevation(EventDetails* l_details);
	void SaveOptions(EventDetails* l_details);
private:
	void SelectionElevationUpdate();
	void UpdateSelectDrawable();
	void UpdateTileSelection();
	void UpdateEntitySelection();
	void UpdateEmitterSelection();
	void DeleteSelection(bool l_deleteAll);

	// Selection data.
	SelectMode m_selectMode;
	sf::RectangleShape m_selectDrawable;
	sf::Color m_selectStartColor;
	sf::Color m_selectEndColor;
	sf::Color m_entityColor;
	sf::Color m_emitterColor;
	sf::Vector2i m_selectRangeX;
	sf::Vector2i m_selectRangeY;
	bool m_selectUpdate;
	// Entity and emitter select info.
	int m_entityId;
	C_Position* m_entity;
	Emitter* m_emitter;
	NameList m_entityNames;
	NameList m_emitterNames;
	// Selection range.
	unsigned int m_layerSelectLow;
	unsigned int m_layerSelectHigh;
	// Interfaces.
	GUI_Interface* m_selectionOptions;
	MapControls* m_mapControls;
	GUI_MapTileSelector* m_tileSelector;
	// Class ties.
	EventManager* m_eventManager;
	GUI_Manager* m_guiManager;
	Map* m_map;
	TileMap* m_brush;
	EntityManager* m_entityManager;
	ParticleSystem* m_particleSystem;
};