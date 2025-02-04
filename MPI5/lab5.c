﻿#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <stddef.h>
#include <pthread.h>
#include <unistd.h>

//Сижу я долго за компом, что затекла нога
//И нету мысли уж давно и слова в тоске опять
//Иванишкин говорит, что с кодом всё не то
//Какой же MPI шальной и всегда с ним всё не то..

//Не прессуй меня MPI, ой не прессуй...
//Без костыля и кода нееет..
//Пусть лажа в проде, её ты подебаж..
//Консоль наш крест и оберееег...

//Не прессуй меня MPI, ой не прессуй...
//Без костыля и кода нееет..
//Пусть лажа в проде, её ты подебажь..
//Расставлю я тебе обед..

//Снова костыли на коде, а за окном 4-ый час
//Сервер падает и стонет и VS горит опять..
//Логи наши в гигабайтах, а код всё равно горит.. 
//Прошу прости меня за все грехи мои..

//Не прессуй меня MPI, ой не прессуй...
//Без костыля и кода нееет..
//Пусть лажа в проде, её ты подебаж..
//Консоль наш крест и оберееег...

//Не прессуй меня MPI, ой не прессуй...
//Без костыля и кода нееет..
//Пусть лажа в проде, её ты подебаж..
//Расставлю я тебе обед..

#define FIRST_PROCESS 0
#define TASK_SIZE 100
#define MAX_PROCESSES_SIZE 2000
//#define BIG_BOY 9999999999999999
#define CYCLES 1
#define _TEST2

typedef long long ll;

typedef struct Task
{
	ll id;
	ll type;
	ll n;
} Task;

MPI_Datatype MPI_Task;

MPI_Status status;

Task TaskList_new[TASK_SIZE];
Task TaskList_ready[TASK_SIZE];
#ifdef _TEST1
ll key = 10000000;
ll offset = 12345679;
ll cycle = 100000000;
#endif

#ifdef _TEST2
ll key = 0;
ll offset = 1000000;
ll cycle = 1000000000;
#endif

int recvcounts[MAX_PROCESSES_SIZE];
int displs[MAX_PROCESSES_SIZE];
int reports[MAX_PROCESSES_SIZE] = { 1 };

int start = 0;
int recvcount = 0;
int how_much_time_until_six = 0;
int tasks_done = 0;

int process = 0;
int process_size = 0;

ll stats_operations = 0;
ll stats_without_operations = 0;
int pref[MAX_PROCESSES_SIZE];


int there_is_nothing_we_can_do = 0;							//если поток, обменивающийся сообщениями мёртв.
int communication_going_well = 0;							//если в балансировке больше нет смысла
int working = 0;

pthread_mutex_t work_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t comm_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t communication_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  signalToWorkForOurRodina = PTHREAD_COND_INITIALIZER;


void *FSB(void * arr2) {
	int recvMessage[2];
	int letter_from;
	int tasks_left;
	for (;;) {
		MPI_Recv(recvMessage, 2, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);		//Получаем запрос 
		letter_from = recvMessage[0];
		tasks_left = recvMessage[1];
		printf("Recv 1. proc: %d from: %d\n", process, letter_from);

		//Завершаем поток
		if (letter_from == -1) {
			break;
		}

		int flag = 0;
		int data_transmission_count = 0;
		int start_point = 0;
		pthread_mutex_lock(&work_mutex);
		if (((tasks_left == 0) || (how_much_time_until_six / tasks_left >= 2)) && (how_much_time_until_six > 3))					//Число задач в этом процессе в два и более раза превосходит число задач в текущем
		{
			data_transmission_count = (how_much_time_until_six - tasks_left) / 2;
			start_point = start;
			start += data_transmission_count;
			how_much_time_until_six = recvcount - start;
			flag = 0;
		}
		else if (how_much_time_until_six <= 3) {
			data_transmission_count = -2;
			flag = 2;
		}
		else {
			data_transmission_count = -1;
			flag = 1;
		}
		pthread_mutex_unlock(&work_mutex);

		if (flag == 0)					//Число задач в этом процессе в два и более раза превосходит число задач в текущем
		{
			printf("Send 3. proc: %d to: %d", process, letter_from);
			MPI_Send(&data_transmission_count, 1, MPI_INT, letter_from, 3, MPI_COMM_WORLD);
			MPI_Send((TaskList_ready + start_point), data_transmission_count, MPI_Task, letter_from, 4, MPI_COMM_WORLD);
		}
		else {
			MPI_Send(&data_transmission_count, 1, MPI_INT, letter_from, 3, MPI_COMM_WORLD);
		}

	}
	pthread_exit(NULL);
}

