#pragma once
#include <glib.h>

#define APPNAME "ibus-slimt-t8n"
#define LOG(...) g_log(APPNAME, G_LOG_LEVEL_MESSAGE, __VA_ARGS__)
