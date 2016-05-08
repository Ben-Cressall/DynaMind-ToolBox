#ifndef RAINWATERHARVESTINGOPTIONS_H
#define RAINWATERHARVESTINGOPTIONS_H

#include <dmmodule.h>
#include <dm.h>
#include <flow.h>
#include <vector>

class MapBasedModel;
class NodeRegistry;
class ISimulation;
class SimulationRegistry;
class DynaMindStreamLogSink;
class Node;
class SimulationParameters;


class RainWaterHarvestingOptions: public DM::Module
{
	DM_DECLARE_NODE(RainWaterHarvestingOptions)

	private:
		// Dynamind varaibels
		std::string rwht_view_name;
		std::string start_date;
		std::string end_date;
		std::string timestep;

		bool start_date_from_global;
		bool add_time_series_data;
		std::string global_viewe_name;
		std::string start_date_name;
		std::string end_date_name;

		DM::ViewContainer parcels;
		DM::ViewContainer rwhts;
		DM::ViewContainer global_object;

		// local cd3 varaibles
		std::string varaibles;
		std::string cd3_end_date;
		std::string cd3_start_date;

		SimulationRegistry *simreg;
		NodeRegistry *nodereg;

		DynaMindStreamLogSink *sink;
		SimulationParameters *p;
		MapBasedModel * m;

		Node * n_d;
		Node * n_r;
		Node * rwht;
        Node * flow_p;
		ISimulation * s;

		std::vector<string> storage_volume_tank;

		std::vector<double> create_montlhy_values(std::vector<double> daily, int seconds);

		std::vector<double> addVectors(std::vector<double> &vec1, std::vector<double> &vec2);
		std::vector<double> substractVectors(std::vector<double> &vec1, std::vector<double> &vec2);

public:
		RainWaterHarvestingOptions();
		void run();
		void init();
		bool initmodel();
		void clear();

		double createTankOption(OGRFeature *rwht, double storage_volume, std::vector<double> & runoff, std::vector<double>  & out_doordemand, std::vector<double> &non_potable_demand);
		Node * addRainTank(double storage_volume, Node* in_flow, Node* nonpot_before);
		Node * addRainwaterTank(Node* flow_probe_runoff, Node* nonpot_before, double storage_volume);

		std::vector<double> mutiplyVector(std::vector<double> & vec, double multiplyer);
};

#endif // RAINWATERHARVESTINGOPTIONS_H
