#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "BaseState.h"
#include "EventManager.h"
#include "FileManager.h"
#include "Map.h"
#include "MapControls.h"

class State_MapEditor : public BaseState {
public:
	State_MapEditor(StateManager* l_stateManager);
	~State_MapEditor();

	void OnCreate();
	void OnDestroy();

	void Activate();
	void Deactivate();

	void Update(const sf::Time& l_time);
	void Draw();

	void ResetSavePath();
	void SetMapRedraw(bool l_redraw);

	void MainMenu(EventDetails* l_details);
	void MapEditorNew(EventDetails* l_details);
	void MapEditorLoad(EventDetails* l_details);
	void MapEditorSave(EventDetails* l_details);
	void MapEditorSaveAs(EventDetails* l_details);
	void MapEditorExit(EventDetails* l_details);
	void MapAction(const std::string& l_name);
private:
	void SaveMap(const std::string& l_path);
	void LoadMap(const std::string& l_path);
	GUI_FileManager m_files;
	MapControls m_mapControls;
	std::string m_mapSavePath;
	bool m_mapRedraw;
};