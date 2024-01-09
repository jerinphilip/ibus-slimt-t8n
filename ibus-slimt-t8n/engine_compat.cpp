#include "ibus-slimt-t8n/engine_compat.h"
#include "ibus-slimt-t8n/slimt_engine.h"
#include <cstring>

namespace ibus::slimt::t8n {

/* code of engine class of GObject */
#define IBUS_SLIMT_T8N_ENGINE(obj)                                             \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), IBUS_TYPE_SLIMT_T8N_ENGINE,               \
                              IBusSlimtEngine))
#define IBUS_SLIMT_T8N_ENGINE_CLASS(klass)                                     \
  (G_TYPE_CHECK_CLASS_CAST((klass), IBUS_TYPE_SLIMT_T8N_ENGINE,                \
                           IBusSlimtEngineClass))
#define IBUS_IS_SLIMT_T8N_ENGINE(obj)                                          \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), IBUS_TYPE_SLIMT_T8N_ENGINE))
#define IBUS_IS_SLIMT_T8N_ENGINE_CLASS(klass)                                  \
  (G_TYPE_CHECK_CLASS_TYPE((klass), IBUS_TYPE_SLIMT_T8N_ENGINE))
#define IBUS_SLIMT_T8N_ENGINE_GET_CLASS(obj)                                   \
  (G_TYPE_INSTANCE_GET_CLASS((obj), IBUS_TYPE_SLIMT_T8N_ENGINE,                \
                             IBusSlimtEngineClass))

using IBusSlimtEngine = struct IBusSlimtEngine;
using IBusSlimtEngineClass = struct IBusSlimtEngineClass;

struct IBusSlimtEngineClass {
  IBusEngineClass parent;
};

struct IBusSlimtEngine {
  IBusEngine parent;

  /* members */
  Engine *engine;
};

/* functions prototype */
static void ibus_slimt_t8n_engine_class_init(IBusSlimtEngineClass *klass);
static void ibus_slimt_t8n_engine_init(IBusSlimtEngine *slimt_t8n);
static GObject *
ibus_slimt_t8n_engine_constructor(GType type, guint n_construct_params,
                                  GObjectConstructParam *construct_params);

static void ibus_slimt_t8n_engine_destroy(IBusSlimtEngine *slimt_t8n);
static gboolean ibus_slimt_t8n_engine_process_key_event(IBusEngine *engine,
                                                        guint keyval,
                                                        guint keycode,
                                                        guint modifiers);
static void ibus_slimt_t8n_engine_focus_in(IBusEngine *engine);
static void ibus_slimt_t8n_engine_focus_out(IBusEngine *engine);
#if IBUS_CHECK_VERSION(1, 5, 4)
static void ibus_slimt_t8n_engine_set_content_type(IBusEngine *engine,
                                                   guint purpose, guint hints);
#endif
static void ibus_slimt_t8n_engine_reset(IBusEngine *engine);
static void ibus_slimt_t8n_engine_enable(IBusEngine *engine);
static void ibus_slimt_t8n_engine_disable(IBusEngine *engine);

#if 0
static void     ibus_engine_set_cursor_location (IBusEngine             *engine,
                                                 gint                    x,
                                                 gint                    y,
                                                 gint                    w,
                                                 gint                    h);
static void     ibus_slimt_t8n_engine_set_capabilities
                                                (IBusEngine             *engine,
                                                 guint                   caps);
#endif

static void ibus_slimt_t8n_engine_page_up(IBusEngine *engine);
static void ibus_slimt_t8n_engine_page_down(IBusEngine *engine);
static void ibus_slimt_t8n_engine_cursor_up(IBusEngine *engine);
static void ibus_slimt_t8n_engine_cursor_down(IBusEngine *engine);
static void ibus_slimt_t8n_engine_property_activate(IBusEngine *engine,
                                                    const gchar *prop_name,
                                                    guint prop_state);
static void ibus_slimt_t8n_engine_candidate_clicked(IBusEngine *engine,
                                                    guint index, guint button,
                                                    guint state);
#if 0
static void ibus_slimt_t8n_engine_property_show    (IBusEngine             *engine,
                                                 const gchar            *prop_name);
static void ibus_slimt_t8n_engine_property_hide    (IBusEngine             *engine,
                                                 const gchar            *prop_name);
#endif

G_DEFINE_TYPE(IBusSlimtEngine, ibus_slimt_t8n_engine, IBUS_TYPE_ENGINE)

