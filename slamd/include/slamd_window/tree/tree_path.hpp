#pragma once
#include <string>
#include <vector>

namespace slamd {

class TreePath {
   public:
    TreePath(const std::string& path_string);
    TreePath(const std::vector<std::string>& components);
    TreePath(const char* path_string);
    bool is_root() const;
    TreePath parent() const;
    std::string string() const;
    TreePath(const TreePath&) = default;
    TreePath& operator=(const TreePath&) = default;
    bool matches_glob(const TreePath& glob_path);

   public:
    std::vector<std::string> components;
};

TreePath operator/(const TreePath& path, const std::string& part);

}  // namespace slamd