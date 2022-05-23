
#ifndef __VIM_HostHardwareSummary_H
#define __VIM_HostHardwareSummary_H

struct HostHardwareSummary {
	int cpuMhz;
	char *cpuModel;
	long memorySize;
	char *model;
	short numCpuCores;
	short numCpuPkgs;
	short numCpuThreads;
	int numHBAs;
	int numNics;
	char *uuid;
	char *vendor;
};

#endif
