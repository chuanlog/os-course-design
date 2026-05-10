#pragma once

#include "FsTypes.h"

#include <string>
#include <vector>

namespace simplefs {

std::string normalizePath(const std::string& path);
std::vector<std::string> splitPath(const std::string& path);
std::string basenameOf(const std::string& path);
std::string parentPathOf(const std::string& path);
std::string buildStatsText(const FsStats& stats);
std::string buildBitmapText(const std::string& bitmap);

}  // namespace simplefs
