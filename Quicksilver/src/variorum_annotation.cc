#include "variorum_annotation.hh"
extern "C" {
#include <variorum.h>
}
#include <jansson.h>
#include <mpi.h>
#include <rankstr_mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define POWER_FILENAME_TEXT "power_data.csv"
#define ENERGY_FILENAME_TEXT "energy_data.csv"

double time_delay_due_to_variorum = 0;
double time_delay_due_to_file_write = 0;
int processor_rank = 0;
char host[1024];
char *energy_filename = NULL;
char *power_filename = NULL;
#ifdef HAVE_MPI
MPI_Comm split_comm;
int comm_size;
#endif

int parse_json(variorum_call_type_t type, char *json_data, double *value,
               uint64_t *timestamp) {
  *value = -1;
  *timestamp = 0;
  json_t *node_obj = NULL;
  json_t *power_obj = json_loads(json_data, JSON_DECODE_ANY, NULL);
  void *iter = json_object_iter(power_obj);
  while (iter) {
    node_obj = json_object_iter_value(iter);
    if (node_obj == NULL) {
      printf("JSON object not found");
      return -1;
    }
    iter = json_object_iter_next(power_obj, iter);
  }

  //  Extract node-levels value from node object
  *timestamp = json_integer_value(json_object_get(node_obj, "timestamp"));
  if (type == VARIORUM_POWER_CALL) {
    if (json_object_get(node_obj, "power_node_watts") != NULL) {
      *value = json_real_value(json_object_get(node_obj, "power_node_watts"));
    }
  } else if (type == VARIORUM_ENERGY_CALL) {
    if (json_object_get(node_obj, "energy_node_joules") != NULL) {
      *value = json_real_value(json_object_get(node_obj, "energy_node_joules"));
    }
  }
  json_decref(power_obj);
  return 0;
}
char *get_file_name(variorum_call_type_t type, const char *hostname) {
  const char *filetype = NULL;
  if (type == VARIORUM_POWER_CALL)
    filetype = POWER_FILENAME_TEXT;
  else
    filetype = ENERGY_FILENAME_TEXT;
  int new_string_length = strlen(hostname) + strlen(filetype) + 2;
  char *new_filename = (char *)malloc(new_string_length * sizeof(char));
  if (new_filename != NULL) {
    snprintf(new_filename, new_string_length, "%s_%s", hostname, filetype);
  }
  return new_filename;
}

void variorum_annotate_init() {

  gethostname(host, 1023);
#ifdef HAVE_MPI
  int rank, numprocs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  rankstr_mpi_comm_split(MPI_COMM_WORLD, host, rank, 1, 2, &split_comm);
  MPI_Comm_rank(split_comm, &processor_rank);
  MPI_Comm_size(split_comm, &comm_size);
#endif
  energy_filename = get_file_name(VARIORUM_ENERGY_CALL, host);
  power_filename = get_file_name(VARIORUM_POWER_CALL, host);
  if (energy_filename == NULL || power_filename == NULL) {
    return;
  }
}

void variorum_annotate_finalize() {

  if (processor_rank == 0) {
    printf("host:%s \n", host);
    printf("Time delay due to variroum %f \n", time_delay_due_to_variorum);
    printf("Time delay due to write_to_file %f \n",
           time_delay_due_to_file_write);
  }
  printf("Enery data written to: %s Power Data to: %s\n", energy_filename,
         power_filename);
  free(energy_filename);
  free(power_filename);
#ifdef HAVE_MPI
  MPI_Comm_free(&split_comm);
#endif
}

void write_to_file(const char *filename, const char *caller_file_name, int line,
                   const char *function, uint64_t timestamp, char *data,
                   variorum_call_type_t type) {
  if (filename == NULL || caller_file_name == NULL || function == NULL ||
      data == NULL) {
    fprintf(stderr, "invalid arguments\n");
    return;
  }
  static bool header_written = true;
  FILE *file;
  file = fopen(filename, "a");
  if (file == NULL) {
    fprintf(stderr, "Error opening file %s \n", filename);
    return;
  }
  if (header_written) {
    const char *name = NULL;
    if (type == VARIORUM_POWER_CALL)
      name = "Node_Power";
    else if (type == VARIORUM_ENERGY_CALL)
      name = "Node_Energy";

    fprintf(file, "Timestamp,File,line,caller_func_name,%s\n", name);
    header_written = false;
  }
  if (fprintf(file, "%ld, ", timestamp) < 0 ||
      fprintf(file, "%s, ", caller_file_name) < 0 ||
      fprintf(file, "%d, ", line) < 0 || fprintf(file, "%s,", function) < 0 ||
      fprintf(file, "%s\n", data) < 0)
    fprintf(stderr, "Error in writing information to file");
  if (fclose(file) != 0) {

    fprintf(stderr, "Error in closing file %s \n", filename);
  }
}

