/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "udevwrappers.h"

#include "logging.h"
#include "hook.h"
#include "DeterministicTimer.h"
#include "fileio/FileHandleList.h"
#include "global.h"
#include "../shared/AllInputs.h"
#include "../shared/SharedConfig.h"

#include <algorithm>
#include <cstring>
#include <forward_list>
#include <functional> /* reference_wrapper */
#include <iterator>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include <libudev.h>
#include <fnmatch.h>
#include <errno.h>
#include <fcntl.h> /* O_NONBLOCK */
#include <unistd.h> /* close */
#include <sys/sysmacros.h> /* makedev */

using namespace libtas;

namespace libtas {
namespace {

class String {
public:
    typedef char value_type;
    typedef value_type &reference;
    typedef const value_type &const_reference;
    typedef value_type *iterator;
    typedef const value_type *const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef std::ptrdiff_t difference_type;
    typedef std::size_t size_type;

    static const size_type npos = -1;

    String(std::nullptr_t = nullptr) : str(nullptr), len(0),
                                       allocated(false), immutable(false), terminated(false) {}
    String(const String &string)
        : str(string.str), len(string.len),
          allocated(false), immutable(true), terminated(string.terminated) {}
    String(String &&string)
        : str(string.str), len(string.len),
          allocated(string.allocated), immutable(string.immutable), terminated(string.terminated) {
        string.allocated = false;
    }
    String(const std::string &string) : String(string.data(), string.length()) {}
    String(std::string &&string) : String(string) { dup(); }
    String(iterator string)
        : str(string), len(string ? std::strlen(string) : 0),
          allocated(false), immutable(false), terminated(true) {}
    String(const_iterator string) : String(const_cast<iterator>(string)) { immutable = true; }
    String(iterator string, size_type length) : str(string), len(length),
                                                allocated(false), immutable(false), terminated(false) {
        if (length && string[--length] == '\0') {
            len = length;
            terminated = true;
        }
    }
    String(const_iterator string, size_type length) : String(const_cast<iterator>(string), length) {
        immutable = true;
    }
    String &operator=(const String &string) {
        this->~String();
        str = string.str;
        len = string.len;
        allocated = false;
        immutable = true;
        terminated = string.terminated;
        return *this;
    }
    String &operator=(String &&string) {
        this->~String();
        str = string.str;
        len = string.len;
        allocated = string.allocated;
        immutable = string.immutable;
        terminated = string.terminated;
        string.allocated = false;
        return *this;
    }
    ~String() {
        if (allocated)
            delete[] str;
    }

    reference operator[](size_type pos) { ensureMutable(); return begin()[pos]; }
    const_reference operator[](size_type pos) const { return cbegin()[pos]; }
    reference front() { ensureMutable(); return *begin(); }
    const_reference front() const { return *cbegin(); }
    reference back() { ensureMutable(); return *rbegin(); }
    const_reference back() const { return *crbegin(); }
    iterator data() { ensureMutable(); return begin(); }
    const_iterator data() const { return cbegin(); }
    const String &ensure_c_str() const { if (!terminated) dup(); return *this; }
    const_iterator c_str() const { return ensure_c_str().cbegin(); }

    String substr(size_type pos = 0, size_type count = npos) const {
        if (pos > len)
            pos = len;
        if (count >= len - pos)
            count = len - pos + terminated;
        return String(std::next(cbegin(), pos), count);
    }

    size_type find(value_type value) const {
        auto pos = std::find(cbegin(), cend(), value);
        return pos != cend() ? std::distance(cbegin(), pos) : npos;
    }
    size_type find(const String &string) const {
        auto pos = std::search(cbegin(), cend(), string.cbegin(), string.cend());
        return pos != cend() ? std::distance(cbegin(), pos) : npos;
    }
    size_type rfind(value_type value) const {
        auto pos = std::find(crbegin(), crend(), value);
        return pos != crend() ? length() - 1 - std::distance(crbegin(), pos) : npos;
    }
    size_type rfind(const String &string) const {
        auto pos = std::search(crbegin(), crend(), string.crbegin(), string.crend());
        return pos != crend() ? length() - string.length() - std::distance(crbegin(), pos) : npos;
    }
    size_type find_first_of(value_type value) const { return find(value); }
    size_type find_first_of(const String &string) const {
        auto pos = std::find_first_of(cbegin(), cend(), string.cbegin(), string.cend());
        return pos != cend() ? std::distance(cbegin(), pos) : npos;
    }
    size_type find_first_not_of(value_type value) const {
        auto pos = std::find_if_not(cbegin(), cend(), [value](value_type cur) {
                                                          return cur == value;
                                                      });
        return pos != cend() ? std::distance(cbegin(), pos) : npos;
    }
    size_type find_first_not_of(const String &string) const {
        auto pos = std::find_if_not(cbegin(), cend(), [string](value_type cur) {
                                                          return string.find(cur) != npos;
                                                      });
        return pos != cend() ? std::distance(cbegin(), pos) : npos;
    }
    size_type find_last_of(value_type value) const { return rfind(value); }
    size_type find_last_of(const String &string) const {
        auto pos = std::find_first_of(crbegin(), crend(), string.crbegin(), string.crend());
        return pos != crend() ? length() - string.length() - std::distance(crbegin(), pos) : npos;
    }
    size_type find_last_not_of(value_type value) const {
        auto pos = std::find_if_not(crbegin(), crend(), [value](value_type cur) {
                                                            return cur == value;
                                                        });
        return pos != crend() ? length() - 1 - std::distance(crbegin(), pos) : npos;
    }
    size_type find_last_not_of(const String &string) const {
        auto pos = std::find_if_not(crbegin(), crend(), [string](value_type cur) {
                                                            return string.find(cur) != npos;
                                                        });
        return pos != crend() ? length() - 1 - std::distance(crbegin(), pos) : npos;
    }

    template<typename Value> std::pair<String, String> split(Value &&value) const {
        auto pos = find_first_of(std::forward<Value>(value));
        if (pos == npos) return {*this, nullptr};
        return {substr(0, pos), substr(pos + 1)};
    }
    template<typename Value> std::pair<String, String> rsplit(Value &&value) const {
        auto pos = find_last_of(std::forward<Value>(value));
        if (pos == npos) return {nullptr, *this};
        return {substr(0, pos), substr(pos + 1)};
    }
    template<typename Value> std::pair<String, String> tok(Value &&value) const {
        auto pos = find_first_not_of(std::forward<Value>(value));
        return {pos ? substr(0, pos) : nullptr, pos != npos ? substr(pos) : nullptr};
    }
    template<typename Value> std::pair<String, String> rtok(Value &&value) const {
        auto pos = find_last_not_of(std::forward<Value>(value));
        if (pos++ == npos) return {nullptr, *this};
        return {substr(0, pos), pos != length() ? substr(pos) : nullptr};
    }

    iterator begin() { ensureMutable(); return str; }
    iterator end() { return std::next(begin(), length()); }
    const_iterator cbegin() const { return str; }
    const_iterator cend() const { return std::next(cbegin(), length()); }
    const_iterator begin() const { return cbegin(); }
    const_iterator end() const { return cend(); }
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }
    const_reverse_iterator rbegin() const { return crbegin(); }
    const_reverse_iterator rend() const { return crend(); }

    bool starts_with(const String &prefix) const {
        return length() >= prefix.length() && std::equal(prefix.cbegin(), prefix.cend(), cbegin());
    }
    bool ends_with(const String &suffix) const {
        return length() >= suffix.length() && std::equal(suffix.crbegin(), suffix.crend(), crbegin());
    }
    bool match(const String &string, int flags = 0) const {
        if (!str || !string.str)
            return str == string.str;
        return !::fnmatch(str, string.str, flags);
    }
    void swap(String &string) {
        auto tempStr = str;
        auto tempLen = len;
        auto tempAllocated = allocated;
        auto tempImmutable = immutable;
        auto tempTerminated = terminated;
        str = string.str;
        len = string.len;
        allocated = string.allocated;
        immutable = string.immutable;
        terminated = string.terminated;
        string.str = tempStr;
        string.len = tempLen;
        string.allocated = tempAllocated;
        string.immutable = tempImmutable;
        string.terminated = tempTerminated;
    }

    size_type size() const { return len; }
    size_type length() const { return len; }
    size_type max_size() const { return (size_type(1) << size_type(24)) - size_type(1); }
    bool empty() const { return !len; }
    operator bool() const { return str; }

