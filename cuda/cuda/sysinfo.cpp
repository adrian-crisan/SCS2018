#include <memory>
#include <iostream>

#include <cuda_runtime.h>
#include "helper_cuda.h"
#include <fstream>

#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <intrin.h>
#include <chrono>
#include "cuda.h"

class Info {
public:
	
	void getGpuInfo(std::ofstream& myfile);
	void getCPUInfo(std::ofstream& myfile);
	void getRAMInfo(std::ofstream& myfile);
	void getHDDInfo(std::ofstream& myfile);
};

class Benchmark {
public:
	float benchmarkCPU(std::ofstream& myfile);
	void primeSum();
	void allocateMemory();
	float benchmarkRAM(std::ofstream& myfile);
	float benchmarkGPU(std::ofstream& myfile);
};

void Info::getGpuInfo(std::ofstream& myfile) {

	int deviceCount = 0;
	cudaError_t error_id = cudaGetDeviceCount(&deviceCount);

	if (error_id != cudaSuccess)
	{
		std::cout << "cudaGetDeviceCount returned" << (int)error_id << cudaGetErrorString(error_id) << std::endl;
		
		std::cout << "Result = FAIL" << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << "GPU Information: \n" << std::endl;
	myfile << "GPU Information: \n" << "\n";

	int dev, driverVersion = 0, runtimeVersion = 0;
	float theoreticalBandwidth = 0.0f;

	for (dev = 0; dev < deviceCount; ++dev)
	{
		cudaSetDevice(dev);
		cudaDeviceProp deviceProp;
		cudaGetDeviceProperties(&deviceProp, dev);

		std::cout << "GPU: " << deviceProp.name << std::endl;
		myfile << "GPU: " << deviceProp.name << "\n";

		cudaDriverGetVersion(&driverVersion);
		cudaRuntimeGetVersion(&runtimeVersion);

		std::cout << "Total amount of global video memory: " << deviceProp.totalGlobalMem / 1048576.0f << " MB." << std::endl;
		myfile << "Total amount of global video memory: " << deviceProp.totalGlobalMem / 1048576.0f << " MB." << "\n";

		std::cout << deviceProp.multiProcessorCount << " multiprocessors, " << _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor)
			<< " CUDA Cores/MP, total CUDA Cores: " << _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor) * deviceProp.multiProcessorCount << std::endl;
		myfile << deviceProp.multiProcessorCount << " multiprocessors, " << _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor)
			<< " CUDA Cores/MP, total CUDA Cores: " << _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor) * deviceProp.multiProcessorCount << "\n";

		std::cout << "GPU Max Clock rate: " << deviceProp.clockRate * 1e-3f << " MHz (" << deviceProp.clockRate * 1e-6f << ") GHz" << std::endl;
		myfile << "GPU Max Clock rate: " << deviceProp.clockRate * 1e-3f << " MHz (" << deviceProp.clockRate * 1e-6f << ") GHz" << "\n";

		std::cout << "Memory Clock rate: " << deviceProp.memoryClockRate * 1e-3f << "MHz (" << deviceProp.memoryClockRate * 1e-6f << ") GHz" << std::endl;
		myfile << "Memory Clock rate: " << deviceProp.memoryClockRate * 1e-3f << "MHz (" << deviceProp.memoryClockRate * 1e-6f << ") GHz" << "\n";

		std::cout << "Memory Bus Width: " << deviceProp.memoryBusWidth << " bits" << std::endl;
		myfile << "Memory Bus Width: " << deviceProp.memoryBusWidth << " bits" << "\n";

		theoreticalBandwidth = deviceProp.memoryClockRate * 1e-3f * 1000000.0f * (deviceProp.memoryBusWidth / 8) * 2 / 1073741824.0f;

		if (deviceProp.l2CacheSize)
		{
			std::cout << "L2 Cache Size: " << deviceProp.l2CacheSize << " bytes (" << deviceProp.l2CacheSize / 1024.0f << ") MB" << std::endl;
			myfile << "L2 Cache Size: " << deviceProp.l2CacheSize << " bytes (" << deviceProp.l2CacheSize / 1024.0f << ") MB" << "\n";
		}
	}

	std::cout << "Theoretical GPU Bandwidth: " << theoreticalBandwidth << " GB/s\n" << std::endl;
	myfile << "Theoretical GPU Bandwidth: " << theoreticalBandwidth << " GB/s\n" << "\n";
	
}

