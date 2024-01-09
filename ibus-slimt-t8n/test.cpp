#include "ibus-slimt-t8n/logging.h"
#include "ibus-slimt-t8n/translator.h"
#include <filesystem>
#include <iostream>

template <class Translator> void repl(const std::string &config) {
  std::cout << "Type in: "
            << "\n";
  std::cout << "   "
            << "<source_lang> <target_lang> <input> \n";

  std::string input;
  using Direction = ibus::slimt::t8n::Direction;
  Direction old;
  Direction current;
  Translator translator(config);

  while (!std::cin.eof()) {
    std::cout << " $ ";
    std::cin >> current.source;
    std::cin >> current.target;
    std::getline(std::cin, input);
    if (current.source != old.source || current.target != old.target) {
      translator.set_direction(current);
      old = current;
    }
    auto translation = translator.translate(input);
    std::cout << translation << "\n";
    LOG("Direction %s -> %s: %s / %s", current.source.c_str(),
        current.target.c_str(), input.c_str(), translation.c_str());
  }
}

int main(int argc, char **argv) {
  std::string mode((argc == 2) ? argv[1] : "");
  auto config = ibus::slimt::t8n::ibus_slimt_t8n_config();
  if (mode == "fake") {
    repl<ibus::slimt::t8n::FakeTranslator>(config);
  } else {
    repl<ibus::slimt::t8n::Translator>(config);
  }

  return 0;
}