    template<typename ...Args> static String concat(Args &&...args) {
        String result;
        result.len = totalLength(std::forward<Args>(args)...);
        result.str = new char[result.len + 1];
        append(result.str, std::forward<Args>(args)...);
        result.allocated = true;
        result.immutable = false;
        result.terminated = true;
        return result;
    }

private:
    mutable iterator str;
    size_type len : 24;
    mutable bool allocated : 1;
    mutable bool immutable : 1;
    mutable bool terminated : 1;
    void dup() const;
    void ensureMutable() { if (immutable) dup(); }

    static size_type totalLength() { return 0; }
    template<typename ...Rest> static size_type totalLength(value_type value, Rest &&...rest) {
        return 1 + totalLength(std::forward<Rest>(rest)...);
    }
    template<typename ...Rest>
    static size_type totalLength(const String &value, Rest &&...rest) {
        return value.length() + totalLength(std::forward<Rest>(rest)...);
    }
    static void append(iterator pos) { *pos = '\0'; }
    template<typename ...Rest> static void append(iterator pos, value_type value, Rest &&...rest) {
        *pos = value;
        append(std::next(pos), std::forward<Rest>(rest)...);
    }
    template<typename ...Rest>
    static void append(iterator pos, const String &value, Rest &&...rest) {
        append(std::copy(value.begin(), value.end(), pos), std::forward<Rest>(rest)...);
    }
};

inline bool operator==(const String &lhs, const String &rhs) {
    return lhs.length() == rhs.length() && std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
}
inline bool operator<(const String &lhs, const String &rhs) {
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

inline std::string &operator+=(std::string &lhs, const String &rhs) { return lhs.append(rhs.data(), rhs.length()); }

void String::dup() const {
    if (*this) {
        iterator copy = new value_type[length() + 1];
        *std::copy(begin(), end(), copy) = '\0';
        this->~String();
        str = copy;
    }
    allocated = true;
    immutable = false;
    terminated = true;
}

template<typename Type> Type *ref(Type *obj) {
    if (!obj)
        return errno = EINVAL, obj;
    MYASSERT(++obj->refs);
    return obj;
}
template<typename Type> Type &ref(Type &obj) { return *ref(&obj); }

template<typename Type> std::nullptr_t unref(Type *obj) {
    if (!obj)
        return errno = EINVAL, nullptr;
    MYASSERT(obj->refs);
    if (!--obj->refs)
        delete obj;
    return nullptr;
}
template<typename Type> void unref(Type &obj) { unref(&obj); }

template<typename Type> struct udev *get_udev(Type *obj) {
    if (!obj)
        return errno = EINVAL, nullptr;
    return &obj->udev;
}
template<typename Type> Type *new_from_udev(struct udev *udev) {
    if (!udev)
        return errno = EINVAL, nullptr;
    return new Type(*udev);
}

struct InactiveQueue {
    unsigned refs;
    struct udev &udev;
    /* We use a non-blocking always-empty pipe to simulate a queue that never
     * produces any events.
     */
    int fd;
    InactiveQueue(struct udev &udev)
        : refs(1), udev(ref(udev)), fd(FileHandleList::createPipe(O_NONBLOCK).first) {}
    /* We are a base class and have a non-virtual destructor, so protect it so
     * that we can only be destructed through our derived type.
     */
protected:
    ~InactiveQueue() {
        /* Closing the read end automatically closes the write end in our close hook,
           so this should NOT be a native call. */
        ::close(fd);
        unref(udev);
    }
};

}
}

struct udev {
    unsigned refs;
    void *userdata;
    udev() : refs(1), userdata() {}
};

struct udev_list_entry {
    const String name, value;
    udev_list_entry(String name = String(), String value = String()) : name(name), value(value) {}
    operator bool() const { return name; }
};

namespace libtas {
namespace {

class Device {
public:
    using ChildrenStorage = std::forward_list<Device>;
    using ChildrenMap = std::map<String, std::reference_wrapper<Device>>;
    using EntryMap = std::map<String, String>;
    using EntryList = std::vector<struct udev_list_entry>;

    class iterator {
    public:
        typedef Device value_type;
        typedef Device &reference;
        typedef Device *pointer;
        typedef std::ptrdiff_t difference_type;
        typedef std::forward_iterator_tag iterator_category;

        iterator(pointer device = nullptr, bool end = false)
            : device(device) { if (end) nextUpward(); }
        reference operator*() const { return *device; }
        pointer operator->() const { return device; }
        iterator &operator++() {
            for (auto &child : device->children())
                if (traverse(child))
                    return *this;
            nextUpward();
            return *this;
        }
        iterator operator++(int) { iterator tmp = *this; ++*this; return tmp; }
        bool operator==(const iterator &other) const { return device == other.device; }
        bool operator!=(const iterator &other) const { return device != other.device; }

    private:
        pointer device;
        void nextUpward() {
            while (true) {
                String name = device->sysname();
                device = device->parent;
                if (!device)
                    return;
                auto &children = device->childrenList.map;
                for (auto child = children.find(name); ++child != children.end(); )
                    if (traverse(child->second))
                        return;
            }
        }
        bool traverse(reference child) {
            if (child.parent != device)
                return false;
            device = &child;
            return true;
        }
    };
    class const_iterator {
    public:
        typedef Device value_type;
        typedef const Device &reference;
        typedef const Device *pointer;
        typedef std::ptrdiff_t difference_type;
        typedef std::forward_iterator_tag iterator_category;

        const_iterator(pointer device = nullptr, bool end = false)
            : device(device) { if (end) nextUpward(); }
        reference operator*() const { return *device; }
        pointer operator->() const { return device; }
        const_iterator &operator++() {
            for (auto &child : device->children())
                if (traverse(child))
                    return *this;
            nextUpward();
            return *this;
        }
        const_iterator operator++(int) { const_iterator tmp = *this; ++*this; return tmp; }
        bool operator==(const const_iterator &other) const { return device == other.device; }
        bool operator!=(const const_iterator &other) const { return device != other.device; }

    private:
        pointer device;
        void nextUpward() {
            while (true) {
                String name = device->sysname();
                device = device->parent;
                if (!device)
                    return;
                auto &children = device->childrenList.map;
                for (auto child = children.find(name); ++child != children.end(); )
                    if (traverse(child->second))
                        return;
            }
        }
        bool traverse(reference child) {
            if (child.parent != device)
                return false;
            device = &child;
            return true;
        }
    };
    class Children {
        friend class Device;
        class iterator {
        public:
            typedef Device value_type;
            typedef Device &reference;
            typedef Device *pointer;
            typedef ChildrenMap::iterator::difference_type difference_type;
            typedef std::bidirectional_iterator_tag iterator_category;

            iterator(ChildrenMap::iterator &&delegate) : delegate(std::move(delegate)) {}
            reference operator*() const { return delegate->second; }
            pointer operator->() const { return &**this; }
            iterator &operator++() { ++delegate; return *this; }
            iterator operator++(int) { iterator tmp = *this; ++*this; return tmp; }
            iterator &operator--() { --delegate; return *this; }
            iterator operator--(int) { iterator tmp = *this; --*this; return tmp; }
            bool operator==(const iterator &other) const { return delegate == other.delegate; }
            bool operator!=(const iterator &other) const { return delegate != other.delegate; }

        private:
            ChildrenMap::iterator delegate;
        };
        class const_iterator {
        public:
            typedef Device value_type;
            typedef const Device &reference;
            typedef const Device *pointer;
            typedef ChildrenMap::const_iterator::difference_type difference_type;
            typedef std::bidirectional_iterator_tag iterator_category;

            const_iterator(ChildrenMap::const_iterator &&delegate) : delegate(std::move(delegate)) {}
            reference operator*() const { return delegate->second; }
            pointer operator->() const { return &**this; }
            const_iterator &operator++() { ++delegate; return *this; }
            const_iterator operator++(int) { const_iterator tmp = *this; ++*this; return tmp; }
            const_iterator &operator--() { --delegate; return *this; }
            const_iterator operator--(int) { const_iterator tmp = *this; --*this; return tmp; }
            bool operator==(const const_iterator &other) const { return delegate == other.delegate; }
            bool operator!=(const const_iterator &other) const { return delegate != other.delegate; }

        private:
            ChildrenMap::const_iterator delegate;
        };
    public:
        std::size_t size() const { return map.size(); }
        bool empty() const { return !size(); }
        iterator begin() { return iterator(map.begin()); }
        const_iterator cbegin() const { return const_iterator(map.begin()); }
        const_iterator begin() const { return cbegin(); }
        iterator end() { return iterator(map.end()); }
        const_iterator cend() const { return const_iterator(map.end()); }
        const_iterator end() const { return cend(); }
        Device &front() { return *begin(); }
        const Device &front() const { return *begin(); }
        Device &back() { return *std::prev(end()); }
        const Device &back() const { return *std::prev(end()); }

    private:
        ChildrenMap map;
    };

