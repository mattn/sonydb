#ifdef GUI
#include <gtk/gtk.h>
#include <gdk/gdkkeys.h>
#include <gdk/gdkkeysyms.h>
#include "images.h"
#endif
#include <string.h>
#include <id3.h>
#include "sonydb.h"


#ifdef __GNUC__
#undef _WIN32
#endif

#ifdef GUI
enum {
COL_ICON,
COL_TEXT,
COL_SONG,
COL_MAX
};
#define EDIT_TYPE_ARTIST 1
#define EDIT_TYPE_ALBUM 2
#define EDIT_TYPE_TITLE 3
#endif

static const char* get_genre(const char* src)
{
	const char* ptr = src;
	while(*ptr && !isdigit(*ptr)) ptr++;
	switch(atol(ptr)) {
	case 0: return "Blues";
	case 1: return "Classic Rock";
	case 2: return "Country";
	case 3: return "Dance";
	case 4: return "Disco";
	case 5: return "Funk";
	case 6: return "Grunge";
	case 7: return "Hip-Hop";
	case 8: return "Jazz";
	case 9: return "Metal";
	case 10: return "New Age";
	case 11: return "Oldies";
	case 12: return "Other";
	case 13: return "Pop";
	case 14: return "R?";
	case 15: return "Rap";
	case 16: return "Reggae";
	case 17: return "Rock";
	case 18: return "Techno";
	case 19: return "Industrial";
	case 20: return "Alternative";
	case 21: return "Ska";
	case 22: return "Death Metal";
	case 23: return "Pranks";
	case 24: return "Soundtrack";
	case 25: return "Euro-Techno";
	case 26: return "Ambient";
	case 27: return "Trip-Hop";
	case 28: return "Vocal";
	case 29: return "Jazz+Funk";
	case 30: return "Fusion";
	case 31: return "Trance";
	case 32: return "Classical";
	case 33: return "Instrumental";
	case 34: return "Acid";
	case 35: return "House";
	case 36: return "Game";
	case 37: return "Sound Clip";
	case 38: return "Gospel";
	case 39: return "Noise";
	case 40: return "AlternRock";
	case 41: return "Bass";
	case 42: return "Soul";
	case 43: return "Punk";
	case 44: return "Space";
	case 45: return "Meditative";
	case 46: return "Instrumental Pop";
	case 47: return "Instrumental Rock";
	case 48: return "Ethnic";
	case 49: return "Gothic";
	case 50: return "Darkwave";
	case 51: return "Techno-Industrial";
	case 52: return "Electronic";
	case 53: return "Pop-Folk";
	case 54: return "Eurodance";
	case 55: return "Dream";
	case 56: return "Southern Rock";
	case 57: return "Comedy";
	case 58: return "Cult";
	case 59: return "Gangsta";
	case 60: return "Top 40";
	case 61: return "Christian Rap";
	case 62: return "Pop/Funk";
	case 63: return "Jungle";
	case 64: return "Native American";
	case 65: return "Cabaret";
	case 66: return "New Wave";
	case 67: return "Psychadelic";
	case 68: return "Rave";
	case 69: return "Showtunes";
	case 70: return "Trailer";
	case 71: return "Lo-Fi";
	case 72: return "Tribal";
	case 73: return "Acid Punk";
	case 74: return "Acid Jazz";
	case 75: return "Polka";
	case 76: return "Retro";
	case 77: return "Musical";
	case 78: return "Rock & Roll";
	case 79: return "Hard Rock";
	case 80: return "Folk";
	case 81: return "Folk-Rock";
	case 82: return "National Folk";
	case 83: return "Swing";
	case 84: return "Fast Fusion";
	case 85: return "Bebob";
	case 86: return "Latin";
	case 87: return "Revival";
	case 88: return "Celtic";
	case 89: return "Bluegrass";
	case 90: return "Avantgarde";
	case 91: return "Gothic Rock";
	case 92: return "Progressive Rock";
	case 93: return "Psychedelic Rock";
	case 94: return "Symphonic Rock";
	case 95: return "Slow Rock";
	case 96: return "Big Band";
	case 97: return "Chorus";
	case 98: return "Easy Listening";
	case 99: return "Acoustic";
	case 100: return "Humour";
	case 101: return "Speech";
	case 102: return "Chanson";
	case 103: return "Opera";
	case 104: return "Chamber Music";
	case 105: return "Sonata";
	case 106: return "Symphony";
	case 107: return "Booty Bass";
	case 108: return "Primus";
	case 109: return "Porn Groove";
	case 110: return "Satire";
	case 111: return "Slow Jam";
	case 112: return "Club";
	case 113: return "Tango";
	case 114: return "Samba";
	case 115: return "Folklore";
	case 116: return "Ballad";
	case 117: return "Power Ballad";
	case 118: return "Rhythmic Soul";
	case 119: return "Freestyle";
	case 120: return "Duet";
	case 121: return "Punk Rock";
	case 122: return "Drum Solo";
	case 123: return "A capella";
	case 124: return "Euro-House";
	case 125: return "Dance Hall";
	}
	return "";
}

static void print_song(Song* song)
{
	printf("songs(%04d): %s,%s,%s\n\tencoding=%s\n\tfilename=%s\n\tgenre=%s\n\tsonglen=%d\n\ttrack_nr=%d\n\tyear=%d\n",
			song->sonyDbOrder,
			song->artist,
			song->album,
			song->title,
			song->encoding,
			song->filename,
			song->genre,
			song->songlen,
			song->track_nr,
			song->year
			);
}

