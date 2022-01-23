#include "gowhatsapp.h"
#include "constants.h"

static void
null_cb(PurpleAccount *account, PurpleRequestFields *fields) {
}

static void
dismiss_cb(PurpleAccount *account, PurpleRequestFields *fields) {
    PurpleConnection *pc = purple_account_get_connection(account);
    purple_connection_error(pc, PURPLE_CONNECTION_ERROR_OTHER_ERROR, "QR code was dismissed.");
}

void
gowhatsapp_close_qrcode(PurpleAccount *account)
{
    purple_request_close_with_handle(account); // close all currently open request fields, if any
}

static void
gowhatsapp_display_qrcode(PurpleAccount *account, const char * challenge, void * image_data, size_t image_data_len)
{
    g_return_if_fail(account != NULL);

    PurpleRequestFields *fields = purple_request_fields_new();
    PurpleRequestFieldGroup *group = purple_request_field_group_new(NULL);
    purple_request_fields_add_group(fields, group);

    PurpleRequestField *string_field = purple_request_field_string_new("qr_string", "QR Code Data", g_strdup(challenge), FALSE);
    purple_request_field_group_add_field(group, string_field);
    PurpleRequestField *image_field = purple_request_field_image_new("qr_image", "QR Code Image", image_data, image_data_len);
    purple_request_field_group_add_field(group, image_field);
    
    g_free(image_data); // purple_request_field_image_new maintains an internal copy

    const char *username = g_strdup(purple_account_get_username(account));
    const char *secondary = g_strdup_printf("WhatsApp account %s", username);

    gowhatsapp_close_qrcode(account);
    purple_request_fields(
        account, /*handle*/
        "Logon QR Code", /*title*/
        "Please scan this QR code with your phone", /*primary*/
        secondary, /*secondary*/
        fields, /*fields*/
        "OK", G_CALLBACK(null_cb), /*OK*/
        "Dismiss", G_CALLBACK(dismiss_cb), /*Cancel*/
        NULL, /*account*/
        username, /*username*/
        NULL, /*conversation*/
        account /*data*/
    );
}

void
gowhatsapp_handle_qrcode(PurpleAccount *account, const char *challenge, const char *terminal, void *image_data, size_t image_data_len)
{
    if (image_data_len > 0) {
        gowhatsapp_display_qrcode(account, challenge, image_data, image_data_len);
    }
    if (purple_account_get_bool(account, GOWHATSAPP_PLAIN_TEXT_LOGIN_OPTION, FALSE)) {
        PurpleConnection *pc = purple_account_get_connection(account);
        purple_serv_got_im(pc, "login@whatsmeow", terminal, PURPLE_MESSAGE_RECV, time(NULL));
    }
}