    Device *const parent;
private:
    String path;
    ChildrenStorage childrenStorage;
    Children childrenList;
    EntryMap devlinksMap, propertiesMap, tagsMap, sysattrsMap;
    EntryList devlinksList, propertiesList, tagsList, sysattrsList;
    bool initialized, valid;
    dev_t dev;

    Device();
    Device(const Device &) = delete;
    Device(Device &&) = delete;
    Device &operator=(const Device &) = delete;
    Device &operator=(Device &&) = delete;

    bool has(const EntryMap &map, const String &name) const {
        return map.find(name) != map.end();
    }
    String value(const EntryMap &map, const String &name) const {
        auto it = map.find(name);
        return it == map.end() ? errno = ENOENT, nullptr : it->second.ensure_c_str();
    }
    struct udev_list_entry *listEntry(const EntryList &list) const {
        if (list.empty())
            return nullptr;
        return const_cast<struct udev_list_entry *>(list.data());
    }
    static String decodeType(char type);

public:
    static const Device &root() { static const Device root; return root; }
    Device(Device &parent, String &&name)
        : parent(&parent), path(String::concat(parent.path, '/', name)),
          initialized(), valid(), dev(makedev(0, 0)) {
        parent.childrenList.map.emplace(std::move(name), *this);
    }

    Device &newChild(String &&name) {
        childrenStorage.emplace_front(*this, std::move(name));
        return childrenStorage.front();
    }
    Device *child(const String &name) {
        auto child = childrenList.map.find(name);
        return child != childrenList.map.end() ? &child->second.get() : nullptr;
    }
    Device &operator[](const String &name) {
        return childrenList.map.at(name);
    }
    Children &children() { return childrenList; }
    Device &newBus(String &&name);
    Device &newDriver(String &&name) {
        auto child = childrenList.map.find(name);
        return child != childrenList.map.end() ? child->second.get()
                                               : newChild(std::move(name)).setDrivers();
    }

    void addChild(Device &child) {
        addChild(child.sysname(), child);
    }
    void addLink(Device &child) {
        addLink(child.sysname(), child);
    }
    void addChild(const String &name, Device &child) {
        childrenList.map.emplace(name, child);
    }
    void addLink(const String &name, Device &child) {
        addChild(name, child);
        sysattrsMap.emplace(name, nullptr);
    }
    void set(const String &property, const String &sysattr, String &&value) {
        propertiesMap.emplace(property, value);
        sysattrsMap.emplace(sysattr, std::move(value));
    }
    void setSubsystem() {
        propertiesMap.emplace("SUBSYSTEM", "subsystem");
    }
    Device &setDrivers() {
        propertiesMap.emplace("SUBSYSTEM", "drivers");
        return *this;
    }
    void setSubsystem(String &&subsystem) {
        set("SUBSYSTEM", "subsystem", std::move(subsystem));
    }
    void setDriver(String &&driver) {
        set("DRIVER", "driver", std::move(driver));
    }
    void setDev(dev_t dev, String &&devnode);
    void setUsb(unsigned busnum, unsigned devnum);
    void setUevent();
    void setController(const String &devStr);
    void finish(bool init = true);
    void copy(const EntryMap &map, EntryList &list, bool copyValues = true) const;

    static const Device *fromSyspath(const String &syspath);
    static const Device *fromDevnum(char type, dev_t dev);
    static const Device *fromSubsystemSysname(const String &subsystem, const String &sysname);
    static const Device *fromDeviceId(const String &id);
    const Device *childFromSyspath(const String &syspath) const;
    const Device *parentWithSubsystemDevtype(const String &subsystem, const String &devtype) const;
    const Device *child(const String &name) const {
        auto child = childrenList.map.find(name);
        return child != childrenList.map.end() ? &child->second.get() : nullptr;
    }
    const Device &operator[](const String &name) const {
        return childrenList.map.at(name);
    }
    const Children &children() const { return childrenList; }

    bool hasDevlink(const String &name) const { return has(devlinksMap, name); }
    String devlinkValue(const String &name) const { return value(propertiesMap, name); }
    bool hasProperty(const String &name) const { return has(propertiesMap, name); }
    String propertyValue(const String &name) const { return value(propertiesMap, name); }
    bool hasTag(const String &name) const { return has(tagsMap, name); }
    String tagValue(const String &name) const { return value(tagsMap, name); }
    bool hasSysattr(const String &name) const { return has(sysattrsMap, name); }
    String sysattrValue(const String &name) const { return value(sysattrsMap, name); }
    String sysattrPathValue(const String &name) const {
        auto components = name.rsplit('/');
        if (const Device *device = childFromSyspath(components.first))
            return device->sysattrValue(components.second);
        return nullptr;
    }

    String devpath() const { return path.ensure_c_str().substr(4); }
    String subsystem() const { return propertyValue("SUBSYSTEM"); }
    String devtype() const { return propertyValue("DEVTYPE"); }
    String syspath() const { return path.ensure_c_str(); }
    String sysname() const { return path.ensure_c_str().rsplit('/').second; }
    String sysnum() const { return path.ensure_c_str().rtok("0123456789").second; }
    String devnode() const { return propertyValue("DEVNAME"); }
    bool isInitialized() const { return initialized; }
    bool isValid() const { return valid; }
    Device *validate() { return isValid() ? this : nullptr; }
    const Device *validate() const { return isValid() ? this : nullptr; }

    struct udev_list_entry *devlinks() const { return listEntry(devlinksList); }
    struct udev_list_entry *properties() const { return listEntry(propertiesList); }
    struct udev_list_entry *tags() const { return listEntry(tagsList); }
    struct udev_list_entry *sysattr() const { return listEntry(sysattrsList); }

    String driver() const { return propertyValue("DRIVER"); }
    dev_t devnum() const { return dev; }
    String action() const { return nullptr; }
    unsigned long long int seqnum() const { return 0; }
    unsigned long long int usecSinceInitialized() const { return 0; }

    friend bool operator!=(const Device &lhs, const Device &rhs) { return &lhs != &rhs; }
    friend bool operator<(const Device &lhs, const Device &rhs) { return lhs.path < rhs.path; }

