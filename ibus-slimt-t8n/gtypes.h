#pragma once

#include <glib.h>
#include <ibus.h>
#include <string>

// This file includes types wrapping the GLIB objects into RAII C++ classes.
// GLIB objects follow some global reference-counting. This is an alteration
// from the original import from https://github.com/libzhuyin/ibus-libzhuyin,
// but appears to work.
//
// Not written in the best of states - expect rough edges.
namespace g {

// RAII wrap automating some things for a GLIB pointer object
// The following concepts
// exist:
//
// 1. Increase ref-count equivalent to increasing the count of usage in
// std::shared_ptr.
// 2. Decrease ref-count equivalent to reducing count of usage in
// std::shared_ptr. When this hits 0, the allocated object is freed.
// 3. Borrowing (equivalent to .get(), when there is no alteration to
// reference-count, but a pointer is passed around).
//
//
// This behavior is translated into a Holder for a Raw pointer (along the same
// lines as an std::shared_ptr) as
// 1. Construction = g_object_ref_sink(...)
// 2. Destruction = g_object_unrf(....)
// 3. Borrowing = .get()
//
// With the above in place, there is not much need to bother about
// reference-count updates, they happen taking advantage of C++'s RAII, similar
// to the operations of a shared_ptr.
template <typename Raw> struct Holder {
public:
  explicit Holder(Raw *p = nullptr) : pointer_(nullptr) { set(p); }
  ~Holder() { set(nullptr); }

  // Assingment from raw-pointer.
  Holder &operator=(Raw *p) {
    set(p);
    return *this;
  }

  // Copy (construction + assignment)
  Holder(const Holder &other) {
    // Simply set this pointer, incrementing reference.
    set(other.pointer_);
  };

  Holder &operator=(const Holder &other) {
    if (this != &other) {
      // Avoid circular references, set.
      set(other.pointer_);
    }
    return *this;
  }

  // Move (construction + assignment)
  Holder(Holder &&other) noexcept {
    // Set this, unset other.
    set(other.pointer_);
    other.set(nullptr);
  }

  Holder &operator=(Holder &&other) noexcept {
    if (this != &other) {
      // Avoid circular messups, set this, unset other.
      set(other.pointer_);
      other.set(nullptr);
    }
    return *this;
  };

  // Consider different cases of dereferencing a Holder<Raw> t

  // x = *t; const read
  const Raw *operator->() const { return pointer_; }
  // t->fn(...) In case t is an object with methods.
  Raw *operator->() { return pointer_; }

  // *t = x; not const, write.
  // operator Raw *() const { return pointer_; }
  Raw *get() const { return pointer_; }

private:
  Raw *pointer_ = nullptr;

  void set(Raw *other) {
    if (pointer_) {
      auto *g_object_pointer = reinterpret_cast<GObject *>(pointer_);
      g_object_unref(g_object_pointer);
    }

    pointer_ = other;
    if (other) {
      g_debug("%s, floating = %d", G_OBJECT_TYPE_NAME(other),
              g_object_is_floating(other));
      g_object_ref_sink(other);
    }
  }
};

// All IBUS type wrappers inherit from Object (Holder<GObject>). A CRTP is used
// to embed the Derived class information at Object for .get().
template <class Derived> class Object {
public:
  explicit Object(Derived *p) : pointer_(reinterpret_cast<GObject *>(p)) {
    // g_assert(pointer_.get() != nullptr);
  }

  explicit operator GObject *() const { return pointer_.get(); }

  Derived *get() const { return reinterpret_cast<Derived *>(pointer_.get()); }

private:
  Holder<GObject> pointer_;
};

class Text : public Object<IBusText> {
public:
  explicit Text(IBusText *text) : Object(text) {}
  explicit Text(const gchar *str) : Object(ibus_text_new_from_string(str)) {}

  explicit Text(const std::string &str)
      : Object(ibus_text_new_from_string(str.c_str())) {}

  explicit Text(gunichar ch) : Object(ibus_text_new_from_unichar(ch)) {}

