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


#ifndef DMMODULE_H
#define DMMODULE_H

#include "dmcompilersettings.h"
#include <list>
#include <map>
#include <dmview.h>
#include <dmviewcontainer.h>
#include <dmsimulationconfig.h>

namespace DM {
	
// Prototypes
class System;
class RasterData;
class ModuleObserver;
class Simulation;
class GDALSystem;
class ISystem;

// enums
#if defined _MSC_VER && defined _WIN32 || __cplusplus <= 199711L
enum PortType
#else
enum PortType : unsigned int
#endif
{
	INPORT = 0,
	OUTPORT,
};

enum  DataTypes 
{
	INT,
	LONG,
	DOUBLE,
	STRING,
	FILENAME,
	STRING_LIST,
	STRING_MAP,
	BOOL,
};

enum ModuleStatus
{
	MOD_UNTOUCHED,
	MOD_EXECUTION_OK,
	MOD_EXECUTING,
	MOD_EXECUTION_ERROR,
	MOD_CHECK_OK,
	MOD_CHECK_ERROR,
};

/**
  */
class DM_HELPER_DLL_EXPORT FilterArgument {
private:
	std::string argument;
public:
	FilterArgument() : argument(""){}
	FilterArgument(std::string argument) : argument(argument){}
	std::string getArgument() {return this->argument;}
};

/**
  */
class DM_HELPER_DLL_EXPORT Filter {
public:
	Filter() : viewName(""), attributeFilter(), spatialFilter(){}
	Filter(std::string viewName, FilterArgument attributeFilter = FilterArgument(), FilterArgument spatialFilter = FilterArgument()): viewName(viewName), attributeFilter(attributeFilter), spatialFilter(spatialFilter){}
	std::string getViewName() {return viewName;}
	FilterArgument getAttributeFilter(){return attributeFilter;}
	FilterArgument getSpatialFilter(){return spatialFilter;}
private:
	std::string viewName;
	FilterArgument attributeFilter;
	FilterArgument spatialFilter;
};



/**
* @class DM::Module
* @brief Abstract class as a base for Modules.
*/
class DM_HELPER_DLL_EXPORT Module
{
	// this ensures that the simulation has full access without opening all methods to public
	friend class Simulation;
public:
	/** @brief parameters are variable values given via gui input, configuring a module */
	struct Parameter
	{
		const std::string	name;
		const DataTypes		type;
		void*				data;
		const std::string	description;

		Parameter(	const std::string name, const DataTypes type, 
			void* data, const std::string description):
		name(name), type(type),
			data(data), description(description)
		{};
	};

	/** @brief constructor */
	Module();

	/** @brief executed before run is called
	 *
	 * used to update the view containers in GDALSystem
	 */
	virtual void preRun();

	/** @brief executed after run has ben executed
	 *
	 * used to update clean diry view container
	 */
	virtual void afterRun();


	/** @brief destructor */
	virtual ~Module();

	/** @brief an optional init function for runtime inits */
	virtual void init() {};

	/** @brief executes the modules with the data given via parameters and 
	inports (those are set by the simulation) */
	virtual void run() = 0;

	/** @brief returns the name of the class - for e.g. logging purposes */
	virtual const char* getClassName() const = 0;

	/** @brief returns the name of the class - for e.g. logging purposes */
	virtual const char* getDisplayName() const = 0;

	/** @brief Returns URL to the help of the module */
	virtual std::string getHelpUrl(){return "";};

	/** @brief Returns if the module comes with its own GUI.
	* The default value is false. If you develop your own GUI for the module the GUI is
	* overwrite this method in the module implementation, call the GUI within the method
	* an return true. */
	virtual bool createInputDialog(){return false;}

	/** @brief overloaded method to determine safly if this module is a group */
	virtual bool isGroup(){return false;};

	/** @brief returns the current status of the module */
	ModuleStatus getStatus(){return status;};

	/** @brief returns a vector of port names on the input side */
	std::vector<std::string> getInPortNames() const;

	/** @brief returns a vector of port names on the output side */
	std::vector<std::string> getOutPortNames() const;

	/** @brief checks if all outports are set or not existing */
	bool outPortsSet() const;

	/** @brief checks if all inports are set or not existing */
	bool inPortsSet() const;

	/** @brief returns all views accessed by this module */
	std::map<std::string, std::map<std::string,View> > getAccessedViews() const;

	/** @brief returns all views accessed by this module on the starndard port*/
	std::map<std::string,View> getAccessedStdViews() const;

