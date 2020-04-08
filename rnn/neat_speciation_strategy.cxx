#include <functional>
using std::function;

#include <chrono>

//#include <iostream>

#include <random>

using std::minstd_rand0;
using std::uniform_real_distribution;

#include <string>
using std::string;

#include "examm.hxx"
#include "rnn_genome.hxx"
#include "neat_speciation_strategy.hxx"

#include "common/log.hxx"

/**
 * 
 */
NeatSpeciationStrategy::NeatSpeciationStrategy(
                double _mutation_rate, double _intra_island_crossover_rate, 
                double _inter_island_crossover_rate, RNN_Genome *_seed_genome,
                int32_t _max_genomes) :
                        generation_species(0),
                        mutation_rate(_mutation_rate), 
                        intra_island_crossover_rate(_intra_island_crossover_rate), 
                        inter_island_crossover_rate(_inter_island_crossover_rate), 
                        generated_genomes(0),
                        inserted_genomes(0), 
                        species_count(0),
                        minimal_genome(_seed_genome), 
                        max_genomes(_max_genomes) {

    double rate_sum = mutation_rate + intra_island_crossover_rate + inter_island_crossover_rate;
    if (rate_sum != 1.0) {
        mutation_rate = mutation_rate / rate_sum;
        intra_island_crossover_rate = intra_island_crossover_rate / rate_sum;
        inter_island_crossover_rate = inter_island_crossover_rate / rate_sum;
    }

    intra_island_crossover_rate += mutation_rate;
    inter_island_crossover_rate += intra_island_crossover_rate;

    // for (int32_t i = 0; i < number_of_islands; i++) {
    //     Neat_Species.push_back(new Species(i));
    // }
    
    //set the generation id for the initial minimal genome
    minimal_genome->set_generation_id(generated_genomes);
    generated_genomes++;

    // set the fitst species with minimal genome
    Neat_Species.push_back(new Species(species_count)); 
    Log::info("initialized the first species, current neat species size: %d \n", Neat_Species.size() );
    species_count++;
    insert_genome(minimal_genome);

    global_best_genome = NULL;
}

/**
 * 
 */


int32_t NeatSpeciationStrategy::get_generated_genomes() const {
    return generated_genomes;
}

int32_t NeatSpeciationStrategy::get_inserted_genomes() const {
    return inserted_genomes;
}

RNN_Genome* NeatSpeciationStrategy::get_best_genome() {
    int32_t best_genome_species = -1;
    double best_fitness = EXAMM_MAX_DOUBLE;

    for (int32_t i = 0; i < (int32_t)Neat_Species.size(); i++) {
        if (Neat_Species[i]->size() > 0) {
            double island_best_fitness = Neat_Species[i]->get_best_fitness();
            if (island_best_fitness <= best_fitness) {
                best_fitness = island_best_fitness;
                best_genome_species = i;
            }
        }
    }
    if (best_genome_species < 0)  return NULL;
     else {
        return Neat_Species[best_genome_species]->get_best_genome();
    }
}

RNN_Genome* NeatSpeciationStrategy::get_worst_genome() {
    int32_t worst_genome_species = -1;
    double worst_fitness = -EXAMM_MAX_DOUBLE;

    for (int32_t i = 0; i < (int32_t)Neat_Species.size(); i++) {
        if (Neat_Species[i]->size() > 0) {
            double island_worst_fitness = Neat_Species[i]->get_worst_fitness();
            if (island_worst_fitness > worst_fitness) {
                worst_fitness = island_worst_fitness;
                worst_genome_species = i;
            }
        }
    }

    if (worst_genome_species < 0) {
        return NULL;
    } else {
        return Neat_Species[worst_genome_species]->get_worst_genome();
    }
}


double NeatSpeciationStrategy::get_best_fitness() {
    RNN_Genome *best_genome = get_best_genome();
    if (best_genome == NULL) return EXAMM_MAX_DOUBLE;
    else return best_genome->get_fitness();
}

double NeatSpeciationStrategy::get_worst_fitness() {
    RNN_Genome *worst_genome = get_worst_genome();
    if (worst_genome == NULL) return EXAMM_MAX_DOUBLE;
    else return worst_genome->get_fitness();
}