    iterator begin() { return iterator(this); }
    const_iterator cbegin() const { return const_iterator(this); }
    const_iterator begin() const { return cbegin(); }
    iterator end() { return iterator(this, true); }
    const_iterator cend() const { return const_iterator(this, true); }
    const_iterator end() const { return cend(); }
};

Device::Device() : parent(), valid() {
    Device &sys = newChild("sys");

    Device &block = sys.newChild("block");

    Device &buses = sys.newChild("bus");
    for (auto subsystem : {"hid", "pci", "usb"})
        buses.newBus(subsystem);

    Device &classes = sys.newChild("class");
    for (auto subsystem : {"input"})
        classes.newChild(subsystem);

    Device &devs = sys.newChild("dev");
    Device &devBlock = devs.newChild("block");
    Device &devChar = devs.newChild("char");

    Device &devices = sys.newChild("devices");

    Device &pci = devices.newChild("pci0000:00");
    pci.setUevent();
    pci.sysattrsMap.emplace("firmware_node", nullptr);
    pci.finish(false);

    Device &hcd = pci.newChild("0000:00:00.0");
    hcd.setDriver("xhci_hcd");
    hcd.setUevent();
    hcd.setSubsystem("pci");
    hcd.finish();

    Device &hub = hcd.newChild("usb1");
    hub.setUsb(1, 1);

    for (int num = 0; num < Global::shared_config.nb_controllers; ++num) {
        int devNum = 2 + num;
        char numStr[2], devStr[2];
        std::snprintf(numStr, sizeof(numStr), "%d", num);
        std::snprintf(devStr, sizeof(devStr), "%d", devNum);

        Device &controller = hub.newChild(String::concat("1-", devStr));
        controller.setUsb(1, devNum);

        Device &interface = controller.newChild(String::concat("1-", devStr, ":1.0"));
        interface.propertiesMap.emplace("DEVTYPE", "usb_interface");
        interface.setDriver("xpad");
        interface.setUevent();
        interface.setSubsystem("usb");
        interface.finish();

        Device &input = interface.newChild("input").newChild(String::concat("input", numStr));
        input.set("NAME", "name", "Microsoft X-Box 360 pad");
        input.set("PHYS", "phys", String::concat("usb-0000:00:00.0-", devStr, "/input", numStr));
        input.setUevent();
        input.setController(devStr);
        input.sysattrsMap.emplace("uniq", "");
        input.setSubsystem("input");
        input.finish();

        Device &id = input.newChild("id");
        id.sysattrsMap.emplace("bustype", "0003");
        id.sysattrsMap.emplace("vendor", "045e");
        id.sysattrsMap.emplace("product", "028e");
        id.sysattrsMap.emplace("version", "0114");
        id.setSubsystem();
        id.finish(false);

        Device &capabilities = input.newChild("capabilities");
        capabilities.sysattrsMap.emplace("ev", "20000b");
        capabilities.sysattrsMap.emplace("key", "7cdb000000000000 0 0 0 0");
        capabilities.sysattrsMap.emplace("rel", "0");
        capabilities.sysattrsMap.emplace("abs", "3003f");
        capabilities.sysattrsMap.emplace("msc", "0");
        capabilities.sysattrsMap.emplace("sw", "0");
        capabilities.sysattrsMap.emplace("led", "0");
        capabilities.sysattrsMap.emplace("ff", "107030000 0");
        capabilities.sysattrsMap.emplace("snd", "0");
        capabilities.setSubsystem();
        capabilities.finish(false);

        Device &event = input.newChild(String::concat("event", numStr));
        event.setDev(makedev(13, 64 + num), String::concat("/dev/input/event", numStr));
        event.setUevent();
        event.setController(devStr);
        event.setSubsystem("input");
        event.finish();

        Device &js = input.newChild(String::concat("js", numStr));
        js.setDev(makedev(13, num), String::concat("/dev/input/js", numStr));
        js.setUevent();
        js.setController(devStr);
        js.sysattrsMap.emplace("device", nullptr);
        js.setSubsystem("input");
        js.finish();
    }

    for (auto &device : devices) {
        if (device.subsystem() == "block") {
            if (device.devtype() == "disk")
                block.addLink(device);
            if (auto devNum = device.sysattrValue("dev"))
                devBlock.addLink(devNum, device);
        }
        else if (auto devNum = device.sysattrValue("dev"))
            devChar.addLink(devNum, device);
        if (auto subsystem = buses.child(device.subsystem())) {
            (*subsystem)["devices"].addLink(device);
            if (auto driver = device.driver())
                (*subsystem)["drivers"].newDriver(std::move(driver)).addLink(device);
        }
        if (auto subsystem = classes.child(device.subsystem()))
            subsystem->addLink(device);
    }

    for (auto subsystem : {&buses, &classes, &devs})
        for (auto &device : *subsystem)
            if (device != *subsystem)
                device.finish(false);
}

String Device::decodeType(char type) {
    switch (type) {
        case 'b': return "block";
        case 'c': return "char";
        default: return nullptr;
    }
}

Device &Device::newBus(String &&name) {
    Device &bus = newChild(std::move(name));
    bus.setSubsystem();
    for (auto child : {"devices", "drivers"})
        bus.newChild(child).setSubsystem();
    bus.sysattrsMap.emplace("drivers_autoprobe", "1");
    bus.sysattrsMap.emplace("resource_alignment", "");
    return bus;
}

void Device::setDev(dev_t devNum, String &&devnode) {
    char majorStr[4], minorStr[4];
    snprintf(majorStr, sizeof(majorStr), "%u", major(devNum));
    propertiesMap.emplace("MAJOR", String::concat(majorStr));
    snprintf(minorStr, sizeof(minorStr), "%u", minor(devNum));
    propertiesMap.emplace("MINOR", String::concat(minorStr));
    propertiesMap.emplace("DEVNAME", std::move(devnode));
    dev = devNum;
    sysattrsMap.emplace("dev", String::concat(majorStr, ':', minorStr));
}

void Device::setUsb(unsigned busNum, unsigned devNum) {
    char busStr[4], devStr[4];
    snprintf(busStr, sizeof(busStr), "%03u", busNum);
    propertiesMap.emplace("BUSNUM", String::concat(busStr));
    snprintf(devStr, sizeof(devStr), "%03u", devNum);
    propertiesMap.emplace("DEVNUM", String::concat(devStr));
    propertiesMap.emplace("DEVTYPE", "usb_device");
    setDriver("usb");
    setDev(makedev(189, devNum - 1), String::concat("/dev/bus/usb/", busStr, '/', devStr));
    setUevent();
    setSubsystem("usb");
    finish();
}

void Device::setUevent() {
    std::string uevent;
    for (const auto &property : propertiesMap) {
        if (!uevent.empty())
            uevent += '\n';
        uevent += property.first;
        uevent += '=';
        if (property.first == "DEVNAME")
            uevent += property.second.substr(5);
        else
            uevent += property.second;
    }
    sysattrsMap.emplace("uevent", String::concat(uevent));
}

void Device::setController(const String &devStr) {
    propertiesMap.emplace("ID_BUS", "usb");
    propertiesMap.emplace("ID_INPUT", "1");
    propertiesMap.emplace("ID_INPUT_JOYSTICK", "1");
    propertiesMap.emplace("ID_PATH", String::concat("pci-0000:00:00.0-usb-0:", devStr, ":1.0"));
    propertiesMap.emplace("ID_PATH_TAG", String::concat("pci-0000_00_00_0-usb-0_", devStr, "_1_0"));
    propertiesMap.emplace("ID_TYPE", "generic");
    propertiesMap.emplace("ID_USB_DRIVER", "xpad");
}

void Device::finish(bool init) {
    propertiesMap.emplace("DEVPATH", devpath());
    copy(devlinksMap, devlinksList);
    copy(propertiesMap, propertiesList);
    copy(tagsMap, tagsList);
    copy(sysattrsMap, sysattrsList, false);
    initialized = init;
    valid = true;
}

void Device::copy(const EntryMap &map, EntryList &list, bool copyValues) const {
    if (map.empty())
        return;
    list.reserve(map.size() + 1);
    for (const auto &entry : map)
        list.emplace_back(entry.first, copyValues ? entry.second : nullptr);
    list.emplace_back();
}

const Device *Device::fromSyspath(const String &syspath) {
    if (syspath.front() != '/')
        return nullptr;
    return root().childFromSyspath(syspath.substr(1));
}

const Device *Device::fromDevnum(char type, dev_t dev) {
    if (const Device *parent = root()["sys"]["dev"].child(decodeType(type)))
        for (const Device &device : parent->children())
            if (device.devnum() == dev)
                return &device;
    return nullptr;
}

const Device *Device::fromSubsystemSysname(const String &subsystem, const String &sysname) {
    if (const Device *parent = root()["sys"]["bus"].child(subsystem))
        return (*parent)["devices"].child(sysname);
    if (const Device *parent = root()["sys"]["class"].child(subsystem))
        return parent->child(sysname);
    return nullptr;
}

const Device *Device::fromDeviceId(const String &id) {
    if (const Device *parent = root()["sys"]["dev"].child(decodeType(id.front())))
        return parent->child(id.substr(1));
    return nullptr;
}

const Device *Device::childFromSyspath(const String &syspath) const {
    if (!syspath)
        return validate();
    auto components = syspath.split('/');
    if (const Device *device = child(components.first))
        return device->childFromSyspath(components.second);
    return nullptr;
}

const Device *Device::parentWithSubsystemDevtype(const String &subsystem, const String &devtype) const {
    if (!parent || !parent->isValid())
        return nullptr;
    if (parent->subsystem() == subsystem && (!devtype || parent->devtype() == devtype))
        return parent;
    return parent->parentWithSubsystemDevtype(subsystem, devtype);
}

}
}

struct udev_device {
    unsigned refs;
    struct udev &udev;
    struct udev_device *parent;
    const Device &device;
    udev_device(struct udev &udev, const Device &device)
        : refs(1), udev(ref(udev)),
          parent(device.parent && device.parent->isValid() ?
                 new struct udev_device(udev, *device.parent) : nullptr), device(device) {}
    ~udev_device() {
        unref(parent);
        unref(udev);
    }
};

struct udev_monitor : InactiveQueue { using InactiveQueue::InactiveQueue; };

struct udev_enumerate {
    typedef std::set<std::reference_wrapper<const Device>> DeviceSet;

    unsigned refs;
    struct udev &udev;
    std::vector<String> matchSubsystems, noMatchSubsystems, matchSysnames, matchTags;
    std::vector<struct udev_list_entry> matchSysattrs, noMatchSysattrs, matchProperties, resultsList;
    DeviceSet resultsSet;
    const Device *matchParent;
    bool matchInitialized;
    udev_enumerate(struct udev &udev) : refs(1), udev(ref(udev)), matchParent(), matchInitialized() {}
    ~udev_enumerate() { unref(udev); }
    int addSyspath(const char *syspath);
    bool matchesSubsystems(const Device &device) const;
    bool matchesSysattr(const Device &device, const struct udev_list_entry &pattern) const;
    bool matchesSysattrs(const Device &device) const;
    bool matchesProperties(const Device &device) const;
    bool matchesSysnames(const Device &device) const;
    bool matchesTags(const Device &device) const;
    bool matchesInitialized(const Device &device) const;
    bool matches(const Device &device) const;
    const Device &scan(const Device &device);
    void scanDevices();
    void scanSubsystems();
    struct udev_list_entry *results();
};

