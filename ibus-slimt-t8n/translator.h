#pragma once
#include "ibus-slimt-t8n/logging.h"
#include "slimt/slimt.hh"
#include "yaml-cpp/yaml.h"
#include <cstddef>
#include <memory>
#include <optional>

namespace ibus::slimt::t8n {

template <class Field> struct Pair {
  Field source;
  Field target;
};

using Direction = Pair<std::string>;
using Strings = std::vector<std::string>;
using StringSet = std::set<std::string>;
using Languages = Pair<StringSet>;

template <class T> using Package = ::slimt::Package<T>;
using Config = ::slimt::Config;
using Model = ::slimt::Model;
using Async = ::slimt::Async;
using Options = ::slimt::Options;
using Handle = ::slimt::Handle;
using Response = ::slimt::Response;

Direction reverse(const Direction &direction);

class Inventory {
public:
  explicit Inventory(const std::string &config_path);
  std::shared_ptr<Model> query(const Direction &direction) const;
  const Languages &languages() const;
  bool verify() const { return verify_; }
  bool exists(const Direction &direction) const;
  const Direction &default_direction() const;

private:
  struct Hash {
    size_t operator()(const Direction &direction) const;
  };

  struct Equal {
    bool operator()(const Direction &lhs, const Direction &rhs) const;
  };

  std::unordered_map<Direction, YAML::Node, Hash, Equal> directions_;
  std::set<std::string> select_languages_;
  Languages languages_;
  Direction default_direction_;
  YAML::Node inventory_;
  bool verify_;
  static YAML::Node load(const std::string &path);
};

class Translator {
public:
  explicit Translator(const std::string &ibus_config_path)
      : service_(Config{}), inventory_(ibus_config_path),
        verify_(inventory_.verify()) {}

  void set_direction(const Direction &direction);
  void set_verify(bool verify);
  bool verify() const { return verify_; }
  bool verifiable() const;
  const Direction &direction() const { return direction_; }

  std::string translate(const std::string &source);
  std::string backtranslate(const std::string &source);

  const Direction &default_direction() const;
  const Languages &languages() const;

private:
  using ModelPtr = std::shared_ptr<Model>;
  using Chain = std::pair<ModelPtr, ModelPtr>;

  void load_model(const Direction &direction, Chain &chain);

  Inventory inventory_;
  Direction direction_;

  Async service_;

  Chain forward_;
  Chain backward_;

  bool verify_;
};

class FakeTranslator {
public:
  explicit FakeTranslator(const std::string &){};

  void set_direction(const Direction &direction);
  void set_verify(bool verify);

  bool verify() const { return verify_; }
  const Direction &direction() const { return direction_; }

  std::string translate(std::string input);
  std::string backtranslate(std::string input);

  const Direction &default_direction() const;
  const Languages &languages() const;

private:
  Languages languages_ = {
      {"English", "German", "French"}, //
      {"English", "German", "French"}  //
  };

  Direction direction_{
      .source = "English", //
      .target = "German"   //
  };

  bool verify_ = false;
};

void make_translator();
std::string ibus_slimt_t8n_config();

} // namespace ibus::slimt::t8n