static void deleteSongPtr(Song *song)
{
     free(song->album);
     free(song->artist);
     free(song->title);
     free(song->genre);
     free(song->filename);
     free(song);
}

std::string wstring2string(const wchar_t* str)
{
	std::string ret;
#ifdef _WIN32
	UINT codePage = GetACP();
	size_t mbssize = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)str,-1,NULL,0,NULL,NULL);
	char* pszStr = new char[mbssize+1];
	WideCharToMultiByte(codePage, 0, (LPCWSTR)str, -1, pszStr, mbssize, NULL, NULL);
	pszStr[mbssize] = '\0';
	ret = pszStr;
	delete [] pszStr;
#else
	if (str) {
		size_t mbssize = wcstombs(0, str, 0)*2;
		char* dest = (char*)malloc(mbssize+1);
		if (dest) {
			wcstombs(dest, str, mbssize);
			ret = dest;
			free(dest);
		}
	}
#endif
	return ret;
}

wstring string2wstring(const char* str)
{
	wstring ret;
#ifdef _WIN32
	size_t in_len = strlen(str);
	UINT codePage = GetACP();
	size_t wcssize = MultiByteToWideChar(codePage, MB_PRECOMPOSED, str,in_len,  NULL, 0);
	wchar_t* pszStr = new wchar_t[wcssize + 1];
	MultiByteToWideChar(codePage, MB_PRECOMPOSED, str, in_len, pszStr, wcssize + 1);
	pszStr[wcssize] = '\0';
	ret = pszStr;
	delete [] pszStr;
#else
	if (str) {
		size_t wcssize = strlen(str)*sizeof(wchar_t);
		wchar_t* dest = (wchar_t*)malloc(wcssize+1);
		if (dest) {
			mbstowcs(dest, str, wcssize);
			ret = dest;
			free(dest);
		}
	}
#endif
	return ret;
}

char* get_tag(ID3Tag* tag, ID3_FrameID id)
{
	unicode_t wbuff[BUFSIZ] = {0};
	ID3Frame* frame = ID3Tag_FindFrameWithID(tag, id);
	ID3Field* field = ID3Frame_GetField(frame, ID3FN_TEXT);
	memset(wbuff, 0, sizeof(wbuff));
	if (ID3Field_GetUNICODE(field, wbuff, ID3Field_Size(field)/sizeof(unicode_t)) == 0) {
		char buff[BUFSIZ] = {0};
		ID3Field_GetASCII(field, buff, ID3Field_Size(field));
		return strdup(buff);
	}
	return utf16_to_ansi(wbuff, sizeof(wbuff)/sizeof(unicode_t), true);
}

