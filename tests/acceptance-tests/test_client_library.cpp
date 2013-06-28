/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Guest <thomas.guest@canonical.com>
 */

#include "mir_test_framework/display_server_test_fixture.h"

#include "mir_toolkit/mir_client_library.h"
#include "src/client/client_buffer.h"

#include "mir/frontend/communicator.h"

#include "mir_protobuf.pb.h"

#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <cstring>

namespace mf = mir::frontend;
namespace mc = mir::compositor;
namespace mcl = mir::client;
namespace mtf = mir_test_framework;

namespace
{
    char const* const mir_test_socket = mtf::test_socket_file().c_str();
}

namespace mir
{
namespace
{
struct ClientConfigCommon : TestingClientConfiguration
{
    ClientConfigCommon()
        : connection(0)
        , surface(0),
        buffers(0)
    {
    }

    static void connection_callback(MirConnection * connection, void * context)
    {
        ClientConfigCommon * config = reinterpret_cast<ClientConfigCommon *>(context);
        config->connection = connection;
    }

    static void create_surface_callback(MirSurface * surface, void * context)
    {
        ClientConfigCommon * config = reinterpret_cast<ClientConfigCommon *>(context);
        config->surface_created(surface);
    }

    static void next_buffer_callback(MirSurface * surface, void * context)
    {
        ClientConfigCommon * config = reinterpret_cast<ClientConfigCommon *>(context);
        config->next_buffer(surface);
    }

    static void release_surface_callback(MirSurface * surface, void * context)
    {
        ClientConfigCommon * config = reinterpret_cast<ClientConfigCommon *>(context);
        config->surface_released(surface);
    }

    virtual void connected(MirConnection * new_connection)
    {
        connection = new_connection;
    }

    virtual void surface_created(MirSurface * new_surface)
    {
        surface = new_surface;
    }

    virtual void next_buffer(MirSurface*)
    {
        ++buffers;
    }

    virtual void surface_released(MirSurface * /*released_surface*/)
    {
        surface = NULL;
    }

