// Copyright (c) 2018 Graphcore Ltd. All rights reserved.

/* This file contains the completed version of Poplar tutorial 1,
   modified to use the IPU hardware.
   See the Poplar user guide for details.
*/

// reference: https://phabricator.sourcevertex.net/T47896

#include <poplar/DeviceManager.hpp>
#include <poplar/Engine.hpp>
#include <poplar/Graph.hpp>

#include <algorithm>
#include <iostream>

using namespace poplar;

#define SIZE 16

int main() {
  // Create the DeviceManager which is used to discover devices
  auto manager = DeviceManager::createDeviceManager();

  // Attempt to attach to a single IPU:
  auto devices = manager.getDevices(poplar::TargetType::IPU, 1);
  std::cout << "Trying to attach to IPU\n";
  auto it = std::find_if(devices.begin(), devices.end(),
                         [](Device &device) { return device.attach(); });

  if (it == devices.end()) {
    std::cerr << "Error attaching to device\n";
    return 1; // EXIT_FAILURE
  }

  auto device = std::move(*it);
  std::cout << "Attached to IPU " << device.getId() << std::endl;

  Target target = device.getTarget();

  // Create the Graph object
  Graph graph(target);

  // Add variables to the graph
  Tensor v1 = graph.addVariable(FLOAT, {SIZE}, "v1");
  Tensor v2 = graph.addVariable(FLOAT, {SIZE}, "v2");
  Tensor v3 = graph.addVariable(FLOAT, {SIZE}, "v3");
  Tensor v4 = graph.addVariable(FLOAT, {SIZE}, "v4");

  // Spread v1 over tiles 0..3
  for (unsigned i = 0; i < SIZE; ++i) {
    graph.setTileMapping(v1[i], i % 4);
    graph.setTileMapping(v2[i], i % 4);
    graph.setTileMapping(v3[i], i % 4);
    graph.setTileMapping(v4[i], i % 4);
  }

  // Create a control program that is a sequence of steps
  program::Sequence prog;

  // Add data stream for v1
  DataStream v1_stream = graph.addHostToDeviceFIFO("v1-in", FLOAT, SIZE);
  DataStream v2_stream = graph.addDeviceToHostFIFO("v2-out", FLOAT, SIZE);
  DataStream v3_stream = graph.addHostToDeviceFIFO("v3-in", FLOAT, SIZE);
  DataStream v4_stream = graph.addDeviceToHostFIFO("v4-out", FLOAT, SIZE);

  // Add program steps to copy from the stream
  prog.add(program::Copy(v1_stream, v1));
  prog.add(program::Copy(v1, v2));
  prog.add(program::Copy(v2, v2_stream));
  prog.add(program::Sync(poplar::SyncType::EXTERNAL));
  prog.add(program::Copy(v3_stream, v3));
  prog.add(program::Copy(v3, v4));
  prog.add(program::Copy(v4, v4_stream));

  // Create the engine
  Engine engine(graph, prog);
  engine.load(device);

  // Copy host data via the write handle to v3 on the device
  std::vector<float> h1(SIZE, 0);
  std::vector<float> h2(SIZE, 0);
  std::vector<float> h3(SIZE, 0);
  std::vector<float> h4(SIZE, 0);

  for (auto i = 0; i < SIZE; ++i) {
    h1[i] = float(i);
  }

  auto d2h_callback = [&](void *ptr) {
    std::cout << "d2h_callback" << std::endl;
    float *data = reinterpret_cast<float *>(ptr);
    for (int i = 0; i < SIZE; i++) {
      h2[i] = data[i] + 5.0f;
      h3[i] = h2[i];
    }
  };

  auto h2d_callback = [&](void *ptr) {
    std::cout << "h2d_callback" << std::endl;
    float *data = reinterpret_cast<float *>(ptr);
    for (int i = 0; i < SIZE; i++) {
      data[i] = h3[i];
    }
  };

  // Connect v1_stream to h1
  engine.connectStream("v1-in", &h1[0], &h1[SIZE]);

  // Connect v2_stream to d2h_callback
  engine.connectStreamToCallback("v2-out", d2h_callback);

  // engine.connectStreamToCallback("v3-in", h2d_callback);
  engine.connectStream("v3-in", &h3[0], &h3[SIZE]);

  engine.connectStream("v4-out",&h4[0], &h4[SIZE]);

  // Run the control program
  std::cout << "Running program\n";
  engine.run(0);
  std::cout << "Program complete\n";

  // Output the IPU result
  std::cout << "h4 data:\n";
  for (unsigned i = 0; i < SIZE; ++i) {
    std::cout << h4[i] << " ";
  }
  std::cout << std::endl;

  // Output the expected result
  std::cout << "expected:\n";
  for (unsigned i = 0; i < SIZE; ++i) {
    std::cout << h1[i] + 5.0f << " ";
  }
  std::cout << std::endl;

  return 0;
}
