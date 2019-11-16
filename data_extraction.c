#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define happy 0
#define disgusted 1
#define calm 2
#define sad 3
#define fear 4
#define confused 5
#define surprised 6
#define angry 7

double  emotion[8] = { 0 };


void init() {
	int i;
	for (i = 0; i < 8; i++) {
		emotion[i] = 0;
	}
}
void insert(FILE *input, int a) {
	int length;
	char temp[30], *stop;

	fscanf(input, "%s", temp);
	fscanf(input, "%s", temp);
	length = strlen(temp);
	strncpy(temp, temp, length - 1);
	temp[length - 1] = 0;
	emotion[a] = strtod(temp,&stop);

}
int main() {
	FILE *input = fopen("input.txt", "rb");
	FILE *output = fopen("terminal.txt", "w");
	char temp[30];
	int count = 0, minute = 0, second=19, length=0, i, person=0;
	double e;

	fprintf(output, "happy\t\tdisgusted\tcalm\t\tsad\t\tfear\t\tconfused\tsurprised\tangry\n\n");

	while (!feof(input)) {
		fscanf(input, "%s", temp);
		if (strcmp("{'Type':", temp) == 0) {
			fscanf(input, "%s", temp);
			if(strcmp(temp, "'HAPPY',")==0){
				insert(input,happy);
			}
			else if (strcmp(temp, "'DISGUSTED',") == 0) {
				insert(input, disgusted);
			}
			else if (strcmp(temp, "'CALM',") == 0) {
				insert(input, calm);
			}
			else if (strcmp(temp, "'SAD',") == 0) {
				insert(input, sad);
			}
			else if (strcmp(temp, "'FEAR',") == 0) {
				insert(input, fear);
			}
			else if (strcmp(temp, "'CONFUSED',") == 0) {
				insert(input, confused);
			}
			else if (strcmp(temp, "'SURPRISED',") == 0) {
				insert(input, surprised);
			}
			else if (strcmp(temp, "'ANGRY',") == 0) {
				insert(input, angry);
			}
			count++;
		}
		if (count == 8) {
			for (i = 0; i < 8; i++) {
				fprintf(output, "%f\t", emotion[i]);
			}
			fprintf(output, "\n");
			init();
			count = 0;
			person++;
		}

		if (person == 8) {
			second += 10;
			if (second > 60) {
				minute++;
				second -= 60;
			}
			fprintf(output, "(%d:%d)\n\n", minute, second);
			person = 0;
		}
	}

	fclose(input);
	fclose(output);
}
