#pragma once

#include "ibus-slimt-t8n/engine_compat.h"
#include "ibus-slimt-t8n/translator.h"
#include <list>
#include <optional>
#include <string>

namespace ibus::slimt::t8n {

/// Idea here is to maintain an active buffer string.
//
// 1. The first suggestion is the translated text.
// 2. The second suggestion is the raw text the user entered.
class SlimtEngine : public Engine {
public:
  explicit SlimtEngine(IBusEngine *engine);
  ~SlimtEngine() override;

  // virtual functions
  gboolean process_key_event(guint keyval, guint keycode,
                             guint modifiers) override;
  void focus_in() override;
  void focus_out() override;
  void reset() override;
  void enable() override;
  void disable() override;
  void page_up() override;
  void page_down() override;
  void cursor_up() override;
  void cursor_down() override;
  gboolean property_activate(const gchar *prop_name, guint prop_state) override;
  void candidate_clicked(guint index, guint button, guint state) override;

private:
  void show_setup_dialog();

  static g::LookupTable
  generate_lookup_table(const std::vector<std::string> &entries);

  void update_buffer(const std::string &append);
  void refresh_translation();
  void commit();

  Pair<std::string> buffer_;
  gint cursor_position_;

  Translator translator_;
  Direction direction_;

  struct Select {
    g::Property node;
    g::PropList options;
  };

  struct UI {
    Select source;
    Select target;
    g::Property verify;
  };

  UI ui_;

  static UI make_ui(Translator &translator);
  static g::PropList make_children(const std::string &side,
                                   const StringSet &languages,
                                   const std::string &default_language);

  static Select make_select(const std::string &key,     //
                            const std::string &tooltip, //
                            const StringSet &languages, //
                            const std::string &value);

  static g::Property make_verify(bool enable_sensitive);
};

} // namespace ibus::slimt::t8n
