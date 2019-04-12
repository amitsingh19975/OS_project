#include "utility.h"

// A library of utility functions.

// This function prints Errors in format Error: message, text colored in red
void eprint(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char err_msg[100] = {0};
	sprintf(err_msg, "\033[0;31mError: %s \033[0m\n", fmt);
	vprintf(err_msg, args);
	va_end(args);
}

// This function prints Warnings, text colored in yellow
void wprint(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char w_msg[100] = {0};
	sprintf(w_msg, "\033[0;33mWarning: %s \033[0m\n", fmt);
	vprintf(w_msg, args);
	va_end(args);
}

// This function prints success messages, text colored in green
void sprint(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char s_msg[100] = {0};
	sprintf(s_msg, "\033[0;32mSuccess: %s \033[0m\n", fmt);
	vprintf(s_msg, args);
	va_end(args);
}

// Trim input string and returns the string
char *trim(char *str)
{
	char *end;
	// Increment str to first no space character
	while (isspace((unsigned char)*str))
		str++;

	if (*str == 0)
		return str;

	end = str + strlen(str) - 1;
	// Decrement end to last no space character
	while (end > str && isspace((unsigned char)*end))
		end--;

	end[1] = '\0';

	return str;
}