#ifdef GUI
void rebuild_tree(GtkWidget* widget, SonyDb* sonydb, bool reload)
{
	GtkTreeModel* model;
	GtkTreeStore* store;
	GtkWidget* button;
	GtkTreeIter iter_artist, iter_album, iter_title;
	vector<Song*> songs;
	vector<Song*>::iterator itsongs;

	if (reload) sonydb->readAllTracks();
	songs = sonydb->getSongs();

	store = (GtkTreeStore*)gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	model = (GtkTreeModel*)gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_view_set_model(GTK_TREE_VIEW(widget), NULL);
	gtk_tree_store_clear(store);
	for(itsongs = songs.begin(); itsongs != songs.end(); itsongs++) {
		Song* song = *itsongs;
		if (song->statusOfSong == REMOVE_FROM_DEVICE) continue;
		gchar* artist = g_locale_to_utf8(song->artist, -1, NULL, NULL, NULL);
		gchar* album = g_locale_to_utf8(song->album, -1, NULL, NULL, NULL);
		gchar* title = g_strdup_printf("%02d) %s", song->track_nr, g_locale_to_utf8(song->title, -1, NULL, NULL, NULL));

		if (gtk_tree_model_get_iter_first(model, &iter_artist)) {
			gchar *str_data;
			while(true) {
				str_data = NULL;
				gtk_tree_model_get(model, &iter_artist, COL_TEXT, &str_data, -1);
				if (str_data && !strcmp(str_data, artist)) {
					g_free(artist);
					artist = NULL;
					if (!gtk_tree_model_iter_children(model, &iter_album, &iter_artist))
						break;
					while(true) {
						str_data = NULL;
						gtk_tree_model_get(model, &iter_album, COL_TEXT, &str_data, -1);
						if (str_data && !strcmp(str_data, album)) {
							g_free(album);
							album = NULL;
							if (!gtk_tree_model_iter_children(model, &iter_title, &iter_album))
								break;
							while(true) {
								str_data = NULL;
								gtk_tree_model_get(model, &iter_title, COL_TEXT, &str_data, -1);
								if (str_data && !strcmp(str_data, title)) {
									g_free(title);
									title = NULL;
									break;
								}
								if (!gtk_tree_model_iter_next(model, &iter_title)) break;
							}
							break;
						}
						if (!gtk_tree_model_iter_next(model, &iter_album)) break;
					}
					break;
				}

				if (!gtk_tree_model_iter_next(model, &iter_artist)) break;
			}
		}

		GdkPixbuf* pixbuf;
		GError* _error = NULL;
#ifdef __IMAGES_H__
		GdkPixbufLoader* loader;
#endif 

		if (artist) {
			gtk_tree_store_append(store, &iter_artist, NULL);
#ifdef __IMAGES_H__
			loader = gdk_pixbuf_loader_new_with_type("png", &_error);
			gdk_pixbuf_loader_write(loader, artist_image, sizeof(artist_image), &_error);
			pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
			gdk_pixbuf_loader_close(loader, &_error);
#else
			pixbuf = gdk_pixbuf_new_from_file("a1.png", &_error);
#endif
			gtk_tree_store_set(
					store,
					&iter_artist,
					COL_ICON,
					pixbuf,
					COL_TEXT,
					artist,
					-1);
		}

		if (album) {
			gtk_tree_store_append(store, &iter_album, &iter_artist);
#ifdef __IMAGES_H__
			loader = gdk_pixbuf_loader_new_with_type("png", &_error);
			gdk_pixbuf_loader_write(loader, album_image, sizeof(album_image), &_error);
			pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
			gdk_pixbuf_loader_close(loader, &_error);
#else
			pixbuf = gdk_pixbuf_new_from_file("a2.png", &_error);
#endif
			gtk_tree_store_set(
					store,
					&iter_album,
					COL_ICON,
					pixbuf,
					COL_TEXT,
					album,
					-1);
		}

		if (title) {
			gtk_tree_store_append(store, &iter_title, &iter_album);
#ifdef __IMAGES_H__
			loader = gdk_pixbuf_loader_new_with_type("png", &_error);
			gdk_pixbuf_loader_write(loader, title_image, sizeof(title_image), &_error);
			pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
			gdk_pixbuf_loader_close(loader, &_error);
#else
			pixbuf = gdk_pixbuf_new_from_file("a3.png", &_error);
#endif
			gtk_tree_store_set(
					store,
					&iter_title,
					COL_ICON,
					pixbuf,
					COL_TEXT,
					title,
					COL_SONG,
					song,
					-1);
		}

		if (artist) g_free(artist);
		if (album) g_free(album);
		if (title) g_free(title);
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(widget), model);
	gtk_tree_view_expand_all(GTK_TREE_VIEW(widget));

	bool enabled = sonydb->isPresent();
	button = (GtkWidget*)g_object_get_data(G_OBJECT(widget), "update");
	gtk_widget_set_sensitive(button, enabled);
	button = (GtkWidget*)g_object_get_data(G_OBJECT(widget), "apply");
	gtk_widget_set_sensitive(button, enabled && !reload);
	button = (GtkWidget*)g_object_get_data(G_OBJECT(widget), "clean");
	gtk_widget_set_sensitive(button, enabled);
}

void on_update_clicked(GtkWidget* widget, gpointer user_data)
{
	SonyDb* sonydb = (SonyDb*)user_data;
	GtkWidget* confirm;

	confirm = gtk_message_dialog_new(
			NULL,
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_NONE,
			"Are you sure you want to reset all changes?");
	gtk_dialog_add_button(GTK_DIALOG (confirm), GTK_STOCK_CANCEL, GTK_RESPONSE_NO);
	gtk_dialog_add_button(GTK_DIALOG (confirm), GTK_STOCK_APPLY, GTK_RESPONSE_YES);
	gtk_dialog_set_default_response(GTK_DIALOG (confirm), GTK_RESPONSE_CANCEL);

	gint response = gtk_dialog_run(GTK_DIALOG(confirm));
	gtk_widget_destroy(confirm);

	if (response == GTK_RESPONSE_YES) {
		GtkWidget* treeview = (GtkWidget*)g_object_get_data(G_OBJECT(widget), "treeview");
		rebuild_tree(treeview, sonydb, true);
	}
}

void on_clean_clicked(GtkWidget* widget, gpointer user_data)
{
	SonyDb* sonydb = (SonyDb*)user_data;
	GtkWidget* confirm;

	confirm = gtk_message_dialog_new(
			NULL,
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_NONE,
			"Are you sure you want to clean all data?");
	gtk_dialog_add_button(GTK_DIALOG (confirm), GTK_STOCK_CANCEL, GTK_RESPONSE_NO);
	gtk_dialog_add_button(GTK_DIALOG (confirm), GTK_STOCK_APPLY, GTK_RESPONSE_YES);
	gtk_dialog_set_default_response(GTK_DIALOG (confirm), GTK_RESPONSE_CANCEL);

	gint response = gtk_dialog_run(GTK_DIALOG(confirm));
	gtk_widget_destroy(confirm);

	if (response == GTK_RESPONSE_YES) {
		GtkWidget* treeview = (GtkWidget*)g_object_get_data(G_OBJECT(widget), "treeview");
		vector<Song*> songs;
		vector<Song*>::iterator itsongs;
		vector<Playlist*> plist;
		vector<Playlist*>::iterator itplist;

		plist = sonydb->getPlaylist();
		for(itplist = plist.begin(); itplist != plist.end(); itplist++)
			sonydb->deletePlaylist((*itplist)->index, false);
		songs = sonydb->getSongs();
		for(itsongs = songs.begin(); itsongs != songs.end(); itsongs++)
			sonydb->delSong(*itsongs);
		rebuild_tree(treeview, sonydb, false);
	}
}

