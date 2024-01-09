#include "ibus-slimt-t8n/application.h"
#include "ibus-slimt-t8n/engine_compat.h"
#include <ibus.h>

int main(int argc, char **argv) {
  /* command line options */
  gboolean ibus = FALSE;
  gboolean verbose = FALSE;

  const GOptionEntry entries[] = {
      {"ibus", 'i', 0, G_OPTION_ARG_NONE, &ibus,
       "component is executed by ibus", nullptr},
      {"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "verbose", nullptr},
      {nullptr},
  };

  GError *error = nullptr;
  GOptionContext *context;

  /* Parse the command line */
  context = g_option_context_new("- ibus slimt-t8n engine component");
  g_option_context_add_main_entries(context, entries, "ibus-slimt-t8n");

  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_print("Option parsing failed: %s\n", error->message);
    g_error_free(error);
    return (-1);
  }

  ibus::slimt::t8n::Application application(ibus);
  ibus::slimt::t8n::Application::run();
  return 0;
}