  void append_attribute(guint type, guint value, guint start, guint end) {
    ibus_text_append_attribute(get(), type, value, start, end);
  }

  const gchar *text() const { return get()->text; }
};

class StaticText : public Text {
public:
  explicit StaticText(const gchar *str)
      : Text(ibus_text_new_from_static_string(str)) {}

  explicit StaticText(const std::string &str)
      : Text(ibus_text_new_from_static_string(str.c_str())) {}

  explicit StaticText(gunichar ch) : Text(ch) {}
};

class LookupTable : public Object<IBusLookupTable> {
public:
  explicit LookupTable(guint page_size = 10, guint cursor_pos = 0,
                       gboolean cursor_visible = TRUE, gboolean round = FALSE)
      : Object(ibus_lookup_table_new(page_size, cursor_pos, cursor_visible,
                                     round)) {}

  guint page_size() const { return ibus_lookup_table_get_page_size(get()); }
  guint orientation() const { return ibus_lookup_table_get_orientation(get()); }
  guint cursor_pos() const { return ibus_lookup_table_get_cursor_pos(get()); }
  guint size() const {
    return ibus_lookup_table_get_number_of_candidates(get());
  }

  gboolean page_up() const { return ibus_lookup_table_page_up(get()); }
  gboolean page_down() const { return ibus_lookup_table_page_down(get()); }
  gboolean cursor_up() const { return ibus_lookup_table_cursor_up(get()); }
  gboolean cursor_down() const { return ibus_lookup_table_cursor_down(get()); }

  void set_page_size(guint size) const {
    ibus_lookup_table_set_page_size(get(), size);
  }
  void set_cursor_pos(guint pos) const {
    ibus_lookup_table_set_cursor_pos(get(), pos);
  }
  void set_orientation(gint orientation) const {
    ibus_lookup_table_set_orientation(get(), orientation);
  }
  void clear() const { ibus_lookup_table_clear(get()); }
  void set_cursor_visable(gboolean visable) const {
    ibus_lookup_table_set_cursor_visible(get(), visable);
  }
  void set_label(guint index, IBusText *text) const {
    ibus_lookup_table_set_label(get(), index, text);
  }
  void append_candidate(IBusText *text) const {
    ibus_lookup_table_append_candidate(get(), text);
  }
  void append_label(IBusText *text) const {
    ibus_lookup_table_append_label(get(), text);
  }

  IBusText *get_candidate(guint index) const {
    return ibus_lookup_table_get_candidate(get(), index);
  }
};

class Property : public Object<IBusProperty> {
public:
  explicit Property(const gchar *key, IBusPropType type = PROP_TYPE_NORMAL,
                    IBusText *label = nullptr, const gchar *icon = nullptr,
                    IBusText *tooltip = nullptr, gboolean sensitive = TRUE,
                    gboolean visible = TRUE,
                    IBusPropState state = PROP_STATE_UNCHECKED,
                    IBusPropList *props = nullptr)
      : Object(ibus_property_new(key, type, label, icon, tooltip, sensitive,
                                 visible, state, props)) {}

  void set_label(IBusText *text) { ibus_property_set_label(get(), text); }

  void set_label(const gchar *text) {
    Text t(text);
    set_label(t.get());
  }

  void set_icon(const gchar *icon) { ibus_property_set_icon(get(), icon); }

  void set_symbol(IBusText *text) { ibus_property_set_symbol(get(), text); }

  void set_symbol(const gchar *text) {
    Text t(text);
    set_symbol(t.get());
  }

  void set_sensitive(gboolean sensitive) {
    ibus_property_set_sensitive(get(), sensitive);
  }

  void set_tooltip(IBusText *text) { ibus_property_set_tooltip(get(), text); }

  void set_tooltip(const gchar *text) {
    Text t(text);
    set_tooltip(t.get());
  }
};

class PropList : public Object<IBusPropList> {
public:
  PropList() : Object(ibus_prop_list_new()) {}

  void append(const Property &property) {
    ibus_prop_list_append(get(), property.get());
  }
};

} // namespace g
