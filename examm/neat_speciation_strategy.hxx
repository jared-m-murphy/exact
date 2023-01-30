#ifndef EXAMM_NEAT_SPECIATION_STRATEGY_HXX
#define EXAMM_NEAT_SPECIATION_STRATEGY_HXX

#include <functional>
using std::function;

#include <string>
using std::string;

#include <random>
using std::minstd_rand0;
using std::uniform_real_distribution;

#include "rnn/rnn_genome.hxx"
#include "speciation_strategy.hxx"
#include "species.hxx"

class NeatSpeciationStrategy : public SpeciationStrategy {
   private:
    int32_t generation_species; /**< Used to track which island to generate the next genome from. */
    int32_t species_count;      /** < Used to keep track of the number of species generated . */
    int32_t population_not_improving_count;
    double species_threshold;
    double fitness_threshold;
    double neat_c1;
    double neat_c2;
    double neat_c3;
    double mutation_rate; /**< How frequently to do mutations. Note that mutation_rate + intra_island_crossover_rate +
                             inter_island_crossover_rate should equal 1, if not they will be scaled down such that they
                             do. */
    double intra_island_crossover_rate; /**< How frequently to do intra-island crossovers. Note that mutation_rate +
                                           intra_island_crossover_rate + inter_island_crossover_rate should equal 1, if
                                           not they will be scaled down such that they do. */
    double inter_island_crossover_rate; /**< How frequently to do inter-island crossovers. Note that mutation_rate +
                                           intra_island_crossover_rate + inter_island_crossover_rate should equal 1, if
                                           not they will be scaled down such that they do. */

    int32_t generated_genomes; /**< How many genomes have been generated by this speciation strategy. */
    int32_t evaluated_genomes; /**< How many genomes have been inserted into this speciatoin strategy. */

    RNN_Genome* seed_genome; /**< keep a reference to a minimal genome so we can re-use it across islands and not
                                duplicate innovation numbers. */

    vector<Species*> Neat_Species;
    RNN_Genome* global_best_genome;

   public:
    NeatSpeciationStrategy(
        double _mutation_rate, double _intra_island_crossover_rate, double _inter_island_crossover_rate,
        RNN_Genome* _seed_genome, double _species_threshold, double _fitness_threshold, double _neat_c1,
        double _neat_c2, double _neat_c3
    );
    /**
     * \return the number of generated genomes.
     */
    int32_t get_generated_genomes() const;

    /**
     * \return the number of inserted genomes.
     */
    int32_t get_evaluated_genomes() const;

    /**
     * Gets the fitness of the best genome of all the islands
     * \return the best fitness over all islands
     */
    double get_best_fitness();

    /**
     * Gets the fitness of the worst genome of all the islands
     * \return the worst fitness over all islands
     */
    double get_worst_fitness();

    /**
     * Gets the best genome of all the islands
     * \return the best genome of all islands
     */
    RNN_Genome* get_best_genome();

    /**
     * Gets the the worst genome of all the islands
     * \return the worst genome of all islands
     */
    RNN_Genome* get_worst_genome();

    /**
     * Inserts a <b>copy</b> of the genome into this speciation strategy.
     *
     * The caller of this method will need to free the memory of the genome passed
     * into this method.
     *
     * \param genome is the genome to insert.
     * \return a value < 0 if the genome was not inserted, 0 if it was a new best genome
     * or > 0 otherwise.
     */
    int32_t insert_genome(RNN_Genome* genome);

    /**
     * Generates a new genome.
     *
     * \param rng_0_1 is the random number distribution that generates random numbers between 0 (inclusive) and 1
     * (non=inclusive). \param generator is the random number generator \param mutate is the a function which performs a
     * mutation on a genome \param crossover is the function which performs crossover between two genomes
     *
     * \return the newly generated genome.
     */
    RNN_Genome* generate_genome(
        uniform_real_distribution<double>& rng_0_1, minstd_rand0& generator,
        function<void(int32_t, RNN_Genome*)>& mutate, function<RNN_Genome*(RNN_Genome*, RNN_Genome*)>& crossover
    );

    RNN_Genome* generate_for_species(
        uniform_real_distribution<double>& rng_0_1, minstd_rand0& generator,
        function<void(int32_t, RNN_Genome*)>& mutate, function<RNN_Genome*(RNN_Genome*, RNN_Genome*)>& crossover
    );

    /**
     * Prints out all the island's populations
     *
     * \param indent is how much to indent what is printed out
     */
    void print(string indent = "") const;

    /**
     * Gets speciation strategy information headers for logs
     */
    string get_strategy_information_headers() const;

    /**
     * Gets speciation strategy information values for logs
     */
    string get_strategy_information_values() const;

    RNN_Genome* get_global_best_genome();

    vector<int32_t> get_random_species_list();

    double get_distance(RNN_Genome* g1, RNN_Genome* g2);

    int32_t get_exceed_number(vector<int32_t> v1, vector<int32_t> v2);

    void rank_species();

    bool check_population();

    void check_species();
    void initialize_population(function<void(int32_t, RNN_Genome*)>& mutate);
    RNN_Genome* get_seed_genome();
};

#endif