	/** @brief returns all streams with their views */
	std::map<std::string, std::map<std::string, View> > getViewsInStream() const;

	/** @brief returns all out streams with their views */
	std::map<std::string, std::map<std::string, View> > getViewsInOutStream() const;


	/** @brief shortcut to getViewsInStream to return all views from a specific stream */
	std::vector<View> getViewsInStream(const std::string& streamName) const;

	/** @brief returns if stream has view **/
	bool isViewInStream(const std::string& streamName, const string &viewName) const;

	/** @brief shortcut to getViewsInStream to return a specific view from a specific stream */
	View getViewInStream(const std::string& streamName, const std::string& viewName) const;

	/** @brief shortcut to getViewsInStream to return all views from the stream view index 0 */
	std::map<std::string,View> getViewsInStdStream() const;

	/** @brief shortcut to getViewsOutStream to return all views from the stream view index 0 */
	std::map<std::string, View> getViewsOutStdStream() const;

	/** @brief deprecated */
	std::string getUuid() const;

	/** @brief deprecated returns all view definitions added via addData */
	std::map<std::string, std::map<std::string, DM::View> > getViews() const;

	/** @brief just nulls out the inport, may get deprecated */
	void removeData(const std::string& name);

	/** @brief get data from outport; public for ModelNode::viewData */
	ISystem* getOutPortData(const std::string &name) const;
	
	/** @brief get data from inport */
	ISystem* getInPortData(const std::string &name) const;

	/** @brief adds an observer to this module */
	void addObserver(ModuleObserver* obs);

	/** @brief removes an observer from this module */
	void removeObserver(ModuleObserver* obs);

	/** @brief clears the observerlist */
	void removeAllObservers();

	/** @brief returns the current owner */
	Module* getOwner() const {return owner;}

	/** @brief returns the name of the module. if no was provided via 
	setName(string), it returns the class name in brakets */
	std::string getName() const;

	/** @brief sets the name of this module */
	void setName(std::string name);

	/** @brief activates the successor mode, forcing the module
	to create a successor of all incoming data streams */
	void setSuccessorMode(bool value);

	/** @brief returns the current status of the successor mode, see setSuccessorMode(bool) */
	bool isSuccessorMode() const;

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
	void addParameter(const std::string &name, const DataTypes type, 
						void * ref, const std::string description = "");

	/** @brief returns a parameter as structure */
	Parameter* getParameter(const std::string& name) const;

	/** @brief returns a vector of all parameters */
	std::vector<Parameter*> getParameters() const;

	/** @brief Returns the parameter as string value
	* As seperator for STRING_LIST *|* is used and for maps also *||*
	* 1*|*2*|*3*|4*||*
	* 5*|*6*|*7*|*8*||*
	*/
	std::string getParameterAsString(const std::string& name) const;

	/** @brief sets the parameter via string value
	* As seperator for STRING_LIST *|* is used and for maps also *||*
	* 1*|*2*|*3*|4*||*
	* 5*|*6*|*7*|*8*||*
	*/
	void setParameterValue(const std::string& name, const std::string& value);

	/** @brief set if the simulation is reset after parameters a changed within the module*/
	void forceRefreshSimulation(bool force);

	/** @brief Returns the status of reseting the simulation*/
	bool getForceRefreshSimulation();

	/**
	 * @brief Set filter
	 */
	void setFilter(std::vector<Filter> filters);

	/**
	 * @brief return filter
	 */
	std::vector<Filter> getFilter();

	/** @brief sets the current status of the module */
	void setStatus(ModuleStatus status);


	/** @brief Returns true if GDAL module */
	bool isGdalModule();

	/** @brief Return current counter of parent group. If no parent group is set counter is -1 **/
	int getGroupCounter();

	/** @brief Returns simulation config */
	DM::SimulationConfig getSimulationConfig();

	void setUUID(const std::string & uuid);

protected:
	/** @brief returns the data from the desired stream */
	ISystem* getIData(const std::string& streamName);
	ISystem *SystemFactory();
	System *getData(const std::string& streamName);

	/** @brief For GDAL systems please use registerViewContainers to manage the data stream
	 *
	 *Returns the data from the desired stream
	*/
	GDALSystem *getGDALData(const string &streamName);

	/** @brief checks if in-port does exist */
	bool hasInPort(const std::string &name) const;

	/** @brief checks if out-port does exist */
	bool hasOutPort(const std::string &name) const;