void on_apply_clicked(GtkWidget* widget, gpointer user_data)
{
	SonyDb* sonydb = (SonyDb*)user_data;
	GtkWidget* confirm;

	confirm = gtk_message_dialog_new(
			NULL,
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_NONE,
			"Are you sure you want to apply changes?");
	gtk_dialog_add_button(GTK_DIALOG (confirm), GTK_STOCK_CANCEL, GTK_RESPONSE_NO);
	gtk_dialog_add_button(GTK_DIALOG (confirm), GTK_STOCK_APPLY, GTK_RESPONSE_YES);
	gtk_dialog_set_default_response(GTK_DIALOG (confirm), GTK_RESPONSE_CANCEL);

	gint response = gtk_dialog_run(GTK_DIALOG(confirm));
	gtk_widget_destroy(confirm);

	if (response == GTK_RESPONSE_YES) {
		GtkWidget* treeview = (GtkWidget*)g_object_get_data(G_OBJECT(widget), "treeview");
		sonydb->writeTracks();
		rebuild_tree(treeview, sonydb, true);
	}
}

void treeview_multi_delete(
		GtkTreeModel* model,
		GtkTreePath* path,
		GtkTreeIter* iter,
		gpointer data) {
	SonyDb* sonydb = (SonyDb*)data;
	Song* song = NULL;
	bool removed = false;
	gtk_tree_model_get(model, iter, COL_SONG, &song, -1);
	if (song) {
		sonydb->delSong(song);
		removed = true;
	} else {
		GtkTreeIter iter_album;
		GtkTreeIter iter_title;
		if (gtk_tree_path_get_depth(path) == 2) {
			if (!gtk_tree_model_iter_children(model, &iter_title, iter))
				return;
			while(true) {
				gtk_tree_model_get(model, &iter_title, COL_SONG, &song, -1);
				if (song) {
					sonydb->delSong(song);
					removed = true;
				}
				if (!gtk_tree_model_iter_next(model, &iter_title)) break;
			}
		}
		else
		if (gtk_tree_path_get_depth(path) == 1) {
			if (!gtk_tree_model_iter_children(model, &iter_album, iter))
				return;
			while(true) {
				if (!gtk_tree_model_iter_children(model, &iter_title, &iter_album))
					return;
				while(true) {
					gtk_tree_model_get(model, &iter_title, COL_SONG, &song, -1);
					if (song) {
						sonydb->delSong(song);
						removed = true;
					}
					if (!gtk_tree_model_iter_next(model, &iter_title)) break;
				}
				if (song) {
					sonydb->delSong(song);
					removed = true;
				}
				if (!gtk_tree_model_iter_next(model, &iter_album)) break;
			}
		}
	}
	if (removed)
		g_object_set_data(G_OBJECT(model), "deleted", (void*)"yes");
}

gboolean on_key_pressed(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
{
	SonyDb* sonydb = (SonyDb*)user_data;
	GtkTreeSelection* selection;
	GtkTreeModel* model;
	GtkTreeIter iter;
	if (event->keyval == GDK_F5) {
		rebuild_tree(widget, sonydb, true);
	}
	else
	if (event->keyval == GDK_Delete) {
		GtkTreeModel* model;
		model = (GtkTreeModel*)gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
		g_object_set_data(G_OBJECT(model), "deleted", NULL);
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
		gtk_tree_selection_selected_foreach(selection, treeview_multi_delete, sonydb);
		if (g_object_get_data(G_OBJECT(model), "deleted"))
			rebuild_tree(widget, sonydb, false);
	}
	return TRUE;
}

void edit_dialog(GtkWidget* widget, SonyDb* sonydb, Song* song, int edit_type)
{
	GtkWidget* dialog;
	GtkWidget* table;
	GtkWidget* label;
	GtkWidget* artist;
	GtkWidget* album;
	GtkWidget* title;

	dialog = gtk_dialog_new();
	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
			GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
			NULL);

	gtk_window_set_title(GTK_WINDOW(dialog), gtk_window_get_title(GTK_WINDOW(gtk_widget_get_toplevel(widget))));
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);

	table = gtk_table_new(2, 3, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(table), 6);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), table); 

	label = gtk_label_new("Artist:");
	gtk_table_attach(
			GTK_TABLE(table),
			label,
			0, 1,                   0, 1,
			GTK_FILL,               GTK_FILL,
			0,                      0);
	artist = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(artist), g_locale_to_utf8(song->artist, -1, NULL, NULL, NULL));
	gtk_label_set_use_underline(GTK_LABEL(label), TRUE);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), artist);
	gtk_table_attach(
			GTK_TABLE(table),
			artist,
			1, 2,                   0, 1,
			(GtkAttachOptions)(GTK_EXPAND|GTK_FILL), GTK_FILL,
			0,                      0);

	label = gtk_label_new("Album:");
	gtk_table_attach(
			GTK_TABLE(table),
			label,
			0, 1,                   1, 2,
			GTK_FILL,               GTK_FILL,
			0,                      0);
	album = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(album), g_locale_to_utf8(song->album, -1, NULL, NULL, NULL));
	gtk_label_set_use_underline(GTK_LABEL(label), TRUE);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), album);
	gtk_table_attach(
			GTK_TABLE(table),
			album,
			1, 2,                   1, 2,
			(GtkAttachOptions)(GTK_EXPAND|GTK_FILL), GTK_FILL,
			0,                      0);

	label = gtk_label_new("Title:");
	gtk_table_attach(
			GTK_TABLE(table),
			label,
			0, 1,                   2, 3,
			GTK_FILL,               GTK_FILL,
			0,                      0);
	title = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(title), g_locale_to_utf8(song->title, -1, NULL, NULL, NULL));
	gtk_label_set_use_underline(GTK_LABEL(label), TRUE);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), title);
	gtk_table_attach(
			GTK_TABLE(table),
			title,
			1, 2,                   2, 3,
			(GtkAttachOptions)(GTK_EXPAND|GTK_FILL), GTK_FILL,
			0,                      0);
	gtk_window_set_transient_for(
			GTK_WINDOW(dialog),
			GTK_WINDOW(gtk_widget_get_toplevel(widget)));

	gtk_widget_show_all(dialog);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		gchar* artist_text = (gchar*)g_locale_from_utf8(gtk_entry_get_text(GTK_ENTRY(artist)), -1, NULL, NULL, NULL);
		gchar* album_text = (gchar*)g_locale_from_utf8(gtk_entry_get_text(GTK_ENTRY(album)), -1, NULL, NULL, NULL);
		gchar* title_text = (gchar*)g_locale_from_utf8(gtk_entry_get_text(GTK_ENTRY(title)), -1, NULL, NULL, NULL);
		free(song->artist);
		free(song->album);
		free(song->title);
		song->artist = strdup(artist_text);
		song->album = strdup(album_text);
		song->title = strdup(title_text);
		sonydb->updSong(song);
		rebuild_tree(widget, sonydb, false);
	}

	gtk_widget_destroy(dialog);
}

