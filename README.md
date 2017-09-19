# CAPSL
Component Authentication Process for Sandboxed Layouts

This repository holds the sources of the CAPSL tool that can be utilized to automatically generate
a hardware sandbox for an untrusted IP in your hardware system. This repo is currently managed by
Taylor JL Whitaker with the Smart Embedded Systems Lab at the University of Arkansas.


## Background ##
Our tool is capable of detecting trojan activation and nullifying possible damage to a system at run-time, avoiding complex
pre-fabrication and pre-deployment testing for trojans. Our approach captures the behavioral properties of non-trusted IPs,
typically from a third-party or components off the shelf (COTS), with the formalism of Interface Automata (IA) and the Property
Specification Language's sequential extended regular expressions (SERE). Using the concept of hardware sandboxing, we translate
the property specifications to checker automata and partition an untrusted sector of the system, with included virtualized resources
and controllers, to isolate sandbox-system interactions upon deviation from the behavioral checkers. Hardware Sandboxing was initially
verified with an anti-jamming application for UAVs. Our CAPSL design flow has been verified in a number of case-studies with
Trust-Hub.org RTL Trojans serving as simulated threats within higher level systems. These evaluations include resource overhead
studies and general flexibility tests of the underlying formal model and produced reference monitors.


## Dependencies ##
CAPSL utilizes SPOT for translating SERE to automata. SPOT 2.1 is the latest version tested with CAPSL.
SPOT is available at: https://spot.lrde.epita.fr


## About the Code ##
CAPSL is performed with a simple C program. The current functionality is summarized below with potential features interjected.

- Retrieve target IP interface specification files
  - Currently stored as two files (.ia and .sere)
  - TODO: Exchange .ia and .sere for the .config format (See Template.config)
    - TODO: Virtual resource specification and handling needs to be implemented

- Process specification
  - Build Interface Automata models
    - Extract and associate signals, states, and transitions from IA specifications
    - Iteratively add models as automata objects
      - Generate transition table
      - Check against IA automata set for composition compatibility
      - Compose compatible automata, append automata set if incompatible

  - Add SERE constraints
    - Translate SERE to reference monitor automata via SPOT
      - Parse SPOT outputs files (.hoa files) to build automata objects
    - Extract signals, states, transitions
    - Resolve signals against IA models of IP interfaces to revise SERE automata set
    - Iteratively add models as SERE generated automata objects
      - Generate transition table
      - Check against SERE generated automata set for composition compatibility
      - Compose compatible automata, append SERE generated automata set if incompatible

  - TODO: Automata handling is riddled with bugs, this should be rewritten and reorganized ASAP
  - TODO: Formal verification of produced automata models

- Sandbox Generation
  - VHDL Output Target
    - Generate 'Checker.vhd' file with optimized IA and SERE automata sets
      - One hot-encoding FSM generated for each automata
      - Illegal states trigger controller to sever interfaces
        - TODO: Illegal state classification needs revising
      - TODO: Checker generation needs .config type specification file for adding the extra logic in that format

    - Generate 'Controller.vhd' file from interface definitions
      - Connects all IP interfaces to sandbox
      - Routes all signals to 'Checker.vhd'
      - TODO: Automatic 'Controller.vhd' generation needed

    - TODO: TCL Interface for packaging sandboxed IP

  - TODO: SystemC Output Targets


## Issues ##
The CAPSL design flow as implemented as a tool is far from complete. There are currently a number of tasks that are not automated
to the degree they should be. We believe CAPSL has the potential to be a powerful tool, thus we would appreciate all contributions.

If you would like to file an issue, please include relevant specification files to recreate the problem.


## Contact ##
If you would like to help develop and improve CAPSL or are interested in utilizing the tool for research, please consider citing
one of the publications listed on our website below. We appreciate your interest so, please feel free to contact our lab for
any questions you might have!

SmartES Lab CAPSL Project Page:
http://SmartES.uark.edu/Projects/CAPSL