void *execute_task(void * arr2) {

	while (there_is_nothing_we_can_do != 2)
	{

		working = 1;
		if (there_is_nothing_we_can_do == 1) {
			there_is_nothing_we_can_do = 2;
		}

		while (start < recvcount)
		{
			pthread_mutex_lock(&work_mutex);
			++start;
			how_much_time_until_six = recvcount - start;
			pthread_mutex_unlock(&work_mutex);
			ll id = TaskList_ready[start - 1].id;
			ll n = TaskList_ready[start - 1].n;


			ll a = 0;
			for (ll i = 0; i < n; ++i) {
				a += 1;
			}
			stats_operations += a;
			++tasks_done;
			printf("proscess: %d, array[0]: %lld, array[1]: %lld, id: %lld, n: %lld, start: %d, recvcount: %d\n", process, id, n, id, a, start, recvcount);
		}

		working = 0;
		if (there_is_nothing_we_can_do == 0) {
			pthread_mutex_lock(&comm_mutex);
			pthread_cond_wait(&signalToWorkForOurRodina, &comm_mutex);
			pthread_mutex_unlock(&comm_mutex);
		}
	}

	pthread_exit(NULL);
}

void *send_report(void* arr2) {
	int data_transmission_count;
	for (;;)
	{
		//printf("Communicationmachines, son!\n");
		int percent = ((recvcount - start) * 100) / (TASK_SIZE / process_size);	//Высчитываем процент оставшихся в пуле задач
		int sendMessage[2];
		sendMessage[0] = process;
		if (percent < 20)
		{
			printf("What's up?\n");
			int ask_to_process = rand() % process_size;
			int flag = 1;
			for (int k = 0; k < process_size; ++k) {
				if (ask_to_process != process)
				{
					sendMessage[1] = how_much_time_until_six;
					MPI_Send(sendMessage, 2, MPI_INT, ask_to_process, 1, MPI_COMM_WORLD);
					printf("Send 1. proc: %d to: %d", process, ask_to_process);
					MPI_Recv(&data_transmission_count, 1, MPI_INT, ask_to_process, 3, MPI_COMM_WORLD, &status);
					printf("Recv 3, %d. proc: %d from: %d\n", data_transmission_count, process, ask_to_process);
					if (data_transmission_count > -1) {
						flag = 0;
						MPI_Recv((TaskList_ready + recvcount), data_transmission_count, MPI_Task, ask_to_process, 4, MPI_COMM_WORLD, &status);
						pthread_mutex_lock(&work_mutex);
						recvcount += data_transmission_count;
						how_much_time_until_six = recvcount - start;
						if (working == 0) {
							pthread_cond_signal(&signalToWorkForOurRodina);
						}
						pthread_mutex_unlock(&work_mutex);
						printf("Recv 4\n");
						break;
					}
					else if (data_transmission_count == -1) {
						flag = 0;
					}
				}


				ask_to_process = (ask_to_process + 1) % process_size;
			}

			if (flag) {
				printf("proc: %d\n", process);
				MPI_Barrier(MPI_COMM_WORLD);
				there_is_nothing_we_can_do = 1;
				int it_is_time_to_stop[2] = { -1, -1 };
				printf("CROOOONK!!\n");
				MPI_Send(it_is_time_to_stop, 2, MPI_INT, (process + 1) % process_size, 1, MPI_COMM_WORLD);
				MPI_Barrier(MPI_COMM_WORLD);
				printf("There is nothing we can do.\n");
				if (working == 0) {
					pthread_cond_signal(&signalToWorkForOurRodina);
				}
				break;
			}
			sleep(1);
		}

	}


	printf("End of sending.\n");
	pthread_exit(NULL);
}

