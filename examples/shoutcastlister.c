/*
 *  Copyright (C) 2007 Guillaume Lecerf
 *   Example of shoutcast crawling and parsing with curl and expat
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
 * gcc shoutcastlister.c -Wall -lexpat -lcurl -g -o shoutcastlister
 */

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <expat.h>

#define GENRE_LIST "http://www.shoutcast.com/sbin/newxml.phtml"
#define PLAYLIST "http://www.shoutcast.com/sbin/newxml.phtml?genre=%s"

typedef struct curl_data_s {
  char *buffer;
  size_t size;
} curl_data_t;

static size_t
curl_http_info_get(void *ptr, size_t size, size_t nmemb, void *data) {
  size_t realsize = size * nmemb;
  curl_data_t *mem = (curl_data_t*)data;

  mem->buffer = realloc(mem->buffer, mem->size + realsize + 1);
  if (mem->buffer) {
    memcpy(&(mem->buffer[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->buffer[mem->size] = 0;
  }
  return realsize;
}

static curl_data_t
get_data(CURL *curl, char* url) {
  curl_data_t chunk;

  chunk.buffer = NULL; /* we expect realloc(NULL, size) to work */
  chunk.size = 0;      /* no data at this point */

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_http_info_get);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  curl_easy_perform (curl);

  return chunk;
}

static void XMLCALL
startElement(void *userData, const char *name, const char **attr) {
  int i;

  // <station name='name' ... id='id'...>
  if(!strncmp(name, "station", 7)) {
    for (i = 0; attr[i]; i += 2) {
      if(!strncmp(attr[i], "name", 4))
        printf("name='%s' ", attr[i+1]);
      if(!strncmp(attr[i], "id", 2))
        printf("id=%s\n", attr[i+1]);
    }
  }
}

static void XMLCALL
startElementGenre(void *userData, const char *name, const char **attr) {
  int i;

  // <genre name='name'...>
  if(!strncmp(name, "genre", 5)) {
    for (i = 0; attr[i]; i += 2) {
      if(!strncmp(attr[i], "name", 4)){
        printf("%s\n", attr[i+1]);
        break;
      }
    }
  }
}

static void
parse_data(curl_data_t chunk, void* start, void* end) {
  XML_Parser parser;
  int depth = 0, done;

  parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, &depth);
  XML_SetElementHandler(parser, start, end);
  do {
    if (XML_Parse(parser, chunk.buffer, chunk.size, done) == XML_STATUS_ERROR) {
      fprintf(stderr,
              "%s at line %d\n",
              XML_ErrorString(XML_GetErrorCode(parser)),
              XML_GetCurrentLineNumber(parser));
      return;
    }
  } while (!done);
}

int
is_valid_genre(char* genre) {
  return 1;
}

void
displayGenreList(CURL *curl) {
  curl_data_t chunk;

  chunk = get_data(curl, GENRE_LIST);
  parse_data(chunk, startElementGenre, NULL); // no need for an endElement callback here

  free(chunk.buffer);
}


void
displayStation(CURL *curl, char* genre) {
  curl_data_t chunk;
  char url[1024];

  sprintf(url, PLAYLIST, genre);
  chunk = get_data(curl, url);
  parse_data(chunk, startElement, NULL); // no need for an endElement callback here

  free(chunk.buffer);
}

int
main(int argc, char **argv) {
  CURL *curl;

  if (argc < 2) {
    printf ("usage: %s genre\n", argv[0]);
    printf ("Options :\n -list\tDisplay available genres\n");
    return -1;
  }

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();

  if(!strncmp(argv[1], "-list", 5)) {
    displayGenreList(curl);
  }
  else
    if(is_valid_genre(argv[1])) {
      displayStation(curl, argv[1]);
    }

  curl_easy_cleanup(curl);

  return 0;
}
