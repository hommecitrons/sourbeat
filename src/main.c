
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "soundcard.h"
#include "parse.h"

struct 
{
	char *filename;
	int stdout_output;
} args;

void help()
{
	fprintf(stderr, "Usage: [-h|-s] [FILE]\n");
	fprintf(stderr, "-s: Use stdout for output rather than /dev/dsp\n");
	fprintf(stderr, "-h: Display this message\n");
}

void parse_args(int argc, char **argv)
{
	args.filename = 0;
	args.stdout_output = 0;
	for (int i=1; i < argc; i++)
	{
		if (strcmp(argv[i], "-s") == 0)
			args.stdout_output = 1;
		else if (strcmp(argv[i], "-h") == 0)
			help();
		else if (argv[i][0] == '-')
			help();
		else
		{
			if (args.filename == 0)
				args.filename = argv[i];
			else
				help();
		}
	}
	if (args.filename == 0)
		args.filename = "/dev/stdin";
}

int main(int argc, char **argv)
{
	parse_args(argc, argv);
	FILE *outfile;
	FILE *infile;

	if (!args.stdout_output)
	{
		if (!(outfile = fopen("/dev/dsp", "w")))
		{
			perror("/dev/dsp");
			fprintf(stderr, "Audio output requires /dev/dsp.\n");
			fprintf(stderr, "If you use pulseaudio, you can use padsp\n");
			fprintf(stderr, "Alternatively, you can use -s to output to stdout\n");
			exit(1);
		}
	}
	else
		outfile = stdout;

	if (!(infile = fopen(args.filename, "r")))
	{
		perror(args.filename);
		exit(1);
	}

	struct program p = parse(infile);
	if (p.sample_rate != 8000)
	{
		if (args.stdout_output) 
			fprintf(stderr, "note: output has intended sample rate of %dHz\n", p.sample_rate);
		else
		{
			if (p.sample_rate < 8000 || p.sample_rate > 200000)
				fprintf(stderr, "warning: invalid sample rate: %d\n", p.sample_rate);
			else
			{
				if (ioctl(outfile, SNDCTL_DSP_SPEED, &p.sample_rate) != 0)
				{
					perror("error setting sample rate");
				}
			}
		}
	}

	if (p.contents == 0)
	{
		fprintf(stderr, "empty or unreadable file\n");
		exit(1);
	}

	for (literal t = 0; !error; t++)
	{
		putc(eval(p,t), outfile);
	}

	fclose(infile);
	fclose(outfile);
}
