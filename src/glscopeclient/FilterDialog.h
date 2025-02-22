/***********************************************************************************************************************
*                                                                                                                      *
* glscopeclient                                                                                                        *
*                                                                                                                      *
* Copyright (c) 2012-2022 Andrew D. Zonenberg                                                                          *
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
	@author Andrew D. Zonenberg
	@brief Dialog for configuring filters
 */

#ifndef FilterDialog_h
#define FilterDialog_h

#include "../scopehal/Oscilloscope.h"
#include "WaveformArea.h"
#include "WaveformGroup.h"

class ChannelSelectorRow
{
public:
	Gtk::Label			m_label;
	Gtk::ComboBoxText	m_chans;

	std::map<std::string, StreamDescriptor> m_chanptrs;
};

class FilterDialog;

class ParameterRowBase
{
public:
	ParameterRowBase(Gtk::Dialog* parent, FilterParameter& param, FlowGraphNode* node);
	virtual ~ParameterRowBase();

	Gtk::Label			m_label;
		Gtk::Grid		m_contentbox;
	Gtk::Dialog* 		m_parent;
	FlowGraphNode*		m_node;

	FilterParameter& 	m_param;

	//set true to suppress event generation when updating the dialog
	bool				m_ignoreEvents;

	//set true to suppress event generation when changing a parameter value externally
	bool				m_ignoreUpdates;
};

class ParameterBlock8B10BSymbol
{
public:
	ParameterBlock8B10BSymbol();

	Gtk::Frame						m_frame;
		Gtk::Grid					m_grid;
			Gtk::Label				m_typeLabel;
				Gtk::ComboBoxText	m_typeBox;
			Gtk::Label				m_disparityLabel;
				Gtk::ComboBoxText	m_disparityBox;
			Gtk::Label				m_symbolLabel;
				Gtk::ComboBoxText	m_symbolBox;		//if in K character mode
				Gtk::Entry			m_symbolEntry;		//if in D character mode

	sigc::connection				m_typeConnection;
	sigc::connection				m_valueConnection;
	sigc::connection				m_disparityConnection;
};

class ParameterRow8B10BPattern : public ParameterRowBase
{
public:
	ParameterRow8B10BPattern(Gtk::Dialog* parent, FilterParameter& param, FlowGraphNode* node);
	virtual ~ParameterRow8B10BPattern();

	void Initialize(const std::vector<T8B10BSymbol>& symbols);

protected:

	void SetupBlock(size_t i, T8B10BSymbol s, bool dotted);

	void OnPatternChanged();
	void OnKValueChanged(size_t i);
	void OnDValueChanged(size_t i);
	void OnDisparityChanged(size_t i);
	void OnTypeChanged(size_t i);

	std::vector<ParameterBlock8B10BSymbol> m_blocks;

	sigc::connection m_connection;
};

class ParameterRowString : public ParameterRowBase
{
public:
	ParameterRowString(Gtk::Dialog* parent, FilterParameter& param, FlowGraphNode* node);
	virtual ~ParameterRowString();

	Gtk::Entry			m_entry;

protected:
	void OnTextChanged();
	void OnValueChanged();
	sigc::connection m_connection;

	sigc::connection m_timerConnection;

	bool OnFocusLostTimer();
	bool m_timerPending;
};

class ParameterRowEnum : public ParameterRowBase
{
public:
	ParameterRowEnum(Gtk::Dialog* parent, FilterParameter& param, FlowGraphNode* node);
	virtual ~ParameterRowEnum();

	Gtk::ComboBoxText	m_box;

	void Refresh();

protected:
	void OnChanged();
	sigc::connection m_connection;
};

class ParameterRowFilename : public ParameterRowString
{
public:
	ParameterRowFilename(Gtk::Dialog* parent, FilterParameter& param, FlowGraphNode* node);
	virtual ~ParameterRowFilename();

	void OnBrowser();
	void OnClear();

	Gtk::Button			m_clearButton;
	Gtk::Button			m_browserButton;
};

/**
	@brief Main application window class for an oscilloscope
 */
class FilterDialog	: public Gtk::Dialog
{
public:
	FilterDialog(OscilloscopeWindow* parent, Filter* filter, StreamDescriptor chan);
	virtual ~FilterDialog();

	Filter* GetFilter()
	{ return m_filter; }

	void ConfigureDecoder();

	static ParameterRowBase* CreateRow(
		Gtk::Grid& grid,
		std::string name,
		FilterParameter& param,
		size_t y,
		Gtk::Dialog* parent,
		FlowGraphNode* node);

	static void ConfigureInputs(FlowGraphNode* node, std::vector<ChannelSelectorRow*>& rows);

	static void PopulateInputBox(
		OscilloscopeWindow* parent,
		Filter* filter,
		ChannelSelectorRow* row,
		size_t ninput,
		StreamDescriptor chan
		);

protected:
	Filter* m_filter;
	OscilloscopeWindow* m_parent;

	void OnRefresh();
	void OnRefreshInputs();
	void OnRefreshParameters();
	void OnInputChanged();

	//Helpers for ganged inputs
	bool ApplyAutomaticInputs();
	StreamDescriptor FindCorrespondingSParameter(
		const std::string& param,
		const std::string& suffix,
		StreamDescriptor ref);

	void OnParameterChanged();

	Gtk::Grid m_grid;
		Gtk::Label m_channelDisplayNameLabel;
			Gtk::Entry m_channelDisplayNameEntry;
		Gtk::Label m_channelColorLabel;
			Gtk::ColorButton m_channelColorButton;

	std::vector<ChannelSelectorRow*> m_rows;
	std::map<std::string, ParameterRowBase*> m_prows;

	bool m_refreshing;
	bool m_inputChanging;

	int m_cachedStreamCount;

	sigc::connection m_paramConnection;
	sigc::connection m_inputConnection;

	std::vector<sigc::connection> m_paramConnections;
};

#endif
