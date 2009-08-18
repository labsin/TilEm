#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <tilem.h>

/* See the testemu2.c to know what's this file */


/* Memory management */

void tilem_free(void* p)
{
	g_free(p);
}

void* tilem_malloc(size_t s)
{
	return g_malloc(s);
}

void* tilem_realloc(void* p, size_t s)
{
	return g_realloc(p, s);
}

void* tilem_try_malloc(size_t s)
{
	return g_try_malloc(s);
}

void* tilem_malloc0(size_t s)
{
	return g_malloc0(s);
}

void* tilem_try_malloc0(size_t s)
{
	return g_try_malloc0(s);
}

void* tilem_malloc_atomic(size_t s)
{
	return g_malloc(s);
}

void* tilem_try_malloc_atomic(size_t s)
{
	return g_try_malloc(s);
}

/* Logging */

void tilem_message(TilemCalc* calc, const char* msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	fprintf(stderr, "x%c: ", calc->hw.model_id);
	vfprintf(stderr, msg, ap);
	fputc('\n', stderr);
	va_end(ap);
}

void tilem_warning(TilemCalc* calc, const char* msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	fprintf(stderr, "x%c: WARNING: ", calc->hw.model_id);
	vfprintf(stderr, msg, ap);
	fputc('\n', stderr);
	va_end(ap);
}

void tilem_internal(TilemCalc* calc, const char* msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	fprintf(stderr, "x%c: INTERNAL ERROR: ", calc->hw.model_id);
	vfprintf(stderr, msg, ap);
	fputc('\n', stderr);
	va_end(ap);
}
