#ifndef EXAMM_DNAS_NODE_HXX
#define EXAMM_DNAS_NODE_HXX

#include <string>
using std::string;

#include <random>
using std::minstd_rand0;
using std::uniform_real_distribution;
using std::uniform_random_bit_generator;
using std::generate_canonical;

#include <vector>
using std::vector;

#include <memory>
using std::unique_ptr;

#include <type_traits>

#include "common/random.hxx"

#include "rnn_node_interface.hxx"
#include "rnn_node.hxx"
#include "rnn_edge.hxx"

class DNASNode : public RNN_Node_Interface {

 private:
  template <uniform_random_bit_generator R>
  static void gumbel_noise(R &rng, vector<double> &output);

  std::mt19937_64 generator;

  // These components are used to create a sub-network within this node, basically
  // to reduce the hackiness of things.
  vector<RNN_Node_Interface *> nodes;

  // Describes the probability distribution we are learning
  vector<double> pi;

  // A sample from the probability distribution described by pi
  vector<double> z;

  // Intermediate values required for backpropagation, equal to:
  // e^{(ln(pi_i)+g_i)/t} for each pi_i in pi
  vector<double> x;
  // Sum of x values, saved for use in backprop.
  double xtotal;

  // A vector to put gumbel noise into; just to avoid re-allocation
  vector<double> noise;

  // Temperature used when drawing samples from Gumbel-Softmax(pi)
  double tao;

  // if > 0, then the samples will be forced to be K-hot (K non-zero values that sum to one)
  int32_t k = -1;

  vector<double> d_pi;
  vector<vector<double>> node_outputs;
  vector<double> output_values;

 public:
  DNASNode(vector<RNN_Node_Interface *> &&nodes, int32_t _innovation_number, int32_t _type, double _depth);
  DNASNode(const DNASNode &node);
  ~DNASNode();
  
  template <uniform_random_bit_generator Rng>
  void sample_gumbel_softmax(Rng &rng);
  
  virtual void initialize_lamarckian(minstd_rand0 &generator, NormalDistribution &normal_distribution, double mu, double sigma);
  virtual void initialize_xavier(minstd_rand0 &generator, uniform_real_distribution<double> &rng_1_1, double range);
  virtual void initialize_kaiming(minstd_rand0 &generator, NormalDistribution &normal_distribution, double range);
  virtual void initialize_uniform_random(minstd_rand0 &generator, uniform_real_distribution<double> &rng);

  virtual void input_fired(int32_t time, double incoming_output);
  virtual void output_fired(int32_t time, double delta);
  virtual void error_fired(int32_t time, double error);
  void try_update_deltas(int32_t time);

  virtual int32_t get_number_weights() const;

  virtual void get_weights(vector<double> &parameters) const;
  virtual void set_weights(const vector<double> &parameters);

  virtual void get_weights(int32_t &offset, vector<double> &parameters) const;
  virtual void set_weights(int32_t &offset, const vector<double> &parameters);

  void set_pi(const vector<double> &new_pi);

  virtual void get_gradients(vector<double> &gradients);
  virtual void reset(int32_t _series_length);
  virtual void write_to_stream(ostream &out);

  virtual RNN_Node_Interface *copy() const;

  friend class RNN_Edge;
};

#endif
