#include <gtk/gtk.h>
#include <mongoc.h>
#include <stdio.h>
#include <stdlib.h>

int testConnexion()
{
    const char *url = g_getenv("MONGO_URL");
    mongoc_uri_t *uri;
    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *collection;
    bson_t *command, reply, *insert;
    bson_error_t error;
    char *str;
    bool retval;

    /*
     * Required to initialize libmongoc's internals
     */
    mongoc_init();
    /*
     * Safely create a MongoDB URI object from the given string
     */
    uri = mongoc_uri_new_with_error(url, &error);
    if (!uri)
    {
        fprintf(stderr,
                "failed to parse URI: %s\n"
                "error message:       %s\n",
                url,
                error.message);
        return EXIT_FAILURE;
    }
    /*
     * Create a new client instance
     */
    client = mongoc_client_new_from_uri(uri);
    if (!client)
    {
        return EXIT_FAILURE;
    }
    /*
     * Register the application name so we can track it in the profile logs
     * on the server. This can also be done from the URI (see other examples).
     */
    mongoc_client_set_appname(client, "connect-example");
    /*
     * Get a handle on the database "db_name" and collection "coll_name"
     */
    database = mongoc_client_get_database(client, "Cluster0");
    collection = mongoc_client_get_collection(client, "Cluster0", "sample_mflix");
    /*
     * Do work. This example pings the database, prints the result as JSON and
     * performs an insert
     */
    command = BCON_NEW("ping", BCON_INT32(1));
    retval = mongoc_client_command_simple(client, "admin", command, NULL, &reply, &error);
    if (!retval)
    {
        fprintf(stderr, "Error while trying to connect to the server: %s\n", error.message);
        return EXIT_FAILURE;
    }
    str = bson_as_json(&reply, NULL);
    printf("%s\n", str);
    insert = BCON_NEW("hello", BCON_UTF8("world"));
    if (!mongoc_collection_insert_one(collection, insert, NULL, NULL, &error))
    {
        fprintf(stderr, "%s\n", error.message);
    }
    bson_destroy(insert);
    bson_destroy(&reply);
    bson_destroy(command);
    bson_free(str);
    /*
     * Release our handles and clean up libmongoc
     */
    mongoc_collection_destroy(collection);
    mongoc_database_destroy(database);
    mongoc_uri_destroy(uri);
    mongoc_client_destroy(client);
    mongoc_cleanup();
    return EXIT_SUCCESS;
}

static void
activate(GtkApplication *app,
         gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *box;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Window");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    gtk_window_set_child(GTK_WINDOW(window), box);

    button = gtk_button_new_with_label("Hello World");

    g_signal_connect_swapped(button, "clicked", G_CALLBACK(testConnexion), NULL);

    gtk_box_append(GTK_BOX(box), button);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[])
{
    // Create a new application
    GtkApplication *app = gtk_application_new("com.the-book-db-company.the-book-db", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    return g_application_run(G_APPLICATION(app), argc, argv);
}
