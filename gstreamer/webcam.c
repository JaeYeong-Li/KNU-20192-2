#include <gst/gst.h>
#include <string.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <math.h>
#include <glib.h>


#include <gdk-pixbuf/gdk-pixbuf.h>

#include <gst/video/videooverlay.h>
#include <gst/video/colorbalance.h>
#include <gst/video/navigation.h>

typedef struct _CustomData
{
	GstElement *playbin;          /* Our one and only element */
	gboolean playing;             /* Are we in the PLAYING state? */
	gboolean terminate;           /* Should we terminate execution? */
	gboolean seek_enabled;        /* Is seeking enabled for this media? */
	gboolean seek_done;           /* Have we performed the seek already? */
	gint64 duration;              /* How long does this media last, in nanoseconds */
} CustomData;


CustomData data;
int idx;
static GMainLoop *loop;

gboolean timeout_callback(gpointer d)
{
	printf("timeout_callback called\n");

		 if (!data.seek_enabled) {
                  GstSample *sample = NULL;
                  GstCaps *caps;
                  
                  GST_DEBUG ("taking snapshot");
                  caps = gst_caps_new_simple ("video/x-raw", "format", G_TYPE_STRING, "RGB",
                     /* Note: we don't ask for a specific width/height here, so that
                     * videoscale can adjust dimensions from a non-1/1 pixel aspect
                     * ratio to a 1/1 pixel-aspect-ratio */
                     "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1, NULL);

                  g_signal_emit_by_name (data.playbin, "convert-sample", caps, &sample);
                  gst_caps_unref (caps);


                  if (sample) {
                     GstBuffer *buffer;
                     GstCaps *caps;
                     GstStructure *s;
                     gboolean res;
                     gint width, height;
                     GdkPixbuf *pixbuf;
                     GError *error = NULL;
                     GstMapInfo map;

                     /* get the snapshot buffer format now. We set the caps on the appsink so
                     * that it can only be an rgb buffer. The only thing we have not specified
                     * on the caps is the height, which is dependent on the pixel-aspect-ratio
                     * of the source material */
                     caps = gst_sample_get_caps (sample);
                     if (!caps) {
                        g_warning ("could not get snapshot format\n");
                        goto done;
                     }

                     s = gst_caps_get_structure (caps, 0);

                     /* we need to get the final caps on the buffer to get the size */
                     res = gst_structure_get_int (s, "width", &width);
                     res |= gst_structure_get_int (s, "height", &height);
                     if (!res) {
                        g_warning ("could not get snapshot dimension\n");
                        goto done;
                     }

                     /* create pixmap from buffer and save, gstreamer video buffers have a stride
                     * that is rounded up to the nearest multiple of 4 */
                     buffer = gst_sample_get_buffer (sample);
                     gst_buffer_map (buffer, &map, GST_MAP_READ);
                     pixbuf = gdk_pixbuf_new_from_data (map.data,
                        GDK_COLORSPACE_RGB, FALSE, 8, width, height,
                        GST_ROUND_UP_4 (width * 3), NULL, NULL);
                     {
                        gchar *snapshot_name = g_strdup_printf ("snapshot-%d.png", idx++);

                        /* save the pixbuf */
                        gdk_pixbuf_save (pixbuf, snapshot_name, "png", &error, NULL);
                        gst_buffer_unmap (buffer, &map);
                        g_clear_error (&error);
                        g_free(snapshot_name);
                     }
                  done:
                     gst_sample_unref (sample);
                  }
                  else{
                      g_print ("could not make snapshot\n");
                  }
	 }
	return TRUE;
}


static gboolean
	my_bus_callback (GstBus * bus, GstMessage * message, gpointer data)
{
//	g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));

	switch (GST_MESSAGE_TYPE (message)) {
	case GST_MESSAGE_ERROR:{
		GError *err;
		gchar *debug;

		gst_message_parse_error (message, &err, &debug);
		g_print ("Error: %s\n", err->message);
		g_error_free (err);
		g_free (debug);

		g_main_loop_quit (loop);
		break;
						   }
	case GST_MESSAGE_EOS:
		/* end-of-stream */
		g_main_loop_quit (loop);
		break;
	default:
		/* unhandled message */
		break;
	}

	/* we want to be notified again the next time there is a message
	* on the bus, so returning TRUE (FALSE means we want to stop watching
	* for messages on the bus and our callback should not be called again)
	*/
	return TRUE;
}
int main(int argc, char *argv[]){

	// GMainLoop * loop = NULL;

	GstMessage *msg;
	GstBus *bus;
	GstStateChangeReturn ret;

	GMainContext * context = NULL;

	GSource * source = NULL;

	guint bus_watch_id;

	int id;

	data.playing = FALSE;
	data.terminate = FALSE;
	data.seek_enabled = FALSE;
	data.seek_done = FALSE;
	data.duration = GST_CLOCK_TIME_NONE;

	/* Initialize GStreamer */
	gst_init (&argc, &argv);
	/* Create the elements */
	data.playbin = gst_element_factory_make ("playbin", "playbin");

	if (!data.playbin) {
		g_printerr ("Not all elements could be created.\n");
		return -1;
	}

	/* Set the URI to play */
	g_object_set (data.playbin, "uri","file:///D:/test.mp4",NULL);

	/* Start playing */
  ret = gst_element_set_state (data.playbin, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (data.playbin);
    return -1;
  }


	/* Listen to the bus */
	bus = gst_element_get_bus (data.playbin);
	bus_watch_id = gst_bus_add_watch (bus, my_bus_callback, NULL);

	//create timeout source
	g_timeout_add(3000, timeout_callback,NULL);

	//source = g_timeout_source_new_seconds(3);

	//create a context

	//context = g_main_context_new();

	//attach source to context

	//id = g_source_attach(source, context);//if context is NULL, the default context will be used.

	//create main loop

	loop = g_main_loop_new(context, FALSE);

	//set the callback for this source

	//g_source_set_callback(source, timeout_callback, loop, NULL);


	g_main_loop_run(loop);

	/* Free resources */
	gst_object_unref (bus);
	gst_element_set_state (data.playbin, GST_STATE_NULL);
	gst_object_unref (data.playbin);
	g_main_loop_unref(loop);
	return 0;

}
