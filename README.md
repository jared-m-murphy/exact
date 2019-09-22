
# Getting Started and Prerequisites

EXONA has been developed to compile using CMake, which should be installed before attempting to compile. To use the MPI version, a version of MPI (such as OpenMPI) should be installed. EXACT currently requires libtiff and libpng
The EXACT algorithm can also checkpoint to a database, however this is not required.  To enable this I recommend installing libmysql-dev via apt-get on Linux systems, or mysql via [homebrew](https://brew.sh) on OSX.  Other than that, EXACT/EXALT/EXAMM has no prerequesites other than c++11 compatible compiler.

To build:

```
~/exact $ mkdir build
~/exact $ cd build
~/exact/build $ cmake ..
~/exact/build $ make
```

You may also want to have graphviz installed so you can generate images of the evolved neural networks.  EXACT/EXALT/EXAMM will write out evolved genomes in a .gv (graphviz) format for this. For example, can generate a pdf from a gv file (assuming graphviz is installed with):

```
$ dot -Tpdf genome.gv -o genome.pdf
```

# EXAMM: Evolutionary eXploration of Augmenting Memory Models and EXALT: Evolutionary eXploration of Augmenting LSTM Topologies

Source code for EXALT/EXAMM can be found in the rnn subdirectory. EXALT has been enhanced with the ability to utilize more recurrent memory cells and has been renamed EXAMM.  The memory cells currently implemented are Delta-RNN, GRU, LSTM, MGU, and UGRNNs. Some example time series data has been provided as part of two publications on EXALT and EXAMM, which also provide implementation details:

1. Alex Ororbia, AbdElRahman ElSaid, and Travis Desell. **Investigating Recurrent Neural Network Memory Structures using Neuro-Evolution.** *The Genetic and Evolutionary Computation Conference (GECCO 2019).* Prague, Czech Republic. July 8-12, 2019. [link](https://dl.acm.org/citation.cfm?id=3321795)

2. AbdElRahman ElSaid, Steven Benson, Shuchita Patwardhan, David Stadem and Travis Desell. **Evolving Recurrent Neural Networks for Time Series Data Prediction of Coal Plant Parameters.** *The 22nd International Conference on the Applications of Evolutionary Computation (EvoStar: EvoApps 2019).* Leipzig, Germany. April 24-26, 2019. [link](https://link.springer.com/chapter/10.1007/978-3-030-16692-2_33)

These datasets can be found in the *datasets* directory, and provide example CSV files which you can use with EXAMM. EXAMM can be run in two different ways, a multithreaded version:

```
./multithreaded/examm_mt --number_threads 9 --training_filenames ../datasets/2018_coal/burner_[0-9].csv --test_filenames ../datasets/2018_coal/burner_1[0-1].csv --time_offset 1 --input_parameter_names Conditioner_Inlet_Temp Conditioner_Outlet_Temp Coal_Feeder_Rate Primary_Air_Flow Primary_Air_Split System_Secondary_Air_Flow_Total Secondary_Air_Flow Secondary_Air_Split Tertiary_Air_Split Total_Comb_Air_Flow Supp_Fuel_Flow Main_Flm_Int --output_parameter_names Main_Flm_Int --number_islands 10 --population_size 10 --max_genomes 2000 --bp_iterations 10 --output_directory "./test_output" --possible_node_types simple UGRNN MGU GRU delta LSTM
```

And a parallel version using MPI:

```
~/exact/build/ $ mpirun -np 9 ./mpi/examm_mpi --training_filenames ../datasets/2018_coal/burner_[0-9].csv \ 
--test_filenames ../datasets/2018_coal/burner_1[0-1].csv --time_offset 1 \
--input_parameter_names Conditioner_Inlet_Temp Conditioner_Outlet_Temp Coal_Feeder_Rate Primary_Air_Flow Primary_Air_Split System_Secondary_Air_Flow_Total Secondary_Air_Flow Secondary_Air_Split Tertiary_Air_Split Total_Comb_Air_Flow Supp_Fuel_Flow Main_Flm_Int --output_parameter_names Main_Flm_Int --number_islands 10 --population_size 10 --max_genomes 2000 --bp_iterations 10 --output_directory "./test_output" --possible_node_types simple UGRNN MGU GRU delta LSTM
```

Which will run EXAMM with 11 threads or 11 processes, respectively. Note that EXAMM uses one thread/process as the master and this typically just waits on the results of backprop so you if you have 8 processors/cores available you can usually run EXAMM with 9 processes/threads for better performance. A performance log of RNN fitnesses will be exported into fitness_log.csv, as well as the best found RNNs into the specified output directory, in this case *./test_output*.

The aviation data can be run similarly, however it the data should be normalized first (which can be done with the *--normalize* command line parameter), e.g.:

```
./multithreaded/examm_mt --number_threads 9 --training_filenames ../datasets/2018_ngafid/flight_[0-7].csv --test_filenames ../datasets/2018_ngafid/flight_[8-9].csv --time_offset 1 --input_parameter_names "AltAGL" "E1 CHT1" "E1 CHT2" "E1 CHT3" "E1 CHT4" "E1 EGT1" "E1 EGT2" "E1 EGT3" "E1 EGT4" "E1 OilP" "E1 OilT" "E1 RPM" "FQtyL" "FQtyR" "GndSpd" "IAS" "LatAc" "NormAc" "OAT" "Pitch" "Roll" "TAS" "volt1" "volt2" "VSpd" "VSpdG" --output_parameter_names Pitch --number_islands 10 --population_size 10 --max_genomes 2000 --bp_iterations 10 --output_directory "./test_output" --possible_node_types simple UGRNN MGU GRU delta LSTM --normalize
```

The *--time_offset* parameter specifies how many time steps in the future EXAMM should predict for the output parameter(s). 

# EXACT: Evolutionary Exploration of Augmenting Convolutional Topologies

This repository provides source code for the Evolutionary Exploration of Augmenting Convolutional Topologies algorithm.  This algorithm progressively evolves convolutional neural networks for image classification problems.  The algorithm is asychronous, which allows for easy multithreading and parallelization. Code is provided for running EXACT as a BOINC volunteer computing project, on a cluster or supercomputer using MPI or on a desktop or laptop using threads.

We've built and run EXACT on both an Ubuntu Linux high performance computing cluser and OSX laptops and desktops. We have not tried it on Windows.  If there are issues with the CMake scripts please let us know and we'll update them.

If you want to set this up on your own BOINC volunteer computing project, we recommend sending us an email as this is a rather complicated process.

## Setting up Training and Testing Data

This version EXACT is set up to run using the [MNIST Handwritten Digits Dataset](http://yann.lecun.com/exdb/mnist/). However it expects a different data format where the images and labels are combined.  You can download the data and convert it as follows:

```
~/exact $ mkdir datasets
~/exact $ cd datasets

~/exact/datasets $ wget http://yann.lecun.com/exdb/mnist/train-images-idx3-ubyte.gz
~/exact/datasets $ wget http://yann.lecun.com/exdb/mnist/train-labels-idx1-ubyte.gz
~/exact/datasets $ wget http://yann.lecun.com/exdb/mnist/t10k-images-idx3-ubyte.gz
~/exact/datasets $ wget http://yann.lecun.com/exdb/mnist/t10k-labels-idx1-ubyte.gz

~/exact/datasets $ unzip train-images-idx3-ubyte.gz
~/exact/datasets $ unzip train-labels-idx1-ubyte.gz
~/exact/datasets $ unzip t10k-images-idx3-ubyte.gz
~/exact/datasets $ unzip t10k-labels-idx1-ubyte.gz

~/exact/datasets $ cd ../build
~/exact/build $ ./image_tools/convert_mnist_data ../datasets/train-images-idx3-ubyte ../datasets/train-labels-idx1-ubyte ../datasets/mnist_training_data.bin 60000
~/exact/build $ ./image_tools/convert_mnist_data ../datasets/t10k-images-idx3-ubyte ../datasets/t10k-labels-idx1-ubyte ../datasets/mnist_testing_data.bin 10000
```

## Running EXACT


## Example Genomes from GECCO 2017

Our submission to GECCO describes a set of best found genomes for the MNIST handwritten digits dataset.  These can be found in the genomes subdirectory of the project. Please checkout the tag for the GECCO paper to use the version of EXACT these CNN genome files were generated with:

```
git checkout -b exact_gecco gecco_2017
```

After compiling this version and setting up the MNIST training and testing data as described in the previous section, these genomes can be run over the training and testing data for validation as follows:

```
~/exact/build/ $ ./tests/evaluate_cnn --training_data ../datasets/mnist_training_data.bin --testing_data ../datasets/mnist_testing_data.bin --genome_file ../genomes/genome_46823
~/exact/build/ $ ./tests/evaluate_cnn --training_data ../datasets/mnist_training_data.bin --testing_data ../datasets/mnist_testing_data.bin --genome_file ../genomes/genome_57302
~/exact/build/ $ ./tests/evaluate_cnn --training_data ../datasets/mnist_training_data.bin --testing_data ../datasets/mnist_testing_data.bin --genome_file ../genomes/genome_59455
~/exact/build/ $ ./tests/evaluate_cnn --training_data ../datasets/mnist_training_data.bin --testing_data ../datasets/mnist_testing_data.bin --genome_file ../genomes/genome_59920
```