    MirConnection* connection;
    MirSurface* surface;
    int buffers;
};
}

TEST_F(DefaultDisplayServerTestFixture, client_library_connects_and_disconnects)
{
    struct ClientConfig : ClientConfigCommon
    {
        void exec()
        {
            mir_wait_for(mir_connect(mir_test_socket, __PRETTY_FUNCTION__, connection_callback, this));

            ASSERT_TRUE(connection != NULL);
            EXPECT_TRUE(mir_connection_is_valid(connection));
            EXPECT_STREQ(mir_connection_get_error_message(connection), "");

            mir_connection_release(connection);
        }
    } client_config;

    launch_client_process(client_config);
}

TEST_F(DefaultDisplayServerTestFixture, synchronous_connection)
{
    struct ClientConfig : ClientConfigCommon
    {
        void exec()
        {
            connection = NULL;
            connection = mir_connect_sync(mir_test_socket,
                                          __PRETTY_FUNCTION__);

            ASSERT_TRUE(connection);
            EXPECT_TRUE(mir_connection_is_valid(connection));
            EXPECT_STREQ(mir_connection_get_error_message(connection), "");

            mir_connection_release(connection);
        }
    } client_config;

    launch_client_process(client_config);
}

TEST_F(DefaultDisplayServerTestFixture, client_library_creates_surface)
{
    struct ClientConfig : ClientConfigCommon
    {
        void exec()
        {

            mir_wait_for(mir_connect(mir_test_socket, __PRETTY_FUNCTION__, connection_callback, this));

            ASSERT_TRUE(connection != NULL);
            EXPECT_TRUE(mir_connection_is_valid(connection));
            EXPECT_STREQ(mir_connection_get_error_message(connection), "");

            MirSurfaceParameters const request_params =
            {
                __PRETTY_FUNCTION__,
                640, 480,
                mir_pixel_format_abgr_8888,
                mir_buffer_usage_hardware
            };

            mir_wait_for(mir_connection_create_surface(connection, &request_params, create_surface_callback, this));

            ASSERT_TRUE(surface != NULL);
            EXPECT_TRUE(mir_surface_is_valid(surface));
            EXPECT_STREQ(mir_surface_get_error_message(surface), "");

            MirSurfaceParameters response_params;
            mir_surface_get_parameters(surface, &response_params);
            EXPECT_EQ(request_params.width, response_params.width);
            EXPECT_EQ(request_params.height, response_params.height);
            EXPECT_EQ(request_params.pixel_format, response_params.pixel_format);
            EXPECT_EQ(request_params.buffer_usage, response_params.buffer_usage);


            mir_wait_for(mir_surface_release( surface, release_surface_callback, this));

            ASSERT_TRUE(surface == NULL);

            surface = mir_connection_create_surface_sync(connection, &request_params);

            ASSERT_TRUE(surface != NULL);
            EXPECT_TRUE(mir_surface_is_valid(surface));
            EXPECT_STREQ(mir_surface_get_error_message(surface), "");

            mir_surface_get_parameters(surface, &response_params);
            EXPECT_EQ(request_params.width, response_params.width);
            EXPECT_EQ(request_params.height, response_params.height);
            EXPECT_EQ(request_params.pixel_format,
                      response_params.pixel_format);
            EXPECT_EQ(request_params.buffer_usage,
                      response_params.buffer_usage);

            mir_surface_release_sync(surface);

            mir_connection_release(connection);
        }
    } client_config;

    launch_client_process(client_config);
}

TEST_F(DefaultDisplayServerTestFixture, surface_types)
{
    struct ClientConfig : ClientConfigCommon
    {
        void exec()
        {

            mir_wait_for(mir_connect(mir_test_socket, __PRETTY_FUNCTION__, connection_callback, this));

            ASSERT_TRUE(connection != NULL);
            EXPECT_TRUE(mir_connection_is_valid(connection));
            EXPECT_STREQ(mir_connection_get_error_message(connection), "");

            MirSurfaceParameters const request_params =
            {
                __PRETTY_FUNCTION__,
                640, 480,
                mir_pixel_format_abgr_8888,
                mir_buffer_usage_hardware
            };

            mir_wait_for(mir_connection_create_surface(connection, &request_params, create_surface_callback, this));

            ASSERT_TRUE(surface != NULL);
            EXPECT_TRUE(mir_surface_is_valid(surface));
            EXPECT_STREQ(mir_surface_get_error_message(surface), "");

            EXPECT_EQ(mir_surface_type_normal,
                      mir_surface_get_type(surface));

            mir_wait_for(mir_surface_set_type(surface,
                                              mir_surface_type_freestyle));
            EXPECT_EQ(mir_surface_type_freestyle,
                      mir_surface_get_type(surface));

            mir_wait_for(mir_surface_set_type(surface,
                                            static_cast<MirSurfaceType>(999)));
            EXPECT_EQ(mir_surface_type_freestyle,
                      mir_surface_get_type(surface));

            mir_wait_for(mir_surface_set_type(surface,
                                              mir_surface_type_dialog));
            EXPECT_EQ(mir_surface_type_dialog,
                      mir_surface_get_type(surface));

            mir_wait_for(mir_surface_set_type(surface,
                                            static_cast<MirSurfaceType>(888)));
            EXPECT_EQ(mir_surface_type_dialog,
                      mir_surface_get_type(surface));

            // Stress-test synchronization logic with some flooding
            for (int i = 0; i < 100; i++)
            {
                mir_surface_set_type(surface, mir_surface_type_normal);
                mir_surface_set_type(surface, mir_surface_type_utility);
                mir_surface_set_type(surface, mir_surface_type_dialog);
                mir_surface_set_type(surface, mir_surface_type_overlay);
                mir_surface_set_type(surface, mir_surface_type_freestyle);
                mir_wait_for(mir_surface_set_type(surface,
                                                  mir_surface_type_popover));
                ASSERT_EQ(mir_surface_type_popover,
                          mir_surface_get_type(surface));
            }

            mir_wait_for(mir_surface_release(surface, release_surface_callback,
                                             this));

            mir_connection_release(connection);
        }
    } client_config;

    launch_client_process(client_config);
}

TEST_F(DefaultDisplayServerTestFixture, client_can_set_surface_state)
{
    struct ClientConfig : ClientConfigCommon
    {
        void exec()
        {
            connection = mir_connect_sync(mir_test_socket, __PRETTY_FUNCTION__);
            ASSERT_TRUE(connection != NULL);
            EXPECT_TRUE(mir_connection_is_valid(connection));
            EXPECT_STREQ(mir_connection_get_error_message(connection), "");

            MirSurfaceParameters const request_params =
            {
                __PRETTY_FUNCTION__,
                640, 480,
                mir_pixel_format_abgr_8888,
                mir_buffer_usage_hardware
            };

            surface = mir_connection_create_surface_sync(connection,
                                                         &request_params);
            ASSERT_TRUE(surface != NULL);
            EXPECT_TRUE(mir_surface_is_valid(surface));
            EXPECT_STREQ(mir_surface_get_error_message(surface), "");

            EXPECT_EQ(mir_surface_state_restored,
                      mir_surface_get_state(surface));

            mir_wait_for(mir_surface_set_state(surface,
                                               mir_surface_state_fullscreen));
            EXPECT_EQ(mir_surface_state_fullscreen,
                      mir_surface_get_state(surface));

            mir_wait_for(mir_surface_set_state(surface,
                                           static_cast<MirSurfaceState>(999)));
            EXPECT_EQ(mir_surface_state_fullscreen,
                      mir_surface_get_state(surface));

            mir_wait_for(mir_surface_set_state(surface,
                                               mir_surface_state_minimized));
            EXPECT_EQ(mir_surface_state_minimized,
                      mir_surface_get_state(surface));

            mir_wait_for(mir_surface_set_state(surface,
                                           static_cast<MirSurfaceState>(888)));
            EXPECT_EQ(mir_surface_state_minimized,
                      mir_surface_get_state(surface));

            // Stress-test synchronization logic with some flooding
            for (int i = 0; i < 100; i++)
            {
                mir_surface_set_state(surface, mir_surface_state_maximized);
                mir_surface_set_state(surface, mir_surface_state_restored);
                mir_wait_for(mir_surface_set_state(surface,
                                                mir_surface_state_fullscreen));
                ASSERT_EQ(mir_surface_state_fullscreen,
                          mir_surface_get_state(surface));
            }

            mir_surface_release_sync(surface);
            mir_connection_release(connection);
        }
    } client_config;

    launch_client_process(client_config);
}

TEST_F(DefaultDisplayServerTestFixture, client_receives_surface_state_events)
{
    struct ClientConfig : ClientConfigCommon
    {
        static void event_callback(MirSurface* surface, MirEvent const* event,
                                   void* ctx)
        {
            ClientConfig* self = static_cast<ClientConfig*>(ctx);
            self->last_event = *event;
            self->last_event_surface = surface;
        }

        void exec()
        {
            connection = mir_connect_sync(mir_test_socket, __PRETTY_FUNCTION__);
            ASSERT_TRUE(connection != NULL);
            ASSERT_TRUE(mir_connection_is_valid(connection));

            MirSurfaceParameters const request_params =
            {
                __PRETTY_FUNCTION__,
                640, 480,
                mir_pixel_format_abgr_8888,
                mir_buffer_usage_hardware
            };

            memset(&last_event, 0, sizeof last_event);
            last_event_surface = nullptr;

            MirEventDelegate delegate{&event_callback, this};
            MirSurface* other_surface =
                mir_connection_create_surface_sync(connection, &request_params);
            ASSERT_TRUE(other_surface != NULL);
            ASSERT_TRUE(mir_surface_is_valid(other_surface));
            mir_surface_set_event_handler(other_surface, nullptr);

            surface =
                mir_connection_create_surface_sync(connection, &request_params);
            ASSERT_TRUE(surface != NULL);
            ASSERT_TRUE(mir_surface_is_valid(surface));

            mir_surface_set_event_handler(surface, &delegate);

            int surface_id = mir_surface_get_id(surface);

            mir_wait_for(mir_surface_set_state(surface,
                                               mir_surface_state_fullscreen));
            mir_wait_for(mir_surface_set_state(other_surface,
                                               mir_surface_state_minimized));
            EXPECT_EQ(surface, last_event_surface);
            EXPECT_EQ(mir_event_type_surface, last_event.type);
            EXPECT_EQ(surface_id, last_event.surface.id);
            EXPECT_EQ(mir_surface_attrib_state, last_event.surface.attrib);
            EXPECT_EQ(mir_surface_state_fullscreen, last_event.surface.value);

            mir_wait_for(mir_surface_set_state(surface,
                                           static_cast<MirSurfaceState>(999)));
            EXPECT_EQ(surface, last_event_surface);
            EXPECT_EQ(mir_event_type_surface, last_event.type);
            EXPECT_EQ(surface_id, last_event.surface.id);
            EXPECT_EQ(mir_surface_attrib_state, last_event.surface.attrib);
            EXPECT_EQ(mir_surface_state_fullscreen, last_event.surface.value);

            memset(&last_event, 0, sizeof last_event);
            last_event_surface = nullptr;

            mir_wait_for(mir_surface_set_state(surface,
                                               mir_surface_state_minimized));
            EXPECT_EQ(surface, last_event_surface);
            EXPECT_EQ(mir_event_type_surface, last_event.type);
            EXPECT_EQ(surface_id, last_event.surface.id);
            EXPECT_EQ(mir_surface_attrib_state, last_event.surface.attrib);
            EXPECT_EQ(mir_surface_state_minimized, last_event.surface.value);

            memset(&last_event, 0, sizeof last_event);
            last_event_surface = nullptr;

            mir_wait_for(mir_surface_set_state(surface,
                                           static_cast<MirSurfaceState>(777)));
            mir_wait_for(mir_surface_set_state(other_surface,
                                               mir_surface_state_maximized));
            EXPECT_EQ(0, last_event_surface);
            EXPECT_EQ(0, last_event.type);
            EXPECT_EQ(0, last_event.surface.id);
            EXPECT_EQ(0, last_event.surface.attrib);
            EXPECT_EQ(0, last_event.surface.value);

            mir_surface_release_sync(surface);
            mir_surface_release_sync(other_surface);
            mir_connection_release(connection);
        }

        MirEvent last_event;
        MirSurface* last_event_surface;
    } client_config;

    launch_client_process(client_config);
}

TEST_F(DefaultDisplayServerTestFixture, client_library_creates_multiple_surfaces)
{
    int const n_surfaces = 13;

    struct ClientConfig : ClientConfigCommon
    {
        ClientConfig(int n_surfaces) : n_surfaces(n_surfaces)
        {
        }

        void surface_created(MirSurface * new_surface)
        {
            surfaces.insert(new_surface);
        }

        void surface_released(MirSurface * surface)
        {
            surfaces.erase(surface);
        }

        MirSurface * any_surface()
        {
            return *surfaces.begin();
        }

        size_t current_surface_count()
        {
            return surfaces.size();
        }

        void exec()
        {
            mir_wait_for(mir_connect(mir_test_socket, __PRETTY_FUNCTION__, connection_callback, this));

            ASSERT_TRUE(connection != NULL);
            EXPECT_TRUE(mir_connection_is_valid(connection));
            EXPECT_STREQ(mir_connection_get_error_message(connection), "");

            for (int i = 0; i != n_surfaces; ++i)
            {
                old_surface_count = current_surface_count();

                MirSurfaceParameters const request_params =
                {
                    __PRETTY_FUNCTION__,
                    640, 480,
                    mir_pixel_format_abgr_8888,
                    mir_buffer_usage_hardware
                };

                mir_wait_for(mir_connection_create_surface(connection, &request_params, create_surface_callback, this));

                ASSERT_EQ(old_surface_count + 1, current_surface_count());
            }
            for (int i = 0; i != n_surfaces; ++i)
            {
                old_surface_count = current_surface_count();

                ASSERT_NE(old_surface_count, 0u);

                MirSurface * surface = any_surface();
                mir_wait_for(mir_surface_release( surface, release_surface_callback, this));

                ASSERT_EQ(old_surface_count - 1, current_surface_count());
            }

            mir_connection_release(connection);
        }

        int n_surfaces;
        std::set<MirSurface *> surfaces;
        size_t old_surface_count;
    } client_config(n_surfaces);

    launch_client_process(client_config);
}

TEST_F(DefaultDisplayServerTestFixture, client_library_accesses_and_advances_buffers)
{
    struct ClientConfig : ClientConfigCommon
    {
        void exec()
        {

            mir_wait_for(mir_connect(mir_test_socket, __PRETTY_FUNCTION__, connection_callback, this));

            ASSERT_TRUE(connection != NULL);
            EXPECT_TRUE(mir_connection_is_valid(connection));
            EXPECT_STREQ("", mir_connection_get_error_message(connection));

            MirSurfaceParameters const request_params =
            {
                __PRETTY_FUNCTION__,
                640, 480,
                mir_pixel_format_abgr_8888,
                mir_buffer_usage_hardware
            };

            mir_wait_for(mir_connection_create_surface(connection, &request_params, create_surface_callback, this));
            ASSERT_TRUE(surface != NULL);

            buffers = 0;
            mir_wait_for(mir_surface_swap_buffers(surface, next_buffer_callback, this));
            EXPECT_EQ(buffers, 1);

            mir_wait_for(mir_surface_release( surface, release_surface_callback, this));

            ASSERT_TRUE(surface == NULL);

            mir_connection_release(connection);
        }
    } client_config;

    launch_client_process(client_config);
}

TEST_F(DefaultDisplayServerTestFixture, fully_synchronous_client)
{
    struct ClientConfig : ClientConfigCommon
    {
        void exec()
        {
            connection = mir_connect_sync(mir_test_socket,
                                          __PRETTY_FUNCTION__);

            ASSERT_TRUE(connection != NULL);
            EXPECT_TRUE(mir_connection_is_valid(connection));
            EXPECT_STREQ("", mir_connection_get_error_message(connection));

            MirSurfaceParameters const request_params =
            {
                __PRETTY_FUNCTION__,
                640, 480,
                mir_pixel_format_abgr_8888,
                mir_buffer_usage_software
            };

            surface = mir_connection_create_surface_sync(connection, &request_params);
            ASSERT_TRUE(surface != NULL);
            EXPECT_TRUE(mir_surface_is_valid(surface));
            EXPECT_STREQ(mir_surface_get_error_message(surface), "");

            mir_surface_swap_buffers_sync(surface);
            EXPECT_TRUE(mir_surface_is_valid(surface));
            EXPECT_STREQ(mir_surface_get_error_message(surface), "");

            mir_surface_release_sync(surface);

            EXPECT_TRUE(mir_connection_is_valid(connection));
            EXPECT_STREQ("", mir_connection_get_error_message(connection));
            mir_connection_release(connection);
        }
    } client_config;

    launch_client_process(client_config);
}

TEST_F(DefaultDisplayServerTestFixture, highly_threaded_client)
{
    struct ClientConfig : ClientConfigCommon
    {
        static void nosey_thread(MirSurface *surf)
        {
            for (int i = 0; i < 10; i++)
            {
                mir_wait_for_one(mir_surface_set_state(surf,
                                                mir_surface_state_maximized));
                mir_wait_for_one(mir_surface_set_type(surf,
                                                mir_surface_type_normal));
                mir_wait_for_one(mir_surface_set_state(surf,
                                                mir_surface_state_restored));
                mir_wait_for_one(mir_surface_set_type(surf,
                                                mir_surface_type_utility));
                mir_wait_for_one(mir_surface_set_state(surf,
                                                mir_surface_state_fullscreen));
                mir_wait_for_one(mir_surface_set_type(surf,
                                                mir_surface_type_dialog));
                mir_wait_for_one(mir_surface_set_state(surf,
                                                mir_surface_state_minimized));
            }
        }

        void exec()
        {
            connection = mir_connect_sync(mir_test_socket,
                                          __PRETTY_FUNCTION__);

            ASSERT_TRUE(connection != NULL);
            EXPECT_TRUE(mir_connection_is_valid(connection));
            EXPECT_STREQ("", mir_connection_get_error_message(connection));

            MirSurfaceParameters const request_params =
            {
                __PRETTY_FUNCTION__,
                640, 480,
                mir_pixel_format_abgr_8888,
                mir_buffer_usage_software
            };

            surface = mir_connection_create_surface_sync(connection,
                                                         &request_params);
            ASSERT_TRUE(surface != NULL);
            EXPECT_TRUE(mir_surface_is_valid(surface));
            EXPECT_STREQ(mir_surface_get_error_message(surface), "");

            std::thread a(nosey_thread, surface);
            std::thread b(nosey_thread, surface);
            std::thread c(nosey_thread, surface);

            a.join();
            b.join();
            c.join();

            EXPECT_EQ(mir_surface_type_dialog,
                      mir_surface_get_type(surface));
            EXPECT_EQ(mir_surface_state_minimized,
                      mir_surface_get_state(surface));

            mir_surface_release_sync(surface);

            EXPECT_TRUE(mir_connection_is_valid(connection));
            EXPECT_STREQ("", mir_connection_get_error_message(connection));
            mir_connection_release(connection);
        }
    } client_config;

    launch_client_process(client_config);
}

TEST_F(DefaultDisplayServerTestFixture, client_library_accesses_platform_package)
{
    struct ClientConfig : ClientConfigCommon
    {
        void exec()
        {
            mir_wait_for(mir_connect(mir_test_socket, __PRETTY_FUNCTION__, connection_callback, this));
            ASSERT_TRUE(connection != NULL);

            MirPlatformPackage platform_package;
            platform_package.data_items = -1;
            platform_package.fd_items = -1;

            mir_connection_get_platform(connection, &platform_package);
            EXPECT_GE(0, platform_package.data_items);
            EXPECT_GE(0, platform_package.fd_items);

            mir_connection_release(connection);
        }
    } client_config;

    launch_client_process(client_config);
}

TEST_F(DefaultDisplayServerTestFixture, client_library_accesses_display_info)
{
    static const unsigned int default_display_width = 1600, default_display_height = 1600;

    struct ClientConfig : ClientConfigCommon
    {
        void exec()
        {
            mir_wait_for(mir_connect(mir_test_socket, __PRETTY_FUNCTION__, connection_callback, this));
            ASSERT_TRUE(connection != NULL);

            MirDisplayInfo display_info;
            display_info.width = -1;
            display_info.height = -1;

            mir_connection_get_display_info(connection, &display_info);
            EXPECT_GE(default_display_width, display_info.width);
            EXPECT_GE(default_display_height, display_info.height);

            mir_connection_release(connection);
        }
    } client_config;

    launch_client_process(client_config);
}

TEST_F(DefaultDisplayServerTestFixture, connect_errors_handled)
{
    struct ClientConfig : ClientConfigCommon
    {
        void exec()
        {
            mir_wait_for(mir_connect("garbage", __PRETTY_FUNCTION__, connection_callback, this));
            ASSERT_TRUE(connection != NULL);

            char const* error = mir_connection_get_error_message(connection);

            if (std::strcmp("connect: No such file or directory", error) &&
                std::strcmp("Can't find MIR server", error))
            {
                FAIL() << error;
            }
        }
    } client_config;

    launch_client_process(client_config);
}

TEST_F(DefaultDisplayServerTestFixture, connect_errors_dont_blow_up)
{
    struct ClientConfig : ClientConfigCommon
    {
        void exec()
        {
            mir_wait_for(mir_connect("garbage", __PRETTY_FUNCTION__, connection_callback, this));

            MirSurfaceParameters const request_params =
            {
                __PRETTY_FUNCTION__,
                640, 480,
                mir_pixel_format_abgr_8888,
                mir_buffer_usage_hardware
            };

            mir_wait_for(mir_connection_create_surface(connection, &request_params, create_surface_callback, this));
// TODO surface_create needs to fail safe too. After that is done we should add the following:
// TODO    mir_wait_for(mir_surface_swap_buffers(surface, next_buffer_callback, this));
// TODO    mir_wait_for(mir_surface_release( surface, release_surface_callback, this));

            mir_connection_release(connection);
        }
    } client_config;

    launch_client_process(client_config);
}
}
