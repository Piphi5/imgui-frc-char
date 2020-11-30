// MIT License

#include <wpi/raw_ostream.h>

#include "backend/DataProcessor.h"
#include "display/Analyzer.h"
#include "display/FRCCharacterization.h"
#include "display/Generator.h"
#include "display/Logger.h"

using namespace frcchar;

int main() {
  FRCCharacterization::GlobalInit();

  DataProcessor processor;
  std::vector<DataProcessor::Data> data;
  data.push_back(DataProcessor::Data{11_V, 3_mps, 37_mps_sq});
  data.push_back(DataProcessor::Data{10_V, 1_mps, 35_mps_sq});
  processor.CalculateFeedforwardGains(DataProcessor::TestInfo{data, true});

  Logger logger;
  Generator generator;
  Analyzer analyzer;

  FRCCharacterization::Add([&generator] { generator.Initialize(); });
  FRCCharacterization::Add([&logger] { logger.Initialize(); });
  FRCCharacterization::Add([&analyzer] { analyzer.Initialize(); });

  if (!FRCCharacterization::Initialize()) return 0;

  FRCCharacterization::Main();
  FRCCharacterization::Exit();
}
