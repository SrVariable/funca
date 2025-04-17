#include <stdio.h>
#include <sys/types.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

// From https://gist.github.com/rexim/b5b0c38f53157037923e7cdd77ce685d
// NOTE(srvariable): I might replace this because I can't control the realloc,
// however it is unlikely to fail so I don't care at the moment
#define da_append(xs, x) do {\
	if ((xs)->count >= (xs)->capacity) {\
		if ((xs)->capacity == 0) (xs)->capacity = 256;\
		else (xs)->capacity *= 2;\
		(xs)->items = realloc((xs)->items, (xs)->capacity*sizeof(*(xs)->items));\
	}\
	(xs)->items[(xs)->count++] = (x);\
} while (0);

typedef struct StringBuffer
{
	char	chars[1024];
}				StringBuffer;

typedef struct Array
{
	StringBuffer	*items;
	size_t	count;
	size_t	capacity;
}				Array;

typedef struct Signature
{
	char	**items;
	size_t	count;
	size_t	capacity;
}				Signature;

typedef enum inform_level
{
	NONE,
	DEBUG,
	INFO,
	ERROR,
}				InformLevel;

const InformLevel g_level = INFO;

void	inform(InformLevel level, char *message, ...)
{
	if (level > g_level)
	{
		return;
	}

	va_list ap;
	va_start(ap, message);
	FILE *f = stderr; // Set to stderr for convenience
	switch (level)
	{
		case DEBUG:
			fprintf(f, "DEBUG: ");
			break;
		case INFO:
			fprintf(f, "INFO: ");
			break;
		case ERROR:
			f = stderr;
			fprintf(f, "ERROR: ");
			break;
		default:
			f = stderr;
			fprintf(f, "???");
			break;
	}
	vfprintf(f, message, ap);
	fprintf(f, "\n");
	va_end(ap);
}

StringBuffer	create_new_path(char *path, char *name)
{
	StringBuffer new_path = {0};

	size_t path_len = strlen(path);
	memcpy(new_path.chars, path, path_len);

	// To avoid copying `name` first time find_files is called
	if (name)
	{
		memcpy(new_path.chars + path_len, "/", 1);
		memcpy(new_path.chars + path_len + 1, name, strlen(name));
	}

	return (new_path);
}

// Find every kind of file, excluding directories, executables and symlinks
// TODO(srvariable): Implement the option to find only certain files.
// For example: `./funca *.c` would only list every file ended with `.c`, I
// don't know if I'll implement a regex system or it would be out of scope.
void	find_files(char *path, Array *dir_names, int depth)
{
	if (!path)
	{
		inform(ERROR, "Failed to open directory '%s'. Reason: Can't be null.", path);
		return;
	}

	DIR *dir = opendir(path);
	if (!dir)
	{
		// To append the path the first time `find_files` is called
		if (depth == 0 && access(path, X_OK) < 0)
		{
			da_append(dir_names, create_new_path(path, NULL));
		}
		if (errno != ENOTDIR)
		{
			inform(ERROR, "Failed to open directory '%s'. Reason: %s.", path, strerror(errno));
		}
		return;
	}

	struct dirent *dirent;
	while ((dirent = readdir(dir)) != NULL)
	{
		if (*dirent->d_name != '.')
		{
			StringBuffer new_path = create_new_path(path, dirent->d_name);
			if (dirent->d_type == DT_REG && access(new_path.chars, X_OK) < 0)
			{
				da_append(dir_names, new_path);
			}
			find_files(new_path.chars, dir_names, depth + 1);
		}
	}

	if (closedir(dir) < 0)
	{
		inform(ERROR, "Failed to close directory '%s'. Reason: %s.", path, strerror(errno));
	}

	return;
}

// NOTE: Remember to free the content
char	*read_entire_file(char *filename)
{
	FILE *f = fopen(filename, "r");
	if (!f)
	{
		inform(ERROR, "Failed to read the file '%s'. Reason: %s.", filename, strerror(errno));
		return (NULL);
	}
	if (fseek(f, 0, SEEK_END) < 0)
	{
		inform(ERROR, "Failed to set the cursor to the end of the file '%s'. Reason: %s.", filename, strerror(errno));
		fclose(f);
		return (NULL);
	}
	long file_size = ftell(f);
	rewind(f);

	char *content = calloc(file_size + 1, sizeof(*content));
	if (!content)
	{
		inform(ERROR, "Failed to allocate memory for the content of the file '%s'. Reason: %s.", filename, strerror(errno));
		fclose(f);
		return (NULL);
	}
	if (fread(content, file_size, sizeof(*content), f) != 1)
	{
		inform(ERROR, "Failed to read file '%s'. Reason: %s.", filename, strerror(errno));
		free(content);
		fclose(f);
		return (NULL);
	}
	fclose(f);

	return (content);
}

