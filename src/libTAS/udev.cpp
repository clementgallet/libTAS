#include "udev.h"
#include "hook.h"
#include "logging.h"

struct udev {int dummy;};
struct udev_list_entry {int dummy;};
struct udev_device {int dummy;};
struct udev_enumerate {int dummy;};

struct udev *(*udev_ref_real)(struct udev *udev);
struct udev *(*udev_unref_real)(struct udev *udev);
struct udev *(*udev_new_real)(void);
struct udev_list_entry *(*udev_list_entry_get_next_real)(struct udev_list_entry *list_entry);
struct udev_list_entry *(*udev_list_entry_get_by_name_real)(struct udev_list_entry *list_entry, const char *name);
const char *(*udev_list_entry_get_name_real)(struct udev_list_entry *list_entry);
const char *(*udev_list_entry_get_value_real)(struct udev_list_entry *list_entry);
struct udev_device *(*udev_device_ref_real)(struct udev_device *udev_device);
struct udev_device *(*udev_device_unref_real)(struct udev_device *udev_device);
struct udev *(*udev_device_get_udev_real)(struct udev_device *udev_device);
struct udev_device *(*udev_device_new_from_syspath_real)(struct udev *udev, const char *syspath);
struct udev_device *(*udev_device_new_from_devnum_real)(struct udev *udev, char type, dev_t devnum);
struct udev_device *(*udev_device_new_from_subsystem_sysname_real)(struct udev *udev, const char *subsystem, const char *sysname);
struct udev_device *(*udev_device_new_from_device_id_real)(struct udev *udev, const char *id);
struct udev_device *(*udev_device_new_from_environment_real)(struct udev *udev);
struct udev_device *(*udev_device_get_parent_real)(struct udev_device *udev_device);
struct udev_device *(*udev_device_get_parent_with_subsystem_devtype_real)(struct udev_device *udev_device, const char *subsystem, const char *devtype);
struct udev_enumerate *(*udev_enumerate_ref_real)(struct udev_enumerate *udev_enumerate);
struct udev_enumerate *(*udev_enumerate_unref_real)(struct udev_enumerate *udev_enumerate);
struct udev *(*udev_enumerate_get_udev_real)(struct udev_enumerate *udev_enumerate);
struct udev_enumerate *(*udev_enumerate_new_real)(struct udev *udev);
int (*udev_enumerate_add_match_subsystem_real)(struct udev_enumerate *udev_enumerate, const char *subsystem);
int (*udev_enumerate_add_nomatch_subsystem_real)(struct udev_enumerate *udev_enumerate, const char *subsystem);
int (*udev_enumerate_add_match_sysattr_real)(struct udev_enumerate *udev_enumerate, const char *sysattr, const char *value);
int (*udev_enumerate_add_nomatch_sysattr_real)(struct udev_enumerate *udev_enumerate, const char *sysattr, const char *value);
int (*udev_enumerate_add_match_property_real)(struct udev_enumerate *udev_enumerate, const char *property, const char *value);
int (*udev_enumerate_add_match_sysname_real)(struct udev_enumerate *udev_enumerate, const char *sysname);
int (*udev_enumerate_add_match_tag_real)(struct udev_enumerate *udev_enumerate, const char *tag);
int (*udev_enumerate_add_match_parent_real)(struct udev_enumerate *udev_enumerate, struct udev_device *parent);
int (*udev_enumerate_add_match_is_initialized_real)(struct udev_enumerate *udev_enumerate);
int (*udev_enumerate_add_syspath_real)(struct udev_enumerate *udev_enumerate, const char *syspath);
int (*udev_enumerate_scan_devices_real)(struct udev_enumerate *udev_enumerate);
int (*udev_enumerate_scan_subsystems_real)(struct udev_enumerate *udev_enumerate);
struct udev_list_entry *(*udev_enumerate_get_list_entry_real)(struct udev_enumerate *udev_enumerate);



void link_udev(void);

struct udev *udev_ref(struct udev *udev)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_ref_real(udev);
}

