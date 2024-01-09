#include "ibus-slimt-t8n/translator.h"
#include <future>
#include <optional>
#include <random>

#include "yaml-cpp/yaml.h"
#include <filesystem>

namespace ibus::slimt::t8n {

Direction reverse(const Direction &direction) {
  return {
      .source = direction.target, //
      .target = direction.source  //
  };
}

Inventory::Inventory(const std::string &config_path) {
  inventory_ = load(config_path);
  using Strings = std::vector<std::string>;
  auto select_languages = inventory_["languages"].as<Strings>();
  select_languages_.insert(select_languages.begin(), select_languages.end());

  YAML::Node models = inventory_["models"];
  for (const YAML::Node &model : models) {
    // std::string type = entry["type"].GetString();
    YAML::Node node = model["direction"];

    Direction direction{
        .source = node["source"].as<std::string>(), //
        .target = node["target"].as<std::string>()  //
    };

    auto preferred = [&, this](const std::string &lang) {
      return select_languages_.find(lang) != select_languages_.end();
    };

    if (preferred(direction.source) and preferred(direction.target)) {
      languages_.source.insert(direction.source);
      languages_.target.insert(direction.target);
    }

    directions_[direction] = model;
  }

  default_direction_ = {
      .source = inventory_["default"]["source"].as<std::string>(), //
      .target = inventory_["default"]["target"].as<std::string>()  //
  };

  verify_ = inventory_["verify"].as<bool>();
}

std::shared_ptr<Model> make_model(const YAML::Node &config) {
  auto root = config["root"].as<std::string>();
  auto prefix_root = [&root](const std::string &path) {
    return root + "/" + path;
  };

  Package<std::string> path{
      .model = prefix_root(config["model"].as<std::string>()), //
      .vocabulary =
          prefix_root(config["vocabs"]["source"].as<std::string>()),  //
      .shortlist = prefix_root(config["shortlist"].as<std::string>()) //
  };

  LOG("model_path: %s", path.model.c_str());
  Model::Config arch = ::slimt::preset::tiny();
  return std::make_shared<Model>(arch, path);
}

std::shared_ptr<Model> Inventory::query(const Direction &direction) const {
  auto query = directions_.find(direction);
  if (query != directions_.end()) {
    return make_model(query->second);
  }
  return nullptr;
}

const Languages &Inventory::languages() const { return languages_; }

bool Inventory::exists(const Direction &direction) const {
  auto query = directions_.find(direction);
  return query != directions_.end();
}

const Direction &Inventory::default_direction() const {
  return default_direction_;
}

bool Inventory::Equal::operator()(const Direction &lhs,
                                  const Direction &rhs) const {
  return lhs.source == rhs.source && lhs.target == rhs.target;
}

size_t Inventory::Hash::operator()(const Direction &direction) const {
  auto hash_combine = [](size_t &seed, size_t next) {
    seed ^= (std::hash<size_t>{}(next) //
             + 0x9e3779b9              // NOLINT
             + (seed << 6)             // NOLINT
             + (seed >> 2)             // NOLINT
    );
  };

  size_t seed = std::hash<std::string>{}(direction.source);
  hash_combine(seed, std::hash<std::string>{}(direction.target));
  return seed;
}

YAML::Node Inventory::load(const std::string &path) {
  YAML::Node tree = YAML::LoadFile(path);
  return tree;
}

void Translator::load_model(const Direction &direction,
                            Translator::Chain &chain) {
  if (direction.source == "English" or direction.target == "English") {
    std::shared_ptr<Model> model = inventory_.query(direction);
    if (model) {
      chain.first = model;
      LOG("Found model for (%s -> %s)", direction.source.c_str(),
          direction.target.c_str());
    } else {
      LOG("No model found for %s -> %s", direction.source.c_str(),
          direction.target.c_str());
    }
  } else {
    // Try to translate by pivoting.
    Direction to_en{
        .source = direction.source, //
        .target = "English"         //
    };

    Direction from_en{
        .source = "English",       //
        .target = direction.target //
    };

    std::shared_ptr<Model> first = inventory_.query(to_en);
    std::shared_ptr<Model> second = inventory_.query(from_en);

    if (first && second) {
      chain.first = first;
      chain.second = second;
      LOG("Found model for (%s -> [en] -> %s)", direction.source.c_str(),
          direction.target.c_str());
    } else {
      LOG("Unable to generate model (%d) %s -> [en]  -> %s %d ",
          first == nullptr, direction.source.c_str(), direction.target.c_str(),
          second == nullptr);
    }
  }
}

void Translator::set_direction(const Direction &direction) {
  direction_ = direction;
  load_model(direction, forward_);
}

void Translator::set_verify(bool verify) {
  verify_ = verify;
  Direction back = reverse(direction_);
  load_model(back, backward_);
}

bool Translator::verifiable() const {
  Direction back = reverse(direction_);
  if (back.source == "English" or back.target == "English") {
    return inventory_.exists(back);
  } else { // NOLINT
    // Try to translate by pivoting.
    Direction to_en{
        .source = back.source, //
        .target = "English"    //
    };

    Direction from_en{
        .source = "English",  //
        .target = back.target //
    };

    return inventory_.exists(to_en) and inventory_.exists(from_en);
  }
}

std::string Translator::translate(const std::string &source) {
  Options options{.html = false};

  if (forward_.first && forward_.second) {
    // Pivoting.
    Handle handle =
        service_.pivot(forward_.first, forward_.second, source, options);
    Response response = handle.future().get();
    return response.target.text;
  }

  assert(forward_.first != nullptr);

  Handle handle = service_.translate(forward_.first, source, options);
  Response response = handle.future().get();
  return response.target.text;
}

std::string Translator::backtranslate(const std::string &source) {
  Options options{.html = false};
  if (backward_.first && backward_.second) {
    // Pivoting.
    Handle handle =
        service_.pivot(backward_.first, backward_.second, source, options);
    Response response = handle.future().get();
    return response.target.text;
  }

  assert(backward_.first != nullptr);

  Handle handle = service_.translate(backward_.first, source, options);
  Response response = handle.future().get();
  return response.target.text;
}

const Languages &Translator::languages() const {
  return inventory_.languages();
}

const Direction &Translator::default_direction() const {
  return inventory_.default_direction();
}

void FakeTranslator::set_direction(const Direction &direction) {
  direction_ = direction;
}

void FakeTranslator::set_verify(bool verify) { verify_ = verify; }

std::string FakeTranslator::translate(std::string input) { // NOLINT

  std::string response;
  if (input.empty()) {
    return response;
  }

  // For a given length, generates a 6 length set of tokens.
  // Entire string is changed by seeding with length each time.
  // Simulates translation in some capacity.
  auto transform = [](size_t length) -> std::string {
    std::mt19937_64 generator;
    constexpr size_t kTruncateLength = 6;
    generator.seed(length);
    std::string target;
    for (size_t i = 0; i < length; i++) {
      if (i != 0) {
        target += " ";
      }
      size_t value = generator();
      constexpr size_t kMaxLength = 20;
      std::string hex(kMaxLength, ' ');
      std::sprintf(hex.data(), "%x", static_cast<unsigned int>(value));
      // 2 to truncate 0x.
      target += hex.substr(2, kTruncateLength);
    }
    return target;
  };

  auto token_count = [](const std::string &input) -> size_t {
    std::string token;
    size_t count = 0;
    for (char c : input) {
      if (isspace(c)) {
        // Check for space.
        if (!token.empty()) {
          // Start of a new word.
          ++count;
          token = "";
        }
      } else {
        token += std::string(1, c);
      }
    }
    // Non space-detected overhang.
    if (!token.empty()) {
      count += 1;
    }

    return count;
  };

  size_t count = token_count(input);
  std::string target = transform(count);
  return target;
}

std::string FakeTranslator::backtranslate(std::string input) {
  return translate(std::move(input));
}

const Languages &FakeTranslator::languages() const { return languages_; }

const Direction &FakeTranslator::default_direction() const {
  return direction_;
}

std::string ibus_slimt_t8n_config() {
  namespace fs = std::filesystem;
  fs::path home = std::getenv("HOME");

  // Setup logging
  // fs::path log_path = home / ".local" / "var" / "ibus-slimt-t8n.log";
  // setup_logging(log_path.string());
  // g_log("ibus-slimt-t8n",    //
  //       G_LOG_LEVEL_MESSAGE, //
  //       "Creating log at: %s", log_path.string().c_str());

  // Pickup config-defaults.
  fs::path config = home / ".config";
  auto path = (config / "ibus-slimt-t8n.yml").string();
  return path;
}

} // namespace ibus::slimt::t8n
