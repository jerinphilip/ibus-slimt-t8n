#include "ibus-slimt-t8n/slimt_engine.h"
#include "ibus-slimt-t8n/engine_compat.h"
#include <cctype>
#include <filesystem>
#include <glib.h>
#include <string>
#include <vector>

namespace ibus::slimt::t8n {

namespace {

template <class T8r> T8r make() {
  auto config = ibus_slimt_t8n_config();
  return T8r(config);
}

} // namespace

g::PropList SlimtEngine::make_children(const std::string &side,
                                       const StringSet &languages,
                                       const std::string &default_language) {
  bool first = false;
  g::PropList properties;
  for (const auto &lang : languages) {
    std::string key = side + "_" + lang; // NOLINT
    g::Text label(lang);
    IBusPropState state =
        (lang == default_language) ? PROP_STATE_CHECKED : PROP_STATE_UNCHECKED;

    IBusPropList *children = nullptr;
    const gchar *icon = nullptr;
    gboolean sensitive = TRUE;
    gboolean visible = TRUE;

    g::Property property( //
        key.c_str(), PROP_TYPE_RADIO, label.get(), icon, label.get(), sensitive,
        visible, state,
        children //
    );

    properties.append(property);
  }

  return properties;
}

SlimtEngine::Select SlimtEngine::make_select(const std::string &key,     //
                                             const std::string &tooltip, //
                                             const StringSet &languages, //
                                             const std::string &value    //
) {
  const gchar *gkey = key.c_str();
  const gchar *icon = nullptr;
  g::Text glabel(key);
  g::Text gtooltip(tooltip);
  g::PropList gchildren = make_children(key, languages, value);
  gboolean sensitive = TRUE;
  gboolean visible = TRUE;

  g::Property node(gkey, PROP_TYPE_MENU, glabel.get(), icon, gtooltip.get(),
                   sensitive, visible, PROP_STATE_CHECKED, gchildren.get());

  Select select{
      .node = std::move(node),        //
      .options = std::move(gchildren) //
  };

  return select;
}

g::Property SlimtEngine::make_verify(bool enable_sensitive) { //
  const gchar *icon = nullptr;
  g::Text glabel("verify");
  g::Text gtooltip("Verify with backtranslated text as second candidate.");
  auto sensitive = static_cast<gboolean>(enable_sensitive);
  gboolean visible = TRUE;
  IBusPropList *children = nullptr;
  g::Property verify("verify", PROP_TYPE_TOGGLE, glabel.get(), icon,
                     gtooltip.get(), sensitive, visible, PROP_STATE_UNCHECKED,
                     children);
  return verify;
}

SlimtEngine::UI SlimtEngine::make_ui(Translator &translator) {

  Direction direction = translator.default_direction();
  translator.set_direction(direction);

  Select source = make_select(       //
      "source", "Source language",   //
      translator.languages().source, //
      direction.source);
  Select target = make_select(       //
      "target", "Target language",   //
      translator.languages().target, //
      direction.target);

  bool enable_sensitive = true;
  auto verify = make_verify(enable_sensitive);

  // Assign UI.
  return {
      .source = std::move(source), //
      .target = std::move(target), //
      .verify = std::move(verify), //
  };
}

/* constructor */
SlimtEngine::SlimtEngine(IBusEngine *engine)
    : Engine(engine), translator_(make<Translator>()),
      ui_(make_ui(translator_)) {
  LOG("slimt-t8n engine started");
}

/* destructor */
SlimtEngine::~SlimtEngine() { hide_lookup_table(); }

gboolean SlimtEngine::process_key_event(guint keyval, guint /*keycode*/,
                                        guint modifiers) {
  // If both langs are set to equal, translation mechanism needn't kick in.
  if (translator_.direction().source == translator_.direction().target) {
    return 0;
  }

  if (content_is_password())
    return FALSE;

  if (modifiers & IBUS_RELEASE_MASK) {
    return FALSE;
  }

  // We are skipping any modifiers. Our workflow is simple. Ctrl-Enter key is
  // send.
  if (modifiers & IBUS_CONTROL_MASK && keyval == IBUS_Return) {
    g::Text text(buffer_.target);
    commit_text(text);
    buffer_.source.clear();
    buffer_.target.clear();
    hide_lookup_table();
    return TRUE;
  }

  // If ctrl modifier or something is else, we let it pass
  if (modifiers & IBUS_CONTROL_MASK) {
    return FALSE;
  }

  gboolean retval = FALSE;
  switch (keyval) {
  case IBUS_space: {
    if (buffer_.source.empty()) {
      update_buffer(" ");
      commit();
    } else if (buffer_.source.back() == ' ') {
      commit();
    } else {
      update_buffer(" ");
      retval = TRUE;
    }

  } break;
  case IBUS_Return: {
    if (buffer_.target.empty()) {
      // We have no use for empty enters.
      return 0;
    }
    buffer_.target += "\n";
    commit();
    retval = TRUE;

  } break;
  case IBUS_BackSpace: {
    if (buffer_.source.empty()) {
      // Let the backspace through.
      retval = FALSE;
    } else {
      buffer_.source.pop_back();
      refresh_translation();
      retval = TRUE;
    }
  } break;
  case IBUS_Left:
  case IBUS_Right:
  case IBUS_Up:
  case IBUS_Down:
    return FALSE;
    break;

  default: {
    if (isprint(static_cast<unsigned char>(keyval))) {
      std::string append;
      append += static_cast<unsigned char>(keyval);
      update_buffer(append);
      retval = TRUE;
    } else {
      retval = FALSE;
    }
  } break;
  }
  return retval;
}

void SlimtEngine::update_buffer(const std::string &append) {
  buffer_.source += append;
  refresh_translation();
}

void SlimtEngine::refresh_translation() {
  if (!buffer_.source.empty()) {
    std::string translation = translator_.translate(buffer_.source);
    buffer_.target = translation;
    std::vector<std::string> entries = {buffer_.source};
    if (translator_.verify()) {
      std::string backtranslation = translator_.backtranslate(translation);
      entries.push_back(backtranslation);
    }
    g::LookupTable table = generate_lookup_table(entries);
    update_lookup_table(table,
                        /*visible=*/static_cast<gboolean>(!entries.empty()));

    cursor_position_ = buffer_.target.size();
    g::Text pre_edit(buffer_.target);
    update_preedit_text(pre_edit, cursor_position_, /*visible=*/TRUE);
    show_lookup_table();
  } else {
    // Buffer is already clear (empty).
    // We will manually clear the buffer_.target.
    buffer_.target.clear();

    cursor_position_ = buffer_.target.size();
    g::Text pre_edit(buffer_.target);
    update_preedit_text(pre_edit, cursor_position_, /*visible=*/FALSE);
    hide_preedit_text();

    hide_lookup_table();
  }
}

void SlimtEngine::commit() {
  g::Text text(buffer_.target);
  commit_text(text);
  hide_lookup_table();

  buffer_.source.clear();
  buffer_.target.clear();

  hide_lookup_table();
  cursor_position_ = 0;
  g::Text pre_edit("");
  update_preedit_text(pre_edit, cursor_position_, TRUE);
}

void SlimtEngine::focus_in() {
  g::PropList properties;
  properties.append(ui_.source.node);
  properties.append(ui_.target.node);
  properties.append(ui_.verify);
  register_properties(properties);
}

void SlimtEngine::focus_out() {
  buffer_.source.clear();
  buffer_.target.clear();
  Engine::focus_out();
}

void SlimtEngine::reset() {}

void SlimtEngine::enable() {}

void SlimtEngine::disable() {}

void SlimtEngine::page_up() {}

void SlimtEngine::page_down() {}

void SlimtEngine::cursor_up() {}

void SlimtEngine::cursor_down() {}

inline void SlimtEngine::show_setup_dialog() {
  // g_spawn_command_line_async(LIBEXECDIR "/ibus-setup-libzhuyin zhuyin",
  // NULL);
}

gboolean SlimtEngine::property_activate(const char *cprop_name,
                                        guint prop_state) {
  std::string prop_name(cprop_name);
  Direction direction = translator_.direction();
  if (prop_name == "verify") {
    LOG("Verify translation is %d -> %d", translator_.verify(), prop_state);
    bool verify = (prop_state != 0U);
    LOG("Enabling backtranslation %s -> %s", direction.target.c_str(),
        direction.source.c_str());
    if (translator_.verifiable()) {
      translator_.set_verify(verify);
    }
  } else {
    const std::string &serialized(prop_name);
    constexpr size_t kPrefixLength = 6;
    constexpr size_t kSeparatorLength = 1;
    std::string side = serialized.substr(0, kPrefixLength);
    std::string lang =
        serialized.substr(kPrefixLength + kSeparatorLength, serialized.size());
    if (prop_state == 1) {
      LOG("%s [%s] [%s]", prop_name.c_str(), side.c_str(), lang.c_str());
      if (side == "source") {
        direction.source = lang;
      } else {
        direction.target = lang;
      }
      translator_.set_direction(direction);

      ui_.verify = make_verify(translator_.verifiable());
      update_property(ui_.verify);
    }
  }
  return FALSE;
}

void SlimtEngine::candidate_clicked(guint index, guint button, guint state) {}

g::LookupTable
SlimtEngine::generate_lookup_table(const std::vector<std::string> &entries) {
  g::LookupTable lookup_table;
  for (const auto &entry : entries) {
    g::Text text(entry);
    lookup_table.append_candidate(text.get());
  }
  return lookup_table;
}

} // namespace ibus::slimt::t8n
