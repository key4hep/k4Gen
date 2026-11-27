#!/usr/bin/env python3
"""Check that an EDM4hep file contains the generator event weights: every event
has an EventHeader with weights[0] equal to weight, and the metadata holds the
matching EventWeightNames and the generator tool info.

Usage: check_event_weights.py <edm4hep_file> <run_number> <generator_name>
"""

import sys

from podio import reading


def fail(msg):
    print(f"check_event_weights: FAILURE: {msg}")
    sys.exit(1)


def as_list(value):
    # podio returns a single value for a vector parameter with one entry, strings possibly as bytes
    values = [value] if isinstance(value, (str, bytes)) else list(value)
    return [v.decode() if isinstance(v, bytes) else v for v in values]


file_name, run_number, generator = sys.argv[1], int(sys.argv[2]), sys.argv[3]
reader = reading.get_reader(file_name)

n_events = 0
n_weights = 0
for frame in reader.get("events"):
    header = frame.get("EventHeader")[0]
    weights = list(header.getWeights())
    if len(weights) < 2:
        fail(f"event {n_events}: weights vector has size {len(weights)}, expected > 1")
    if weights[0] != header.getWeight():
        fail(f"event {n_events}: weights[0] = {weights[0]} != EventHeader.weight = {header.getWeight()}")
    if header.getRunNumber() != run_number:
        fail(f"event {n_events}: runNumber = {header.getRunNumber()}, expected {run_number}")
    if header.getEventNumber() != n_events:
        fail(f"event {n_events}: eventNumber = {header.getEventNumber()}, expected {n_events}")
    n_weights = len(weights)
    n_events += 1

if n_events == 0:
    fail("no events in file")

metadata = reader.get("metadata")[0]
names = as_list(metadata.get_parameter("EventWeightNames"))
if len(names) != n_weights:
    fail(f"EventWeightNames has {len(names)} entries but the weights vector has {n_weights}")

tools = as_list(metadata.get_parameter("GeneratorToolNames"))
versions = as_list(metadata.get_parameter("GeneratorToolVersions"))
if generator not in tools:
    fail(f"GeneratorToolNames {tools} does not contain {generator}")
if len(versions) != len(tools):
    fail(f"GeneratorToolVersions has {len(versions)} entries but GeneratorToolNames has {len(tools)}")

print(f"check_event_weights: OK: {n_events} events x {n_weights} weights, {len(names)} names, tools {tools}")