static void ibus_slimt_t8n_engine_class_init(IBusSlimtEngineClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS(klass);
  IBusEngineClass *engine_class = IBUS_ENGINE_CLASS(klass);

  object_class->constructor = ibus_slimt_t8n_engine_constructor;
  ibus_object_class->destroy =
      reinterpret_cast<IBusObjectDestroyFunc>(ibus_slimt_t8n_engine_destroy);

  engine_class->process_key_event = ibus_slimt_t8n_engine_process_key_event;

  engine_class->reset = ibus_slimt_t8n_engine_reset;
  engine_class->enable = ibus_slimt_t8n_engine_enable;
  engine_class->disable = ibus_slimt_t8n_engine_disable;

  engine_class->focus_in = ibus_slimt_t8n_engine_focus_in;
  engine_class->focus_out = ibus_slimt_t8n_engine_focus_out;

#if IBUS_CHECK_VERSION(1, 5, 4)
  engine_class->set_content_type = ibus_slimt_t8n_engine_set_content_type;
#endif

  engine_class->page_up = ibus_slimt_t8n_engine_page_up;
  engine_class->page_down = ibus_slimt_t8n_engine_page_down;

  engine_class->cursor_up = ibus_slimt_t8n_engine_cursor_up;
  engine_class->cursor_down = ibus_slimt_t8n_engine_cursor_down;

  engine_class->property_activate = ibus_slimt_t8n_engine_property_activate;

  engine_class->candidate_clicked = ibus_slimt_t8n_engine_candidate_clicked;
}

static void ibus_slimt_t8n_engine_init(IBusSlimtEngine *slimt_t8n) {
  if (g_object_is_floating(slimt_t8n))
    g_object_ref_sink(slimt_t8n); // make engine sink
}

static GObject *
ibus_slimt_t8n_engine_constructor(GType type, guint n_construct_params,
                                  GObjectConstructParam *construct_params) {
  IBusSlimtEngine *engine;
  const gchar *name;

  engine = reinterpret_cast<IBusSlimtEngine *>(
      G_OBJECT_CLASS(ibus_slimt_t8n_engine_parent_class)
          ->constructor(type, n_construct_params, construct_params));
  name = ibus_engine_get_name(reinterpret_cast<IBusEngine *>(engine));
  engine->engine = new SlimtEngine(IBUS_ENGINE(engine));
  return reinterpret_cast<GObject *>(engine);
}

static void ibus_slimt_t8n_engine_destroy(IBusSlimtEngine *slimt_t8n) {
  delete slimt_t8n->engine;
  (static_cast<IBusObjectClass *>(ibus_slimt_t8n_engine_parent_class))
      ->destroy(reinterpret_cast<IBusObject *>(slimt_t8n));
}

static gboolean ibus_slimt_t8n_engine_process_key_event(IBusEngine *engine,
                                                        guint keyval,
                                                        guint keycode,
                                                        guint modifiers) {
  auto *slimt_t8n = reinterpret_cast<IBusSlimtEngine *>(engine);
  return slimt_t8n->engine->process_key_event(keyval, keycode, modifiers);
}

#if IBUS_CHECK_VERSION(1, 5, 4)
static void ibus_slimt_t8n_engine_set_content_type(IBusEngine *engine,
                                                   guint purpose, guint hints) {
  auto *slimt_t8n = reinterpret_cast<IBusSlimtEngine *>(engine);
  return slimt_t8n->engine->set_content_type(purpose, hints);
}
#endif

static void ibus_slimt_t8n_engine_property_activate(IBusEngine *engine,
                                                    const gchar *prop_name,
                                                    guint prop_state) {
  auto *slimt_t8n = reinterpret_cast<IBusSlimtEngine *>(engine);
  slimt_t8n->engine->property_activate(prop_name, prop_state);
}
static void ibus_slimt_t8n_engine_candidate_clicked(IBusEngine *engine,
                                                    guint index, guint button,
                                                    guint state) {
  auto *slimt_t8n = reinterpret_cast<IBusSlimtEngine *>(engine);
  slimt_t8n->engine->candidate_clicked(index, button, state);
}

#define FUNCTION(name, Name)                                                   \
  static void ibus_slimt_t8n_engine_##name(IBusEngine *engine) {               \
    IBusSlimtEngine *slimt_t8n = (IBusSlimtEngine *)engine;                    \
    slimt_t8n->engine->Name();                                                 \
    ((IBusEngineClass *)ibus_slimt_t8n_engine_parent_class)->name(engine);     \
  }
FUNCTION(focus_in, focus_in)
FUNCTION(focus_out, focus_out)
FUNCTION(reset, reset)
FUNCTION(enable, enable)
FUNCTION(disable, disable)
FUNCTION(page_up, page_up)
FUNCTION(page_down, page_down)
FUNCTION(cursor_up, cursor_up)
FUNCTION(cursor_down, cursor_down)
#undef FUNCTION

Engine::Engine(IBusEngine *engine) : engine_holder_(engine), engine_(engine) {
#if IBUS_CHECK_VERSION(1, 5, 4)
  m_input_purpose_ = IBUS_INPUT_PURPOSE_FREE_FORM;
#endif
}

gboolean Engine::content_is_password() {
#if IBUS_CHECK_VERSION(1, 5, 4)
  return static_cast<gboolean>(IBUS_INPUT_PURPOSE_PASSWORD == m_input_purpose_);
#else
  return FALSE;
#endif
}

void Engine::focus_out() {
#if IBUS_CHECK_VERSION(1, 5, 4)
  m_input_purpose_ = IBUS_INPUT_PURPOSE_FREE_FORM;
#endif
}

#if IBUS_CHECK_VERSION(1, 5, 4)
void Engine::set_content_type(guint purpose, guint /*hints*/) {
  m_input_purpose_ = static_cast<IBusInputPurpose>(purpose);
}
#endif

} // namespace ibus::slimt::t8n