int udev_enumerate::addSyspath(const char *syspath) {
    if (struct udev_device *device = udev_device_new_from_syspath(&udev, syspath)) {
        resultsSet.emplace(device->device);
        unref(device);
        return 0;
    }
    return -EINVAL;
}

bool udev_enumerate::matchesSubsystems(const Device &device) const {
    auto subsystem = device.subsystem();
    if (!subsystem)
        subsystem = device.sysname();
    for (const auto &pattern : noMatchSubsystems)
        if (pattern.match(subsystem))
            return false;
    for (const auto &pattern : matchSubsystems)
        if (pattern.match(subsystem))
            return true;
    return matchSubsystems.empty();
}

bool udev_enumerate::matchesSysattr(const Device &device, const struct udev_list_entry &pattern) const {
    if (auto value = device.sysattrValue(pattern.name))
        return pattern.value.match(value);
    return false;
}

bool udev_enumerate::matchesSysattrs(const Device &device) const {
    for (const auto &pattern : noMatchSysattrs)
        if (matchesSysattr(device, pattern))
            return false;
    for (const auto &pattern : matchSysattrs)
        if (!matchesSysattr(device, pattern))
            return false;
    return true;
}

bool udev_enumerate::matchesProperties(const Device &device) const {
    for (const auto &pattern : matchProperties)
        for (auto property = device.properties(); *property; ++property)
            if (pattern.name.match(property->name) && pattern.value.match(property->value))
                return true;
    return matchProperties.empty();
}

bool udev_enumerate::matchesSysnames(const Device &device) const {
    auto sysname = device.sysname();
    for (const auto &pattern : matchSysnames)
        if (pattern.match(sysname))
            return true;
    return matchSysnames.empty();
}

bool udev_enumerate::matchesTags(const Device &device) const {
    for (const auto &tag : matchTags)
        if (device.hasTag(tag))
            return true;
    return matchTags.empty();
}

bool udev_enumerate::matchesInitialized(const Device &device) const {
    return !matchInitialized || device.isInitialized();
}

bool udev_enumerate::matches(const Device &device) const {
    return device.isValid() &&
        matchesInitialized(device) &&
        matchesSubsystems(device) &&
        matchesSysattrs(device) &&
        matchesProperties(device) &&
        matchesSysnames(device) &&
        matchesTags(device);
}

const Device &udev_enumerate::scan(const Device &device) {
    if (matches(device))
        resultsSet.emplace(device);
    return device;
}

void udev_enumerate::scanDevices() {
    if (matchParent) {
        for (auto &child : *matchParent)
            scan(child);
        return;
    }
    auto &sys = Device::root()["sys"];
    for (auto &deviceClass : sys["class"].children())
        for (auto &device : deviceClass.children())
            for (auto &child : device)
                scan(child);
    for (auto &bus : sys["bus"].children())
        for (auto &device : bus["devices"].children())
            for (auto &child : device)
                scan(child);
}

void udev_enumerate::scanSubsystems() {
    auto &sys = Device::root()["sys"];
    for (auto &module : sys["module"].children())
        scan(module);
    for (auto &bus : sys["bus"].children())
        for (auto &driver : scan(bus)["drivers"])
            scan(driver);
}

struct udev_list_entry *udev_enumerate::results() {
    if (resultsSet.empty())
        return nullptr;
    if (resultsList.size() <= resultsSet.size()) {
        resultsList.clear();
        resultsList.reserve(resultsSet.size() + 1);
        for (const Device &device : resultsSet)
            resultsList.emplace_back(device.syspath(), nullptr);
        resultsList.emplace_back();
    }
    return resultsList.empty() ? errno = ENODATA, nullptr : resultsList.data();
}

struct udev_queue : InactiveQueue { using InactiveQueue::InactiveQueue; };

struct udev_hwdb { unsigned refs; udev_hwdb() : refs(1) {} };

namespace libtas {
    
DEFINE_ORIG_POINTER(udev_ref)
DEFINE_ORIG_POINTER(udev_unref)
DEFINE_ORIG_POINTER(udev_new)
DEFINE_ORIG_POINTER(udev_get_userdata)
DEFINE_ORIG_POINTER(udev_set_userdata)

/*
 * udev_list
 *
 * access to libudev generated lists
 */
DEFINE_ORIG_POINTER(udev_list_entry_get_next)
DEFINE_ORIG_POINTER(udev_list_entry_get_by_name)
DEFINE_ORIG_POINTER(udev_list_entry_get_name)
DEFINE_ORIG_POINTER(udev_list_entry_get_value)

/*
 * udev_device
 *
 * access to sysfs/kernel devices
 */
DEFINE_ORIG_POINTER(udev_device_ref)
DEFINE_ORIG_POINTER(udev_device_unref)
DEFINE_ORIG_POINTER(udev_device_get_udev)
DEFINE_ORIG_POINTER(udev_device_new_from_syspath)
DEFINE_ORIG_POINTER(udev_device_new_from_devnum)
DEFINE_ORIG_POINTER(udev_device_new_from_subsystem_sysname)
DEFINE_ORIG_POINTER(udev_device_new_from_device_id)
DEFINE_ORIG_POINTER(udev_device_new_from_environment)
/* udev_device_get_parent_*() does not take a reference on the returned device, it is automatically unref'd with the parent */
DEFINE_ORIG_POINTER(udev_device_get_parent)
DEFINE_ORIG_POINTER(udev_device_get_parent_with_subsystem_devtype)
/* retrieve device properties */
DEFINE_ORIG_POINTER(udev_device_get_devpath)
DEFINE_ORIG_POINTER(udev_device_get_subsystem)
DEFINE_ORIG_POINTER(udev_device_get_devtype)
DEFINE_ORIG_POINTER(udev_device_get_syspath)
DEFINE_ORIG_POINTER(udev_device_get_sysname)
DEFINE_ORIG_POINTER(udev_device_get_sysnum)
DEFINE_ORIG_POINTER(udev_device_get_devnode)
DEFINE_ORIG_POINTER(udev_device_get_is_initialized)
DEFINE_ORIG_POINTER(udev_device_get_devlinks_list_entry)
DEFINE_ORIG_POINTER(udev_device_get_properties_list_entry)
DEFINE_ORIG_POINTER(udev_device_get_tags_list_entry)
DEFINE_ORIG_POINTER(udev_device_get_sysattr_list_entry)
DEFINE_ORIG_POINTER(udev_device_get_property_value)
DEFINE_ORIG_POINTER(udev_device_get_driver)
DEFINE_ORIG_POINTER(udev_device_get_devnum)
DEFINE_ORIG_POINTER(udev_device_get_action)
DEFINE_ORIG_POINTER(udev_device_get_seqnum)
DEFINE_ORIG_POINTER(udev_device_get_usec_since_initialized)
DEFINE_ORIG_POINTER(udev_device_get_sysattr_value)
DEFINE_ORIG_POINTER(udev_device_set_sysattr_value)
DEFINE_ORIG_POINTER(udev_device_has_tag)

/*
 * udev_monitor
 *
 * access to kernel uevents and udev events
 */
DEFINE_ORIG_POINTER(udev_monitor_ref)
DEFINE_ORIG_POINTER(udev_monitor_unref)
DEFINE_ORIG_POINTER(udev_monitor_get_udev)
/* kernel and udev generated events over netlink */
DEFINE_ORIG_POINTER(udev_monitor_new_from_netlink)
/* bind socket */
DEFINE_ORIG_POINTER(udev_monitor_enable_receiving)
DEFINE_ORIG_POINTER(udev_monitor_set_receive_buffer_size)
DEFINE_ORIG_POINTER(udev_monitor_get_fd)
DEFINE_ORIG_POINTER(udev_monitor_receive_device)
/* in-kernel socket filters to select messages that get delivered to a listener */
DEFINE_ORIG_POINTER(udev_monitor_filter_add_match_subsystem_devtype)
DEFINE_ORIG_POINTER(udev_monitor_filter_add_match_tag)
DEFINE_ORIG_POINTER(udev_monitor_filter_update)
DEFINE_ORIG_POINTER(udev_monitor_filter_remove)

/*
 * udev_enumerate
 *
 * search sysfs for specific devices and provide a sorted list
 */
DEFINE_ORIG_POINTER(udev_enumerate_ref)
DEFINE_ORIG_POINTER(udev_enumerate_unref)
DEFINE_ORIG_POINTER(udev_enumerate_get_udev)
DEFINE_ORIG_POINTER(udev_enumerate_new)
/* device properties filter */
DEFINE_ORIG_POINTER(udev_enumerate_add_match_subsystem)
DEFINE_ORIG_POINTER(udev_enumerate_add_nomatch_subsystem)
DEFINE_ORIG_POINTER(udev_enumerate_add_match_sysattr)
DEFINE_ORIG_POINTER(udev_enumerate_add_nomatch_sysattr)
DEFINE_ORIG_POINTER(udev_enumerate_add_match_property)
DEFINE_ORIG_POINTER(udev_enumerate_add_match_sysname)
DEFINE_ORIG_POINTER(udev_enumerate_add_match_tag)
DEFINE_ORIG_POINTER(udev_enumerate_add_match_parent)
DEFINE_ORIG_POINTER(udev_enumerate_add_match_is_initialized)
DEFINE_ORIG_POINTER(udev_enumerate_add_syspath)
/* run enumeration with active filters */
DEFINE_ORIG_POINTER(udev_enumerate_scan_devices)
DEFINE_ORIG_POINTER(udev_enumerate_scan_subsystems)
/* return device list */
DEFINE_ORIG_POINTER(udev_enumerate_get_list_entry)

/*
 * udev_queue
 *
 * access to the currently running udev events
 */
DEFINE_ORIG_POINTER(udev_queue_ref)
DEFINE_ORIG_POINTER(udev_queue_unref)
DEFINE_ORIG_POINTER(udev_queue_get_udev)
DEFINE_ORIG_POINTER(udev_queue_new)
DEFINE_ORIG_POINTER(udev_queue_get_udev_is_active)
DEFINE_ORIG_POINTER(udev_queue_get_queue_is_empty)
DEFINE_ORIG_POINTER(udev_queue_get_fd)
DEFINE_ORIG_POINTER(udev_queue_flush)

/*
 *  udev_hwdb
 *
 *  access to the static hardware properties database
 */
DEFINE_ORIG_POINTER(udev_hwdb_new)
DEFINE_ORIG_POINTER(udev_hwdb_ref)
DEFINE_ORIG_POINTER(udev_hwdb_unref)
DEFINE_ORIG_POINTER(udev_hwdb_get_properties_list_entry)


/* Override */ struct udev *udev_ref(struct udev *udev) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_ref, "udev");
        return orig::udev_ref(udev);
    }

    return ref(udev);
}
/* Override */ struct udev *udev_unref(struct udev *udev) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_unref, "udev");
        return orig::udev_unref(udev);
    }

    return unref(udev);
}
/* Override */ struct udev *udev_new(void) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_new, "udev");
        return orig::udev_new();
    }

    return new struct udev;
}
/* Override */ void *udev_get_userdata(struct udev *udev) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_get_userdata, "udev");
        return orig::udev_get_userdata(udev);
    }

    if (!udev)
        return errno = EINVAL, nullptr;
    return udev->userdata;
}
/* Override */ void udev_set_userdata(struct udev *udev, void *userdata) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_set_userdata, "udev");
        return orig::udev_set_userdata(udev, userdata);
    }

    if (!udev)
        return void(errno = EINVAL);
    udev->userdata = userdata;
}

