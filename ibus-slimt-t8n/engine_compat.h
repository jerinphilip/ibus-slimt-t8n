#pragma once
#include <ibus.h>

#include "gtypes.h"

namespace ibus::slimt::t8n {

#define IBUS_TYPE_SLIMT_T8N_ENGINE (ibus_slimt_t8n_engine_get_type())

GType ibus_slimt_t8n_engine_get_type();

class Engine {
public:
  explicit Engine(IBusEngine *engine);
  virtual ~Engine() = default;

  gboolean content_is_password();

  // virtual functions
  virtual gboolean process_key_event(guint keyval, guint keycode,
                                     guint modifiers) = 0;
  virtual void focus_in() = 0;
  virtual void focus_out();
#if IBUS_CHECK_VERSION(1, 5, 4)
  virtual void set_content_type(guint purpose, guint hints);
#endif
  virtual void reset() = 0;
  virtual void enable() = 0;
  virtual void disable() = 0;
  virtual void page_up() = 0;
  virtual void page_down() = 0;
  virtual void cursor_up() = 0;
  virtual void cursor_down() = 0;
  virtual gboolean property_activate(const gchar *prop_name,
                                     guint prop_state) = 0;
  virtual void candidate_clicked(guint index, guint button, guint state) = 0;

protected:
  void commit_text(const g::Text &text) const {
    ibus_engine_commit_text(engine_, text.get());
  }

  void update_preedit_text(const g::Text &text, guint cursor,
                           gboolean visible) const {
    ibus_engine_update_preedit_text(engine_, text.get(), cursor, visible);
  }

  void show_preedit_text() const { ibus_engine_show_preedit_text(engine_); }

  void hide_preedit_text() const { ibus_engine_hide_preedit_text(engine_); }

  void update_auxiliary_text(const g::Text &text, gboolean visible) const {
    ibus_engine_update_auxiliary_text(engine_, text.get(), visible);
  }

  void show_auxiliary_text() const { ibus_engine_show_auxiliary_text(engine_); }

  void hide_auxiliary_text() const { ibus_engine_hide_auxiliary_text(engine_); }

  void update_lookup_table(const g::LookupTable &table,
                           gboolean visible) const {
    ibus_engine_update_lookup_table(engine_, table.get(), visible);
  }

  void update_lookup_table_fast(const g::LookupTable &table,
                                gboolean visible) const {
    ibus_engine_update_lookup_table_fast(engine_, table.get(), visible);
  }

  void show_lookup_table() const { ibus_engine_show_lookup_table(engine_); }

  void hide_lookup_table() const { ibus_engine_hide_lookup_table(engine_); }

  static void clear_lookup_table(const g::LookupTable &table) {
    ibus_lookup_table_clear(table.get());
  }

  void register_properties(const g::PropList &props) const {
    ibus_engine_register_properties(engine_, props.get());
  }

  void update_property(const g::Property &prop) const {
    ibus_engine_update_property(engine_, prop.get());
  }

  g::Holder<IBusEngine> engine_holder_; // engine pointer
  IBusEngine *engine_;

#if IBUS_CHECK_VERSION(1, 5, 4)
  IBusInputPurpose m_input_purpose_;
#endif
};

} // namespace ibus::slimt::t8n
