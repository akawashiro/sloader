#pragma once

#include "utils.h"

class DynLoader {
   public:
    DynLoader(const std::filesystem::path& main_path) : main_path_(main_path) {}

   private:
    std::filesystem::path main_path_;
};

std::unique_ptr<DynLoader> MakeDynLoader(
    const std::filesystem::path& main_path) {
    return std::make_unique<DynLoader>(main_path);
}
