/*
 * AUTOGENERATED - DO NOT EDIT
 *
 * This file is generated from wlr-foreign-toplevel-management-unstable-v1.xml
 * To regenerate, run the “refresh-wayland-wrapper” target.
 */

#ifndef MIR_FRONTEND_WAYLAND_WLR_FOREIGN_TOPLEVEL_MANAGEMENT_UNSTABLE_V1_XML_WRAPPER
#define MIR_FRONTEND_WAYLAND_WLR_FOREIGN_TOPLEVEL_MANAGEMENT_UNSTABLE_V1_XML_WRAPPER

#include <experimental/optional>

#include "mir/fd.h"
#include <wayland-server-core.h>

#include "mir/wayland/wayland_base.h"

namespace mir
{
namespace wayland
{

class ForeignToplevelManagerV1;
class ForeignToplevelHandleV1;

class ForeignToplevelManagerV1 : public Resource
{
public:
    static char const constexpr* interface_name = "zwlr_foreign_toplevel_manager_v1";

    static ForeignToplevelManagerV1* from(struct wl_resource*);

    ForeignToplevelManagerV1(struct wl_resource* resource, Version<2>);
    virtual ~ForeignToplevelManagerV1() = default;

    void send_toplevel_event(struct wl_resource* toplevel) const;
    void send_finished_event() const;

    void destroy_wayland_object() const;

    struct wl_client* const client;
    struct wl_resource* const resource;

    struct Opcode
    {
        static uint32_t const toplevel = 0;
        static uint32_t const finished = 1;
    };

    struct Thunks;

    static bool is_instance(wl_resource* resource);

    class Global : public wayland::Global
    {
    public:
        Global(wl_display* display, Version<2>);

        auto interface_name() const -> char const* override;

    private:
        virtual void bind(wl_resource* new_zwlr_foreign_toplevel_manager_v1) = 0;
        friend ForeignToplevelManagerV1::Thunks;
    };

private:
    virtual void stop() = 0;
};

class ForeignToplevelHandleV1 : public Resource
{
public:
    static char const constexpr* interface_name = "zwlr_foreign_toplevel_handle_v1";

    static ForeignToplevelHandleV1* from(struct wl_resource*);

    ForeignToplevelHandleV1(ForeignToplevelManagerV1 const& parent);
    virtual ~ForeignToplevelHandleV1() = default;

    void send_title_event(std::string const& title) const;
    void send_app_id_event(std::string const& app_id) const;
    void send_output_enter_event(struct wl_resource* output) const;
    void send_output_leave_event(struct wl_resource* output) const;
    void send_state_event(struct wl_array* state) const;
    void send_done_event() const;
    void send_closed_event() const;

    void destroy_wayland_object() const;

    struct wl_client* const client;
    struct wl_resource* const resource;

    struct State
    {
        static uint32_t const maximized = 0;
        static uint32_t const minimized = 1;
        static uint32_t const activated = 2;
        static uint32_t const fullscreen = 3;
    };

    struct Error
    {
        static uint32_t const invalid_rectangle = 0;
    };

    struct Opcode
    {
        static uint32_t const title = 0;
        static uint32_t const app_id = 1;
        static uint32_t const output_enter = 2;
        static uint32_t const output_leave = 3;
        static uint32_t const state = 4;
        static uint32_t const done = 5;
        static uint32_t const closed = 6;
    };

    struct Thunks;

    static bool is_instance(wl_resource* resource);

private:
    virtual void set_maximized() = 0;
    virtual void unset_maximized() = 0;
    virtual void set_minimized() = 0;
    virtual void unset_minimized() = 0;
    virtual void activate(struct wl_resource* seat) = 0;
    virtual void close() = 0;
    virtual void set_rectangle(struct wl_resource* surface, int32_t x, int32_t y, int32_t width, int32_t height) = 0;
    virtual void destroy() = 0;
    virtual void set_fullscreen(std::experimental::optional<struct wl_resource*> const& output) = 0;
    virtual void unset_fullscreen() = 0;
};

}
}

#endif // MIR_FRONTEND_WAYLAND_WLR_FOREIGN_TOPLEVEL_MANAGEMENT_UNSTABLE_V1_XML_WRAPPER
