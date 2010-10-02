#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <cstring>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <cstdlib>
#include <cstdio>

#include "libtabfile.h"
#include "libbabylonfile.h"
#include "libstardict2txt.h"
#include "libstardictverify.h"
#include "libbgl2txt.h"
#include "lib_stardict_bin2text.h"
#include "resourcewrap.hpp"

static GtkWidget *main_window;
static GtkTextBuffer *compile_page_text_view_buffer;
static GtkWidget *compile_page_combo_box;
static GtkWidget *decompile_page_combo_box;
static GtkTextBuffer *decompile_page_text_view_buffer;
static GtkTextBuffer *edit_page_text_view_buffer;
static GtkWidget *decompile_page_entry_chunk_size;
static GtkWidget *decompile_page_textual_stardict_hbox;

static void on_browse_button_clicked(GtkButton *button, gpointer data)
{
	GtkEntry *entry = GTK_ENTRY(data);
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new ("Open file...",
		GTK_WINDOW(main_window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER (dialog), gtk_entry_get_text(entry));
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		gchar *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		gtk_entry_set_text(entry, filename);
		g_free (filename);
	}
	gtk_widget_destroy (dialog);
}

static void compile_page_print_info(const char *info, ...)
{
	va_list va;
	va_start(va, info);
	char *str = g_strdup_vprintf(info, va);
	gtk_text_buffer_insert_at_cursor(compile_page_text_view_buffer, str, -1);
	g_free(str);
	va_end(va);
}

static void on_compile_page_compile_button_clicked(GtkButton *button, gpointer data)
{
	GtkEntry *entry = GTK_ENTRY(data);
	gtk_text_buffer_set_text(compile_page_text_view_buffer, "Building...\n", -1);
	gint output_format_ind = gtk_combo_box_get_active(GTK_COMBO_BOX(compile_page_combo_box));
	bool res = true;
	if (output_format_ind == 0)
		res = convert_tabfile(gtk_entry_get_text(entry), compile_page_print_info);
	else if (output_format_ind == 1)
		convert_babylonfile(gtk_entry_get_text(entry), compile_page_print_info, true);
	else
		convert_bglfile(gtk_entry_get_text(entry), "", "");
	gtk_text_buffer_insert_at_cursor(compile_page_text_view_buffer, 
		res ? "Done!\n" : "Failed!\n",
		-1);
}

static void create_compile_page(GtkWidget *notebook)
{
	GtkWidget *vbox = gtk_vbox_new(false, 6);
	GtkWidget *label = gtk_label_new("Compile");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, label);
	GtkWidget *hbox = gtk_hbox_new(false, 6);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
	label = gtk_label_new("File name:");
	gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);
	GtkWidget *entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), entry, false, false, 0);
	GtkWidget *button = gtk_button_new_with_mnemonic("Bro_wse...");
	gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_browse_button_clicked), entry);
	label = gtk_label_new("This file should be encoded in UTF-8!");
	gtk_box_pack_start(GTK_BOX(vbox), label, false, false, 0);
	GtkWidget *text_view = gtk_text_view_new();
	gtk_widget_set_size_request(text_view, -1, 150);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
	compile_page_text_view_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
	gtk_text_buffer_set_text(compile_page_text_view_buffer,
		"Here is a example dict.tab file:\n"
		"============\n"
		"a\t1\\n2\\n3\n"
		"b\t4\\\\5\\n6\n"
		"c\t789\n"
		"============\n"
		"Each line contains a word - definition pair. The word is splitted from definition with a tab character. "
		"You may use the following escapes: \\n - new line, \\\\ - \\, \\t - tab character.\n"
		"\n\n"
		"Another format that StarDict recommends is babylon source file format, it is just like this:\n"
		"======\n"
		"apple|apples\n"
		"the meaning of apple\n"
		"\n"
		"2dimensional|2dimensionale|2dimensionaler|2dimensionales|2dimensionalem|2dimensionalen\n"
		"two dimensional's meaning<br>the second line.\n"
		"\n"
		"======\n"
		, -1);
	GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, true, true, 0);
	hbox = gtk_hbox_new(false, 6);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
	
	compile_page_combo_box = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(compile_page_combo_box), "Tab file");
	gtk_combo_box_append_text(GTK_COMBO_BOX(compile_page_combo_box), "Babylon file");
	gtk_combo_box_append_text(GTK_COMBO_BOX(compile_page_combo_box), "BGL file");
	gtk_combo_box_set_active(GTK_COMBO_BOX(compile_page_combo_box), 0);
	gtk_box_pack_start(GTK_BOX(hbox), compile_page_combo_box, true, false, 0);

	button = gtk_button_new_with_mnemonic("_Compile");
	gtk_box_pack_start(GTK_BOX(hbox), button, true, false, 0);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_compile_page_compile_button_clicked), entry);
}

