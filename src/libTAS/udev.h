/***
  This file is part of systemd.

  Copyright 2008-2012 Kay Sievers <kay@vrfy.org>

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#ifndef UDEV_H_INCL
#define UDEV_H_INCL

#include "global.h"
#include <sys/types.h> // For dev_t type

/*
 * udev - library context
 *
 * reads the udev config and system environment
 * allows custom logging
 */
struct udev;
OVERRIDE struct udev *udev_ref(struct udev *udev);
OVERRIDE struct udev *udev_unref(struct udev *udev);
OVERRIDE struct udev *udev_new(void);

/*
 * udev_list
 *
 * access to libudev generated lists
 */
struct udev_list_entry;
OVERRIDE struct udev_list_entry *udev_list_entry_get_next(struct udev_list_entry *list_entry);
OVERRIDE struct udev_list_entry *udev_list_entry_get_by_name(struct udev_list_entry *list_entry, const char *name);
OVERRIDE const char *udev_list_entry_get_name(struct udev_list_entry *list_entry);
OVERRIDE const char *udev_list_entry_get_value(struct udev_list_entry *list_entry);

/*
 * udev_device
 *
 * access to sysfs/kernel devices
 */
struct udev_device;
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

/*
 * udev_enumerate
 *
 * search sysfs for specific devices and provide a sorted list
 */
struct udev_enumerate;
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

#endif

