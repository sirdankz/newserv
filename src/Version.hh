#pragma once

#include <inttypes.h>



enum class GameVersion {
  PATCH = 0,
  DC,
  PC,
  GC,
  XB,
  BB,
};

enum class ServerBehavior {
  PC_CONSOLE_DETECT = 0,
  LOGIN_SERVER,
  LOBBY_SERVER,
  DATA_SERVER_BB,
  PATCH_SERVER_PC,
  PATCH_SERVER_BB,
  PROXY_SERVER,
};

uint16_t flags_for_version(GameVersion version, int64_t sub_version);
const char* name_for_version(GameVersion version);
GameVersion version_for_name(const char* name);

const char* name_for_server_behavior(ServerBehavior behavior);
ServerBehavior server_behavior_for_name(const char* name);