struct udev *udev_unref(struct udev *udev)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_unref_real(udev);
}

struct udev *udev_new(void)
{
    DEBUGLOGCALL(LCF_UDEV);
    link_udev();
    return udev_new_real();
}

struct udev_list_entry *udev_list_entry_get_next(struct udev_list_entry *list_entry)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_list_entry_get_next_real(list_entry);
}
struct udev_list_entry *udev_list_entry_get_by_name(struct udev_list_entry *list_entry, const char *name)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_list_entry_get_by_name_real(list_entry, name);
}
const char *udev_list_entry_get_name(struct udev_list_entry *list_entry)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_list_entry_get_name_real(list_entry);
}
const char *udev_list_entry_get_value(struct udev_list_entry *list_entry)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_list_entry_get_value_real(list_entry);
}

struct udev_device *udev_device_ref(struct udev_device *udev_device)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_device_ref_real(udev_device);
}

struct udev_device *udev_device_unref(struct udev_device *udev_device)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_device_unref_real(udev_device);
}

struct udev *udev_device_get_udev(struct udev_device *udev_device)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_device_get_udev_real(udev_device);
}
struct udev_device *udev_device_new_from_syspath(struct udev *udev, const char *syspath)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_device_new_from_syspath_real(udev, syspath);
}
struct udev_device *udev_device_new_from_devnum(struct udev *udev, char type, dev_t devnum)
{
    debuglog(LCF_UDEV, __func__, " call with type ", type, " and devnum ", devnum);
    return udev_device_new_from_devnum_real(udev, type, devnum);
}
struct udev_device *udev_device_new_from_subsystem_sysname(struct udev *udev, const char *subsystem, const char *sysname)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_device_new_from_subsystem_sysname_real(udev, subsystem, sysname);
}
struct udev_device *udev_device_new_from_device_id(struct udev *udev, const char *id)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_device_new_from_device_id_real(udev, id);
}
struct udev_device *udev_device_new_from_environment(struct udev *udev)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_device_new_from_environment_real(udev);
}
struct udev_device *udev_device_get_parent(struct udev_device *udev_device)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_device_get_parent_real(udev_device);
}
struct udev_device *udev_device_get_parent_with_subsystem_devtype(struct udev_device *udev_device,
                                                                  const char *subsystem, const char *devtype)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_device_get_parent_with_subsystem_devtype_real(udev_device, subsystem, devtype);
}

struct udev_enumerate *udev_enumerate_ref(struct udev_enumerate *udev_enumerate)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_ref_real(udev_enumerate);
}

struct udev_enumerate *udev_enumerate_unref(struct udev_enumerate *udev_enumerate)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_unref_real(udev_enumerate);
}

struct udev *udev_enumerate_get_udev(struct udev_enumerate *udev_enumerate)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_get_udev_real(udev_enumerate);
}

struct udev_enumerate *udev_enumerate_new(struct udev *udev)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_new_real(udev);
}

int udev_enumerate_add_match_subsystem(struct udev_enumerate *udev_enumerate, const char *subsystem)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_add_match_subsystem_real(udev_enumerate, subsystem);
}

int udev_enumerate_add_nomatch_subsystem(struct udev_enumerate *udev_enumerate, const char *subsystem)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_add_nomatch_subsystem_real(udev_enumerate, subsystem);
}

int udev_enumerate_add_match_sysattr(struct udev_enumerate *udev_enumerate, const char *sysattr, const char *value)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_add_match_sysattr_real(udev_enumerate, sysattr, value);
}

int udev_enumerate_add_nomatch_sysattr(struct udev_enumerate *udev_enumerate, const char *sysattr, const char *value)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_add_nomatch_sysattr_real(udev_enumerate, sysattr, value);
}

int udev_enumerate_add_match_property(struct udev_enumerate *udev_enumerate, const char *property, const char *value)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_add_match_property_real(udev_enumerate, property, value);
}

