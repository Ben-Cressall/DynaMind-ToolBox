/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @author  Markus Sengthaler <m.sengthaler@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2011-2012  Christian Urich, Markus Sengthaler

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

/**
  * @addtogroup DynaMind-Core
  */
#ifndef DMMODULE_H
#define DMMODULE_H

#include "dmcompilersettings.h"
#include <list>
#include <map>
#include <dmlog.h>
#include <dmlogger.h>
#include <dmview.h>
#include <dmstdutilities.h>

class QThreadPool;
class PortObserver;

namespace DM {

// deprecated parameter type
typedef std::map<std::string, std::string> parameter_type;

#if defined _MSC_VER && defined _WIN32
enum PortType
#else
enum PortType : unsigned int
#endif
{
	INPORT = 0,
	OUTPORT,
};


enum  DataTypes {    
	INT,
	LONG,
	DOUBLE,
	STRING,
	FILENAME,
	STRING_LIST,
	STRING_MAP,
	BOOL,
};

class System;
class RasterData;
class ModuleObserver;

enum ModuleStatus
{
	MOD_UNTOUCHED,
	MOD_EXECUTION_OK,
	MOD_EXECUTING,
	MOD_EXECUTION_ERROR,
	MOD_CHECK_OK,
	MOD_CHECK_ERROR,
};

class DM_HELPER_DLL_EXPORT Module
{
	friend Simulation;

	struct Parameter;
	ModuleStatus status;

	std::vector<ModuleObserver*>	observers;
	std::vector<Parameter*>			parameters;
	std::map<std::string, System*>	inPorts;
	std::map<std::string, System*>	outPorts;
	Module*		owner;
	bool		successorMode;
	std::string name;
	// all view inits in module::init will be stored here as: streamname | views
	std::map<std::string, std::map<std::string,View> > accessedViews;
	// a temporary storage for all streams and viewnames in the stream up to this module
	// it is updated by simulation::checkModuleStream
	std::map<std::string, std::map<std::string,View> > streamViews;

	/** @brief get data from inport */
	System* getInPortData(const std::string &name);
	/** @brief sets its owner, e.g. a group. this method is called by sim::addModule */
	void setOwner(Module* owner);
public:
	/** @brief constructor */
	Module();
	/** @brief destructor */
	virtual ~Module();
	/** @brief an optional init function for runtime inits */
	virtual void init() {};
	/** @brief executes the modules with the data given via parameters and 
	inports (those are set by the simulation) */
	virtual void run() = 0;
	/** @brief returns the name of the class - for e.g. logging purposes */
	virtual const char* getClassName() = 0;
	/** @brief Returns URL to the help of the module */
	virtual std::string getHelpUrl(){return "";};
	/** @brief Returns if the module comes with its own GUI.
	* The default value is false. If you develop your own GUI for the module the GUI is
	* overwrite this method in the module implementation, call the GUI within the method
	* an return true. */
	virtual bool createInputDialog(){return false;}

	/** @brief returns the current status of the module */
	ModuleStatus getStatus(){return status;};
	/** @brief resets the streamviews from sim::checkStream() and deletes all systems on the ports */
	void reset();
	/** @brief returns the data from the desired stream */
	System* getData(const std::string& streamName);

	/** @brief returns a vector of port names on the input side */
	std::vector<std::string> getInPortNames() const;
	/** @brief returns a vector of port names on the output side */
	std::vector<std::string> getOutPortNames() const;
	/** @brief checks if in-port does exist */
	bool hasInPort(const std::string &name);
	/** @brief checks if out-port does exist */
	bool hasOutPort(const std::string &name);
	/** @brief checks if all outports are set or not existing */
	bool outPortsSet();
	/** @brief checks if all inports are set or not existing */
	bool inPortsSet();

	/** @brief returns all views accessed by this module */
	std::map<std::string, std::map<std::string,View> > getAccessedViews() const;
	/** @brief returns all streams with their views */
	std::map<std::string, std::map<std::string,View> > getViewsInStream() const;
	/** @brief shortcut to getViewsInStream to return all views from a specific stream */
	std::vector<View> getViewsInStream(const std::string& streamName) const;
	/** @brief shortcut to getViewsInStream to return a specific view from a specific stream */
	View getViewInStream(const std::string& streamName, const std::string& viewName) const;
	/** @brief shortcut to getViewsInStream to return all views from the stream view index 0 */
	std::map<std::string,View> getViewsInStdStream();