//this will insert a COPY, original needs to be deleted
//returns 0 if a new global best, < 0 if not inserted, > 0 otherwise
int32_t NeatSpeciationStrategy::insert_genome(RNN_Genome* genome) {
    bool inserted = false;
    int32_t insert_position;
    Log::debug("inserting genome!\n");

    inserted_genomes++;

    vector<int32_t> species_list = get_random_species_list();
    // if (species_list.size() == 0) {
    //     insert_position = Neat_Species[0]
    // }
    for (int i = 0; i < species_list.size(); i++){
        Species* random_species = Neat_Species[species_list[i]];
        if (Neat_Species[species_list[i]] -> size() == 0) {
            insert_position = Neat_Species[species_list[i]]->insert_genome(genome);
            break;
        }
        else {
            RNN_Genome* genome_representation = random_species->get_latested_genome();
            double distance = get_distance(genome_representation, genome);
            if (distance < distance_th) {
                Log::info("inserting genome to species: %d\n", species_list[i]);
                insert_position = Neat_Species[species_list[i]]->insert_genome(genome);
                inserted = true;
                break;
            }
        }

    }
    //1. get random island
    //2. get last genome
    //3. get distance
    //4. insert or make new species
    if (!inserted) {
        Species* new_species = new Species(species_count);
        species_count++;
        Neat_Species.push_back(new_species);
        insert_position = new_species->insert_genome(genome);
    }

    if (insert_position == 0) {
        //check and see if the inserted genome has the same fitness as the best fitness
        //of all islands
        double best_fitness = get_best_fitness();
        if (genome->get_fitness() == best_fitness) return 0;
        else return 1; //was the best for the island but not the global best
    } else {
        return insert_position; //will be -1 if not inserted, or > 0 if not the global best
    }
}


RNN_Genome* NeatSpeciationStrategy::generate_genome(uniform_real_distribution<double> &rng_0_1, minstd_rand0 &generator, function<void (int32_t, RNN_Genome*)> &mutate, function<RNN_Genome* (RNN_Genome*, RNN_Genome *)> &crossover) {
    //generate the genome from the next island in a round
    //robin fashion.
    RNN_Genome *genome = NULL;

    Log::debug("getting species: %d\n", generation_species);
    Species *currentSpecies = Neat_Species[generation_species];

    // Log::info("generating new genome for island[%d], island_size: %d, max_island_size: %d, mutation_rate: %lf, intra_island_crossover_rate: %lf, inter_island_crossover_rate: %lf\n", generation_island, island->size(), max_island_size, mutation_rate, intra_island_crossover_rate, inter_island_crossover_rate);

    // Log::debug("islands.size(): %d, selected island is null? %d\n", islands.size(), (island == NULL));

    if (currentSpecies->size() <= 1) {
        Log::info("current species has less than 2 genomes, doing mutation!\n");

        while (genome == NULL) {
            currentSpecies->copy_random_genome(rng_0_1, generator, &genome);

            //TODO: make max_mutations an IslandSpeciationStrategy option
            mutate(1 /*max_mutations*/, genome);

            if (genome->outputs_unreachable()) {
                //no path from at least one input to the outputs
                delete genome;
                genome = NULL;
            }

            genome->initialize_randomly();
            generated_genomes++;
            genome->set_generation_id(generated_genomes);
            genome->set_group_id(generation_species);

            generation_species++;
            if (generation_species >= (signed) Neat_Species.size()) generation_species = 0; 

            return genome;
        }

    } else {
        //generate a genome via crossover or mutation
        Log::info("current species size %d, doing mutaion or crossover\n", currentSpecies->size());

        while (genome == NULL) {
            genome = generate_for_species(rng_0_1, generator, mutate, crossover);
        }

    }
    if (genome != NULL) { 
        //set th generation id and increment generated genomes
        generated_genomes++;
        genome->set_generation_id(generated_genomes);
        genome->set_group_id(generation_species);

        //set the island for the genome and increment to the next island
        generation_species++;
        if (generation_species >= (signed) Neat_Species.size()) generation_species = 0; 
        // Neat_Species[generation_species] -> set_latest_generation_id(generated_genomes);    

    } else {
        Log::fatal("ERROR: genome was NULL at the end of generate genome!\n");
        Log::fatal( "This should never happen.\n");
        exit(1);
    }

    return genome;
}

