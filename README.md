# Homomorphic Tree Evaluator
The aim of this project is to implement a client/server architecture such that:
- The client sends **encrypted feature vectors** to the server.
- The server evaluates an encrypted decision tree against these feature vectors. 
- **At no step does the server decrypt the feature vectors from the client.** No information leaks are possible, and everything is highly confidential, since the server cannot see the client's inputs, and the client cannot see the server's decision tree.

## How is that possible?
This is possible using [Homomorphic Encryption](https://en.wikipedia.org/wiki/Homomorphic_encryption).

## How to run
Run the following commands:
- `git clone https://github.com/a3y3/Homomorphic-Tree-Evaluator && cd Homomorphic-Tree-Evaluator`
- `./bootstrap.sh` (Grab a coffee, this might take a while)
- `cd source/`
- `cmake .`
- `make`
- `../deps/bin/HomomorphicTreeEvaluator`

