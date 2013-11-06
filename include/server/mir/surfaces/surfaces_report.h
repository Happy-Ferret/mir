/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */


#ifndef MIR_SURFACES_SURFACES_REPORT_H_
#define MIR_SURFACES_SURFACES_REPORT_H_

#include <memory>

namespace mir
{
namespace shell { class BasicSurface; }

namespace surfaces
{
class SurfacesReport
{
public:
    virtual void surface_created(shell::BasicSurface* const surface) = 0;
    virtual void surface_added(shell::BasicSurface* const surface) = 0;

    virtual void surface_removed(shell::BasicSurface* const surface) = 0;
    virtual void surface_deleted(shell::BasicSurface* const surface) = 0;

protected:
    SurfacesReport() = default;
    virtual ~SurfacesReport() = default;
    SurfacesReport(SurfacesReport const&) = delete;
    SurfacesReport& operator=(SurfacesReport const&) = delete;
};

class NullSurfacesReport : public SurfacesReport
{
public:
    virtual void surface_created(shell::BasicSurface* const /*surface*/) override;
    virtual void surface_added(shell::BasicSurface* const /*surface*/) override;

    virtual void surface_removed(shell::BasicSurface* const /*surface*/) override;
    virtual void surface_deleted(shell::BasicSurface* const /*surface*/) override;
};
}
}


#endif /* MIR_SURFACES_SURFACES_REPORT_H_ */
