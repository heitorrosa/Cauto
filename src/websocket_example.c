#include "resources/include.c"
#include <libwebsockets.h>

static int websocket_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf("WebSocket connection established\n");
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            printf("Received data: %.*s\n", (int)len, (char*)in);
            break;

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            printf("WebSocket connection error\n");
            break;

        case LWS_CALLBACK_CLOSED:
            printf("WebSocket connection closed\n");
            break;

        default:
            break;
    }
    return 0;
}

static struct lws_protocols protocols[] = {
    {
        "example-protocol",
        websocket_callback,
        0,
        1024,
    },
    { NULL, NULL, 0, 0 } /* terminator */
};

int websocket_example() {
    struct lws_context_creation_info info;
    struct lws_context *context;
    struct lws_client_connect_info ccinfo = {0};

    memset(&info, 0, sizeof info);
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    context = lws_create_context(&info);
    if (!context) {
        printf("Failed to create libwebsockets context\n");
        return -1;
    }

    ccinfo.context = context;
    ccinfo.address = "echo.websocket.org";
    ccinfo.port = 80;
    ccinfo.path = "/";
    ccinfo.host = ccinfo.address;
    ccinfo.origin = ccinfo.address;
    ccinfo.protocol = protocols[0].name;

    struct lws *wsi = lws_client_connect_via_info(&ccinfo);
    if (!wsi) {
        printf("Failed to connect to WebSocket server\n");
        lws_context_destroy(context);
        return -1;
    }

    printf("Connecting to WebSocket server...\n");

    // Service the connection for a few seconds
    int n = 0;
    while (n >= 0 && n < 100) {
        n = lws_service(context, 50);
    }

    lws_context_destroy(context);
    return 0;
}