void on_row_activated(
		GtkTreeView* treeview,
		GtkTreePath* path,
		GtkTreeViewColumn* column,
		gpointer user_data)
{
	SonyDb* sonydb = (SonyDb*)user_data;
	GtkTreeIter iter;
	Song* song;

	GtkTreeModel* model = (GtkTreeModel*)gtk_tree_view_get_model(treeview);
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, COL_SONG, &song, -1);
	if (song) {
		edit_dialog(GTK_WIDGET(treeview), sonydb, song, EDIT_TYPE_TITLE);
	}
}

void on_drag_data_received(
		GtkWidget* widget,
		GdkDragContext* drag_context,
		gint x,
		gint y,
		GtkSelectionData* data,
		guint info,
		guint time,
		gpointer user_data)
{
	SonyDb* sonydb = (SonyDb*)user_data;
	gchar* uri_list = (gchar*)data->data;
	gchar** uris = g_strsplit(uri_list, "\r\n", 255);
	for(; *uris; uris++) {
		gchar* file_utf = g_filename_from_uri(g_strchomp(*uris), NULL, NULL);
		if (!file_utf) continue;
		gchar* file_loc = g_locale_from_utf8(file_utf, -1, NULL, NULL, NULL);
		std::string file = file_loc;
		g_free(file_utf);
		g_free(file_loc);

		ID3Tag* tag = ID3Tag_New();
		if (ID3Tag_LinkWithFlags(tag, file.c_str(), ID3TT_ID3V2) == 0) {
			ID3Tag_Delete(tag);
			continue;
		}
		Song* song = new Song;
		memset(song, 0, sizeof(Song));

		song->album=strdup(get_tag(tag, ID3FID_ALBUM));
		song->artist=strdup(get_tag(tag, ID3FID_LEADARTIST));
		song->title=strdup(get_tag(tag, ID3FID_TITLE));
		song->genre=strdup(get_genre(get_tag(tag, ID3FID_CONTENTTYPE)));
		song->track_nr=atol(get_tag(tag, ID3FID_TRACKNUM));
		song->filename = strdup(file.c_str());
		song->year=atol(get_tag(tag, ID3FID_RELEASETIME));
		song->songlen=atol(get_tag(tag, ID3FID_SONGLEN))/1000;
		song->wAlbum = 0; //(utf16char*)wcsdup(tag.getAlbum());
		song->wArtist = 0; //(utf16char*)wcsdup(tag.getArtist());
		song->wGenre = 0; //(utf16char*)wcsdup(string2wstring(song->genre).c_str());
		song->wTitle = 0; //(utf16char*)wcsdup(tag.getTitle());
		song->statusOfSong = ADD_TO_DEVICE;
		print_song(song);
		sonydb->addSong(song);
		ID3Tag_Delete(tag);
		rebuild_tree(widget, sonydb, false);
	}
}

