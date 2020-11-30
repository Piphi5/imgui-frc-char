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
  data.push_back(DataProcessor::Data{10_V, 5_mps, 39_mps_sq});
  data.push_back(DataProcessor::Data{9_V, 4_mps, 39_mps_sq});
  data.push_back(DataProcessor::Data{6_V, 1_mps, 54_mps_sq});
  auto g = processor.CalculateFeedforwardGains(data);

  wpi::outs() << g.Ks.to<double>() << ", " << g.Kv.to<double>() << ", "
              << g.Ka.to<double>() << ", " << g.rSquared << "\n";

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
