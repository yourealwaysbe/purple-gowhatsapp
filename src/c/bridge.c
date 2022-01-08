#include "gowhatsapp.h"
#include "constants.h"

/////////////////////////////////////////////////////////////////////
//                                                                 //
//      WELCOME TO THE LAND OF ABANDONMENT OF TYPE AND SAFETY      //
//                        Wanderer, beware.                        //
//                                                                 //
/////////////////////////////////////////////////////////////////////

/*
 * Handler for a message received by go-whatsapp.
 * Called inside of the GTK eventloop.
 *
 * @return Whether to execute again. Always FALSE.
 */
static gboolean
process_message_bridge(gpointer data)
{
    gowhatsapp_message_t * gwamsg = (gowhatsapp_message_t *)data;
    if (gwamsg->msgtype == gowhatsapp_message_type_log) {
        // log messages do not need an active connection
        purple_debug(gwamsg->loglevel, GOWHATSAPP_NAME, "%s", gwamsg->text);
    } else {
        // query Pidgin for a list of all accounts. bail if no connection exists
        int account_exists = 0;
        PurpleAccount *account = NULL;
        PurpleConnection *connection = NULL;
        for (GList *iter = purple_accounts_get_all(); iter != NULL && !account_exists; iter = iter->next) {
            account = (PurpleAccount *)iter->data;
            const char *protocol_id = purple_account_get_protocol_id(account);
            const char *username = purple_account_get_username(account);
            account_exists = purple_strequal(protocol_id, GOWHATSAPP_PRPL_ID) && purple_strequal(gwamsg->username, username);
            if (account_exists) {
                connection = purple_account_get_connection(account);
                purple_debug_info(GOWHATSAPP_NAME, "Account %p exists for username %s, connection is %p.\n", account, username, connection);
            }
        }
        if (connection == NULL) {
            purple_debug_info(GOWHATSAPP_NAME, "No active connection for account %s. Ignoring message.\n", gwamsg->username);
        } else {
            gowhatsapp_process_message(account, gwamsg);
        }
    }
    // always clean up data in heap
    g_free(gwamsg->id);
    g_free(gwamsg->remoteJid);
    g_free(gwamsg->senderJid);
    g_free(gwamsg->text);
    g_free(gwamsg->name);
    //g_free(gwamsg->blob); this is cleared after handling the attachment / qrcode
    g_free(gwamsg);
    return FALSE;
}

/*
 * Handler for a message received by go-whatsapp.
 * Called by go-whatsapp (outside of the GTK eventloop).
 * 
 * Yes, this is indeed neccessary – we checked.
 */
void
gowhatsapp_process_message_bridge(gowhatsapp_message_t gwamsg_go)
{
    // copying Go-managed struct into heap
    // the strings inside the struct already reside in the heap, according to https://golang.org/cmd/cgo/#hdr-C_references_to_Go
    gowhatsapp_message_t *gwamsg_heap = g_memdup(&gwamsg_go, sizeof gwamsg_go);
    purple_timeout_add(
        0, // schedule for immediate execution
        process_message_bridge, // handle message in main thread
        gwamsg_heap // data to handle in main thread
    );
}
