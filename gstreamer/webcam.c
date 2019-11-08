#include <gst/gst.h>
#include <string.h>//gst-launch-1.0 ksvideosrc ! image/jpeg, weigt= , height= , ! jpegdec ! videoconvert ! autovideosink
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

GST_DEBUG_CATEGORY_STATIC (debug_category);
#define GST_CAT_DEFAULT debug_category

typedef struct _CustomData
{
	GstElement *pipeline;
	GstElement *source;
	GstElement *sink;
	GstElement *convert;
	GstElement *filter;
	GstElement *dec;
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

	  GstElement *pipeline =d;
	
		 if (!data.seek_enabled) {
                  GstSample *sample = NULL;
                  GstCaps *caps;
				
                  
                  GST_DEBUG ("taking snapshot");
                  caps = gst_caps_new_simple ("video/x-raw", "format", G_TYPE_STRING, "RGB",
                     /* Note: we don't ask for a specific width/height here, so that
                     * videoscale can adjust dimensions from a non-1/1 pixel aspect
                     * ratio to a 1/1 pixel-aspect-ratio */
                     "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1, NULL);

				  //tqtqtq
                  g_signal_emit_by_name (GST_BIN(pipeline), "convert-sample", caps, &sample);
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
		g_print ("%s\n", err->message);
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

	GstBus *bus;
	GstStateChangeReturn ret;

	GMainContext * context = NULL;

	GSource * source = NULL;

	GstCaps *filtercaps;

	guint bus_watch_id;

	data.playing = FALSE;
	data.terminate = FALSE;
	data.seek_enabled = FALSE;
	data.seek_done = FALSE;
	data.duration = GST_CLOCK_TIME_NONE;

	/* Initialize GStreamer */
	//gst_debug_set_threshold_from_string ("*:4",TRUE);
	 _putenv_s ("GST_DEBUG","*:4");
	gst_init (&argc, &argv);


	/* Create the elements */
	data.source = gst_element_factory_make ("ksvideosrc", "source");
	data.convert=gst_element_factory_make("autovideoconvert","convert");
	data.sink=gst_element_factory_make("autovideosink","sink");
	data.filter = gst_element_factory_make ("capsfilter", "filter");
	data.dec=gst_element_factory_make("jpegdec","dec");


  /* Create the empty pipeline */
	data.pipeline = gst_pipeline_new ("pipeline");
	//data.pipeline=gst_bin_new("bin");
	data.pipeline = gst_parse_launch("pipeline",NULL);

  if (!data.pipeline || !data.source || !data.convert || !data.sink || !data.dec || !data.filter) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }


	gst_bin_add_many(GST_BIN(data.pipeline),data.source,data.filter,data.dec,data.convert,data.sink,NULL);
	 if (!gst_element_link (data.convert, data.sink)) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }

	if(gst_element_link_many(data.source,data.filter,data.dec,data.convert,data.sink,NULL)){
			g_printerr ("Elements could not be linked.\n");
		gst_object_unref (data.pipeline);
		return -1;
	}

	filtercaps = gst_caps_new_simple ("image/jpeg",
               "width", G_TYPE_INT, 1280,//내생각엔 해상도같음
               "height", G_TYPE_INT, 720,
               NULL);
  g_object_set (G_OBJECT (data.filter), "caps", filtercaps, NULL);
  gst_caps_unref (filtercaps);

  
	/* Set the URI to play */
	g_object_set (data.source, "device-index",0,NULL);//웹캠연결시 1



	/* Start playing */
  ret = gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }

	/* Listen to the bus */
	bus = gst_element_get_bus (data.pipeline);
	bus_watch_id = gst_bus_add_watch (bus, my_bus_callback, NULL);

	//create timeout source
	g_timeout_add(3000, timeout_callback,data.pipeline);


	//create main loop

	loop = g_main_loop_new(context, FALSE);

	g_main_loop_run(loop);

	/* Free resources */
	gst_object_unref (bus);
	gst_element_set_state (data.pipeline, GST_STATE_NULL);
	gst_object_unref (data.pipeline);
	g_main_loop_unref(loop);
	return 0;

}
