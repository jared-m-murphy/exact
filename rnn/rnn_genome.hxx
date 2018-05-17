#ifndef RNN_BPTT_HXX
#define RNN_BPTT_HXX

#include <map>
using std::map;

#include <random>
using std::minstd_rand0;
using std::uniform_real_distribution;

#include <vector>
using std::vector;

#include "rnn.hxx"
#include "rnn_node_interface.hxx"
#include "rnn_edge.hxx"
#include "rnn_recurrent_edge.hxx"

#include "common/random.hxx"

//mysql can't handl the max float value for some reason
#define EXALT_MAX_DOUBLE 10000000


class RNN_Genome {
    private:
        int32_t generation_id;

        int32_t bp_iterations;
        double learning_rate;
        bool adapt_learning_rate;
        bool use_nesterov_momentum;
        bool use_reset_weights;

        bool use_high_norm;
        double high_threshold;
        bool use_low_norm;
        double low_threshold;

        bool use_dropout;
        double dropout_probability;

        string log_filename;

        map<string, int> generated_by_map;

        vector<double> initial_parameters;

        double best_validation_error;
        double best_validation_mae;
        vector<double> best_parameters;

        minstd_rand0 generator;
        uniform_real_distribution<double> rng_0_1;
        NormalDistribution normal_distribution;

        vector<RNN_Node_Interface*> nodes;
        vector<RNN_Edge*> edges;
        vector<RNN_Recurrent_Edge*> recurrent_edges;

        vector<string> input_parameter_names;
        vector<string> output_parameter_names;

    public:
        void sort_nodes_by_depth();
        void sort_edges_by_depth();

        RNN_Genome(vector<RNN_Node_Interface*> &_nodes, vector<RNN_Edge*> &_edges);
        RNN_Genome(vector<RNN_Node_Interface*> &_nodes, vector<RNN_Edge*> &_edges, vector<RNN_Recurrent_Edge*> &_recurrent_edges);
        RNN_Genome(vector<RNN_Node_Interface*> &_nodes, vector<RNN_Edge*> &_edges, vector<RNN_Recurrent_Edge*> &_recurrent_edges, uint16_t seed);

        RNN_Genome* copy();

        ~RNN_Genome();

        void set_parameter_names(const vector<string> &_input_parameter_names, const vector<string> &_output_parameter_names);

        string generated_by_string();

        int32_t get_enabled_node_count();
        int32_t get_enabled_edge_count();
        int32_t get_enabled_recurrent_edge_count();

        void set_bp_iterations(int32_t _bp_iterations);
        void set_learning_rate(double _learning_rate);
        void set_adapt_learning_rate(bool _adapt_learning_rate);
        void set_nesterov_momentum(bool _use_nesterov_momentum);
        void set_reset_weights(bool _use_reset_weights);
        void disable_high_threshold();
        void enable_high_threshold(double _high_threshold);
        void disable_low_threshold();
        void enable_low_threshold(double _low_threshold);
        void disable_dropout();
        void enable_dropout(double _dropout_probability);
        void set_log_filename(string _log_filename);

        void get_weights(vector<double> &parameters);
        void set_weights(const vector<double> &parameters);
        uint32_t get_number_weights();
        void initialize_randomly();

        int32_t get_generation_id() const;
        double get_validation_error() const;

        void set_generated_by(string type);
        int32_t get_generated_by(string type);


        RNN* get_rnn();

        void get_analytic_gradient(vector<RNN*> &rnns, const vector<double> &parameters, const vector< vector< vector<double> > > &inputs, const vector< vector< vector<double> > > &outputs, double &mse, vector<double> &analytic_gradient, bool training);

        void backpropagate(const vector< vector< vector<double> > > &inputs, const vector< vector< vector<double> > > &outputs, const vector< vector< vector<double> > > &validation_inputs, const vector< vector< vector<double> > > &validation_outputs);

        void backpropagate_stochastic(const vector< vector< vector<double> > > &inputs, const vector< vector< vector<double> > > &outputs, const vector< vector< vector<double> > > &validation_inputs, const vector< vector< vector<double> > > &validation_outputs);


        double get_mse(const vector<double> &parameters, const vector< vector< vector<double> > > &inputs, const vector< vector< vector<double> > > &outputs, bool verbose = false);
        double get_mae(const vector<double> &parameters, const vector< vector< vector<double> > > &inputs, const vector< vector< vector<double> > > &outputs, bool verbose = false);

        void get_mu_sigma(const vector<double> &p, double &mu, double &sigma);

        bool sanity_check();
        void assign_reachability();
        bool outputs_unreachable();

        RNN_Node_Interface* create_node(double mu, double sigma, double lstm_node_rate, int32_t &node_innovation_count, double depth);
        bool attempt_edge_insert(RNN_Node_Interface *n1, RNN_Node_Interface *n2, double mu, double sigma, int32_t &edge_innovation_count);
        bool attempt_recurrent_edge_insert(RNN_Node_Interface *n1, RNN_Node_Interface *n2, double mu, double sigma, int32_t &edge_innovation_count);

        bool add_edge(double mu, double sigma, int32_t &edge_innovation_count);
        bool add_recurrent_edge(double mu, double sigma, int32_t &edge_innovation_count);
        bool disable_edge();
        bool enable_edge();
        bool split_edge(double mu, double sigma, double lstm_node_rate, int32_t &edge_innovation_count, int32_t &node_innovation_count);

        bool add_node(double mu, double sigma, double lstm_node_rate, int32_t &edge_innovation_count, int32_t &node_innovation_count);
        bool enable_node();
        bool disable_node();
        bool split_node(double mu, double sigma, double lstm_node_rate, int32_t &edge_innovation_count, int32_t &node_innovation_count);
        bool merge_node(double mu, double sigma, double lstm_node_rate, int32_t &edge_innovation_count, int32_t &node_innovation_count);


        bool equals(RNN_Genome *other);

        string get_color(double weight, bool is_recurrent);
        void print_graphviz(string filename);


        friend class EXALT;
};

struct sort_genomes_by_validation_error {
    bool operator()(RNN_Genome *g1, RNN_Genome *g2) {
        return g1->get_validation_error() < g2->get_validation_error();
    }
};


#endif