int udev_enumerate_add_match_sysname(struct udev_enumerate *udev_enumerate, const char *sysname)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_add_match_sysname_real(udev_enumerate, sysname);
}

int udev_enumerate_add_match_tag(struct udev_enumerate *udev_enumerate, const char *tag)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_add_match_tag_real(udev_enumerate, tag);
}

int udev_enumerate_add_match_parent(struct udev_enumerate *udev_enumerate, struct udev_device *parent)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_add_match_parent_real(udev_enumerate, parent);
}

int udev_enumerate_add_match_is_initialized(struct udev_enumerate *udev_enumerate)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_add_match_is_initialized_real(udev_enumerate);
}

int udev_enumerate_add_syspath(struct udev_enumerate *udev_enumerate, const char *syspath)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_add_syspath_real(udev_enumerate, syspath);
}

int udev_enumerate_scan_devices(struct udev_enumerate *udev_enumerate)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_scan_devices_real(udev_enumerate);
}

int udev_enumerate_scan_subsystems(struct udev_enumerate *udev_enumerate)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_scan_subsystems_real(udev_enumerate);
}

struct udev_list_entry *udev_enumerate_get_list_entry(struct udev_enumerate *udev_enumerate)
{
    DEBUGLOGCALL(LCF_UDEV);
    return udev_enumerate_get_list_entry_real(udev_enumerate);
}

void link_udev(void)
{
    LINK_SUFFIX(udev_ref, "libudev");
    LINK_SUFFIX(udev_unref, "libudev");
    LINK_SUFFIX(udev_new, "libudev");
    LINK_SUFFIX(udev_list_entry_get_next, "libudev");
    LINK_SUFFIX(udev_list_entry_get_by_name, "libudev");
    LINK_SUFFIX(udev_list_entry_get_name, "libudev");
    LINK_SUFFIX(udev_list_entry_get_value, "libudev");
    LINK_SUFFIX(udev_device_ref, "libudev");
    LINK_SUFFIX(udev_device_unref, "libudev");
    LINK_SUFFIX(udev_device_get_udev, "libudev");
    LINK_SUFFIX(udev_device_new_from_syspath, "libudev");
    LINK_SUFFIX(udev_device_new_from_devnum, "libudev");
    LINK_SUFFIX(udev_device_new_from_subsystem_sysname, "libudev");
    LINK_SUFFIX(udev_device_new_from_device_id, "libudev");
    LINK_SUFFIX(udev_device_new_from_environment, "libudev");
    LINK_SUFFIX(udev_device_get_parent, "libudev");
    LINK_SUFFIX(udev_device_get_parent_with_subsystem_devtype, "libudev");
    LINK_SUFFIX(udev_enumerate_ref, "libudev");
    LINK_SUFFIX(udev_enumerate_unref, "libudev");
    LINK_SUFFIX(udev_enumerate_get_udev, "libudev");
    LINK_SUFFIX(udev_enumerate_new, "libudev");
    LINK_SUFFIX(udev_enumerate_add_match_subsystem, "libudev");
    LINK_SUFFIX(udev_enumerate_add_nomatch_subsystem, "libudev");
    LINK_SUFFIX(udev_enumerate_add_match_sysattr, "libudev");
    LINK_SUFFIX(udev_enumerate_add_nomatch_sysattr, "libudev");
    LINK_SUFFIX(udev_enumerate_add_match_property, "libudev");
    LINK_SUFFIX(udev_enumerate_add_match_sysname, "libudev");
    LINK_SUFFIX(udev_enumerate_add_match_tag, "libudev");
    LINK_SUFFIX(udev_enumerate_add_match_parent, "libudev");
    LINK_SUFFIX(udev_enumerate_add_match_is_initialized, "libudev");
    LINK_SUFFIX(udev_enumerate_add_syspath, "libudev");
    LINK_SUFFIX(udev_enumerate_scan_devices, "libudev");
    LINK_SUFFIX(udev_enumerate_scan_subsystems, "libudev");
    LINK_SUFFIX(udev_enumerate_get_list_entry, "libudev");
}

