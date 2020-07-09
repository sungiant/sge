// SGE-LINUX
// Reference SGE host implementation.
// ---------------------------------- //
// todo: hook up linux properly. right now this stub is only used by CI.

#if TARGET_LINUX

#include "sge.hh"
#include "sge_core.hh"

#include <unistd.h>      /* pause() */
#include <stdlib.h>


auto g_sge = std::make_unique<sge::core::engine>();


int main ()
{
    /* Open the connection to the X server */
    xcb_connection_t *connection = xcb_connect (NULL, NULL);


    /* Get the first screen */
    const xcb_setup_t      *setup  = xcb_get_setup (connection);
    xcb_screen_iterator_t   iter   = xcb_setup_roots_iterator (setup);
    xcb_screen_t           *screen = iter.data;


    /* Create the window */
    xcb_window_t window = xcb_generate_id (connection);
    xcb_create_window (connection,                    /* Connection          */
                       XCB_COPY_FROM_PARENT,          /* depth (same as root)*/
                       window,                        /* window Id           */
                       screen->root,                  /* parent window       */
                       0, 0,                          /* x, y                */
                       150, 150,                      /* width, height       */
                       10,                            /* border_width        */
                       XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
                       screen->root_visual,           /* visual              */
                       0, NULL );                     /* masks, not used yet */


    /* Map the window on the screen */
    xcb_map_window (connection, window);


    /* Make sure commands are sent before we pause so that the window gets shown */
    xcb_flush (connection);

    g_sge->setup (connection, window);

    g_sge->register_set_window_title_callback ([](const char* s) {});
    g_sge->register_set_window_fullscreen_callback ([](bool v) {});
    g_sge->register_set_window_size_callback ([](int w, int h) {});
    g_sge->register_shutdown_request_callback ([]() {});

    bool running = true;
    xcb_generic_event_t *event;
    xcb_client_message_event_t *cm;
    xcb_atom_t wmDeleteWin;


    g_sge->start ();

    while (running) {
        event = xcb_wait_for_event(connection);

        switch (event->response_type & ~0x80) {
            case XCB_CLIENT_MESSAGE: {
                cm = (xcb_client_message_event_t *)event;

                if (cm->data.data32[0] == wmDeleteWin)
                    running = false;

                break;
            }
        }

        free(event);

        sge::core::client_state client_state;
        sge::core::input_state input_state;
        g_sge->update (client_state, input_state);

    }

    g_sge->stop ();

    g_sge->shutdown();

    xcb_disconnect (connection);

    xcb_destroy_window(connection, window);

    return 0;
}

#endif