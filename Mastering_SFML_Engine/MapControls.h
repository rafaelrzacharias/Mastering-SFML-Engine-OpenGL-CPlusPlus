#pragma once
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include "MapTileSelector.h"
#include "SelectionOptions.h"
#include "TileMap.h"

class Window;
class EventManager;
struct EventDetails;
class Map;
class GUI_Manager;
class GUI_Interface;
class EntityManager;
class ParticleSystem;
class StateManager;

enum class ControlMode{ None, Pan, Brush, Bucket, Eraser, Select };

class MapControls {
public:
	MapControls(Window* l_window, EventManager* l_eventManager, StateManager* l_stateManager,
		Map* l_map, GUI_Manager* l_gui, EntityManager* l_entityMgr,
		ParticleSystem* l_particles, sf::View& l_view);
	~MapControls();

	void Update(float l_dT);
	void Draw(sf::RenderWindow* l_window);

	void NewMap();

	void SetTileSheetTexture(const std::string& l_sheet);
	ControlMode GetMode() const;
	bool IsInAction() const;
	bool IsInSecondary() const;
	GUI_SelectionOptions* GetSelectionOptions();

	sf::Vector2i GetMouseTileStart()const;
	sf::Vector2i GetMouseTile()const;
	sf::Vector2f GetMouseDifference()const;

	bool DrawSelectedLayers()const;
	void ToggleDrawingSelectedLayers();

	void MouseClick(EventDetails* l_details);
	void MouseRelease(EventDetails* l_details);
	void MouseWheel(EventDetails* l_details);
	void ToolSelect(EventDetails* l_details);
	void DeleteTiles(EventDetails* l_details);
	void NewMapCreate(EventDetails* l_details);
	void NewMapClose(EventDetails* l_details);

	void SelectMode(ControlMode l_mode);
	void RedrawBrush();
private:
	void UpdateMouse();
	void ResetTools();

	void PanUpdate();
	void BrushUpdate();
	void BucketUpdate();
	void EraserUpdate();
	void SelectionUpdate();

	void PlaceBrushTiles();

	void ResetZoom();

	// Mode and mouse/layer flags.
	ControlMode m_mode;
	bool m_action;
	bool m_secondaryAction;
	bool m_rightClickPan;
	bool m_drawSelectedLayers;
	// Mouse information.
	sf::Vector2i m_mousePosition;
	sf::Vector2i m_mouseStartPosition;
	sf::Vector2f m_mouseDifference;
	sf::Vector2i m_mouseTilePosition;
	sf::Vector2i m_mouseTileStartPosition;
	float m_zoom;
	// Brush information, and map bounds.
	TileMap m_brush;
	sf::RenderTexture m_brushTexture;
	sf::RectangleShape m_brushDrawable;
	sf::RectangleShape m_mapBoundaries;
	// Other interfaces used here.
	GUI_MapTileSelector m_tileSelector;
	GUI_SelectionOptions m_selectionOptions;
	GUI_Interface* m_mapSettings;
	// Ties to other classes.
	Window* m_window;
	EventManager* m_eventManager;
	StateManager* m_stateManager;
	Map* m_map;
	GUI_Manager* m_guiManager;
	EntityManager* m_entityManager;
	ParticleSystem* m_particleSystem;
	sf::View& m_view;
};