/* Override */ struct udev_list_entry *udev_list_entry_get_next(struct udev_list_entry *list_entry) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_list_entry_get_next, "udev");
        return orig::udev_list_entry_get_next(list_entry);
    }

    if (!list_entry)
        return errno = EINVAL, nullptr;
    return *++list_entry ? list_entry : nullptr;
}
/* Override */ struct udev_list_entry *udev_list_entry_get_by_name(struct udev_list_entry *list_entry, const char *name) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_list_entry_get_by_name, "udev");
        return orig::udev_list_entry_get_by_name(list_entry, name);
    }

    if (!list_entry || !name)
        return errno = EINVAL, nullptr;
    const String key(name);
    do
        if (list_entry->name == key)
            return list_entry;
    while (*++list_entry);
    return errno = ENOENT, nullptr;
}
/* Override */ const char *udev_list_entry_get_name(struct udev_list_entry *list_entry) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_list_entry_get_name, "udev");
        return orig::udev_list_entry_get_name(list_entry);
    }

    if (!list_entry)
        return errno = EINVAL, nullptr;
    return list_entry->name.c_str();
}
/* Override */ const char *udev_list_entry_get_value(struct udev_list_entry *list_entry) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_list_entry_get_value, "udev");
        return orig::udev_list_entry_get_value(list_entry);
    }

    if (!list_entry)
        return errno = EINVAL, nullptr;
    return list_entry->value.c_str();
}