static void decompile_page_print_info(const char *info, ...)
{
	va_list va;
	va_start(va, info);
	char *str = g_strdup_vprintf(info, va);
	gtk_text_buffer_insert_at_cursor(decompile_page_text_view_buffer, str, -1);
	g_free(str);
	va_end(va);
}

static void on_decompile_page_build_button_clicked(GtkButton *button, gpointer data)
{
	GtkEntry *entry = GTK_ENTRY(data);
	gtk_text_buffer_set_text(decompile_page_text_view_buffer, "Building...\n", -1);
	gint output_format_ind = gtk_combo_box_get_active(GTK_COMBO_BOX(decompile_page_combo_box));
	int res = EXIT_SUCCESS;
	if(output_format_ind == 0)
		convert_stardict2txt(gtk_entry_get_text(entry), decompile_page_print_info);
	else {
		std::string ifofilename(gtk_entry_get_text(entry));
		std::string::size_type pos = ifofilename.find_last_of('.');
		std::string xmlfilename = (pos == std::string::npos ? ifofilename : ifofilename.substr(0, pos)) + ".xml";
		const gchar* chunk_size_str = gtk_entry_get_text(GTK_ENTRY(decompile_page_entry_chunk_size));
		int chunk_size = atoi(chunk_size_str);
		if(chunk_size < 0)
			chunk_size = 0;
		glib::CharStr temp(g_strdup_printf("%d", chunk_size));
		// set the chunk size back to the entry for the user to see what value was actually used.
		gtk_entry_set_text(GTK_ENTRY(decompile_page_entry_chunk_size), get_impl(temp));
		res = stardict_bin2text(ifofilename, xmlfilename, chunk_size, decompile_page_print_info);
	}
	gtk_text_buffer_insert_at_cursor(decompile_page_text_view_buffer, 
		(res == EXIT_SUCCESS) ? "Done!\n" : "Failed!\n",
		-1);
}

static void on_decompile_page_verify_button_clicked(GtkButton *button, gpointer data)
{
	GtkEntry *entry = GTK_ENTRY(data);
	gtk_text_buffer_set_text(decompile_page_text_view_buffer, "Verifing dictionary files...\n", -1);
	int res = stardict_verify(gtk_entry_get_text(entry), decompile_page_print_info);
	gtk_text_buffer_insert_at_cursor(decompile_page_text_view_buffer, 
		(res == EXIT_SUCCESS) ? "Done!\n" : "Failed!\n",
		-1);
}

static void set_parameter_panel(void)
{
	gint output_format_ind = gtk_combo_box_get_active(GTK_COMBO_BOX(decompile_page_combo_box));
	if(output_format_ind == 1)
		gtk_widget_show(GTK_WIDGET(decompile_page_textual_stardict_hbox));
	else
		gtk_widget_hide(GTK_WIDGET(decompile_page_textual_stardict_hbox));
}

static void on_decompile_page_combo_box_changed(GtkComboBox *widget, gpointer user_data)
{
	set_parameter_panel();
}


static void create_decompile_page(GtkWidget *notebook)
{
	GtkWidget *vbox = gtk_vbox_new(false, 6);
	GtkWidget *label = gtk_label_new("DeCompile/Verify");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, label);
	GtkWidget *hbox = gtk_hbox_new(false, 6);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
	label = gtk_label_new("File name:");
	gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);
	GtkWidget *entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), entry, false, false, 0);
	GtkWidget *button = gtk_button_new_with_mnemonic("Bro_wse...");
	gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_browse_button_clicked), entry);
	label = gtk_label_new("Please choose the somedict.ifo file.");
	gtk_box_pack_start(GTK_BOX(vbox), label, false, false, 0);
	GtkWidget *text_view = gtk_text_view_new();
	gtk_widget_set_size_request(text_view, -1, 150);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
	decompile_page_text_view_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
	GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, true, true, 0);
	hbox = gtk_hbox_new(false, 6);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);

	decompile_page_combo_box = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(decompile_page_combo_box), "Tab file");
	gtk_combo_box_append_text(GTK_COMBO_BOX(decompile_page_combo_box), "Textual StarDict dictionary");
	gtk_combo_box_set_active(GTK_COMBO_BOX(decompile_page_combo_box), 0);
	gtk_box_pack_start(GTK_BOX(hbox), decompile_page_combo_box, true, false, 0);

	button = gtk_button_new_with_mnemonic("_Decompile");
	gtk_box_pack_start(GTK_BOX(hbox), button, true, false, 0);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_decompile_page_build_button_clicked), entry);
	button = gtk_button_new_with_mnemonic("_Verify");
	gtk_box_pack_start(GTK_BOX(hbox), button, true, false, 0);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_decompile_page_verify_button_clicked), entry);

	// parameter panel
	decompile_page_textual_stardict_hbox = gtk_hbox_new(false, 6);
	gtk_box_pack_start(GTK_BOX(vbox), decompile_page_textual_stardict_hbox, false, false, 3);
	label = gtk_label_new("Chunk size (in bytes, 0 - do not split):");
	gtk_box_pack_start(GTK_BOX(decompile_page_textual_stardict_hbox), label, false, false, 0);
	decompile_page_entry_chunk_size = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(decompile_page_entry_chunk_size), "0");
	gtk_box_pack_start(GTK_BOX(decompile_page_textual_stardict_hbox), decompile_page_entry_chunk_size, false, false, 0);

	// must be after the parameter panel is created
	g_signal_connect(G_OBJECT(decompile_page_combo_box), "changed", G_CALLBACK(on_decompile_page_combo_box_changed), NULL);
}