void Info::getCPUInfo(std::ofstream& myfile) {

	std::cout << "" << std::endl;

	std::cout << "\nCPU Information: \n" << std::endl;
	myfile << "CPU Information: \n\n";

	int CPUInfo[4] = { -1 };
	unsigned nExIds, i = 0;
	char CPUBrandString[0x40];
	__cpuid(CPUInfo, 0x80000000);
	nExIds = CPUInfo[0];

	for (i = 0x80000000; i <= nExIds; ++i) {
		__cpuid(CPUInfo, i);
		if (i == 0x80000002) {
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		}
		else if (i == 0x80000003) {
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		}
		else if (i == 0x80000004) {
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
		}
	}

	std::cout << "CPU Type: " << CPUBrandString << std::endl;
	myfile << "CPU Type: " << CPUBrandString << "\n";

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	std::cout << "Number of Cores: " << sysInfo.dwNumberOfProcessors << std::endl;
	myfile << "Number of Cores: " << sysInfo.dwNumberOfProcessors << "\n";

	wchar_t Buffer[_MAX_PATH];
	DWORD BufSize = _MAX_PATH;
	DWORD dwMhz = _MAX_PATH;
	HKEY hKey;
	long lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
		0, KEY_READ, &hKey);
	RegQueryValueEx(hKey, "~Mhz", NULL, NULL, (LPBYTE)&dwMhz, &BufSize);
	std::cout << "CPU Frequency: " << (double)dwMhz << " Mhz, " << (double)dwMhz / 1024.0f << " Ghz\n" << std::endl;
	myfile << "CPU Frequency: " << (double)dwMhz << " Mhz, " << (double)dwMhz / 1024.0f << " Ghz\n";
}

void Info::getRAMInfo(std::ofstream& myfile) {
	std::cout << "" << std::endl;
	std::cout << "\nRAM Memory Information: \n" << std::endl;
	myfile << "RAM Memory Information: \n\n";

	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	std::cout << "System RAM: " << (statex.ullTotalPhys / 1024) / 1024 << " MB" << std::endl;
	myfile << "System RAM: " << (statex.ullTotalPhys / 1024) / 1024 << " MB" << "\n\n";
}

void Info::getHDDInfo(std::ofstream& myfile) {
	std::cout << "\nExternal Memory/HDD Information: \n" << std::endl;
	myfile << "External Memory/HDD Information: \n\n";

	__int64 total, free, totalS = 0;

	std::cout << "Drive C:\\ " << std::endl;
	myfile << "Drive C:\\ " << "\n";

	if (GetDriveType("C:\\") != DRIVE_FIXED) {
		std::cout << "Error: not a fixed drive. " << std::endl;
	}
	else {
		GetDiskFreeSpaceEx("C:\\", NULL, (PULARGE_INTEGER)&total, (PULARGE_INTEGER)&free);
		totalS = totalS + total;
		std::cout << "Total space: " << total / 1048576.0f << " MB, " << total / 1073741824.0f << " GB" << std::endl;
		myfile << "Total space: " << total / 1048576.0f << " MB, " << total / 1073741824.0f << " GB" << "\n";
		std::cout << "Free space: " << free / 1048576.0f << " MB, " << free / 1073741824.0f << " GB" << std::endl;
		myfile << "Free space: " << free / 1048576.0f << " MB, " << free / 1073741824.0f << " GB" << "\n";
	}

	std::cout << "Drive D:\\ " << std::endl;
	myfile << "Drive D:\\ " << "\n";

	if (GetDriveType("D:\\") != DRIVE_FIXED) {
		std::cout << "Error: not a fixed drive. " << std::endl;
	}
	else {
		GetDiskFreeSpaceEx("D:\\", NULL, (PULARGE_INTEGER)&total, (PULARGE_INTEGER)&free);
		totalS = totalS + total;
		std::cout << "Total space: " << total / 1048576.0f << " MB, " << total / 1073741824.0f << " GB" << std::endl;
		myfile << "Total space: " << total / 1048576.0f << " MB, " << total / 1073741824.0f << " GB" << "\n";
		std::cout << "Free space: " << free / 1048576.0f << " MB, " << free / 1073741824.0f << " GB" << std::endl;
		myfile << "Free space: " << free / 1048576.0f << " MB, " << free / 1073741824.0f << " GB" << "\n";
	}

	std::cout << "Drive E:\\ " << std::endl;
	myfile << "Drive E:\\ " << "\n";

	if (GetDriveType("E:\\") != DRIVE_FIXED) {
		std::cout << "Error: not a fixed drive. " << std::endl;
	}
	else {
		GetDiskFreeSpaceEx("E:\\", NULL, (PULARGE_INTEGER)&total, (PULARGE_INTEGER)&free);
		totalS = totalS + total;
		std::cout << "Total space: " << total / 1048576.0f << " MB, " << total / 1073741824.0f << " GB" << std::endl;
		myfile << "Total space: " << total / 1048576.0f << " MB, " << total / 1073741824.0f << " GB" << "\n";
		std::cout << "Free space: " << free / 1048576.0f << " MB, " << free / 1073741824.0f << " GB" << std::endl;
		myfile << "Free space: " << free / 1048576.0f << " MB, " << free / 1073741824.0f << " GB" << "\n";
	}

	std::cout << "Drive F:\\ " << std::endl;
	myfile << "Drive F:\\ " << "\n";

	if (GetDriveType("F:\\") != DRIVE_FIXED) {
		std::cout << "Error: not a fixed drive. " << std::endl;
	}
	else {
		GetDiskFreeSpaceEx("F:\\", NULL, (PULARGE_INTEGER)&total, (PULARGE_INTEGER)&free);
		totalS = totalS + total;
		std::cout << "Total space: " << total / 1048576.0f << " MB, " << total / 1073741824.0f << " GB" << std::endl;
		myfile << "Total space: " << total / 1048576.0f << " MB, " << total / 1073741824.0f << " GB" << "\n";
		std::cout << "Free space: " << free / 1048576.0f << " MB, " << free / 1073741824.0f << " GB" << std::endl;
		myfile << "Free space: " << free / 1048576.0f << " MB, " << free / 1073741824.0f << " GB" << "\n";
	}

	std::cout << "Total amount of external memory/HDD size: " << totalS / 1073741824.0f << " GB\n" << std::endl;
	myfile << "Total amount of external memory/HDD size: " << totalS / 1073741824.0f << " GB" << "\n\n";
}

