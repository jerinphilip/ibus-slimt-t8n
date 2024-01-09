#pragma once
#include "ibus-slimt-t8n/engine_compat.h"
#include "ibus-slimt-t8n/ibus_config.h"
#include <ibus-slimt-t8n/gtypes.h>
#include <ibus.h>

namespace ibus::slimt::t8n {
class Application {
public:
  explicit Application(gboolean ibus);
  static void run();

private:
  g::Holder<IBusBus> bus_{nullptr};
  g::Holder<IBusFactory> factory_{nullptr};
};
} // namespace ibus::slimt::t8n
