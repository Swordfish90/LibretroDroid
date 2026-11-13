/*
 *     Copyright (C) 2026  Filippo Scognamiglio
 *
 *     This program is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 */

#ifndef LIBRETRODROID_VFS_INTERFACE_NEGOTIATION_H
#define LIBRETRODROID_VFS_INTERFACE_NEGOTIATION_H

#include "libretro.h"

namespace libretrodroid {

inline bool negotiateVFSInterface(
    retro_vfs_interface_info *info,
    bool enabled,
    uint32_t supportedVersion,
    retro_vfs_interface *interface
) {
    if (!enabled || info == nullptr || info->required_interface_version > supportedVersion) {
        return false;
    }

    info->required_interface_version = supportedVersion;
    info->iface = interface;
    return true;
}

} // namespace libretrodroid

#endif // LIBRETRODROID_VFS_INTERFACE_NEGOTIATION_H
