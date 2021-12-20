#include "myshell_parser.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

bool is_background(const char* command_line) {
	size_t index = strcspn(command_line, "&");
	return (index < strlen(command_line));
}

char* remove_ampersand(const char* command_line) {
	char* new_command = strdup(command_line);
	int j = 0;
	for (int i = 0; new_command[i]; i++)
		if (new_command[i] != '&')
			new_command[j++] = new_command[i];
	new_command[j] = '\0';
	return new_command;
}

struct command_node {
	char* data;
	struct command_node* next;
};

struct command_node* create_command_list(char* command_line) {
	struct command_node* head = NULL;
	struct command_node* ptr = NULL;
	head = (struct command_node*)malloc(sizeof(struct command_node));
	ptr = head;
	char* pch = strtok(command_line, "|");
	head->data = pch;
	while (pch != NULL) {
		pch = strtok(NULL, "|");
		head->next = NULL;
		head->next = (struct command_node*)malloc(sizeof(struct command_node));
		head->next->data = pch;
		head = head->next;
	}
	head -> next = NULL;
	return ptr;
}

void delete_list(struct command_node** head) {
	struct command_node* ptr = NULL;
	ptr = *head;
	while (ptr) {
		struct command_node* arrow = ptr;
		ptr = ptr->next;
		free(arrow);
	}
}

int get_indirect_index(char* c) {
	size_t x = strcspn(c, "<");
	if (x == strlen(c))
		return -1;
	int count = 1;
	for (int i = 0; c[i]; i++) {
		if (c[i] == ' ') {
			if (c[i-1] && c[i-1] != '<' && c[i-1] != '>')
				if (c[i+1] && c[i+1] != '<' && c[i+1] != '>')
					count++;
		} else if (c[i] == '>') {
			count++;
		} else if (c[i] == '<') {
			count++;
			break;
		}
	}
	return count;
}

int get_outdirect_index(char* c) {
	size_t x = strcspn(c, ">");
	if (x == strlen(c))
		return -1;
	int count = 1;
	for (int i = 0; c[i]; i++) {
		if (c[i] == ' ') {
			if (c[i-1] && c[i-1] != '>' && c[i-1] != '<')
				if (c[i+1] && c[i+1] != '>' && c[i+1] != '<')
					count++;
		} else if (c[i] == '<') {
			count++;
		} else if (c[i] == '>') {
			count++;
			break;
		}
	}
	return count;
}

struct pipeline_command* pipeline_command_build(char* input_string) {
	struct pipeline_command* a = NULL;
	a = (struct pipeline_command*)malloc(sizeof(struct pipeline_command));
	for (int i = 0; i < MAX_ARGV_LENGTH; i++)
		a->command_args[i] = NULL;
	a->redirect_in_path = NULL;
	a->redirect_out_path = NULL;
	a->next = NULL;
	
	int in_index = get_indirect_index(input_string);
	int out_index = get_outdirect_index(input_string);
	int current_index = 0;
	int ending_index = 0;

	char* pch = strtok(input_string, "<> ");
	current_index++;
	while (pch != NULL) {
		if (current_index == in_index)
			a->redirect_in_path = pch;
		else if (current_index == out_index)
			a->redirect_out_path = pch;
		else {
			a->command_args[ending_index] = pch;
			ending_index += 1;
		}
		pch = strtok(NULL, "<> ");
		current_index++;
	}
	a->command_args[ending_index] = NULL;
	return a;
}

struct pipeline* pipeline_build(const char* command_line) {
	struct pipeline* a = NULL;
	a = (struct pipeline*)malloc(sizeof(struct pipeline));
	a->is_background = is_background(command_line);
	a->commands = NULL;

	struct pipeline_command** tail = NULL;
	tail = &a->commands;
	char* no_ampersand_command = remove_ampersand(command_line);
	static char raw_command_line[MAX_LINE_LENGTH];

	strcpy(raw_command_line, no_ampersand_command);
	int j, k;
	for (j=k=0; raw_command_line[j]; j++)
		if (!isspace(raw_command_line[j]) || (j > 0 && !isspace(raw_command_line[j-1])))
			raw_command_line[k++] = raw_command_line[j];
	raw_command_line[k] = '\0';
	j = 0;
	k = 0;
	for (j=k=0; raw_command_line[j]; j++)
		if (raw_command_line[j] != '\n')
			raw_command_line[k++] = raw_command_line[j];
	raw_command_line[k] = '\0';
	j = 0;
	k = 0;
	for (j=k=0; raw_command_line[j]; j++)
		if (raw_command_line[j] != '\t')
			raw_command_line[k++] = raw_command_line[j];
	raw_command_line[k] = '\0';

	free(no_ampersand_command);
	struct command_node* c = NULL;
	c = create_command_list(raw_command_line);
	struct command_node* ptr = NULL;
	ptr = c;
	while (c && c->next) {
		*tail = pipeline_command_build(c->data);
		c = c->next;
		tail = &(*tail)->next;
	}

	delete_list(&ptr);
	return a;
}

void pipeline_free(struct pipeline* pipeline) {
	while(pipeline->commands) {
		struct pipeline_command* temp = pipeline->commands;
		pipeline->commands = pipeline->commands->next;
		free(temp);
	}
	free(pipeline);
}