int variorum_annotate_get_power_info(const char *file, int line,
                                     const char *function_name,
                                     char **parsed_data, uint64_t *timestamp) {

  char *s = NULL;
  int ret;
  double *parsed_result = (double *)malloc(sizeof(double));
  if (parsed_result == NULL)
    return -1;
  ret = variorum_get_power_json(&s);
  if (ret != 0) {
    fprintf(stderr, "variorum:JSON get node power failed.\n");
    free(s);
    return -1;
  }
  parse_json(VARIORUM_POWER_CALL, s, parsed_result, timestamp);
  // Determine the required size of the buffer
  int size = snprintf(NULL, 0, "%f", *parsed_result);
  if (size < 0) {
    fprintf(stderr, "Error occurred while determining buffer size.\n");
    free(parsed_result);
    free(s);
    return -1;
  }
  *parsed_data = (char *)malloc(size + 1);
  if (*parsed_data == NULL) {
    fprintf(stderr, "Memory allocation failed.\n");
    free(parsed_result);
    free(s);
    return -1;
  }

  snprintf(*parsed_data, size + 1, "%f", *parsed_result);
  free(parsed_result);
  free(s);
  return 0;
}
int variorum_annotate_get_energy_info(const char *file, int line,
                                      const char *function_name,
                                      char **parsed_data, uint64_t *timestamp) {

  char *s = NULL;
  int ret;
  double *parsed_result = (double *)malloc(sizeof(double));
  if (parsed_result == NULL)
    return -1;
  ret = variorum_get_energy_json(&s);
  if (ret != 0) {
    fprintf(stderr, "variorum:JSON get node energy failed.\n");
    free(s);
    return -1;
  }
  parse_json(VARIORUM_ENERGY_CALL, s, parsed_result, timestamp);
  // Determine the required size of the buffer
  int size = snprintf(NULL, 0, "%f", *parsed_result);
  if (size < 0) {
    fprintf(stderr, "Error occurred while determining buffer size.\n");
    free(s);
    return -1;
  }
  *parsed_data = (char *)malloc(size + 1);
  if (*parsed_data == NULL) {
    fprintf(stderr, "Memory allocation failed.\n");
    free(s);
    return -1;
  }

  snprintf(*parsed_data, size + 1, "%f", *parsed_result);
  free(parsed_result);
  free(s);

  return 0;
}
void variorum_annotate_function_call(variorum_call_type_t type,
                                     const char *file, int line,
                                     const char *function_name) {
  int ret = 0;
  char *data_to_write = NULL;
  char *filename = NULL;
  uint64_t *timestamp = (uint64_t *)malloc(sizeof(uint64_t));
  struct timespec start, end;
  struct timespec start_1, end_1;
  if (processor_rank == 0) {
    clock_gettime(CLOCK_MONOTONIC, &start);
    switch (type) {
    case VARIORUM_POWER_CALL:
      if (variorum_annotate_get_power_info(file, line, function_name,
                                           &data_to_write, timestamp) < 0) {
        if (data_to_write)
          free(data_to_write);
        if (timestamp)
          free(timestamp);
        return;
      }
      filename = power_filename;
      break;
    case VARIORUM_ENERGY_CALL:
      if (variorum_annotate_get_energy_info(file, line, function_name,
                                            &data_to_write, timestamp) < 0) {
        if (data_to_write)
          free(data_to_write);
        if (timestamp)
          free(timestamp);
        printf("exiting\n");
        return;
      }
      filename = energy_filename;

      break;
    case VARIORUM_DOMAIN_CALL:
      break;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    // end=clock();
    time_delay_due_to_variorum +=
        ((end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1E9);
    clock_gettime(CLOCK_MONOTONIC, &start_1);

    write_to_file(filename, file, line, function_name, *timestamp,
                  data_to_write, type);
    // double end_1=MPI_Wtime();
    clock_gettime(CLOCK_MONOTONIC, &end_1);
    time_delay_due_to_file_write = ((end_1.tv_sec - start_1.tv_sec) +
                                    (end_1.tv_nsec - start_1.tv_nsec) / 1E9);
    // time_delay_due_to_file_write+=((double)end_1-start_1)/CLOCKS_PER_SEC;
    if (data_to_write)
      free(data_to_write);
    if (timestamp)
      free(timestamp);
  }
}
