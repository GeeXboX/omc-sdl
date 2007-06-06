/*
 *  Copyright (C) 2007 Benjamin Zores
 *   Example of crawler that uses FFMPEG to retrieve movie info
 *    and dumps a thumbnail of some video frame to a PNG file.
 *
 *   This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* Compile with:
 * gcc crawler.c -Wall -lavformat -lavcodec -lswscale -lavutil -g -o crawler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <ffmpeg/swscale.h>
#include <ffmpeg/md5.h>

#define MAX_W 256
#define MAX_H 192

static void
media_save_frame_to_png (AVFrame *frame, char *filename,
                          int width, int height)
{
  AVCodecContext *ctx = NULL;
  AVCodec *oc;
  int out_size, outbuf_size;
  uint8_t *outbuf;
  char file[64];
  FILE *f;

  memset (file, '\0', 64);
  snprintf (file, 64, "%s.png", filename);
  f = fopen (file, "wb");
  if (!f)
    return;
  
  oc = avcodec_find_encoder (CODEC_ID_PNG);
  if (!oc)
  {
    fprintf (stderr, "codec not found\n");
    goto encoder_end;
  }

  ctx = avcodec_alloc_context ();
  ctx->width = width;
  ctx->height = height;
  ctx->pix_fmt = PIX_FMT_RGB24;

  if (avcodec_open (ctx, oc) < 0)
  {
    fprintf (stderr, "could not open codec\n");
    goto encoder_end;
  }
 
  outbuf_size = width * height * 3;
  outbuf = malloc (outbuf_size);

  out_size = avcodec_encode_video (ctx, outbuf, outbuf_size, frame);
  printf ("encoded frame (%s, size=%5d)\n", file, out_size);
  fwrite (outbuf, 1, out_size, f);

 encoder_end:
  fclose (f);

  if (outbuf)
    free (outbuf);

  if (ctx)
  {
    avcodec_close (ctx);
    av_free (ctx);
  }
}

static char *
media_get_md5sum (char *filename)
{
  unsigned char md5sum[16];
  char md5[33];
  int i;
  
  av_md5_sum (md5sum, (const uint8_t *) filename, strlen (filename));
  memset (md5, '\0', 33);
  
  for (i = 0; i < 16; i++)
  {
    char tmp[3];
    sprintf (tmp, "%02x", md5sum[i]);
    strcat (md5, tmp);
  }
  
  return strdup (md5);
}

static void
media_get_infos (AVFormatContext *ctx)
{
  if (av_find_stream_info (ctx) < 0)
  {
    printf ("can't find stream info\n");
    return;
  }

  dump_format (ctx, 0, NULL, 0);
}

static void
media_seek_stream (AVFormatContext *pFormatCtx, int vs)
{
  double pos;
  int64_t ts, seek_target;
    
  pos = av_gettime () / 1000000.0;
  pos += 180.0;
  ts = (int64_t) (pos * AV_TIME_BASE);

  seek_target = av_rescale_q (pos, AV_TIME_BASE_Q,
                              pFormatCtx->streams[vs]->time_base);
  
  if (av_seek_frame (pFormatCtx, vs, seek_target, 0) < 0)
    printf ("unable to seek\n");

}

static void
media_get_thumbnail (AVFormatContext *pFormatCtx, char *md5sum)
{
  AVCodecContext *pCodecCtx;
  AVCodec *ic;
  AVFrame *iFrame, *oFrame;
  AVPacket packet;

  int decoded = 0;

  int numBytes, dstW, dstH;
  uint8_t *buffer;
  float ratio;
  
  int vs = -1;
  int i;
  
  for (i = 0; i < pFormatCtx->nb_streams; i++)
    if (pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
    {
      vs = i;
      break;
    }

  if (vs == -1)
    return; /* Didn't find a video stream */

  /* look for movie decoder */
  pCodecCtx = pFormatCtx->streams[vs]->codec;
  ic = avcodec_find_decoder (pCodecCtx->codec_id);
  if (!ic)
  {
    fprintf (stderr, "Decoder not found !\n");
    return;
  }

  if (avcodec_open (pCodecCtx, ic) < 0)
    return;

  /* allocate input frame */
  iFrame = avcodec_alloc_frame ();

  /* calculate output frame size, according to input one */
  if (pCodecCtx->width >= pCodecCtx->height)
  {
    ratio = (float)pCodecCtx->width / (float)pCodecCtx->height;
    dstW = (pCodecCtx->width < MAX_W) ? pCodecCtx->width : MAX_W;
    dstH = (int) (dstW / ratio);
  }
  else
  {
    ratio = (float)pCodecCtx->height / (float)pCodecCtx->width;
    dstH = (pCodecCtx->height < MAX_H) ? pCodecCtx->height : MAX_H;
    dstW = (int) (dstH / ratio);
  }

  /* allocate output frame */
  oFrame = avcodec_alloc_frame ();
  numBytes = avpicture_get_size (PIX_FMT_RGB24, dstW, dstH);
  buffer = av_malloc (numBytes * sizeof (uint8_t));
  avpicture_fill ((AVPicture *) oFrame, buffer, PIX_FMT_RGB24, dstW, dstH);
  
  /* try to seek in file */
  media_seek_stream (pFormatCtx, vs);
  
  /* read video packets till we reach a video frame */
  while (av_read_frame (pFormatCtx, &packet) >= 0)
  {
    if (packet.stream_index != vs)
      continue;

    avcodec_decode_video (pCodecCtx, iFrame, &decoded,
                          packet.data, packet.size);
     
    if (decoded)
    {
      struct SwsContext *sws;
      
      sws = sws_getContext (pCodecCtx->width, pCodecCtx->height,
                            pCodecCtx->pix_fmt,
                            dstW, dstH, PIX_FMT_RGB24,
                            SWS_BICUBIC, NULL, NULL, NULL);

      if (!sws)
      {
        fprintf (stderr, "Cannot initialize the conversion context!\n");
        av_free_packet (&packet);
        goto decoder_end;
      }

      /* now convert input frame to a lower resolution output frame */
      sws_scale (sws, iFrame->data, iFrame->linesize,
                 0, pCodecCtx->height, oFrame->data, oFrame->linesize);

      /* save output frame to some PNG file */
      media_save_frame_to_png (oFrame, md5sum, dstW, dstH);
      av_free_packet (&packet);
      break;
    }
    av_free_packet (&packet);
  }

 decoder_end:
  av_free (buffer);
  av_free (iFrame);
  av_free (oFrame);
  avcodec_close (pCodecCtx);
}

int
main (int argc, char **argv)
{
  AVFormatContext *pFormatCtx;
  char *md5sum;
  
  if (argc < 1)
  {
    printf ("usage: %s media_filename\n", argv[0]);
    return -1;
  }

  av_register_all ();
  
  if (av_open_input_file (&pFormatCtx, argv[1], NULL, 0, NULL) != 0)
  {
    printf ("can't open file: %s\n", argv[1]);
    return -1;
  }

  media_get_infos (pFormatCtx);

  md5sum = media_get_md5sum (argv[1]);
  media_get_thumbnail (pFormatCtx, md5sum);
  
  av_close_input_file (pFormatCtx);
  free (md5sum);
  
  return 0;
}
