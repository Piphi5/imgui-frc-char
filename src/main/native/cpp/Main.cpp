// MIT License

#include <wpi/raw_ostream.h>

#include "display/Analyzer.h"
#include "display/FRCCharacterization.h"
#include "display/Generator.h"
#include "display/Logger.h"

using namespace frcchar;

int main() {
  FRCCharacterization::GlobalInit();

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
