#include "ibus-slimt-t8n/application.h"
#include "ibus-slimt-t8n/logging.h"

namespace ibus::slimt::t8n {

Application::Application(gboolean ibus) {
  ibus_init();

  // TODO(jerin): Bus can be g::Object derived.
  bus_ = ibus_bus_new();

  if (!ibus_bus_is_connected(bus_.get())) {
    LOG("Cannot connect to ibus!");
    g_warning("Can not connect to ibus!");
    std::abort();
  }

  if (!ibus_bus_get_config(bus_.get())) {
    LOG("IBus config component is not ready!");
    g_warning("IBus config component is not ready!");
    std::abort();
  }

  auto callback = +[](IBusBus *, gpointer) { ibus_quit(); };

  g_signal_connect(bus_.get(), "disconnected", G_CALLBACK(callback), NULL);

  LOG("Adding factory");
  factory_ = ibus_factory_new(ibus_bus_get_connection(bus_.get()));

  ibus_factory_add_engine(factory_.get(), PROJECT_SHORTNAME,
                          IBUS_TYPE_SLIMT_T8N_ENGINE);

  if (ibus) {
    LOG("ibus = true, requesting bus");
    ibus_bus_request_name(bus_.get(), IBUS_BUS_NAME, 0);
  } else {
    LOG("ibus = false, creating new bus");
    g::Holder<IBusComponent> component( //
        ibus_component_new(             //
            IBUS_BUS_NAME,              //
            PROJECT_DESCRIPTION,        //
            PROJECT_VERSION,            //
            PROJECT_LICENSE,            //
            AUTHOR,                     //
            PROJECT_HOMEPAGE,           //
            IBUS_COMPONENT_COMMANDLINE, //
            IBUS_TEXTDOMAIN             //
            ));

    if (component.get()) {
      LOG("creating component success");
    }

    g::Holder<IBusEngineDesc> description( //
        ibus_engine_desc_new(              //
            PROJECT_SHORTNAME,             //
            PROJECT_LONGNAME,              //
            PROJECT_DESCRIPTION,           //
            IBUS_LANGUAGE,                 //
            PROJECT_LICENSE,               //
            AUTHOR,                        //
            IBUS_ICON,                     //
            IBUS_LAYOUT                    //
            ));

    ibus_component_add_engine(component.get(), description.get());
    ibus_bus_register_component(bus_.get(), component.get());
  }
}

void Application::run() {
  LOG("Spawning ibus main");
  ibus_main();
  LOG("Ending ibus main");
}
} // namespace ibus::slimt::t8n