bool	is_keyword(char *line)
{
	return (strncmp(line, "if", 2) == 0
			|| strncmp(line, "for", 3) == 0
			|| strncmp(line, "try", 3) == 0
			|| strncmp(line, "while", 5) == 0
			|| strncmp(line, "catch", 5) == 0
			|| strncmp(line, "switch", 6) == 0
			|| strncmp(line, "finally", 7) == 0
			);
}

// THINKING: So basically to get the signature, in C for example is when you find the
// following sequence:
//
// - ]) {
//
// - )\n{
//
// In some languages like python could be as easy as seaching for:
//
// - def .* :
//
// Other languages like JavaScript or Go follow the same convention as C, but could
// be also found by searching for
//
// - func
// - function
//
// Right now I'm focusing on C, but I might expand later
// I've realized that it's not that easy, because there can be this edge-case:
//
// (Vector2){.x = 5, .y = 2}
//
// So this will be a little more difficult than I expected, but not that much I hope
void	get_signatures(Signature *signatures, char *filename)
{
	char *content = read_entire_file(filename);
	if (!content)
	{
		return;
	}
	size_t content_length = strlen(content);
	size_t start = 0;
	size_t parenthesis_counter = 0;
	size_t end = 0;
	bool is_single_line_comment = false;
	bool is_multi_line_comment = false;
	for (size_t i = 0; i < content_length; ++i)
	{
		// Skip spaces
		while (i < content_length && isblank(content[i]))
		{
			++i;
		}

		bool parenthesis_found = false;
		// Skip lines, unless a there are parenthesis opened
		while (i < content_length && (content[i] != '\n' || parenthesis_counter > 0))
		{

			// Check if it's inside a comment
			if (i + 1 < content_length && content[i] == '/' && content[i + 1] == '/')
			{
				is_single_line_comment = true;
			}
			else if (i + 1 < content_length && content[i + 1] == '\n')
			{
				is_single_line_comment = false;
			}
			else if (i + 1 < content_length && content[i] == '/' && content[i + 1] == '*')
			{
				is_multi_line_comment = true;
			}
			else if (i + 1 < content_length && content[i] == '*' && content[i + 1] == '/')
			{
				is_multi_line_comment = false;
			}

			// Check if a parenthesis was found
			if (!is_single_line_comment && !is_multi_line_comment && content[i] == '(')
			{
				parenthesis_found = true;
				++parenthesis_counter;
			}
			if (!is_single_line_comment && !is_multi_line_comment && content[i] == ')')
			{
				--parenthesis_counter;
			}
			end = i + 1;
			++i;
		}

		// If the character after the new line is a '{'
		if (i + 1 < content_length && content[i + 1] == '{')
		{
			// And has found a parenthesis
			if (parenthesis_found && end > start)
			{
				da_append(signatures, strndup(&content[start], end - start));
			}
		}

		// Skip spaces from the start
		while (start < content_length && isblank(content[start]))
		{
			++start;
		}

		// If the character before the new line is a '{'
		if (i > 0 && content[i - 1] == '{'
			&& !is_keyword(&content[start]))
		{
			// Set the end to the last parenthesis
			while (end > 0 && content[end - 1] != ')')
			{
				--end;
			}
			// And has found a parenthesis
			if (parenthesis_found && end > start)
			{
				da_append(signatures, strndup(&content[start], end - start));
			}
		}
		start = i + 1;
	}
	if (parenthesis_counter > 0)
	{
		inform(ERROR, "Couldn't parse '%s' correctly. Reason: You probably left parenthesis opened.", filename);
	}
	free(content);
}

// NOTE(srvariable): I've just noticed that my find_files implementation is like find, but faster
// in combination with read_entire_file it's like a pretty fast find + cat
int	main(int argc, char **argv)
{
	// TODO(srvariable): Implement .ignore
	// TODO(srvariable): Implement flags
	Array dir_names = {0};
	if (argc < 2)
	{
		find_files(".", &dir_names, 0);
	}
	for (int i = 1; i < argc; ++i)
	{
		find_files(argv[i], &dir_names, 0);
	}

	Signature signatures = {0};
	size_t j = 0;
	for (size_t i = 0; i < dir_names.count; ++i)
	{
		char *filename = dir_names.items[i].chars;
		get_signatures(&signatures, filename);

		// Print only files with functions
		if (j != signatures.count)
		{
			printf("%s\n", filename);
			for (; j < signatures.count; ++j)
			{
				printf(" | - %s\n", signatures.items[j]);
			}
		}
	}
	free(dir_names.items);
	for (size_t i = 0; i < signatures.count; ++i)
	{
		free(signatures.items[i]);
	}
	free(signatures.items);

	return (0);
}