float Benchmark::benchmarkCPU(std::ofstream& myfile) {
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	primeSum();
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "CPU Benchmark time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " milliseconds, " <<
		(float)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000 << " seconds." << std::endl;
	myfile << "CPU Benchmark time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " milliseconds, " <<
		(float)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000 << " seconds." << "\n";

	float score = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	score *= 10;
	std::cout << "CPU Benchmark final score: " << score << std::endl;
	myfile << "CPU Benchmark final score: " << score << "\n\n";
	return score;
}

void Benchmark::primeSum() {
	double sum = 0;
	bool prime;
	for (int i = 2; i <= 1024 * 256; i++)
	{
		prime = true;
		for (int j = 2; j < i; j++)
		{
			if (i % j == 0)
			{
				prime = false;
				break;
			}
		}
		if (prime)
		{
			sum += i;
		}
	}
}

void Benchmark::allocateMemory() {

	float *arr = (float*)malloc(100000000* sizeof(float));
	for (int i = 0; i < 100000000; i++) {
		arr[i] = i;
	}
	for (int i = 0; i < 100000000; i++) {
		float val = arr[i];
	}
}

float Benchmark::benchmarkRAM(std::ofstream& myfile) {

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	allocateMemory();
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "\nRAM Benchmark time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " milliseconds, " <<
		(float)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000 << " seconds." << std::endl;
	myfile << "RAM Benchmark time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " milliseconds, " <<
		(float)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000 << " seconds." << "\n";

	float score = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	score *= 100;
	std::cout << "RAM Benchmark final score: " << score << std::endl;
	myfile << "RAM Benchmark final score: " << score << "\n\n";
	return score;
}

float Benchmark::benchmarkGPU(std::ofstream& myfile) {

	float bandwidth = myFunc();
	std::cout << "\nEffective GPU Bandwidth: " << bandwidth << " GB/s\n" << std::endl;
	myfile << "\nEffective GPU Bandwidth: " << bandwidth << " GB/s" << "\n\n";
	return bandwidth;
}

int main()
{
	std::ofstream myfile;
	myfile.open("result.txt");

	Info info;
	info.getGpuInfo(myfile);
	info.getCPUInfo(myfile);
	info.getRAMInfo(myfile);
	info.getHDDInfo(myfile);

	Benchmark benchmark;
	float cpuScore = benchmark.benchmarkCPU(myfile);
	float ramScore = benchmark.benchmarkRAM(myfile);
	float gpuScore = benchmark.benchmarkGPU(myfile);
	float finalScore1 = (cpuScore + ramScore) / 2.0f;
	float finalScore = ((gpuScore * 10000) + finalScore1) / 2.0f;
	std::cout << "Final benchmark score of the system: " << finalScore << "\n";
	myfile << "Final benchmark score of the system: " << finalScore << "\n";

	myfile.close();
	exit(0);
	return 0;
}
