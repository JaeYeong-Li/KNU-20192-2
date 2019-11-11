#include <gst/gst.h>
#include <string.h>//gst-launch-1.0 ksvideosrc ! image/jpeg, weigt= , height= , ! jpegdec ! videoconvert ! videorate ! video/x-raw, framerate=1/3 ! pngenc ! multifilesink location="frame-%d.png" 
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
	GstElement *source;//ksvideosrc
	GstElement *sink;//autovideosink->multifilesink
	GstElement *convert;//vieoconvert
	GstElement *filter;//image/jpg
	GstElement *jpegdec;
	GstElement *rate;//videorate
	GstElement *framerate;//video/x-raw
	GstElement *pngenc;

	GstElement *playbin;          /* Our one and only element */
	gboolean playing;             /* Are we in the PLAYING state? */
	gboolean terminate;           /* Should we terminate execution? */
	gboolean seek_enabled;        /* Is seeking enabled for this media? */
	gboolean seek_done;           /* Have we performed the seek already? */
	gint64 duration;              /* How long does this media last, in nanoseconds */
} CustomData;


CustomData data;
int idx;
int count;
static GMainLoop *loop;

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
	//data.sink=gst_element_factory_make("autovideosink","sink");
	data.filter = gst_element_factory_make ("capsfilter", "filter");
	data.jpegdec=gst_element_factory_make("jpegdec","dec");
	data.rate=gst_element_factory_make("videorate","rate");
	data.framerate=gst_element_factory_make("appsrc","framerate");
	data.sink=gst_element_factory_make("multifilesink","sink");
	data.pngenc=gst_element_factory_make("pngenc","pngenc");

  /* Create the empty pipeline */
	data.pipeline = gst_pipeline_new ("pipeline");
	//data.pipeline=gst_bin_new("bin");
	data.pipeline = gst_parse_launch("pipeline",NULL);

  if (!data.pipeline || !data.source || !data.convert || !data.sink || !data.jpegdec || !data.filter || !data.rate || !data.framerate ||!data.pngenc) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }


	gst_bin_add_many(GST_BIN(data.pipeline),data.source,data.filter,data.jpegdec,data.convert,data.rate,data.framerate,data.pngenc,data.sink,NULL);
	 if (!gst_element_link (data.convert, data.sink)) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }

	if(gst_element_link_many(data.source,data.filter,data.jpegdec,data.convert,data.rate,data.framerate,data.pngenc,data.sink,NULL)){
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

   g_object_set (G_OBJECT (data.framerate), "caps",
        gst_caps_new_simple ("video/x-raw",
                     "framerate", GST_TYPE_FRACTION, 3, 1,//이부분모르겠음 3분의 1로 어떻게함?
                     NULL), NULL);

   g_object_set(data.sink,"location","frame-%d.png",count++,NULL);//어떻게 저장함?

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
