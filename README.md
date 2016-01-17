
The FS Functional STRIPS planner
=================================

`FS` is a classical planner that accepts a subset of the Functional STRIPS planning language, along with a number
of other extensions such as state constraints, a limited library of global constraints and the possibility of using external procedures.

-- Guillem Francès <guillem.frances@upf.edu>

Installation
--------------
In order to install and run the `FS` planner, you need the following software components:

1. The [LAPKT Planning Toolkit](http://lapkt.org/), which provides the base search algorithms used with our heuristics.

1. A [custom version](https://bitbucket.org/gfrances/downward-aig) of the Fast Downward PDDL 3.0 parser (written in Python), modified with the purpose of fully supporting the functional capabilities of the language and allowing for constraints and external procedures to be used on the specification of the domain.

1. The [Gecode](http://www.gecode.org/) CSP Solver (Tested with version 4.4.0 only). The recommended way to install it is on `~/local`, i.e. by running `./configure --prefix=~/local` before the actual compilation.

1. The Clingo ASP Solver, from [Potassco](http://potassco.sourceforge.net/), the Potsdam Answer Set Solving Collection.


Once you have installed these projects locally, your system needs to be configured with the following environment variables, e.g. by setting them up in your  `~/.bashrc` configuration file:


```
#!shell
export LAPKT_PATH="${HOME}/projects/code/lapkt"
export FD_AIG_PATH="${HOME}/projects/code/downward/downward-aig"
export FS0_PATH="${HOME}/projects/code/fs0"
export CLINGO_PATH="${HOME}/lib/clingo-4.5.4-source"

# Local C++ library installations
export LD_LIBRARY_PATH=$LIBRARY_PATH:/usr/local/lib
if [[ -d ${HOME}/local/lib ]]; then
	export LIBRARY_PATH=$LIBRARY_PATH:${HOME}/local/lib
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${HOME}/local/lib
fi
if [[ -d ${HOME}/local/include ]]; then
	export CPATH=$CPATH:${HOME}/local/include
fi

# AIG Fast Downward PDDL Parser
if [[ -d ${FD_AIG_PATH}/src/translate ]]; then
	export PYTHONPATH="${FD_AIG_PATH}/src/translate:$PYTHONPATH"
fi

# Clingo C++ Library
if [[ -d ${CLINGO_PATH}/build/release ]]; then
	export LIBRARY_PATH=$LIBRARY_PATH:${CLINGO_PATH}/build/release
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${CLINGO_PATH}/build/release
fi

```

Once all this is set up, you can build the `FS0` library by doing

```
#!shell
cd $FS0_PATH
scons
```

You can run `scons debug=1` to build the debug version of the library, or `scons debug=1 fdebug=1` to build an extremely-verbose debug version.


Solving planning instances
----------------------------------

The actual process of solving a planning problem involves a preprocessing phase in which a Python script is run to parse a PDDL 3.0 problem specification and generate certain data, as well as a bunch of C++ classes that need to be compiled against the `FS0` main library. The main Python preprocessing script is `$FS0_PATH/preprocessor/generator`.
You can bootstrap the whole process by running e.g. (replace `$BENCHMARKS` by an appropriate directory):

```
#!shell
python3 generator.py --set test --instance $BENCHMARKS/fn-simple-sokoban/instance_6.pddl
```

Where `instance_6` is a PDDL3.0 instance file, and 
`test` is an arbitrary name that will be used to determine the output directory where the executable solver will be left, which in this case will be
`$FS0_PATH/generated/test/fn-simple-sokoban/instance_6`.
In order to solve the instance, we need to run the automatically generated `solver.bin` executable from that directory (add the `-h` flag for further options :

```
#!shell
cd $FS0_PATH/generated/test/fn-simple-sokoban/instance_6
./solver.bin
```

Note that only the non-debug executable is built by default, but you can run `scons debug=1` or `scons edebug=1` from the previous directory to generate debug solvers as well.