void gui(int argc, char* argv[])
{
	GtkWidget* window;
	GtkWidget* treeview;
	GtkWidget* swin;
	GtkWidget* button;
	GtkTreeSelection* selection;
	GtkTreeStore* store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkWidget* vbox;
	GtkWidget* hbox;
	GtkTargetList* targetlist;
	GtkTargetEntry drag_types[] = {
    	{"text/uri-list", 0, 1}
	};
	gint n_drag_types = sizeof (drag_types) / sizeof (drag_types [0]);
	bool detected = false;
	SonyDb sonydb;

	gtk_init(&argc, &argv);

	char* playerPath = getenv("SONYDB_PLAYERPATH");
	if (playerPath != NULL) {
		if (!sonydb.detectPlayer(playerPath)) {
			argc = 0;
		}
	} else {
		if (!sonydb.detectPlayer()) {
			argc = 0;
		}
	}

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "SonyDb");
	g_signal_connect(G_OBJECT(window), "delete-event", gtk_main_quit, NULL);

	vbox = gtk_vbox_new(FALSE, 6);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	store = gtk_tree_store_new(COL_MAX,
			GDK_TYPE_PIXBUF,
			GTK_TYPE_STRING,
			GTK_TYPE_POINTER);
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_drag_dest_set(
			treeview,
			GTK_DEST_DEFAULT_ALL,
			drag_types,
			n_drag_types,
			GDK_ACTION_COPY);
	g_signal_connect(G_OBJECT(treeview), "drag-data-received", G_CALLBACK(on_drag_data_received), &sonydb);
	g_signal_connect(G_OBJECT(treeview), "key-press-event", G_CALLBACK(on_key_pressed), &sonydb);
	g_signal_connect(G_OBJECT(treeview), "row_activated", G_CALLBACK(on_row_activated), &sonydb);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), FALSE);
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(treeview), COL_TEXT);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(treeview), TRUE);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	column = gtk_tree_view_column_new();

	// artist
    renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(renderer, "xalign", 0.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", COL_ICON);
    renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "xalign", 0.0, NULL);
	g_object_set(renderer, "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", COL_TEXT);

    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	swin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(
			GTK_SCROLLED_WINDOW(swin),
			GTK_POLICY_AUTOMATIC, 
			GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swin), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(swin), treeview);

	gtk_box_pack_start(GTK_BOX(vbox), swin, TRUE, TRUE, 0);

	hbox = gtk_hbox_new(FALSE, 6);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 6);
	gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

	button = gtk_button_new_with_label("_Update");
	gtk_label_set_use_underline(GTK_LABEL(GTK_BIN(button)->child), TRUE);
	g_object_set_data(G_OBJECT(button), "treeview", treeview);
	g_object_set_data(G_OBJECT(treeview), "update", button);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_update_clicked), &sonydb);
	gtk_container_add(GTK_CONTAINER(hbox), button);

	button = gtk_button_new_with_label("_Apply");
	gtk_label_set_use_underline(GTK_LABEL(GTK_BIN(button)->child), TRUE);
	g_object_set_data(G_OBJECT(button), "treeview", treeview);
	g_object_set_data(G_OBJECT(treeview), "apply", button);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_apply_clicked), &sonydb);
	gtk_container_add(GTK_CONTAINER(hbox), button);

	button = gtk_button_new_with_label("C_lean");
	gtk_label_set_use_underline(GTK_LABEL(GTK_BIN(button)->child), TRUE);
	g_object_set_data(G_OBJECT(button), "treeview", treeview);
	g_object_set_data(G_OBJECT(treeview), "clean", button);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_clean_clicked), &sonydb);
	gtk_container_add(GTK_CONTAINER(hbox), button);

	button = gtk_button_new_with_label("_Close");
	gtk_label_set_use_underline(GTK_LABEL(GTK_BIN(button)->child), TRUE);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(gtk_main_quit), NULL);
	gtk_container_add(GTK_CONTAINER(hbox), button);

	gtk_widget_set_size_request(window, 300, 200);
	gtk_widget_queue_resize(window);
	gtk_widget_show_all(window);

	if (detected) {
		rebuild_tree(treeview, &sonydb, true);
	} else {
		GtkWidget* error;
		error  = gtk_message_dialog_new(
			GTK_WINDOW(window),
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK,
			"Can't detect devices!");
		gtk_window_set_title(GTK_WINDOW(error), gtk_window_get_title(GTK_WINDOW(window)));
		gint response = gtk_dialog_run(GTK_DIALOG(error));
		gtk_widget_destroy(error);
		return;
	}

	gtk_main();
}
#endif

