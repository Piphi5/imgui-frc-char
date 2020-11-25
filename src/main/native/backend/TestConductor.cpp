#include "TestConductor.h"

using namespace frcchar;

void TestConductor::Conduct(const frcchar::Type& type) {
  // This will all be called in a separate thread so this is safe to do.
  while (true) {
    // Ensure we always have a connection.
    // if (!IsRobotConnected())
    //   return Error(true, "The robot disconnected during characterization.");
    
    
  }
}