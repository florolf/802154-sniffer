#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

volatile bool running = true;
time_t starttime;

void quit_handler(int sig) {
	running = false;
}

int serial_open(const char *device) {
	int fd;

	fd = open(device, O_RDWR | O_NOCTTY);
	if(fd < 0) {
		perror("opening serial device failed");
		return -1;
	}

	struct termios attr;
	memset(&attr, 0, sizeof(attr));

	attr.c_cflag = CS8 | CREAD | CLOCAL;
	attr.c_lflag = ICANON;
	cfsetispeed(&attr, 38400);
	cfsetospeed(&attr, 38400);

	if(tcsetattr(fd, TCSANOW, &attr) < 0) {
		perror("setting serial attributes failed");
		return -1;
	}

	return fd;
}

void write_header(FILE *outfile) {
	uint32_t magic = 0xa1b2c3d4;
	fwrite(&magic, 4, 1, outfile);

	uint16_t major = 2;
	uint16_t minor = 4;
	fwrite(&major, 2, 1, outfile);
	fwrite(&minor, 2, 1, outfile);

	int32_t zone = 0;
	fwrite(&zone, 4, 1, outfile);

	uint32_t sigfigs = 0;
	fwrite(&sigfigs, 4, 1, outfile);

	uint32_t snaplen = 65535;
	fwrite(&snaplen, 4, 1, outfile);

	uint32_t network = 195; // 802.15.4 + FCS
	fwrite(&network, 4, 1, outfile);

	fflush(outfile);
}

uint8_t read_hex(const char *buf) {
	uint8_t out;

	out = (*buf >= 'a') ? (*buf - 'a' + 10) : (*buf - '0');

	out <<= 4;
	buf++;

	out |= (*buf >= 'a') ? (*buf - 'a' + 10) : (*buf - '0');

	return out;
}

void write_packet(FILE *outfile, const char *buf) {
	struct timeval now;
	gettimeofday(&now, NULL);

	uint32_t sec = now.tv_sec - starttime;
	fwrite(&sec, 4, 1, outfile);

	uint32_t usec = now.tv_usec;
	fwrite(&usec, 4, 1, outfile);

	buf += 2;

	uint32_t len = read_hex(buf);
	fwrite(&len, 4, 1, outfile);
	fwrite(&len, 4, 1, outfile);

	buf += 3;

	while(len--) {
		uint8_t byte = read_hex(buf);

		fputc(byte, outfile);

		buf += 2;
	}

	fflush(outfile);
}

int main(int argc, char **argv) {
	const char *device = "/dev/ttyUSB0";
	const char *outpath = "capture.pcap";
	int channel = 11;

	int opt;
	while ((opt = getopt(argc, argv, "d:o:c:h")) != -1) {
		switch(opt) {
			case 'd':
				device = optarg;
				break;
			case 'o':
				outpath = optarg;
				break;
			case 'c':
				channel = atoi(optarg);
				break;
			case 'h':
			default:
				fprintf(stderr, "usage: %s [-d device] [-o outfile] [-c channel]", argv[0]);
				break;
		}
	}

	int fd = serial_open(device);
	if(fd < 0)
		return EXIT_FAILURE;

	FILE *serial = fdopen(fd, "w+");
	if(!serial) {
		perror("fdopen failed");
		return EXIT_FAILURE;
	}

	FILE *outfile;
	if(*outpath == '-')
		outfile = stdout;
	else {
		outfile = fopen(outpath, "w");
		if(!outfile) {
			perror("opening output file failed");
			return EXIT_FAILURE;
		}
	}

	write_header(outfile);

	starttime = time(NULL);

	struct sigaction sa;
	sa.sa_handler = quit_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if(sigaction(SIGINT, &sa, NULL) < 0) {
		perror("sigaction failed");
		return EXIT_FAILURE;
	}

	fprintf(serial, "c%02x", channel); fflush(serial);

	char buf[1024];
	while(running && fgets(buf, 1024, serial)) {
		if(buf[0] == 'M')
			fprintf(stderr, "msg: %s\n", buf + 2);
		else if(buf[0] == 'D') {
			fputc('.', stderr); fflush(stderr);
			write_packet(outfile, buf + 2);
		} else {
			fprintf(stderr, "ERROR: unkown record type 0x%02X\n", buf[0]);
		}
	}

	return EXIT_SUCCESS;
}
