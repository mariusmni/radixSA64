/*
 * runtimes.cpp
 *
 *  Created on: Oct 3, 2011
 *      Author: marius
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "radix.h"
#include <string>

//#define BENCHMARK
#define NREPEATS 5

uchar* readData(const char * const filename, unsigned long& n) {
    struct stat fileInfo;
    FILE *file;
    if (stat(filename, &fileInfo)) {
        printf("Unable to get stat of file %s \n", filename);
        return NULL;
    }
    n = fileInfo.st_size;
    uchar *result = new uchar[n];
    if (!(file = fopen(filename, "r"))) {
        printf("Unable to open file %s \n", filename);
        return NULL;
    }
    rewind(file);
    if (n > fread(result, sizeof(uchar), n, file)) {
        printf("Error reading file %s \n", filename);
        fclose(file);
        delete[] result;
        return NULL;
    }
    fclose(file);
    return result;
}

template<class unum>
void printIntData(unum *data, unum n, const char *outputFile) {
    if (access(outputFile, F_OK) != -1) {
        cout << "Output file " << outputFile << " already exists. Aborting!" << endl;
        return;
    }

    FILE *f = fopen(outputFile, "w");
    for (unum i = 0; i < n; ++i) {
        fprintf(f, "%ld ", (unsigned long) data[i]);
    }
    fprintf(f, "\n");
    fclose(f);
}

void printHelp() {
    cout << "Arguments: [-L <lmerLength>] inputFile outputFile" << endl;
    cout << "Where:" << endl;
    cout << "  -L: does L-mer sorting instead of suffix sorting; specify max value of L" << endl;
    cout << "  -w: force 64 bit indexes even for short strings (uses more memory)" << endl;
    cout << "  -h: print this help" << endl;
}

void parseIntOrExit(char option, char *argument, long int *result) {
    if (1 != sscanf(argument, "%ld", result)) {
        cerr << "Option -" << option << " requires integer argument" << endl;
        exit(1);
    }
}

void parseArgs(int argc, char **argv, long int &k, string& inputFile, string& outputFile,
        bool& use64Bit) {
    use64Bit = false;
    for (int c; (c = getopt(argc, argv, "L:wh")) != -1;) {
        switch (c) {
        case 'h':
            printHelp();
            exit(0);
        case 'w':
            use64Bit = true;
            break;
        case 'L':
            parseIntOrExit(c, optarg, &k);
            break;
        default:
            cerr << "Unknown option `-" << optopt << "' or missing required argument." << endl;
            exit(0);
        }
    }

    if (optind < argc)
        inputFile = string(argv[optind]);
    else {
        cerr << "No input file specified!" << endl;
        exit(1);
    }

    optind++;
    if (optind < argc)
        outputFile = string(argv[optind]);
    else {
        cerr << "No output file specified!" << endl;
        exit(1);
    }
}

#ifdef BENCHMARK
#include <ctime>
int compareWithBPR(int argc, char *argv[], int k);
#endif

template<class unum>
void runRadixSAAndPrint(uchar *ustr, unum n, unum k, string& outputFile) {
    clock_t startTime = clock();
    unum *radixSA = Radix<unum>(ustr, n, k).build();
    clock_t endTime = clock();
    float mseconds = clock_diff_to_msec(endTime - startTime);

    printf("RadixSA took [%.2fs]\n", mseconds / 1000.0);
    cout << "Writing suffix array to file " << outputFile << "..." << endl;
    printIntData(radixSA, n, outputFile.c_str());
    delete[] radixSA;
}

int main(int argc, char *argv[]) {
    {
        int i = 1;
        char *c = (char *) &i;
#ifdef LITTLE_ENDIAN_FLAG
        bool fail = (*c == 0);
#else
        bool fail = (*c == 1);
#endif
        if (fail) {
            printf(
                    "Failed to detect endianness! \nPlease manually undefine LITTLE_ENDIAN_FLAG in radix.h and recompile\n");
            exit(0);
        }
    }
#ifdef BENCHMARK
    {
        time_t rawtime;
        time(&rawtime);

        struct tm * timeinfo = localtime(&rawtime);

        char buffer[80];
        strftime(buffer, 80, "logs/%h-%d-%y-%H-%M-%S.log", timeinfo);

        freopen(buffer, "w", stdout);

        long k = 0;
        int c = getopt(argc, argv, "L:");
        if (c != -1) {
            parseIntOrExit(c, optarg, &k);
            if (k < 0)
                k = 0;
        }
        argv += optind;
        argc -= optind;

        compareWithBPR(argc, argv, k);
        return 0;
    }
#endif

    if (argc == 1) {
        printHelp();
        exit(0);
    }
    string inputFile;
    string outputFile;
    long k = 0;
    bool use64Bit;
    parseArgs(argc, argv, k, inputFile, outputFile, use64Bit);
    if (k < 0)
        k = 0;
    unsigned long n = 0;
    uchar *ustr = readData(inputFile.c_str(), n);
    if (ustr == NULL)
        return 0;

    if (ustr[n - 1] == 10) { // remove line feed
        ustr[--n] = 0;
    }

    cout << "Read " << n << " bytes from file " << inputFile << endl;
    cout << "Computing Suffix Array..." << endl;
    if (use64Bit || n >= (1L << 31)) {
        cout << "Using 64 bits per suffix array position" << endl;
        runRadixSAAndPrint<unsigned long long>(ustr, n, k, outputFile);
    } else {
        cout << "Using 32 bits per suffix array position" << endl;
        runRadixSAAndPrint<unsigned int>(ustr, n, k, outputFile);
    }
    return 0;
}

#ifdef BENCHMARK
extern "C" {
#include "kbs_Error.h"
#include "kbs_Math.h"
#include "kbs_String.h"
#include "kbs_SuffixArray.h"
//#include "kbs_SuffixArrayAnnotated.h"
#include "kbs_SuffixArrayChecker.h"
#include "kbs_SuffixArrayConstDStepAndPre.h"
#include "kbs_Types.h"
}
#endif

#ifdef BENCHMARK

template<class unum>
unum *findSA(Kbs_Ustring *ustr) {
    return Radix<unum>(ustr->str, ustr->strLength).build();
}

template<class unum>
void findSAAndDelete(Kbs_Ustring *ustr) {
    unum *sa = findSA<unum>(ustr);
    delete[] sa;
}

Kbs_SuffixArray *bprFindSA(Kbs_Ustring *ustr) {
    Kbs_Ulong q = 3;
    kbs_get_AlphabetForUstring(ustr);
    if (ustr == NULL) {
        KBS_ERROR(KBS_ERROR_NULLPOINTER);
        exit(1);
    }
    //		if (argc == 2) {
    if (ustr->alphabet->alphaSize <= 9) {
        q = 7;
    }
    if (9 < ustr->alphabet->alphaSize && ustr->alphabet->alphaSize <= 13) {
        q = 6;
    }
    if (13 < ustr->alphabet->alphaSize && ustr->alphabet->alphaSize <= 21) {
        q = 5;
    }
    if (13 < ustr->alphabet->alphaSize && ustr->alphabet->alphaSize <= 21) {
        q = 5;
    }
    if (21 < ustr->alphabet->alphaSize && ustr->alphabet->alphaSize <= 46) {
        q = 4;
    }
    if (46 < ustr->alphabet->alphaSize) {
        q = 3;
    }
    // implementation using direct pointers as bucket pointers
    return kbs_buildDstepUsePrePlusCopyFreqOrder_SuffixArray(ustr, q);
}

void bprFindSAAndDelete(Kbs_Ustring *ustr) {
    Kbs_SuffixArray *sa;
    sa = bprFindSA(ustr);
    kbs_delete_SA_WithoutString(sa);
}

template<class unum>
void compareAnswers(unsigned char* data, unum* mySA, unsigned long* bprSA, unum bases) {
    unum i;
    unum j = 0;
    unum m = bases;
    for (i = 0; i < bases; ++i) {
        if (mySA[i] != bprSA[i]) {
            if (mySA[i] < m)
                m = mySA[i], j = i;
        }
    }
    i = j;
    if (m < bases) {
        printf("NO GOOD! position %d suffix %d but bpr says %d\n", i, mySA[i], bprSA[i]);
        for (unum a = mySA[i], b = bprSA[i]; a < bases && b < bases; ++a, ++b) {
            if (data[a] != data[b]) {
                printf("Suffix at %d = %d ...\n", a, data[a]);
                printf("Suffix at %d = %d ...\n", b, data[b]);
                break;
            }
        }
//        for (int j = 0; j <= 40; ++j) {
//            printf("%d ", data[mySA[i] + j]);
//        }
//        printf("\n");
//        for (int j = 0; j <= 40; ++j) {
//            printf("%d ", data[bprSA[i] + j]);
//        }
//        printf("\n");
    } else {
        printf("OK\n");
    }

}

template<class unum>
void compareOne(Kbs_Ustring* ustr, bool timeOnlyMode, int repeats, char *fileName,
        double *runTime) {
    double seconds;
    unum *radixSA;
    Kbs_SuffixArray *sa;

    {
        clock_t startTime = clock();
        for (int k = 1; k < repeats; ++k) {
            findSAAndDelete<unum>(ustr);
        }
        if (timeOnlyMode) {
            findSAAndDelete<unum>(ustr);
        } else {
            radixSA = findSA<unum>(ustr);
        }
        clock_t endTime = clock();
        seconds = clock_diff_to_msec((endTime - startTime) / repeats);

        runTime[0] = seconds;
        printf("RadixSA \t %s \t %.3fs ", fileName, runTime[0] / 1000.0);
        cout << endl;

    }

//        if (false)
    {
        clock_t startTime = clock();
        for (int k = 1; k < repeats; ++k) {
            bprFindSAAndDelete(ustr);
        }
        if (timeOnlyMode) {
            bprFindSAAndDelete(ustr);
        } else {
            sa = bprFindSA(ustr);
        }
        clock_t endTime = clock();
        seconds = clock_diff_to_msec((endTime - startTime) / repeats);

        runTime[1] = seconds;
        printf("BPR \t %s \t %.3fs", fileName, runTime[1] / 1000.0);
        cout << endl;
    }

    if (!timeOnlyMode) {
        unum n = ustr->strLength;
        compareAnswers(ustr->str, radixSA, sa->posArray, n);
        kbs_delete_SA_IncludingString(sa);
        delete[] radixSA;
    }

}
int compareWithBPR(int nFiles, char *fileNames[], int k) {
    srand(123);
    Kbs_Ustring* ustr = NULL;

    bool timeOnlyMode = false;
    int repeats = NREPEATS;

    double *runTime[nFiles];
    for (int i = 0; i < nFiles; ++i) {
        char *fileName = fileNames[i];
        runTime[i] = new double[2];
        ustr = kbs_getUstring_FromFile(fileName);
//		if (ustr->str[ustr->strLength - 1] == 10) { // remove line feed
//			ustr->str[--ustr->strLength] = 0;
//		}
        if (!timeOnlyMode) {
            cout << "===============================================" << endl;
            cout << "string length " << ustr->strLength << " of file " << endl << fileName << endl;
        }

        if (ustr->strLength >= (1L << 32)) {
            compareOne<unsigned long>(ustr, timeOnlyMode, repeats, fileName, runTime[i]);
        } else {
            compareOne<unsigned int>(ustr, timeOnlyMode, repeats, fileName, runTime[i]);
        }
        cout << "===== Dataset ============= \t Radix ==== \t BPR ==== \t Ratio ==========="
                << endl;
        for (int j = 0; j <= i; ++j) {
            printf("%-25s \t %.2f \t %.2f \t %.2f\n", fileNames[j], runTime[j][0], runTime[j][1],
                    runTime[j][0] / runTime[j][1]);
        }

        cout << endl;

        //exit(0);
    }
    for (int i = 0; i < nFiles; ++i) {
        delete[] runTime[i];
    }
    return 0;
}
#endif
