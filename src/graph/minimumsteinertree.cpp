/**
 * @file
 * @author  Michael Mair <michael.mair@uibk.ac.at>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2012  Michael Mair

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

#include <minimumsteinertree.h>

//DynaMind includes
#include <dmsystem.h>
#include <dmlogsink.h>
#include <tbvectordata.h>

#include <iostream>
#include <fstream>
#include <QFile>
#include <QTime>
#include <map>
#include <math.h>

//BOOST GRAPH includes
//#include <boosttraits.h>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <dynamindboostgraphhelper.h>

using namespace boost;

DM_DECLARE_NODE_NAME(MinimumSteinerTree,Graph)

MinimumSteinerTree::MinimumSteinerTree()
{
	std::vector<DM::View> views;
	DM::View view;

	view = defhelper.getCompleteView(DM::GRAPH::EDGES,DM::WRITE);
	views.push_back(view);
	viewdef[DM::GRAPH::EDGES]=view;

	view = defhelper.getView(DM::GRAPH::NODES,DM::WRITE);
	views.push_back(view);
	viewdef[DM::GRAPH::NODES]=view;

	view = DM::View("FORCEDNODES", DM::NODE, DM::READ);
	views.push_back(view);
	forcednodesview = view;

	this->addData("Layout", views);
}

void MinimumSteinerTree::run()
{
	DM::Logger(DM::Standard) << "Setup Graph";

	typedef std::pair < int, int >E;

	this->sys = this->getData("Layout");
	DynamindBoostGraph::Compmap nodes = sys->getAllComponentsInView(viewdef[DM::GRAPH::NODES]);
	DynamindBoostGraph::Compmap dedges = sys->getAllComponentsInView(viewdef[DM::GRAPH::EDGES]);
	DynamindBoostGraph::Compmap forcednodes = sys->getAllComponentsInView(forcednodesview);
	std::map<DM::Node* ,int> nodesindex;
	std::map< std::pair<int,int> ,DM::Edge*> nodes2edge;
	std::list<int> forcednodeslist;
	DynamindBoostGraph::Graph g;
	DynamindBoostGraph::createBoostGraph(nodes,dedges,g,nodesindex,nodes2edge);

	for(DynamindBoostGraph::Compitr itr = forcednodes.begin(); itr != forcednodes.end(); ++itr)
		forcednodeslist.push_back(nodesindex[static_cast<DM::Node*>((*itr).second)]);


	//check if graph is conntected
	std::vector<int> component(num_vertices(g));
	int num = connected_components(g, &component[0]);

	std::map<int,int> componentsizes;

	for (uint i = 0; i != component.size(); ++i)
		componentsizes[component[i]]=++componentsizes[component[i]];

	int maxgraphindex=0;

	for(uint index=0; index < componentsizes.size(); index++)
	{
		if(componentsizes[maxgraphindex] < componentsizes[index])
			maxgraphindex = index;

		DM::Logger(DM::Standard) << "Graph " << (int)index+1 << " has " << componentsizes[index] << " elements";
	}

	if(num!=1)
	{
		DM::Logger(DM::Error) << "Graph is not connected -> Forest of size: " << num;
		return;
	}

	//calculate min steiner tree approximation

	DM::Logger(DM::Standard) << "Start steiner tree algorithm with " << num_vertices(g) << " nodes and " << dedges.size() << " edges";

	typedef std::vector<graph_traits<DynamindBoostGraph::Graph>::vertex_descriptor> path;
	typedef adjacency_list < vecS, vecS, undirectedS, property<vertex_distance_t, int>, property < edge_weight_t, double > > SteinerGraph;

	std::map<std::pair<graph_traits<SteinerGraph>::vertex_descriptor, graph_traits<SteinerGraph>::vertex_descriptor>, path> edgetopath;
	SteinerGraph completegraph(num_vertices(g));

	//create complete graph between forced nodes
	int finished=0;
	int lastlog=-1;

	#pragma omp parallel for
	for(int vai = 0; vai<num_vertices(g); vai++)
	{
		#pragma omp critical
		{
			finished++;
		}

		if(std::find(forcednodeslist.begin(),forcednodeslist.end(),vai) == forcednodeslist.end())
			continue;

		std::vector<int> d(num_vertices(g));
		std::vector < graph_traits < DynamindBoostGraph::Graph >::vertex_descriptor > p(num_vertices(g));
		dijkstra_shortest_paths(g,vai,predecessor_map(&p[0]).distance_map(&d[0]));
		graph_traits< DynamindBoostGraph::Graph >::vertex_iterator vi, vend;

		for (boost::tie(vi, vend) = vertices(g); vi != vend; ++vi)
		{
			path currentpath;
			graph_traits< DynamindBoostGraph::Graph >::vertex_descriptor currentsoure = *vi;

			if(*vi==vai)
				continue;

			if(std::find(forcednodeslist.begin(),forcednodeslist.end(),*vi) == forcednodeslist.end())
				continue;

			if(edge(vai,*vi,completegraph).second)
				continue;

			while(currentsoure!=vai)
			{
				currentpath.push_back(currentsoure);
				currentsoure=p[currentsoure];
			}

			currentpath.push_back(currentsoure);

			#pragma omp critical
			{
				edgetopath[std::make_pair(*vi,vai)]=currentpath;
				add_edge(*vi,vai,d[*vi],completegraph);
			}
		}



		double completepercent = (100.0/num_vertices(g))*finished;
		int steps = 5;

		#pragma omp critical
		{
			if(int(completepercent)%steps==0 && lastlog!=int(completepercent))
			{
				DM::Logger(DM::Standard) << "DONE..........." << completepercent << "%";
				lastlog = int(completepercent);
			}
		}
	}

	DM::Logger(DM::Standard) << "Steiner calculation has finished -> extracting results";

	//calculate minimum spanning tree of complete tree
	std::vector < graph_traits < SteinerGraph >::vertex_descriptor >p(num_vertices(completegraph));
	prim_minimum_spanning_tree(completegraph,&p[0],root_vertex(*forcednodeslist.begin()));

	//build steiner tree approximation
	for (std::size_t i = 0; i != p.size(); ++i)
	{
		if(i != p[i])
		{
			//remove_edge(i,p[i],completegraph);
			path currentpath;

			if(edgetopath.find(std::make_pair<int,int>((int)i,(int)p[i]))!=edgetopath.end())
				currentpath = edgetopath[std::pair<int,int>(i,p[i])];
			else
				currentpath = edgetopath[std::pair<int,int>(p[i],i)];

			if(!currentpath.size())
				DM::Logger(DM::Debug) << "ERROR";

			for(uint index=1; index<currentpath.size(); index++)
				add_edge(currentpath[index-1],currentpath[index],0,completegraph);
		}
	}

	//clean view
	for(DynamindBoostGraph::Compitr itr = nodes.begin(); itr != nodes.end(); ++itr)
		sys->removeComponentFromView((*itr).second,viewdef[DM::GRAPH::NODES]);

	for(DynamindBoostGraph::Compitr itr = dedges.begin(); itr != dedges.end(); ++itr)
		sys->removeComponentFromView((*itr).second,viewdef[DM::GRAPH::EDGES]);

	//map to dynamind data structure
	graph_traits< SteinerGraph >::edge_iterator ei,eend;

	for(boost::tie(ei,eend) = edges(completegraph); ei != eend; ++ei)
	{
		int s = source(*ei,completegraph);
		int t = target(*ei,completegraph);
		DM::Edge *e = 0;

		if(nodes2edge.find(std::make_pair(s,t))!=nodes2edge.end())
			e = nodes2edge[std::make_pair(s,t)];

		if(nodes2edge.find(std::make_pair(t,s))!=nodes2edge.end())
			e = nodes2edge[std::make_pair(t,s)];

		if(e)
		{
			sys->addComponentToView(e,viewdef[DM::GRAPH::EDGES]);
			sys->addComponentToView(e->getStartNode(),viewdef[DM::GRAPH::NODES]);
			sys->addComponentToView(e->getEndNode(),viewdef[DM::GRAPH::NODES]);
		}
	}
}

void MinimumSteinerTree::initmodel()
{
}

string MinimumSteinerTree::getHelpUrl()
{
	return "https://docs.google.com/document/d/1xn4UMr5fniKuzFBkgUHZfSQMfIZY8UW9eVhHWfCoKD0/edit?usp=sharing";
}