int main(int argc, char ** argv)
{
	for (int i = 0; i < MAX_PROCESSES_SIZE; ++i) {
		reports[i] = 1;
	}

	int provided;
	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
	MPI_Comm_rank(MPI_COMM_WORLD, &process);				// Получаем номер конкретного процесса на котором запущена программа
	MPI_Comm_size(MPI_COMM_WORLD, &process_size);			// Получаем количество запущенных процессов
	if (process == FIRST_PROCESS) {
		printf("process_size: %d, %d\n", process_size, reports[100]);
	}


	int blocklengths[3] = { 1, 1, 1 };
	MPI_Datatype types[3] = { MPI_LONG_LONG, MPI_LONG_LONG, MPI_LONG_LONG };
	MPI_Aint offsets[3];

	offsets[0] = offsetof(Task, id);
	offsets[1] = offsetof(Task, type);
	offsets[2] = offsetof(Task, n);

	MPI_Type_create_struct(3, blocklengths, offsets, types, &MPI_Task);
	MPI_Type_commit(&MPI_Task);

	int rem, base;
	for (int o = 0; o < CYCLES; ++o) {
		start = 0;
		///////////////////Составляем список задач///////////////////
		if (process == FIRST_PROCESS) {
			for (int i = 0; i < TASK_SIZE; ++i) {
				TaskList_new[i].id = i;
				TaskList_new[i].type = 1;
				TaskList_new[i].n = key;
				key = (key + offset) % cycle;
			}

			// Расчет количества элементов для каждого процесса
			int base_count = TASK_SIZE / process_size;
			int remainder = TASK_SIZE % process_size;

			// Расчет смещений и количества элементов для каждого процесса
			for (int i = 0; i < process_size; i++) {
				recvcounts[i] = base_count + (i < remainder ? 1 : 0);
				displs[i] = (i > 0) ? (displs[i - 1] + recvcounts[i - 1]) : 0;
			}
		}
		int base_count = TASK_SIZE / process_size;
		int remainder = TASK_SIZE % process_size;
		recvcount = base_count + (process < remainder ? 1 : 0);
		MPI_Scatterv(TaskList_new, recvcounts, displs, MPI_Task, TaskList_ready, recvcount, MPI_Task, FIRST_PROCESS, MPI_COMM_WORLD);
		rem = TASK_SIZE % process_size;
		base = TASK_SIZE / process_size;
		stats_without_operations = ((TaskList_ready[0].n + TaskList_ready[base + rem - 1].n)*(base + rem)) / 2;
		pthread_t comrade_Malinovskii, comrade_Ivanishkin, comrade_Maliosi;
		int iret1, iret2, iret3;

		printf("process: %d, start: %d, recvcount: %d\n", process, start, recvcount);
		iret1 = pthread_create(&comrade_Maliosi, NULL, execute_task, NULL);
		if (iret1) {
			fprintf(stderr, "Ошибка 1 - pthread_create() return code: %d\n", iret1);
			exit(EXIT_FAILURE);
		}

		iret2 = pthread_create(&comrade_Malinovskii, NULL, send_report, NULL);
		if (iret2) {
			fprintf(stderr, "Ошибка 2 - pthread_create() return code: %d\n", iret2);
			exit(EXIT_FAILURE);
		}

		iret3 = pthread_create(&comrade_Ivanishkin, NULL, FSB, NULL);
		if (iret3) {
			fprintf(stderr, "Ошибка 3 - pthread_create() return code: %d\n", iret3);
			exit(EXIT_FAILURE);
		}

		void *result_Maliosi;
		pthread_join(comrade_Maliosi, &result_Maliosi);
		void *result_Malinovskii;
		pthread_join(comrade_Malinovskii, &result_Malinovskii);
		void *result_Ivanishkin;
		pthread_join(comrade_Ivanishkin, &result_Ivanishkin);
	}




	MPI_Barrier(MPI_COMM_WORLD);
	printf("EEEEEEE BOYYYYY! proc: %d, tasks_done: %d, operations_with_ballance: %lld, operations_without: %lld, %d, %d\n", process, tasks_done, stats_operations, stats_without_operations, base*process + (process < rem ? process : rem), base*(process + 1) + ((process + 1) < rem ? process : rem));
	int sum_of_tasks = 0;
	ll sum_of_operations = 0;
	ll sum_of_operations_without = 0;
	MPI_Reduce(&tasks_done, &sum_of_tasks, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	if (process == 0) {
		printf("TASKS DONE: %d/%d\n", sum_of_tasks, TASK_SIZE);

	}
	MPI_Finalize();
	return 0;
}