/* Override */ struct udev_device *udev_device_ref(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_ref, "udev");
        return orig::udev_device_ref(udev_device);
    }

    return ref(udev_device);
}
/* Override */ struct udev_device *udev_device_unref(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_unref, "udev");
        return orig::udev_device_unref(udev_device);
    }

    return unref(udev_device);
}
/* Override */ struct udev *udev_device_get_udev(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_udev, "udev");
        return orig::udev_device_get_udev(udev_device);
    }

    return get_udev(udev_device);
}
/* Override */ struct udev_device *udev_device_new_from_syspath(struct udev *udev, const char *syspath) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_new_from_syspath, "udev");
        return orig::udev_device_new_from_syspath(udev, syspath);
    }

    if (!udev || !syspath)
        return errno = EINVAL, nullptr;
    if (const Device *device = Device::fromSyspath(syspath))
        return new udev_device(*udev, *device);
    return errno = ENOENT, nullptr;
}
/* Override */ struct udev_device *udev_device_new_from_devnum(struct udev *udev, char type, dev_t devnum) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_new_from_devnum, "udev");
        return orig::udev_device_new_from_devnum(udev, type, devnum);
    }

    if (!udev)
        return errno = EINVAL, nullptr;
    if (const Device *device = Device::fromDevnum(type, devnum))
        return new struct udev_device(*udev, *device);
    return errno = ENOENT, nullptr;
}
/* Override */ struct udev_device *udev_device_new_from_subsystem_sysname(struct udev *udev, const char *subsystem, const char *sysname) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_new_from_subsystem_sysname, "udev");
        return orig::udev_device_new_from_subsystem_sysname(udev, subsystem, sysname);
    }

    if (!udev || !subsystem || !sysname)
        return errno = EINVAL, nullptr;
    if (const Device *device = Device::fromSubsystemSysname(subsystem, sysname))
        return new struct udev_device(*udev, *device);
    return errno = ENOENT, nullptr;
}
/* Override */ struct udev_device *udev_device_new_from_device_id(struct udev *udev, const char *id) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_new_from_device_id, "udev");
        return orig::udev_device_new_from_device_id(udev, id);
    }

    if (!udev || !id)
        return errno = EINVAL, nullptr;
    if (const Device *device = Device::fromDeviceId(id))
        return new struct udev_device(*udev, *device);
    return errno = ENOENT, nullptr;
}
/* Override */ struct udev_device *udev_device_new_from_environment(struct udev *udev) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_new_from_environment, "udev");
        return orig::udev_device_new_from_environment(udev);
    }

    if (!udev)
        return errno = EINVAL, nullptr;
    return errno = ENOENT, nullptr;
}
/* Override */ struct udev_device *udev_device_get_parent(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_parent, "udev");
        return orig::udev_device_get_parent(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    return udev_device->parent;
}
/* Override */ struct udev_device *udev_device_get_parent_with_subsystem_devtype(struct udev_device *udev_device,
                                                                                 const char *subsystem, const char *devtype) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_parent_with_subsystem_devtype, "udev");
        return orig::udev_device_get_parent_with_subsystem_devtype(udev_device, subsystem, devtype);
    }

    if (!udev_device || !subsystem)
        return errno = EINVAL, nullptr;
    if (const Device *device = udev_device->device.parentWithSubsystemDevtype(subsystem, devtype)) {
        while (udev_device && udev_device->device != *device)
            udev_device = udev_device->parent;
        return udev_device;
    }
    return errno = ENOENT, nullptr;
}
/* Override */ const char *udev_device_get_devpath(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_devpath, "udev");
        return orig::udev_device_get_devpath(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    return udev_device->device.devpath().c_str();
}
/* Override */ const char *udev_device_get_subsystem(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_subsystem, "udev");
        return orig::udev_device_get_subsystem(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    return udev_device->device.subsystem().c_str();
}
/* Override */ const char *udev_device_get_devtype(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_devtype, "udev");
        return orig::udev_device_get_devtype(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    return udev_device->device.devtype().c_str();
}
/* Override */ const char *udev_device_get_syspath(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_syspath, "udev");
        return orig::udev_device_get_syspath(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    return udev_device->device.syspath().c_str();
}
/* Override */ const char *udev_device_get_sysname(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_sysname, "udev");
        return orig::udev_device_get_sysname(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    return udev_device->device.sysname().c_str();
}
/* Override */ const char *udev_device_get_sysnum(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_sysnum, "udev");
        return orig::udev_device_get_sysnum(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    return udev_device->device.sysnum().c_str();
}
/* Override */ const char *udev_device_get_devnode(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_devnode, "udev");
        return orig::udev_device_get_devnode(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    return udev_device->device.devnode().c_str();
}
/* Override */ int udev_device_get_is_initialized(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_is_initialized, "udev");
        return orig::udev_device_get_is_initialized(udev_device);
    }

    if (!udev_device)
        return -EINVAL;
    return udev_device->device.isInitialized();
}
/* Override */ struct udev_list_entry *udev_device_get_devlinks_list_entry(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_devlinks_list_entry, "udev");
        return orig::udev_device_get_devlinks_list_entry(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    if (auto devlinks = udev_device->device.devlinks())
        return devlinks;
    return errno = ENODATA, nullptr;
}
/* Override */ struct udev_list_entry *udev_device_get_properties_list_entry(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_properties_list_entry, "udev");
        return orig::udev_device_get_properties_list_entry(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    if (auto properties = udev_device->device.properties())
        return properties;
    return errno = ENODATA, nullptr;
}
/* Override */ struct udev_list_entry *udev_device_get_tags_list_entry(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_tags_list_entry, "udev");
        return orig::udev_device_get_tags_list_entry(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    if (auto tags = udev_device->device.tags())
        return tags;
    return errno = ENODATA, nullptr;
}
/* Override */ struct udev_list_entry *udev_device_get_sysattr_list_entry(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_sysattr_list_entry, "udev");
        return orig::udev_device_get_sysattr_list_entry(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    if (auto sysattr = udev_device->device.sysattr())
        return sysattr;
    return errno = ENODATA, nullptr;
}
/* Override */ const char *udev_device_get_property_value(struct udev_device *udev_device, const char *key) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_property_value, "udev");
        return orig::udev_device_get_property_value(udev_device, key);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    return udev_device->device.propertyValue(key).c_str();
}
/* Override */ const char *udev_device_get_driver(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_driver, "udev");
        return orig::udev_device_get_driver(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    return udev_device->device.driver().c_str();
}
/* Override */ dev_t udev_device_get_devnum(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_devnum, "udev");
        return orig::udev_device_get_devnum(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, makedev(0, 0);
    return udev_device->device.devnum();
}
/* Override */ const char *udev_device_get_action(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_action, "udev");
        return orig::udev_device_get_action(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    return udev_device->device.action().c_str();
}
/* Override */ unsigned long long int udev_device_get_seqnum(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_seqnum, "udev");
        return orig::udev_device_get_seqnum(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, 0;
    return udev_device->device.seqnum();
}
/* Override */ unsigned long long int udev_device_get_usec_since_initialized(struct udev_device *udev_device) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_usec_since_initialized, "udev");
        return orig::udev_device_get_usec_since_initialized(udev_device);
    }

    if (!udev_device)
        return errno = EINVAL, 0;
    return udev_device->device.usecSinceInitialized();
}
/* Override */ const char *udev_device_get_sysattr_value(struct udev_device *udev_device, const char *sysattr) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_get_sysattr_value, "udev");
        return orig::udev_device_get_sysattr_value(udev_device, sysattr);
    }

    if (!udev_device)
        return errno = EINVAL, nullptr;
    return udev_device->device.sysattrPathValue(sysattr).c_str();
}
/* Override */ int udev_device_set_sysattr_value(struct udev_device *udev_device, const char *sysattr, const char *value) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_set_sysattr_value, "udev");
        return orig::udev_device_set_sysattr_value(udev_device, sysattr, value);
    }

    if (!udev_device || !sysattr)
        return -EINVAL;
    return udev_device->device.hasSysattr(sysattr) ? -EPERM : -ENOENT;
}
/* Override */ int udev_device_has_tag(struct udev_device *udev_device, const char *tag) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_device_has_tag, "udev");
        return orig::udev_device_has_tag(udev_device, tag);
    }

    if (!udev_device)
        return -EINVAL;
    return udev_device->device.hasTag(tag);
}

/* Override */ struct udev_monitor *udev_monitor_ref(struct udev_monitor *udev_monitor) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_monitor_ref, "udev");
        return orig::udev_monitor_ref(udev_monitor);
    }

    return ref(udev_monitor);
}
/* Override */ struct udev_monitor *udev_monitor_unref(struct udev_monitor *udev_monitor) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_monitor_unref, "udev");
        return orig::udev_monitor_unref(udev_monitor);
    }

    return unref(udev_monitor);
}
/* Override */ struct udev *udev_monitor_get_udev(struct udev_monitor *udev_monitor) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_monitor_get_udev, "udev");
        return orig::udev_monitor_get_udev(udev_monitor);
    }

    return get_udev(udev_monitor);
}
/* Override */ struct udev_monitor *udev_monitor_new_from_netlink(struct udev *udev, const char *name) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_monitor_new_from_netlink, "udev");
        return orig::udev_monitor_new_from_netlink(udev, name);
    }

    return new_from_udev<struct udev_monitor>(udev);
}
/* Override */ int udev_monitor_enable_receiving(struct udev_monitor *udev_monitor) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_monitor_enable_receiving, "udev");
        return orig::udev_monitor_enable_receiving(udev_monitor);
    }

    if (!udev_monitor)
        return -EINVAL;
    return 0;
}
/* Override */ int udev_monitor_set_receive_buffer_size(struct udev_monitor *udev_monitor, int size) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_monitor_set_receive_buffer_size, "udev");
        return orig::udev_monitor_set_receive_buffer_size(udev_monitor, size);
    }

    if (!udev_monitor)
        return -EINVAL;
    return 0;
}
/* Override */ int udev_monitor_get_fd(struct udev_monitor *udev_monitor) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_monitor_get_fd, "udev");
        return orig::udev_monitor_get_fd(udev_monitor);
    }

    if (!udev_monitor)
        return -EINVAL;
    return udev_monitor->fd;
}
/* Override */ struct udev_device *udev_monitor_receive_device(struct udev_monitor *udev_monitor) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_monitor_receive_device, "udev");
        return orig::udev_monitor_receive_device(udev_monitor);
    }

    if (!udev_monitor)
        return errno = EINVAL, nullptr;
    return errno = EAGAIN, nullptr;
}
/* Override */ int udev_monitor_filter_add_match_subsystem_devtype(struct udev_monitor *udev_monitor,
                                                                   const char *subsystem, const char *devtype) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_monitor_filter_add_match_subsystem_devtype, "udev");
        return orig::udev_monitor_filter_add_match_subsystem_devtype(udev_monitor, subsystem, devtype);
    }

    if (!udev_monitor || !subsystem)
        return -EINVAL;
    return 0;
}
/* Override */ int udev_monitor_filter_add_match_tag(struct udev_monitor *udev_monitor, const char *tag) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_monitor_filter_add_match_tag, "udev");
        return orig::udev_monitor_filter_add_match_tag(udev_monitor, tag);
    }

    if (!udev_monitor || !tag)
        return -EINVAL;
    return 0;
}
/* Override */ int udev_monitor_filter_update(struct udev_monitor *udev_monitor) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_monitor_filter_update, "udev");
        return orig::udev_monitor_filter_update(udev_monitor);
    }

    if (!udev_monitor)
        return -EINVAL;
    return 0;
}
/* Override */ int udev_monitor_filter_remove(struct udev_monitor *udev_monitor) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_monitor_filter_remove, "udev");
        return orig::udev_monitor_filter_remove(udev_monitor);
    }

    if (!udev_monitor)
        return -EINVAL;
    return 0;
}

