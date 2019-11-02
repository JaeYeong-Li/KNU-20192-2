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


static GMainLoop *loop;

static gboolean
	my_bus_callback (GstBus * bus, GstMessage * message, gpointer data)
{
	g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));

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

typedef struct _CustomData
{
	//GstElement *pipeline;
	//GstElement *source;
	//GstElement *convert;
	//GstElement *sink;
	GstElement *playbin;          /* Our one and only element */
	gboolean playing;             /* Are we in the PLAYING state? */
	gboolean terminate;           /* Should we terminate execution? */
	gboolean seek_enabled;        /* Is seeking enabled for this media? */
	gboolean seek_done;           /* Have we performed the seek already? */
	gint64 duration;  
} CustomData;

/* Handler for the pad-added signal */
//static void pad_added_handler (GstElement * src, GstPad * pad,
//	CustomData * data);
static guint idx = 0;
static void handle_message (CustomData * data, GstMessage * msg);

int main (int argc, char *argv[])
{
	//  GstElement *pipeline;
	//gboolean terminate = FALSE;
	int n=0;
	CustomData data;
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;
	guint bus_watch_id;

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
	g_object_set (data.playbin, "uri","file:///D:/jongp1/video2.mp4",NULL);

	/* Start playing */
	ret = gst_element_set_state (data.playbin, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr ("Unable to set the pipeline to the playing state.\n");
		gst_object_unref (data.playbin);
		return -1;
	}

	/* Listen to the bus */
	bus = gst_element_get_bus (data.playbin);
	bus_watch_id = gst_bus_add_watch(bus,my_bus_callback,NULL);

	do {
		if (bus_watch_id==TRUE) {
			gint64 current = -1;

			/* Query the current position of the stream */
			if (!gst_element_query_position (data.playbin, GST_FORMAT_TIME,
				&current)) {
					g_printerr ("Could not query current position.\n");
			}

			/* If we didn't know it yet, query the stream duration */
			if (!GST_CLOCK_TIME_IS_VALID (data.duration)) {
				if (!gst_element_query_duration (data.playbin, GST_FORMAT_TIME,
					&data.duration)) {
						g_printerr ("Could not query current duration.\n");
				}
			}

			/* Print current position and total duration */
			g_print ("Position %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\r",
				GST_TIME_ARGS (current), GST_TIME_ARGS (data.duration));

			/* If seeking is enabled, we have not done it yet, and the time is right, seek */
			if (data.seek_enabled && !data.seek_done && current > n * GST_SECOND) {
				g_print ("\nReached +3s, performing seek...\n");
				n=n+3;

				if (data.seek_enabled) {
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
			}
		}

		else{
			g_print ("End-Of-Stream reached.\n");
			data.terminate = TRUE;
			break;
		}
	}while(!data.terminate);

	/* Free resources */
	gst_object_unref (bus);
	gst_element_set_state (data.playbin, GST_STATE_NULL);//
	gst_object_unref (data.playbin);
	return 0;
}

static void handle_message (CustomData * data, GstMessage * msg){
	GError *err;
	gchar *debug_info;

	switch (GST_MESSAGE_TYPE (msg)) {

	case GST_MESSAGE_ERROR:
		gst_message_parse_error (msg, &err, &debug_info);
		g_printerr ("Error received from element %s: %s\n",
			GST_OBJECT_NAME (msg->src), err->message);
		g_printerr ("Debugging information: %s\n",
			debug_info ? debug_info : "none");
		g_clear_error (&err);
		g_free (debug_info);
		data->terminate = TRUE;
		break;

	case GST_MESSAGE_EOS:
		g_print ("End-Of-Stream reached.\n");
		data->terminate = TRUE;
		break;

	case GST_MESSAGE_DURATION:
		/* The duration has changed, mark the current one as invalid */
		data->duration = GST_CLOCK_TIME_NONE;
		break;

	case GST_MESSAGE_STATE_CHANGED:{
		//이즈음에서 액션을 호출하도록 추가하기!
		GstState old_state, new_state, pending_state;
		gst_message_parse_state_changed (msg, &old_state, &new_state,
			&pending_state);
		if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data->playbin)) {
			g_print ("Pipeline state changed from %s to %s:\n",
				gst_element_state_get_name (old_state),
				gst_element_state_get_name (new_state));

			//tutorial-4 time seek 하는 부분
			//playing  상태에 들어가는 경우
			data->playing = (new_state == GST_STATE_PLAYING);

			if (data->playing) {
				/* We just moved to PLAYING. Check if seeking is possible */
				GstQuery *query;
				gint64 start, end;

				query = gst_query_new_seeking (GST_FORMAT_TIME);

				if (gst_element_query (data->playbin, query)) {
					gst_query_parse_seeking (query, NULL, &data->seek_enabled, &start,&end);

					if (data->seek_enabled) {
						g_print ("Seeking is ENABLED from %" GST_TIME_FORMAT " to %"
							GST_TIME_FORMAT "\n", GST_TIME_ARGS (start),
							GST_TIME_ARGS (end));
					} 
					else {
						g_print ("Seeking is DISABLED for this stream.\n");
					}
				}

				else {
					g_printerr ("Seeking query failed.");
				}
				gst_query_unref (query);
			}
		}
								   }break;
	default:
		/* We should not reach here */
		g_print ("Unexpected message received.\n");
		break;
	}
	gst_message_unref (msg);
}
