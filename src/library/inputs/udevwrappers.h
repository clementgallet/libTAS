/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_UDEV_H_INCLUDED
#define LIBTAS_UDEV_H_INCLUDED

#include "hook.h"

#include <sys/types.h>

struct udev;
struct udev_list_entry;
struct udev_device;
struct udev_monitor;
struct udev_enumerate;
struct udev_queue;
struct udev_hwdb;

namespace libtas {

/*
 * udev - library context
 *
 * reads the udev config and system environment
 * allows custom logging
 */
OVERRIDE struct udev *udev_ref(struct udev *udev);
OVERRIDE struct udev *udev_unref(struct udev *udev);
OVERRIDE struct udev *udev_new(void);
OVERRIDE void *udev_get_userdata(struct udev *udev);
OVERRIDE void udev_set_userdata(struct udev *udev, void *userdata);

/*
 * udev_list
 *
 * access to libudev generated lists
 */
OVERRIDE struct udev_list_entry *udev_list_entry_get_next(struct udev_list_entry *list_entry);
OVERRIDE struct udev_list_entry *udev_list_entry_get_by_name(struct udev_list_entry *list_entry, const char *name);
OVERRIDE const char *udev_list_entry_get_name(struct udev_list_entry *list_entry);
OVERRIDE const char *udev_list_entry_get_value(struct udev_list_entry *list_entry);

/*
 * udev_device
 *
 * access to sysfs/kernel devices
 */
OVERRIDE struct udev_device *udev_device_ref(struct udev_device *udev_device);
OVERRIDE struct udev_device *udev_device_unref(struct udev_device *udev_device);
OVERRIDE struct udev *udev_device_get_udev(struct udev_device *udev_device);
OVERRIDE struct udev_device *udev_device_new_from_syspath(struct udev *udev, const char *syspath);
OVERRIDE struct udev_device *udev_device_new_from_devnum(struct udev *udev, char type, dev_t devnum);
OVERRIDE struct udev_device *udev_device_new_from_subsystem_sysname(struct udev *udev, const char *subsystem, const char *sysname);
OVERRIDE struct udev_device *udev_device_new_from_device_id(struct udev *udev, const char *id);
OVERRIDE struct udev_device *udev_device_new_from_environment(struct udev *udev);
/* udev_device_get_parent_*() does not take a reference on the returned device, it is automatically unref'd with the parent */
OVERRIDE struct udev_device *udev_device_get_parent(struct udev_device *udev_device);
OVERRIDE struct udev_device *udev_device_get_parent_with_subsystem_devtype(struct udev_device *udev_device,
                                                                           const char *subsystem, const char *devtype);
/* retrieve device properties */
OVERRIDE const char *udev_device_get_devpath(struct udev_device *udev_device);
OVERRIDE const char *udev_device_get_subsystem(struct udev_device *udev_device);
OVERRIDE const char *udev_device_get_devtype(struct udev_device *udev_device);
OVERRIDE const char *udev_device_get_syspath(struct udev_device *udev_device);
OVERRIDE const char *udev_device_get_sysname(struct udev_device *udev_device);
OVERRIDE const char *udev_device_get_sysnum(struct udev_device *udev_device);
OVERRIDE const char *udev_device_get_devnode(struct udev_device *udev_device);
OVERRIDE int udev_device_get_is_initialized(struct udev_device *udev_device);
OVERRIDE struct udev_list_entry *udev_device_get_devlinks_list_entry(struct udev_device *udev_device);
OVERRIDE struct udev_list_entry *udev_device_get_properties_list_entry(struct udev_device *udev_device);
OVERRIDE struct udev_list_entry *udev_device_get_tags_list_entry(struct udev_device *udev_device);
OVERRIDE struct udev_list_entry *udev_device_get_sysattr_list_entry(struct udev_device *udev_device);
OVERRIDE const char *udev_device_get_property_value(struct udev_device *udev_device, const char *key);
OVERRIDE const char *udev_device_get_driver(struct udev_device *udev_device);
OVERRIDE dev_t udev_device_get_devnum(struct udev_device *udev_device);
OVERRIDE const char *udev_device_get_action(struct udev_device *udev_device);
OVERRIDE unsigned long long int udev_device_get_seqnum(struct udev_device *udev_device);
OVERRIDE unsigned long long int udev_device_get_usec_since_initialized(struct udev_device *udev_device);
OVERRIDE const char *udev_device_get_sysattr_value(struct udev_device *udev_device, const char *sysattr);
OVERRIDE int udev_device_set_sysattr_value(struct udev_device *udev_device, const char *sysattr, const char *value);
OVERRIDE int udev_device_has_tag(struct udev_device *udev_device, const char *tag);

/*
 * udev_monitor
 *
 * access to kernel uevents and udev events
 */
OVERRIDE struct udev_monitor *udev_monitor_ref(struct udev_monitor *udev_monitor);
OVERRIDE struct udev_monitor *udev_monitor_unref(struct udev_monitor *udev_monitor);
OVERRIDE struct udev *udev_monitor_get_udev(struct udev_monitor *udev_monitor);
/* kernel and udev generated events over netlink */
OVERRIDE struct udev_monitor *udev_monitor_new_from_netlink(struct udev *udev, const char *name);
/* bind socket */
OVERRIDE int udev_monitor_enable_receiving(struct udev_monitor *udev_monitor);
OVERRIDE int udev_monitor_set_receive_buffer_size(struct udev_monitor *udev_monitor, int size);
OVERRIDE int udev_monitor_get_fd(struct udev_monitor *udev_monitor);
OVERRIDE struct udev_device *udev_monitor_receive_device(struct udev_monitor *udev_monitor);
/* in-kernel socket filters to select messages that get delivered to a listener */
OVERRIDE int udev_monitor_filter_add_match_subsystem_devtype(struct udev_monitor *udev_monitor,
                                                             const char *subsystem, const char *devtype);
OVERRIDE int udev_monitor_filter_add_match_tag(struct udev_monitor *udev_monitor, const char *tag);
OVERRIDE int udev_monitor_filter_update(struct udev_monitor *udev_monitor);
OVERRIDE int udev_monitor_filter_remove(struct udev_monitor *udev_monitor);

/*
 * udev_enumerate
 *
 * search sysfs for specific devices and provide a sorted list
 */
OVERRIDE struct udev_enumerate *udev_enumerate_ref(struct udev_enumerate *udev_enumerate);
OVERRIDE struct udev_enumerate *udev_enumerate_unref(struct udev_enumerate *udev_enumerate);
OVERRIDE struct udev *udev_enumerate_get_udev(struct udev_enumerate *udev_enumerate);
OVERRIDE struct udev_enumerate *udev_enumerate_new(struct udev *udev);
/* device properties filter */
OVERRIDE int udev_enumerate_add_match_subsystem(struct udev_enumerate *udev_enumerate, const char *subsystem);
OVERRIDE int udev_enumerate_add_nomatch_subsystem(struct udev_enumerate *udev_enumerate, const char *subsystem);
OVERRIDE int udev_enumerate_add_match_sysattr(struct udev_enumerate *udev_enumerate, const char *sysattr, const char *value);
OVERRIDE int udev_enumerate_add_nomatch_sysattr(struct udev_enumerate *udev_enumerate, const char *sysattr, const char *value);
OVERRIDE int udev_enumerate_add_match_property(struct udev_enumerate *udev_enumerate, const char *property, const char *value);
OVERRIDE int udev_enumerate_add_match_sysname(struct udev_enumerate *udev_enumerate, const char *sysname);
OVERRIDE int udev_enumerate_add_match_tag(struct udev_enumerate *udev_enumerate, const char *tag);
OVERRIDE int udev_enumerate_add_match_parent(struct udev_enumerate *udev_enumerate, struct udev_device *parent);
OVERRIDE int udev_enumerate_add_match_is_initialized(struct udev_enumerate *udev_enumerate);
OVERRIDE int udev_enumerate_add_syspath(struct udev_enumerate *udev_enumerate, const char *syspath);
/* run enumeration with active filters */
OVERRIDE int udev_enumerate_scan_devices(struct udev_enumerate *udev_enumerate);
OVERRIDE int udev_enumerate_scan_subsystems(struct udev_enumerate *udev_enumerate);
/* return device list */
OVERRIDE struct udev_list_entry *udev_enumerate_get_list_entry(struct udev_enumerate *udev_enumerate);

/*
 * udev_queue
 *
 * access to the currently running udev events
 */
OVERRIDE struct udev_queue *udev_queue_ref(struct udev_queue *udev_queue);
OVERRIDE struct udev_queue *udev_queue_unref(struct udev_queue *udev_queue);
OVERRIDE struct udev *udev_queue_get_udev(struct udev_queue *udev_queue);
OVERRIDE struct udev_queue *udev_queue_new(struct udev *udev);
OVERRIDE int udev_queue_get_udev_is_active(struct udev_queue *udev_queue);
OVERRIDE int udev_queue_get_queue_is_empty(struct udev_queue *udev_queue);
OVERRIDE int udev_queue_get_fd(struct udev_queue *udev_queue);
OVERRIDE int udev_queue_flush(struct udev_queue *udev_queue);

/*
 *  udev_hwdb
 *
 *  access to the static hardware properties database
 */
OVERRIDE struct udev_hwdb *udev_hwdb_new(struct udev *udev);
OVERRIDE struct udev_hwdb *udev_hwdb_ref(struct udev_hwdb *hwdb);
OVERRIDE struct udev_hwdb *udev_hwdb_unref(struct udev_hwdb *hwdb);
OVERRIDE struct udev_list_entry *udev_hwdb_get_properties_list_entry(struct udev_hwdb *hwdb, const char *modalias, unsigned int flags);

}

#endif
