/***********************************************************************************************************************
*                                                                                                                      *
* glscopeclient                                                                                                        *
*                                                                                                                      *
* Copyright (c) 2022 Louis A. Goessling                                                                                *
* All rights reserved.                                                                                                 *
*                                                                                                                      *
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the     *
* following conditions are met:                                                                                        *
*                                                                                                                      *
*    * Redistributions of source code must retain the above copyright notice, this list of conditions, and the         *
*      following disclaimer.                                                                                           *
*                                                                                                                      *
*    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the       *
*      following disclaimer in the documentation and/or other materials provided with the distribution.                *
*                                                                                                                      *
*    * Neither the name of the author nor the names of any contributors may be used to endorse or promote products     *
*      derived from this software without specific prior written permission.                                           *
*                                                                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED   *
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL *
* THE AUTHORS BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES        *
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR       *
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
* POSSIBILITY OF SUCH DAMAGE.                                                                                          *
*                                                                                                                      *
***********************************************************************************************************************/

/**
	@file
	@author Louis A. Goessling
	@brief Dialog for viewing detailed performance/diagnostic information about a scope
 */

#ifndef ScopeInfoWindow_h
#define ScopeInfoWindow_h

class ShownGraph
{
public:
	Graph* graph;
	Gtk::Label* label;
	FilterParameter* param;
	Graphable data;
	double minval;
	double maxval;
};

class ScopeInfoGraphWindow : public Gtk::Dialog
{
public:
	ScopeInfoGraphWindow(std::string title);
	virtual ~ScopeInfoGraphWindow();

	void AddGraphedValue(std::string name, FilterParameter* value);
	void RemoveGraphedValue(std::string name);
	void DoValueUpdate(ShownGraph* shown);
	void OnTick();

protected:
	Gtk::Grid m_grid;

	std::map<std::string, ShownGraph> m_graphs;
};

/**
	@brief Dialog for interacting with a Oscilloscope
 */
class ScopeInfoWindow : public Gtk::Dialog
{
public:
	ScopeInfoWindow(OscilloscopeWindow* oscWindow, Oscilloscope* meter);
	virtual ~ScopeInfoWindow();

	void OnWaveformDataReady();

protected:
	OscilloscopeWindow* m_oscWindow;
	Oscilloscope* m_scope;

	std::deque<std::string> m_consoleText;
	FilterParameter m_driver;
	FilterParameter m_transport;
	FilterParameter m_bufferedWaveformParam;
	FilterParameter m_bufferedWaveformTimeParam;
	FilterParameter m_uiDisplayRate;

	Gtk::Grid m_grid;
		Gtk::Grid				m_commonValuesGrid;
			std::map<FilterParameter*, Gtk::Label*> m_commonValuesLabels;
		Gtk::Grid				m_valuesGrid;
			std::map<std::string, Gtk::Label*> m_valuesLabels;
		Gtk::ScrolledWindow     m_consoleFrame; 
			Gtk::TextView       m_console;
				Glib::RefPtr<Gtk::TextBuffer> m_consoleBuffer;
		Gtk::Button m_saveButton;

	ScopeInfoGraphWindow m_graphWindow;

	Gtk::Label* BindValue(Gtk::Grid& container, std::string name, FilterParameter* value);
	void OnValueUpdate(Gtk::Label* label, FilterParameter* value);
	void OnClickGraphSwitch(Gtk::Switch* graphSwitch, std::string name, FilterParameter* value);
	void OnGraphWindowClosed();
	void OnSaveClicked();
	bool OnTick();
};

#endif
