#pragma once
#include <set>
#include <memory>
#include <iomanip>
#include <iostream>

#include "timing_analyzers.hpp"
#include "TimingGraph.hpp"
#include "TimingTags.hpp"
#include "FixedDelayCalculator.hpp"

namespace tatum {

float time_sec(struct timespec start, struct timespec end);

void print_histogram(const std::vector<float>& values, int nbuckets);

void print_level_histogram(const TimingGraph& tg, int nbuckets);
void print_node_fanin_histogram(const TimingGraph& tg, int nbuckets);
void print_node_fanout_histogram(const TimingGraph& tg, int nbuckets);

void print_timing_graph(std::shared_ptr<const TimingGraph> tg);
void print_levelization(std::shared_ptr<const TimingGraph> tg);

void dump_level_times(std::string fname, const TimingGraph& timing_graph, std::map<std::string,float> serial_prof_data, std::map<std::string,float> parallel_prof_data);

/*
 * Templated function implementations
 */

template<class DelayCalc=FixedDelayCalculator>
void write_dot_file_setup(std::ostream& os, 
                          const TimingGraph& tg, 
                          std::shared_ptr<const TimingAnalyzer> analyzer = std::shared_ptr<const TimingAnalyzer>(), 
                          std::shared_ptr<const DelayCalc> delay_calc = std::shared_ptr<const DelayCalc>()) {

    if(tg.nodes().size() > 1000) {
        std::cout << "Skipping setup dot file due to large timing graph size\n"; 
    }

    auto setup_analyzer = std::dynamic_pointer_cast<const SetupTimingAnalyzer>(analyzer);

    //Write out a dot file of the timing graph
    os << "digraph G {" <<std::endl;
    os << "\tnode[shape=record]" <<std::endl;

    for(const NodeId inode : tg.nodes()) {
        os << "\tnode" << size_t(inode);
        os << "[label=\"";
        os << "{" << inode << " (" << tg.node_type(inode) << ")";
        if(setup_analyzer) {
            auto data_tags = setup_analyzer->get_setup_data_tags(inode);
            if(data_tags.num_tags() > 0) {
                for(const TimingTag& tag : data_tags) {
                    os << " | {";
                    os << "DATA - " << tag.clock_domain();
                    os << " launch: " << tag.launch_node();
                    os << "\\n";
                    os << " arr: " << tag.arr_time().value();
                    os << " req: " << tag.req_time().value();
                    os << "}";
                }
            }
            auto clock_tags = setup_analyzer->get_setup_clock_tags(inode);
            if(clock_tags.num_tags() > 0) {
                for(const TimingTag& tag : clock_tags) {
                    os << " | {";
                    os << "CLOCK - " << tag.clock_domain();
                    os << " launch: " << tag.launch_node();
                    os << "\\n";
                    os << " arr: " << tag.arr_time().value();
                    os << " req: " << tag.req_time().value();
                    os << "}";
                }
            }
        }
        os << "}\"]";
        os <<std::endl;
    }

    //Force drawing to be levelized
    for(const LevelId ilevel : tg.levels()) {
        os << "\t{rank = same;";

        for(NodeId node_id : tg.level_nodes(ilevel)) {
            os << " node" << size_t(node_id) <<";";
        }
        os << "}" <<std::endl;
    }

    //Add edges with delays annoated
    for(const LevelId ilevel : tg.levels()) {
        for(NodeId node_id : tg.level_nodes(ilevel)) {
            for(EdgeId edge_id : tg.node_out_edges(node_id)) {

                NodeId sink_node_id = tg.edge_sink_node(edge_id);

                os << "\tnode" << size_t(node_id) << " -> node" << size_t(sink_node_id);
                if(delay_calc) {
                    os << " [ label=\"" << delay_calc->max_edge_delay(tg, edge_id) << "\" ]";
                }
                os << ";" <<std::endl;
            }
        }
    }

    os << "}" <<std::endl;
}

template<class DelayCalc=FixedDelayCalculator>
void write_dot_file_hold(std::ostream& os, 
                         const TimingGraph& tg, 
                         std::shared_ptr<const TimingAnalyzer> analyzer = std::shared_ptr<const TimingAnalyzer>(), 
                         std::shared_ptr<const DelayCalc> delay_calc = std::shared_ptr<const DelayCalc>()) {
    if(tg.nodes().size() > 1000) {
        std::cout << "Skipping hold dot file due to large timing graph size\n"; 
    }

    auto hold_analyzer = std::dynamic_pointer_cast<const HoldTimingAnalyzer>(analyzer);

    //Write out a dot file of the timing graph
    os << "digraph G {" <<std::endl;
    os << "\tnode[shape=record]" <<std::endl;

    //Declare nodes and annotate tags
    for(const NodeId inode : tg.nodes()) {
        os << "\tnode" << size_t(inode);
        os << "[label=\"";
        os << "{" << inode << " (" << tg.node_type(inode) << ")";
        auto data_tags = hold_analyzer->get_hold_data_tags(inode);
        if(data_tags.num_tags() > 0) {
            for(const TimingTag& tag : data_tags) {
                os << " | {";
                os << "DATA - " << tag.clock_domain();
                os << " launch: " << tag.launch_node();
                os << "\\n";
                os << " arr: " << tag.arr_time().value();
                os << " req: " << tag.req_time().value();
                os << "}";
            }
        }
        auto clock_tags = hold_analyzer->get_hold_clock_tags(inode);
        if(clock_tags.num_tags() > 0) {
            for(const TimingTag& tag : clock_tags) {
                os << " | {";
                os << "CLOCK - " << tag.clock_domain();
                os << " launch: " << tag.launch_node();
                os << "\\n";
                os << " arr: " << tag.arr_time().value();
                os << " req: " << tag.req_time().value();
                os << "}";
            }
        }
        os << "}\"]";
        os <<std::endl;
    }

    //Force drawing to be levelized
    for(const LevelId ilevel : tg.levels()) {
        os << "\t{rank = same;";

        for(NodeId node_id : tg.level_nodes(ilevel)) {
            os << " node" << size_t(node_id) <<";";
        }
        os << "}" <<std::endl;
    }

    //Add edges with delays annoated
    for(const LevelId ilevel : tg.levels()) {
        for(NodeId node_id : tg.level_nodes(ilevel)) {
            for(EdgeId edge_id : tg.node_out_edges(node_id)) {

                NodeId sink_node_id = tg.edge_sink_node(edge_id);

                os << "\tnode" << size_t(node_id) << " -> node" << size_t(sink_node_id);
                if(delay_calc) {
                    os << " [ label=\"" << delay_calc->min_edge_delay(tg, edge_id) << "\" ]";
                }
                os << ";" <<std::endl;
            }
        }
    }

    os << "}" <<std::endl;
}

void print_setup_tags_histogram(const TimingGraph& tg, const SetupTimingAnalyzer& analyzer);
void print_hold_tags_histogram(const TimingGraph& tg, const HoldTimingAnalyzer& analyzer);

void print_setup_tags(const TimingGraph& tg, const SetupTimingAnalyzer& analyzer);
void print_hold_tags(const TimingGraph& tg, const HoldTimingAnalyzer& analyzer);

} //namepsace
