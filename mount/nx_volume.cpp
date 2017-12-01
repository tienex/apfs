/*
 * Copyright (c) 2017-present Orlando Bassotto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "nx_volume.h"

#include <sstream>

using apfs_fuse::nx_volume;

nx_volume::nx_volume(apfs::session *session)
    : volume  (nullptr)
    , _session(session)
{
    //
    // Open all subvolumes.
    //
    std::map<std::string, std::vector<volume *>> subvolumes;

    nx::container::info info;
    _session->get_container()->get_info(info);

    for (size_t n = 0; n < (info.volumes - info.vfree); n++) {
        auto subvolume = _session->open(n);
        if (subvolume == nullptr)
            break;

        //
        // Now insert this volume name into the map, do so
        // so that if we have two volumes with the same name
        // we can number them.
        //
        subvolumes[subvolume->get_name()].push_back(new volume(subvolume));
    }

    //
    // Now insert into the _volumes map, first those with count
    // equals 1.
    //
    for (auto &i : subvolumes) {
        if (i.second.size() == 1) {
            _volumes[i.first] = i.second.front();
        }
    }

    //
    // Now insert all the other, numbering, if we have already a
    // volume with the name "name number", we skip number until
    // we find a free slot.
    //
    for (auto &i : subvolumes) {
        if (i.second.size() > 1) {
            for (size_t n = 0; n < i.second.size(); n++) {
                for (size_t index = 0;; index++) {
                    std::stringstream ss;

                    ss << i.first << ' ' << index;

                    auto name = ss.str();
                    if (_volumes.count(name) != 0)
                        continue;

                    _volumes[name] = i.second[n];
                    break;
                }
            }
        }
    }

    //
    // Construct the fake name
    //
    std::stringstream ss;
    nx_uuid_t uuid = _session->get_container()->get_uuid();
    char buf[128];

    ss << "APFS Container " << nx_uuid_format(&uuid, buf, sizeof(buf));

    _name = ss.str();
}

nx_volume::~nx_volume()
{
    for (auto &i : _volumes) {
        delete i.second;
    }

    _volumes.clear();
}

void nx_volume::
stat(statfs_t *st, bool) const
{
    _volumes.begin()->second->stat(st, _volumes.size() != 1);
}

#ifdef __APPLE__
void nx_volume::
stat(struct statvfs *st, bool) const
{
    _volumes.begin()->second->stat(st, _volumes.size() != 1);
}
#endif

char const *nx_volume::
get_name() const
{
    if (_volumes.size() == 1)
        return _volumes.begin()->second->get_name();

    return _name.c_str();
}

nx_uuid_t const &nx_volume::
get_uuid() const
{
    if (_volumes.size() == 1)
        return _volumes.begin()->second->get_uuid();

    return _session->get_container()->get_uuid();
}

apfs_fuse::volume *nx_volume::
get_volume_path(std::string const &path, std::string &subpath) const
{
    if (path == "/") {
        subpath = path;
        return const_cast<nx_volume *>(this);
    }

    auto end = path.find('/', 1);
    if (end == std::string::npos) {
        end = path.length();
    }

    auto element = path.substr(1, end - 1);
    auto i = _volumes.find(element);
    if (i == _volumes.end()) {
        errno = ENOENT;
        return nullptr;
    }

    subpath = path.substr(end);
    if (subpath.empty() || subpath[0] != '/') {
        subpath = "/" + subpath;
    }

    return i->second;
}

apfs_fuse::object *nx_volume::
open(std::string const &path) const
{
    if (_volumes.size() == 1)
        return _volumes.begin()->second->open(path);

    if (path == "/")
        return open_directory(path);

    std::string subpath;
    auto volume = get_volume_path(path, subpath);
    if (volume != nullptr) {
        return volume->open(subpath);
    }

    return nullptr;
}

apfs_fuse::file *nx_volume::
open_file(std::string const &path) const
{
    if (_volumes.size() == 1)
        return _volumes.begin()->second->open_file(path);

    if (path == "/") {
        errno = EISDIR;
        return nullptr;
    }

    std::string subpath;
    auto volume = get_volume_path(path, subpath);
    if (volume != nullptr) {
        return volume->open_file(subpath);
    }

    return nullptr;
}

apfs_fuse::directory *nx_volume::
open_directory(std::string const &path) const
{
    if (_volumes.size() == 1)
        return _volumes.begin()->second->open_directory(path);

    if (path == "/")
        return new nx_root(this);

    std::string subpath;
    auto volume = get_volume_path(path, subpath);
    if (volume != nullptr) {
        return volume->open_directory(subpath);
    }

    return nullptr;
}