RNN_Genome* NeatSpeciationStrategy::generate_for_species(uniform_real_distribution<double> &rng_0_1, minstd_rand0 &generator, function<void (int32_t, RNN_Genome*)> &mutate, function<RNN_Genome* (RNN_Genome*, RNN_Genome *)> &crossover) {
    //if we haven't filled ALL of the island populations yet, only use mutation
    //otherwise do mutation at %, crossover at %, and island crossover at %
    Species *currentSpecies = Neat_Species[generation_species];
    RNN_Genome* genome;
    double r = rng_0_1(generator);
    if ( r < mutation_rate) {
        Log::info("performing mutation\n");

        currentSpecies->copy_random_genome(rng_0_1, generator, &genome);

        //TODO: make max_mutations an IslandSpeciationStrategy option
        mutate(1 /*max_mutations*/, genome);

    } else if (r < intra_island_crossover_rate || Neat_Species.size() == 1) {
        //intra-island crossover
        Log::info("performing intra-island crossover\n");

        //select two distinct parent genomes in the same island
        RNN_Genome *parent1 = NULL, *parent2 = NULL;
        currentSpecies->copy_two_random_genomes(rng_0_1, generator, &parent1, &parent2);

        genome = crossover(parent1, parent2);
        delete parent1;
        delete parent2;
    } else {
        //inter-island crossover
        // Log::info("performing inter-island crossover\n");

        //get a random genome from this island
        RNN_Genome *parent1 = NULL; 
        currentSpecies->copy_random_genome(rng_0_1, generator, &parent1);

        //select a different island randomly
        int32_t other_island = rng_0_1(generator) * (Neat_Species.size() - 1);
        if (other_island >= generation_species) other_island++;

        //get the best genome from the other island
        RNN_Genome *parent2 = Neat_Species[other_island]->get_best_genome()->copy(); // new RNN GENOME

        //swap so the first parent is the more fit parent
        if (parent1->get_fitness() > parent2->get_fitness()) {
            RNN_Genome *tmp = parent1;
            parent1 = parent2;
            parent2 = tmp;
        }

        genome = crossover(parent1, parent2); // new RNN GENOME
        delete parent1;
        delete parent2;
    }
    
    if (genome->outputs_unreachable()) {
        //no path from at least one input to the outputs
        delete genome;
        genome = NULL;
    }
    return genome;
}

void NeatSpeciationStrategy::print(string indent) const {
    Log::info("%NEAT Species: \n", indent.c_str());
    for (int32_t i = 0; i < (int32_t)Neat_Species.size(); i++) {
        Log::info("%sSpecies %d:\n", indent.c_str(), i);
        Neat_Species[i]->print(indent + "\t");
    }
}

/**
 * Gets speciation strategy information headers for logs
 */
string NeatSpeciationStrategy::get_strategy_information_headers() const {
    string info_header="";
    for (int32_t i = 0; i < (int32_t)Neat_Species.size(); i++) {
        info_header.append(",");
        info_header.append("Species_");
        info_header.append(to_string(i));
        info_header.append("_best_fitness");
        info_header.append(",");
        info_header.append("Species_");
        info_header.append(to_string(i));
        info_header.append("_worst_fitness");
    }
    return info_header;
}

/**
 * Gets speciation strategy information values for logs
 */
string NeatSpeciationStrategy::get_strategy_information_values() const {
    string info_value="";
    for (int32_t i = 0; i < (int32_t)Neat_Species.size(); i++) {
        double best_fitness = Neat_Species[i]->get_best_fitness();
        double worst_fitness = Neat_Species[i]->get_worst_fitness();
        info_value.append(",");
        info_value.append(to_string(best_fitness));
        info_value.append(",");
        info_value.append(to_string(worst_fitness));
    }
    return info_value;
}

RNN_Genome* NeatSpeciationStrategy::get_global_best_genome() {
    return global_best_genome;
}

vector<int32_t> NeatSpeciationStrategy::get_random_species_list() {

    vector<int32_t> species_list;
    for (int i = 0; i < Neat_Species.size(); i++) {
        species_list.push_back(i);
    }
    std::random_shuffle ( species_list.begin(), species_list.end() );
    Log::info("species shuffle list: \n");
    for (std::vector<int32_t>::iterator it=species_list.begin(); it!=species_list.end(); ++it) 
        std::cout << ' ' << *it;
    std::cout << '\n';
    return species_list;
}

double NeatSpeciationStrategy::get_distance(RNN_Genome* g1, RNN_Genome* g2) {
    double distance = 5;
    double c1 = 1;
    double c2 = 1;
    int32_t N;

    vector<int32_t> innovation1 = g1 -> get_innovation_list();
    vector<int32_t> innovation2 = g2 -> get_innovation_list();
    if (innovation1.size() >= innovation2.size()){
        N = innovation1.size();
    }
    else {
        N = innovation2.size();
    }

    

    return distance;

}