/* Override */ struct udev_enumerate *udev_enumerate_ref(struct udev_enumerate *udev_enumerate) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_ref, "udev");
        return orig::udev_enumerate_ref(udev_enumerate);
    }

    return ref(udev_enumerate);
}
/* Override */ struct udev_enumerate *udev_enumerate_unref(struct udev_enumerate *udev_enumerate) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_unref, "udev");
        return orig::udev_enumerate_unref(udev_enumerate);
    }

    return unref(udev_enumerate);
}
/* Override */ struct udev *udev_enumerate_get_udev(struct udev_enumerate *udev_enumerate) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_get_udev, "udev");
        return orig::udev_enumerate_get_udev(udev_enumerate);
    }

    return get_udev(udev_enumerate);
}
/* Override */ struct udev_enumerate *udev_enumerate_new(struct udev *udev) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_new, "udev");
        return orig::udev_enumerate_new(udev);
    }

    return new_from_udev<struct udev_enumerate>(udev);
}
/* Override */ int udev_enumerate_add_match_subsystem(struct udev_enumerate *udev_enumerate, const char *subsystem) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_add_match_subsystem, "udev");
        return orig::udev_enumerate_add_match_subsystem(udev_enumerate, subsystem);
    }

    if (!udev_enumerate || !subsystem)
        return -EINVAL;
    udev_enumerate->matchSubsystems.emplace_back(String::concat(subsystem));
    return 0;
}
/* Override */ int udev_enumerate_add_nomatch_subsystem(struct udev_enumerate *udev_enumerate, const char *subsystem) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_add_nomatch_subsystem, "udev");
        return orig::udev_enumerate_add_nomatch_subsystem(udev_enumerate, subsystem);
    }

    if (!udev_enumerate || !subsystem)
        return -EINVAL;
    udev_enumerate->noMatchSubsystems.emplace_back(String::concat(subsystem));
    return 0;
}
/* Override */ int udev_enumerate_add_match_sysattr(struct udev_enumerate *udev_enumerate, const char *sysattr, const char *value) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_add_match_sysattr, "udev");
        return orig::udev_enumerate_add_match_sysattr(udev_enumerate, sysattr, value);
    }

    if (!udev_enumerate || !sysattr)
        return -EINVAL;
    udev_enumerate->matchSysattrs.emplace_back(String::concat(sysattr), String::concat(value));
    return 0;
}
/* Override */ int udev_enumerate_add_nomatch_sysattr(struct udev_enumerate *udev_enumerate, const char *sysattr, const char *value) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_add_nomatch_sysattr, "udev");
        return orig::udev_enumerate_add_nomatch_sysattr(udev_enumerate, sysattr, value);
    }

    if (!udev_enumerate || !sysattr)
        return -EINVAL;
    udev_enumerate->noMatchSysattrs.emplace_back(String::concat(sysattr), String::concat(value));
    return 0;
}
/* Override */ int udev_enumerate_add_match_property(struct udev_enumerate *udev_enumerate, const char *property, const char *value) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_add_match_property, "udev");
        return orig::udev_enumerate_add_match_property(udev_enumerate, property, value);
    }

    if (!udev_enumerate || !property)
        return -EINVAL;
    udev_enumerate->matchProperties.emplace_back(String::concat(property), String::concat(value));
    return 0;
}
/* Override */ int udev_enumerate_add_match_sysname(struct udev_enumerate *udev_enumerate, const char *sysname) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_add_match_sysname, "udev");
        return orig::udev_enumerate_add_match_sysname(udev_enumerate, sysname);
    }

    if (!udev_enumerate || !sysname)
        return -EINVAL;
    udev_enumerate->matchSysnames.emplace_back(String::concat(sysname));
    return 0;
}
/* Override */ int udev_enumerate_add_match_tag(struct udev_enumerate *udev_enumerate, const char *tag) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_add_match_tag, "udev");
        return orig::udev_enumerate_add_match_tag(udev_enumerate, tag);
    }

    if (!udev_enumerate || !tag)
        return -EINVAL;
    udev_enumerate->matchTags.emplace_back(String::concat(tag));
    return 0;
}
/* Override */ int udev_enumerate_add_match_parent(struct udev_enumerate *udev_enumerate, struct udev_device *parent) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_add_match_parent, "udev");
        return orig::udev_enumerate_add_match_parent(udev_enumerate, parent);
    }

    if (!udev_enumerate || !parent)
        return -EINVAL;
    udev_enumerate->matchParent = &parent->device;
    return 0;
}
/* Override */ int udev_enumerate_add_match_is_initialized(struct udev_enumerate *udev_enumerate) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_add_match_is_initialized, "udev");
        return orig::udev_enumerate_add_match_is_initialized(udev_enumerate);
    }

    if (!udev_enumerate)
        return -EINVAL;
    udev_enumerate->matchInitialized = true;
    return 0;
}
/* Override */ int udev_enumerate_add_syspath(struct udev_enumerate *udev_enumerate, const char *syspath) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_add_syspath, "udev");
        return orig::udev_enumerate_add_syspath(udev_enumerate, syspath);
    }

    if (!udev_enumerate || !syspath)
        return -EINVAL;
    udev_enumerate->addSyspath(syspath);
    return 0;
}
/* Override */ int udev_enumerate_scan_devices(struct udev_enumerate *udev_enumerate) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_scan_devices, "udev");
        return orig::udev_enumerate_scan_devices(udev_enumerate);
    }

    if (!udev_enumerate)
        return -EINVAL;
    udev_enumerate->scanDevices();
    return 0;
}
/* Override */ int udev_enumerate_scan_subsystems(struct udev_enumerate *udev_enumerate) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_scan_subsystems, "udev");
        return orig::udev_enumerate_scan_subsystems(udev_enumerate);
    }

    if (!udev_enumerate)
        return -EINVAL;
    udev_enumerate->scanSubsystems();
    return 0;
}
/* Override */ struct udev_list_entry *udev_enumerate_get_list_entry(struct udev_enumerate *udev_enumerate) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_enumerate_get_list_entry, "udev");
        return orig::udev_enumerate_get_list_entry(udev_enumerate);
    }

    if (!udev_enumerate)
        return errno = EINVAL, nullptr;
    if (auto results = udev_enumerate->results())
        return results;
    return errno = ENODATA, nullptr;
}

/* Override */ struct udev_queue *udev_queue_ref(struct udev_queue *udev_queue) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_queue_ref, "udev");
        return orig::udev_queue_ref(udev_queue);
    }

    return ref(udev_queue);
}
/* Override */ struct udev_queue *udev_queue_unref(struct udev_queue *udev_queue) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_queue_unref, "udev");
        return orig::udev_queue_unref(udev_queue);
    }

    return unref(udev_queue);
}
/* Override */ struct udev *udev_queue_get_udev(struct udev_queue *udev_queue) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_queue_get_udev, "udev");
        return orig::udev_queue_get_udev(udev_queue);
    }

    return get_udev(udev_queue);
}
/* Override */ struct udev_queue *udev_queue_new(struct udev *udev) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_queue_new, "udev");
        return orig::udev_queue_new(udev);
    }

    return new_from_udev<struct udev_queue>(udev);
}
/* Override */ int udev_queue_get_udev_is_active(struct udev_queue *udev_queue) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_queue_get_udev_is_active, "udev");
        return orig::udev_queue_get_udev_is_active(udev_queue);
    }

    if (!udev_queue)
        return -EINVAL;
    return true;
}
/* Override */ int udev_queue_get_queue_is_empty(struct udev_queue *udev_queue) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_queue_get_queue_is_empty, "udev");
        return orig::udev_queue_get_queue_is_empty(udev_queue);
    }

    if (!udev_queue)
        return -EINVAL;
    return false;
}
/* Override */ int udev_queue_get_fd(struct udev_queue *udev_queue) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_queue_get_fd, "udev");
        return orig::udev_queue_get_fd(udev_queue);
    }

    if (!udev_queue)
        return -EINVAL;
    return udev_queue->fd;
}
/* Override */ int udev_queue_flush(struct udev_queue *udev_queue) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_queue_flush, "udev");
        return orig::udev_queue_flush(udev_queue);
    }

    if (!udev_queue)
        return -EINVAL;
    return 0;
}

/* Override */ struct udev_hwdb *udev_hwdb_new(struct udev *udev) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_hwdb_new, "udev");
        return orig::udev_hwdb_new(udev);
    }

    return new struct udev_hwdb;
}
/* Override */ struct udev_hwdb *udev_hwdb_ref(struct udev_hwdb *hwdb) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_hwdb_ref, "udev");
        return orig::udev_hwdb_ref(hwdb);
    }

    return ref(hwdb);
}
/* Override */ struct udev_hwdb *udev_hwdb_unref(struct udev_hwdb *hwdb) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_hwdb_unref, "udev");
        return orig::udev_hwdb_unref(hwdb);
    }

    return unref(hwdb);
}
/* Override */ struct udev_list_entry *udev_hwdb_get_properties_list_entry(struct udev_hwdb *hwdb, const char *modalias, unsigned int flags) {
    DEBUGLOGCALL(LCF_FILEIO);
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO) {
        LINK_NAMESPACE(udev_hwdb_get_properties_list_entry, "udev");
        return orig::udev_hwdb_get_properties_list_entry(hwdb, modalias, flags);
    }

    if (!hwdb || !modalias)
        return errno = EINVAL, nullptr;
    return errno = ENODATA, nullptr;
}

}