	/** @brief @deprecated */
	std::string getUuid()
	{
		DM::Logger(DM::Warning) << "module::getUuid() deprecated";
		return "<Module::getUuid deprecated>";
	}
	/** @brief returns all view definitions added via addData */
	std::map<std::string, std::map<std::string, DM::View> > getViews()
	{
		return getAccessedViews();
	}
	/** @brief just nulls out the inport, may get deprecated */
	void removeData(const std::string& name);
	/** @brief calls the init function if parameters have changed */
	void updateParameter()
	{
		DM::Logger(DM::Warning) << "module::updateParameter() deprecated";
	}
	/** @brief get data from outport; public for ModelNode::viewData */
	System* getOutPortData(const std::string &name);
	/** @brief adds an observer to this module */
	void addObserver(ModuleObserver* obs);
	/** @brief removes an observer from this module */
	void removeObserver(ModuleObserver* obs);
	/** @brief returns the current owner */
	Module* getOwner(){return owner;}
	/** @brief overloaded method to determine safly if this module is a group */
	virtual bool isGroup(){return false;};
	/** @brief returns the name of the module. if no was provided via 
	setName(string), it returns the class name in brakets */
	std::string getName();
	/** @brief sets the name of this module */
	void setName(std::string name);
	/** @brief activates the successor mode, forcing the module
	to create a successor of all incoming data streams */
	void setSuccessorMode(bool value);
	/** @brief returns the current status of the successor mode, see setSuccessorMode(bool) */
	bool isSuccessorMode();

	/** @brief parameters are variable values given via gui input, configuring a module */
	struct Parameter
	{
		const std::string	name;
		const DataTypes		type;
		void*				data;
		const std::string	description;

		Parameter(const std::string name, const DataTypes type, const std::string description):
			name(name), type(type), description(description)
		{};
		template<typename T>
		void set(T value)
		{
			*(T*)data = value;
		}
		template<typename T>
		T get()
		{
			return *(T*)data;
		}
	};
	/** @brief adds a Parameter to the module.
	* availiable types:
	* - DM::DOUBLE
	* - DM::INT
	* - DM::BOOL
	* - DM::STRING
	* - DM::FILENAME
	* - DM::LONG
	* - DM::STRING_LIST
	* - DM::STRING_MAP
	*/
	void addParameter(const std::string &name, const DataTypes type, void * ref, const std::string description = "");
	template<typename T>
	T getParameter(const std::string& name)
	{
		if(Parameter* p = getParameter(name))
			return p->get<T>();
		return T();
	}
	Parameter* getParameter(const std::string& name)
	{
		// foreach will cause a compile error in modules not including qt headers
		for(std::vector<Parameter*>::iterator it = parameters.begin();
			it != parameters.end(); ++it)
			if((*it)->name == name)
				return *it;
		return NULL;
	}
	std::vector<Parameter*> getParameters() const
	{
		return parameters;
	}

	/*********************
	* Backward comp.
	**********************/
	/** @brief Returns the parameter as string value
	*
	* As seperator for STRING_LIST *|* is used and for maps also *||*
	*
	* 1*|*2*|*3*|4*||*
	*
	* 5*|*6*|*7*|*8*||*
	*
	*/
	std::string getParameterAsString(const std::string& name);
	template<typename T>
	void setParameterNative(const std::string& name, T data)
	{
		if(Parameter* p = getParameter(name))
			p->set(data);
	}
	void setParameterValue(const std::string& name, const std::string& value);

protected:
	/** @brief adds a new port, which can be connected to a single other node*/
	void addPort(const std::string &name, const PortType type);
	/** @brief removes a port from the module, may corrupt links! */
	void removePort(const std::string &name, const PortType type);
	/** @brief */
	void setStatus(ModuleStatus status) {this->status = status;};
	/** @brief Used to define the data that are used in the module.
	*
	* The data is defined as a vetor of views. The stream name represents the name of the ports created.
	* - AccessType Read:   Inport
	* - AccessType Write:  Outport
	* - AccessType Modify: In and Outport
	*
	* If a port already exists no new port is added, existing definitions are overwritten. */
	void addData(const std::string& streamName, std::vector<View> views);
	/** @brief Returns a pointer raster data set assigend to a view **/
	RasterData* getRasterData(std::string name, View view);
	/** @brief sets inport data - may only by used by DM::Simulation and loopgroup */
	void setInPortData(const std::string &name, System* data);
	/** @brief */
	void setOutPortData(const std::string &name, System* data);
};

class ModuleObserver
{
protected:
	DM::Module* module;
public:
	ModuleObserver(Module* m): module(m)
	{
		module->addObserver(this);
	}
	virtual void notifyAddPort(const std::string &name, const PortType type) = 0;
	virtual void notifyRemovePort(const std::string &name, const PortType type) = 0;
	virtual void notifyChangeName(const std::string &name) = 0;
};
}

#define DM_DECLARE_NODE(node)  \
	public: \
	static const char *classname; \
	static const char *filename; \
	virtual const char *getClassName() ; \
	virtual const char *getFileName() ; \
	private:

#define  DM_DECLARE_NODE_NAME(nodename, module) \
	const char *nodename::classname = #nodename; \
	const char *nodename::getClassName()  { return nodename::classname; } \
	const char *nodename::filename = #module; \
	const char *nodename::getFileName()  { return nodename::filename; }

#endif // MODULE_H
