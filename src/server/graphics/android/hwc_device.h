/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#ifndef MIR_GRAPHICS_ANDROID_HWC_DEVICE_H_
#define MIR_GRAPHICS_ANDROID_HWC_DEVICE_H_

#include "mir/geometry/size.h"

namespace mir
{
namespace graphics
{
namespace android
{

class HWCDevice
{
public:
    HWCDevice() = default;
    virtual ~HWCDevice() {}
    virtual geometry::Size display_size() = 0; 
    virtual void wait_for_vsync() = 0;
    virtual void commit_frame() = 0;
private:
    HWCDevice(HWCDevice const&) = delete;
    HWCDevice& operator=(HWCDevice const&) = delete;
};

}
}
}
#endif /* MIR_GRAPHICS_ANDROID_HWC_DEVICE_H_ */