int main(int argc, char* argv[])
{
#ifdef GUI
	gui(argc, argv);
#else
	setlocale(LC_CTYPE, "");
	SonyDb sonydb;

	char* playerPath = getenv("SONYDB_PLAYERPATH");
	if (playerPath != NULL) {
		if (!sonydb.detectPlayer(playerPath)) {
			argc = 0;
		}
	} else {
		if (!sonydb.detectPlayer()) {
			argc = 0;
		}
	}

	if (argc == 2 && !strcmp(argv[1], "songs")) {
		vector<Song*> songs;
		vector<Song*>::iterator itsongs;
		sonydb.readAllTracks();
		songs = sonydb.getSongs();
		for(itsongs = songs.begin(); itsongs != songs.end(); itsongs++) {
			printf("%04d,%s,%s,%s\n",
					(*itsongs)->sonyDbOrder,
					(*itsongs)->artist,
					(*itsongs)->album,
					(*itsongs)->title
					);
			deleteSongPtr(*itsongs);
		}
	}
	else
	if (argc == 3 && !strcmp(argv[1], "songs")) {
		vector<Song*> songs;
		vector<Song*>::iterator itsongs;
		sonydb.readAllTracks();
		songs = sonydb.getSongs();
		for(itsongs = songs.begin(); itsongs != songs.end(); itsongs++) {
			if ((*itsongs)->sonyDbOrder == atol(argv[2])) {
				printf("songs(%04d): %s,%s,%s\n\tencoding=%s\n\tfilename=%s\n\tgenre=%s\n\tsonglen=%d\n\ttrack_nr=%d\n\tyear=%d\n",
						(*itsongs)->sonyDbOrder,
						(*itsongs)->artist,
						(*itsongs)->album,
						(*itsongs)->title,
						(*itsongs)->encoding,
						(*itsongs)->filename,
						(*itsongs)->genre,
						(*itsongs)->songlen,
						(*itsongs)->track_nr,
						(*itsongs)->year
						);
			}
			deleteSongPtr(*itsongs);
		}
	}
	else
	if (argc == 3 && !strcmp(argv[1], "songsdel")) {
		vector<Song*> songs;
		vector<Song*>::iterator itsongs;
		sonydb.readAllTracks();
		songs = sonydb.getSongs();
		for(itsongs = songs.begin(); itsongs != songs.end(); itsongs++) {
			if ((*itsongs)->sonyDbOrder == atol(argv[2]))
				sonydb.delSong(*itsongs);
			deleteSongPtr(*itsongs);
		}
		sonydb.writeTracks();
	}
	else
	if (argc >= 3 && !strcmp(argv[1], "songsadd")) {
		vector<Song*> songs;
		sonydb.readAllTracks();
		songs = sonydb.getSongs();

		ID3Tag* tag;

		std::string title;
		std::string artist;
		std::string album;
		std::string genre;
		std::vector<std::string> files;
		int track_nr = 0;
		int n;
		for(n = 2; n < argc; n++) {
			if (!strcmp(argv[n], "-title"))
				title = argv[++n];
			else
			if (!strcmp(argv[n], "-artist"))
				artist = argv[++n];
			else
			if (!strcmp(argv[n], "-album"))
				album = argv[++n];
			else
			if (!strcmp(argv[n], "-genre"))
				genre = argv[++n];
			else
			if (!strcmp(argv[n], "-track_nr"))
				track_nr = atol(argv[++n]);
			else
				break;
		}

		for(; n < argc; n++) {
			tag = ID3Tag_New();
			ID3Tag_Link(tag, argv[n]);
			Song* song = new Song;
			memset(song, 0, sizeof(Song));

			if (!album.size())
				song->album=strdup(get_tag(tag, ID3FID_ALBUM));
			else
				song->album=strdup(album.c_str());
			if (!artist.size())
				song->artist=strdup(get_tag(tag, ID3FID_LEADARTIST));
			else
				song->artist=strdup(artist.c_str());
			if (!title.size())
				song->title=strdup(get_tag(tag, ID3FID_TITLE));
			else
				song->title=strdup(title.c_str());
			if (!genre.size())
				song->genre=strdup(get_genre(get_tag(tag, ID3FID_CONTENTTYPE)));
			else
				song->genre=strdup(genre.c_str());
			if (track_nr < 1)
				song->track_nr=atol(get_tag(tag, ID3FID_TRACKNUM));
			else
				song->track_nr=track_nr;
			song->filename = strdup(argv[n]);
			song->year=atol(get_tag(tag, ID3FID_RELEASETIME));
			song->songlen=atol(get_tag(tag, ID3FID_SONGLEN))/1000;
			song->wAlbum = 0; //(utf16char*)wcsdup(tag.getAlbum());
			song->wArtist = 0; //(utf16char*)wcsdup(tag.getArtist());
			song->wGenre = 0; //(utf16char*)wcsdup(string2wstring(song->genre).c_str());
			song->wTitle = 0; //(utf16char*)wcsdup(tag.getTitle());
			song->statusOfSong = ADD_TO_DEVICE;
			sonydb.addSong(song);
			ID3Tag_Delete(tag);
		}
		sonydb.writeTracks();
	}
	else
	if (argc == 3 && !strcmp(argv[1], "songschk")) {
		vector<Song*> songs;
		sonydb.readAllTracks();
		songs = sonydb.getSongs();

		ID3Tag* tag;

		tag = ID3Tag_New();
		ID3Tag_Link(tag, argv[2]);
		Song* song = new Song;
		memset(song, 0, sizeof(Song));

		song->album=strdup(get_tag(tag, ID3FID_ALBUM));
		song->artist=strdup(get_tag(tag, ID3FID_LEADARTIST));
		song->title=strdup(get_tag(tag, ID3FID_TITLE));
		song->genre=strdup(get_genre(get_tag(tag, ID3FID_CONTENTTYPE)));
		song->filename = strdup(argv[2]);
		song->track_nr=atol(get_tag(tag, ID3FID_TRACKNUM));
		song->year=atol(get_tag(tag, ID3FID_RELEASETIME));
		song->songlen=atol(get_tag(tag, ID3FID_SONGLEN))/1000;
		song->wAlbum = 0; //(utf16char*)wcsdup(tag.getAlbum());
		song->wArtist = 0; //(utf16char*)wcsdup(tag.getArtist());
		song->wGenre = 0; //(utf16char*)wcsdup(string2wstring(song->genre).c_str());
		song->wTitle = 0; //(utf16char*)wcsdup(tag.getTitle());
		song->statusOfSong = ADD_TO_DEVICE;
		printf("songs(%04d): %s,%s,%s\n\tencoding=%s\n\tfilename=%s\n\tgenre=%s\n\tsonglen=%d\n\ttrack_nr=%d\n\tyear=%d\n",
				song->sonyDbOrder,
				song->artist,
				song->album,
				song->title,
				song->encoding,
				song->filename,
				song->genre,
				song->songlen,
				song->track_nr,
				song->year
				);
	}
	else
	if (argc == 2 && !strcmp(argv[1], "plist")) {
		vector<Playlist*> plist;
		vector<Playlist*>::iterator itplist;
		sonydb.readAllPlaylist();
		plist = sonydb.getPlaylist();
		for(itplist = plist.begin(); itplist != plist.end(); itplist++) {
			printf("%04d,%s\n",
					(*itplist)->index,
					(*itplist)->name
					);
		}
	}
	else
	if (argc == 3 && !strcmp(argv[1], "plist")) {
		vector<Playlist*> plist;
		vector<Song*> songs;
		vector<Playlist*>::iterator itplist;
		sonydb.readAllTracks();
		sonydb.readAllPlaylist();
		plist = sonydb.getPlaylist();
		for(itplist = plist.begin(); itplist != plist.end(); itplist++) {
			if ((*itplist)->index == atol(argv[2])) {
				printf("playlist(%04d): %s\n",
						(*itplist)->index,
						(*itplist)->name
						);
				songs = sonydb.getSongsInPlaylist((*itplist)->index);
				vector<Song*>::iterator itsongs;
				for(itsongs = songs.begin(); itsongs != songs.end(); itsongs++) {
					printf("%04d,%s,%s,%s\n",
							(*itsongs)->sonyDbOrder,
							(*itsongs)->artist,
							(*itsongs)->album,
							(*itsongs)->title
							);
					deleteSongPtr(*itsongs);
				}
			}
		}
	}
	else
	if (argc >= 3 && !strcmp(argv[1], "plistadd")) {
		vector<Song*> songs;
		vector<Song*>::iterator itsongs;
		vector<Playlist*> plist;

		sonydb.readAllTracks();
		sonydb.readAllPlaylist();
		songs = sonydb.getSongs();
		plist = sonydb.getPlaylist();

		Playlist* newplist = new Playlist;
		newplist->name = strdup(argv[2]);
		newplist->index = plist.size() + 1;
		for(itsongs = songs.begin(); itsongs != songs.end(); itsongs++) {
			for(int n = 2; n < argc; n++) {
				if (atol(argv[n]) == (*itsongs)->sonyDbOrder)
					newplist->songs.push_back(*itsongs);
			}
		}
		sonydb.addPlaylist(newplist);
		sonydb.writeTracks();
	}
	else
	if (argc == 3 && !strcmp(argv[1], "plistdel")) {
		sonydb.readAllTracks();
		sonydb.readAllPlaylist();
		sonydb.deletePlaylist(atol(argv[2]), false);
		sonydb.writeTracks();
	}
	else
	if (argc == 2 && !strcmp(argv[1], "forceclean")) {
		vector<Song*> songs;
		vector<Song*>::iterator itsongs;
		vector<Playlist*> plist;
		vector<Playlist*>::iterator itplist;
		sonydb.readAllTracks();
		sonydb.readAllPlaylist();

		plist = sonydb.getPlaylist();
		for(itplist = plist.begin(); itplist != plist.end(); itplist++)
			sonydb.deletePlaylist((*itplist)->index, false);
		songs = sonydb.getSongs();
		for(itsongs = songs.begin(); itsongs != songs.end(); itsongs++)
			sonydb.delSong(*itsongs);
		sonydb.writeTracks();
	}
	else
	if (argc == 2 && !strcmp(argv[1], "check")) {
		vector<Song*> songs;
		vector<Song*>::iterator itsongs;
		sonydb.readAllTracks();
		sonydb.readAllPlaylist();
		sonydb.readAllTracks();
		songs = sonydb.getSongs();
		for(itsongs = songs.begin(); itsongs != songs.end(); itsongs++) {
			struct stat statbuf = {0};
			if (stat((*itsongs)->filename, &statbuf) == -1)
				printf("%04d:%s is not exist\n",
						(*itsongs)->sonyDbOrder,
						(*itsongs)->filename);
			deleteSongPtr(*itsongs);
		}
	}
	else
	if (argc >= 2 && !strcmp(argv[1], "rebuild")) {
		vector<Song*> songs;
		vector<Song*>::iterator itsongs;
		sonydb.readAllTracks();
		sonydb.readAllPlaylist();
		songs = sonydb.getSongs();
		for(itsongs = songs.begin(); itsongs != songs.end(); itsongs++) {
			struct stat statbuf = {0};
			if (stat((*itsongs)->filename, &statbuf) == -1) {
				printf("deleted link of ", (*itsongs)->filename);
				sonydb.delSong(*itsongs);
			}
			deleteSongPtr(*itsongs);
		}
		if (argc >= 3)
			strncpy(sonydb.getDriveLetter(), argv[2], 2);
		sonydb.writeTracks();
	}
	else
	{
		if (sonydb.getDriveLetter())
			printf("detected device %s\n", sonydb.getDriveLetter());
		else
			printf("no detected devices\n");
		printf(
			"usage...\n"
			"\tsongs [id]\n"
			"\tsongsdel [id]\n"
			"\tsongsadd [file]\n"
			"\tsongschk [file]\n"
			"\tplist [id]\n"
			"\tplistdel [id]\n"
			"\tplistadd [file]\n"
			);
	}
#endif
	return 0;
}

