#ifndef EXAMM_ISLAND_SPECIATION_STRATEGY_HXX
#define EXAMM_ISLAND_SPECIATION_STRATEGY_HXX

#include <functional>
using std::function;

#include <random>
using std::minstd_rand0;
using std::uniform_real_distribution;


#include <string>
using std::string;

#include "island.hxx"
#include "rnn_genome.hxx"
#include "speciation_strategy.hxx"

class IslandSpeciationStrategy : public SpeciationStrategy {
    private:
        int32_t generation_island; /**< Used to track which island to generate the next genome from. */

        int32_t number_of_islands; /**< the number of islands to have. */

        int32_t max_island_size; /**< the maximum number of genomes in an island. */

        double mutation_rate; /**< How frequently to do mutations. Note that mutation_rate + intra_island_crossover_rate + inter_island_crossover_rate should equal 1, if not they will be scaled down such that they do. */
        double intra_island_crossover_rate; /**< How frequently to do intra-island crossovers. Note that mutation_rate + intra_island_crossover_rate + inter_island_crossover_rate should equal 1, if not they will be scaled down such that they do. */
        double inter_island_crossover_rate; /**< How frequently to do inter-island crossovers. Note that mutation_rate + intra_island_crossover_rate + inter_island_crossover_rate should equal 1, if not they will be scaled down such that they do. */

        int32_t generated_genomes; /**< How many genomes have been generated by this speciation strategy. */
        int32_t inserted_genomes; /**< How many genomes have been inserted into this speciatoin strategy. */


        RNN_Genome *minimal_genome; /**< keep a reference to a minimal genome so we can re-use it across islands and not duplicate innovation numbers. */

        /**
         * All the islands which contain the genomes for this speciation strategy.
         */ 
        vector<Island*> islands;
    
    public:
        //static void register_command_line_arguments();
        //static IslandSpeciationStrategy* generate_from_command_line();

        /**
         * Creates a new IslandSpeciationStrategy.
         *
         * \param number_of_islands specifies how many islands it will us e
         * \param max_island_size specifies the maximum number of gneomes in an island
         */
        IslandSpeciationStrategy(int32_t _number_of_islands, int32_t _max_island_size, double _mutation_rate, double _intra_island_crossover_rate, double _inter_island_crossover_rate, RNN_Genome *seed_genome);

        /**
         * \return the number of generated genomes.
         */
        int32_t get_generated_genomes() const;

        /**
         * \return the number of inserted genomes.
         */
        int32_t get_inserted_genomes() const;

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
         * \return the best genome of all islands or NULL if no genomes have yet been inserted
         */
        RNN_Genome* get_best_genome();

        /**
         * Gets the the worst genome of all the islands
         * \return the worst genome of all islands or NULL if no genomes have yet been inserted
         */
        RNN_Genome* get_worst_genome();

        /**
         *  \return true if all the islands are full
         */
        bool islands_full() const;

        /**
         * Inserts a <b>copy</b> of the genome into one of the islands handled by this
         * strategy, determined by the RNN_Genome::get_group_id() method.
         *
         * The caller of this method will need to free the memory of the genome passed
         * into this method.
         *
         * \param genome is the genome to insert.
         * \return a value < 0 if the genome was not inserted, 0 if it was a new best genome
         * for all the islands, or > 0 otherwise.
         */
        int32_t insert_genome(RNN_Genome* genome);

        /**
         * Generates a new genome.
         *
         * \param rng_0_1 is the random number distribution that generates random numbers between 0 (inclusive) and 1 (non=inclusive).
         * \param generator is the random number generator
         * \param mutate is the a function which performs a mutation on a genome
         * \param crossover is the function which performs crossover between two genomes
         *
         * \return the newly generated genome.
         */
        RNN_Genome* generate_genome(uniform_real_distribution<double> &rng_0_1, minstd_rand0 &generator, function<void (int32_t, RNN_Genome*)> &mutate, function<RNN_Genome* (RNN_Genome*, RNN_Genome *)> &crossover);

        /**
         * Prints out all the island's populations
         * 
         * \param indent is how much to indent what is printed out
         */
        void print(string indent = "") const;
};


#endif