	/** @brief adds a new port, which can be connected to a single other node*/
	void addPort(const std::string &name, const PortType type);

	/** @brief removes a port from the module, may corrupt links! */
	void removePort(const std::string &name, const PortType type);

	/** @brief Used to define the data that are used in the module.
	*
	* The data is defined as a vetor of views. The stream name represents the name of the ports created.
	* - AccessType Read:   Inport
	* - AccessType Write:  Outport
	* - AccessType Modify: In and Outport
	*
	* If a port already exists no new port is added, existing definitions are overwritten. */
	void addData(const std::string& streamName, std::vector<View> views);

	/**
	 * @brief Registers a GDAL data stream in the simulation. Recommended for interal use only. Module developer
	 * should use registerViewContainers to register a data stream.
	 *
	 * If this method is used the ViewContainer need to be updated with the current system during before
	 * data can be accessed in run(). Further before run is finished the ViewContainer need to be
	 * syncronised.
	 */
	void addGDALData(const std::string& streamName, std::vector<ViewContainer> views);

	/**
	 * @brief Registers a GDAL data stream in the simulation. ViewContianers are automatically initialised and don't need
	 * to be updated
	 */
	void registerViewContainers(std::vector<ViewContainer*> views);

	/** @brief Returns a pointer raster data set assigend to a view **/
	RasterData* getRasterData(std::string name, View view);

	/** @brief Sets inport data - may only by used by DM::Simulation and loopgroup */
	void setInPortData(const std::string &name, ISystem* data);

	/** @brief */
	void setOutPortData(const std::string &name, ISystem *data);

	/** @brief Sets if module uses GDAL in Data stream */
	void setIsGDALModule(bool b);

	/** @brief True if module uses GDAL in Data stream. Default is False */
	bool GDALModule;

	bool SQLExclusive;

	/** @brief Sets the exlicuve access to SQL. This means that viewcontainers are not used **/
	void setIsSQLExclusive(bool b);

	bool isSQLExclusive();

	/** @brief Returns simulation */
	DM::Simulation *getSimulation();


private:
	/** @brief sets its owner, e.g. a group. this method is called by sim::addModule */
	void setOwner(Module* owner);

	void setSimulation(Simulation* sim);

	/** @brief resets the streamviews from sim::checkStream() and deletes all systems on the ports */
	void reset();

	// all view inits in module::init will be stored here as: streamname | views
	std::map<std::string, std::map<std::string,View> > accessedViews;

	// a temporary storage for all streams and viewnames in the stream up to this module
	// it is updated by simulation::checkModuleStream
	std::map<std::string, std::map<std::string, View> > streamViews;

	// temporary storage for out going streams, necessary when adding a link afterwards
	std::map<std::string, std::map<std::string, View> > outStreamViews;

	std::vector<ModuleObserver*>	observers;
	std::vector<Parameter*>			parameters;
	std::map<std::string, ISystem*>	inPorts;
	std::map<std::string, ISystem*>	outPorts;

	ModuleStatus	status;
	Module*			owner;
	bool			successorMode;
	std::string		name;
	DM::Simulation *sim;
	bool			forceUpdate;
	std::vector<Filter> moduleFilter;

	GDALSystem * reconnect_sys;

	std::string uuid;

	//View containers registered in the simulation and therefore managed by the simulation
	std::vector<DM::ViewContainer *> regiseredViewContainers;
};

}

#define DM_DECLARE_NODE(node)  \
	public: \
	static const char *classname; \
	static const char *filename; \
	static const char *displayname; \
	virtual const char *getClassName() const; \
	virtual const char *getDisplayName() const; \
	virtual const char *getFileName() const; \
	private:

#define  DM_DECLARE_NODE_NAME(nodename, module) \
	const char *nodename::classname = #nodename; \
	const char *nodename::displayname = #nodename; \
	const char *nodename::getClassName() const { return nodename::classname; } \
	const char *nodename::getDisplayName() const { return nodename::displayname; } \
	const char *nodename::filename = #module; \
	const char *nodename::getFileName() const { return nodename::filename; }

#define  DM_DECLARE_CUSTOM_NODE_NAME(nodename, display_name ,module) \
	const char *nodename::classname = #nodename; \
	const char *nodename::displayname = #display_name; \
	const char *nodename::getClassName() const { return nodename::classname; } \
	const char *nodename::getDisplayName() const { return nodename::displayname; } \
	const char *nodename::filename = #module; \
	const char *nodename::getFileName() const { return nodename::filename; }

#endif // MODULE_H