static void on_edit_page_open_button_clicked(GtkButton *button, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new ("Open file...",
		GTK_WINDOW(main_window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		gchar *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		gchar *buffer;
		if (g_file_get_contents(filename, &buffer, NULL, NULL)) {
			gtk_text_buffer_set_text(edit_page_text_view_buffer, buffer, -1);
			g_free(buffer);
		}
		g_free (filename);
	}
	gtk_widget_destroy (dialog);

}

static void on_edit_page_saveas_button_clicked(GtkButton *button, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new ("Save file...",
		GTK_WINDOW(main_window),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		gchar *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		GtkTextIter start, end;
		gtk_text_buffer_get_bounds(edit_page_text_view_buffer, &start, &end);
		gchar *buffer = gtk_text_buffer_get_text(edit_page_text_view_buffer, &start, &end, FALSE);
		FILE *file = g_fopen(filename, "wb");
		fwrite(buffer, 1, strlen(buffer), file);
		fclose(file);
		g_free(buffer);
		g_free (filename);
	}
	gtk_widget_destroy (dialog);
}

static void create_edit_page(GtkWidget *notebook)
{
	GtkWidget *vbox = gtk_vbox_new(false, 6);
	GtkWidget *label = gtk_label_new("Edit");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, label);
	GtkWidget *hbox = gtk_hbox_new(false, 6);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
	GtkWidget *button = gtk_button_new_with_mnemonic("_Open");
	gtk_box_pack_start(GTK_BOX(hbox), button, true, false, 0);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_edit_page_open_button_clicked), NULL);
	button = gtk_button_new_with_mnemonic("_Save as");
	gtk_box_pack_start(GTK_BOX(hbox), button, true, false, 0);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_edit_page_saveas_button_clicked), NULL);
	GtkWidget *text_view = gtk_text_view_new();
	gtk_widget_set_size_request(text_view, -1, 150);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
	edit_page_text_view_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
	gtk_text_buffer_set_text(edit_page_text_view_buffer,
		"This is a simple UTF-8 text file editor.\n"
		, -1);
	GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, true, true, 0);
}

static gboolean on_delete_event(GtkWidget * window, GdkEvent *event , gpointer data)
{
	gtk_main_quit();
	return FALSE;
}

static void create_window()
{
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position (GTK_WINDOW (main_window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW (main_window), "StarDict-Editor");
	gtk_container_set_border_width (GTK_CONTAINER (main_window), 5);
	g_signal_connect (G_OBJECT (main_window), "delete_event", G_CALLBACK (on_delete_event), NULL);
	GtkWidget *notebook = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(main_window), notebook);
	create_compile_page(notebook);
	create_decompile_page(notebook);
	create_edit_page(notebook);
	gtk_widget_show_all(main_window);
	set_parameter_panel();
}

#ifdef _WIN32
int stardict_editor_main(int argc,char **argv)
#else
int main(int argc,char **argv)
#endif
{
	gtk_set_locale();
	gtk_init(&argc, &argv);
	create_window();
	gtk_main();
	return 0;
}

#ifdef _WIN32

#ifdef __GNUC__
#  ifndef _stdcall
#    define _stdcall  __attribute__((stdcall))
#  endif
#endif

int _stdcall
WinMain (struct HINSTANCE__ *hInstance,
struct HINSTANCE__ *hPrevInstance,
	char               *lpszCmdLine,
	int                 nCmdShow)
{
	return stardict_editor_main (__argc, __argv);
}
#endif
