/*
 * SortMeRNA - next-generation reads filter for metatranscriptomic or total RNA
 * Copyright (C) 2014 Bonsai Bioinformatics Research Group
 *
 * This file is part of SortMeRNA.
 *
 * SortMeRNA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SortMeRNA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 * file: main.cpp
 * contact: jenya.kopylov@gmail.com, laurent.noe@lifl.fr, helene.touzet@lifl.fr
 *
 */

#include <dirent.h>
#include <string.h>

#include "../include/paralleltraversal.hpp"
#include <limits>

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif


const char map_nt[122] = {
	/* 0,   1,   2,   3,   4,   5,   6,   7,   8,   9   */
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	/* 10,  11,  12,  13,  14,  15,  16,  17,  18,  19  */
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	/* 20,  21,  22,  23,  24,  25,  26,  27,  28,  29  */
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	/* 30,  31,  32,  33,  34,  35,  36,  37,  38,  39  */
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	/* 40,  41,  42,  43,  44,  45,  46,  47,  48,  49  */
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	/* 50,  51,  52,  53,  54,  55,  56,  57,  58,  59  */
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	/* 60,  61,  62,  63,  64,  65,  66,  67,  68,  69  */
		  0,   0,   0,   0,   0,   0,   1,   1,   1,   0,
	/* 70,  71,  72,  73,  74,  75,  76,  77,  78,  79  */
		  0,   2,   0,   0,   0,   2,   0,   0,   0,   0,
	/* 80,  81,  82,  83,  84,  85,  86,  87,  88,  89  */
		  0,   0,   0,   2,   3,   3,   0,   1,   2,   1,
	/* 90,  91,  92,  93,  94,  95,  96,  97,  98,  99  */
		  0,   0,   0,   0,   0,   0,   0,   0,   1,   1,
	/* 100, 101, 102, 103, 104, 105, 106, 107, 108, 109 */
		  0,   0,   0,   2,   0,   0,   0,   2,   0,   0,
	/* 110, 111, 112, 113, 114, 115, 116, 117, 118, 119 */
		  0,   0,   0,   0,   0,   2,   3,   3,   0,   1,
	/* 120, 121 */
		  2,   1};


using namespace std;

/// uncommented variables & functions are defined in {paralleltraversal, bitvector}.hpp
	timeval	t;
	double evalue = -1.0;
    double align_id = -1.0;
    double align_cov = -1.0;

/// defined in common.hpp
	bool forward_gv = false;
	bool reverse_gv = false;
	int	numcpu_gv = -1;
	bool verbose  = false;
	bool pairedin_gv = false;
	bool pairedout_gv = false;
    bool chimeraout_gv = false;
    bool logout_gv = false;
	long unsigned int pagesize_gv = sysconf(_SC_PAGE_SIZE);
	long unsigned int maxpages_gv = 0;
	long unsigned int map_size_gv = pagesize_gv;
	bool map_size_set_gv = false;
	bool samout_gv = false;
	bool blastout_gv = false;
	bool fastxout_gv = false;
#ifdef OTU_MAP
    bool otumapout_gv = false;
#endif
    /// by default best_gv is off (-1), a value of 0 means to search all high scoring reference sequences, a value > 0 means to search best_gv sequences
	int32_t best_gv = -1;
    /// by default num_alignments_gv is off (-1), a value of 0 means to output all alignments, a value > 0 means to output num_alignments_gv alignments
	int32_t num_alignments_gv = -1;
	int32_t seed_hits_gv = -1;
	int32_t edges_gv = -1;
	bool full_search_gv = false;
	char version_num[] = "1.99 beta, 11/03/2014"; /// change version number here
    bool feeling_lucky_gv = false;
	bool as_percent_gv = false;
    bool nomask_gv = false;
    bool pid_gv = false;



/*
 *
 * FUNCTION 	: welcome()
 * PARAMETERS	: none
 * PURPOSE	: program name, copyright information and contact
 *
 **************************************************************************************************************/
void welcome()
{
	eprintf("\n  Program:    SortMeRNA version %s\n",version_num );
	eprintf("  Copyright:  2012-2014 Bonsai Bioinformatics Research Group\n");
	eprintf("              LIFL, University Lille 1, CNRS UMR 8022, INRIA Nord-Europe\n" );
    eprintf("              SortMeRNA comes with ABSOLUTELY NO WARRANTY; without even the\n");
    eprintf("              implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
    eprintf("              See the GNU Lesser General Public License for more details.\n");
	eprintf("  Contact:    Evguenia Kopylova, jenya.kopylov@gmail.com \n");
	eprintf("              Laurent Noé, laurent.noe@lifl.fr\n");
	eprintf("              Hélène Touzet, helene.touzet@lifl.fr\n\n");
}



/*
 *
 * FUNCTION 	: printlist()
 * PARAMETERS	: none
 * PURPOSE	: print the options list of SortMeRNA
 *
 **************************************************************************************************************/
void printlist()
{
	printf("\n  usage:   ./sortmerna <input> <output> <options>:\n\n");
  printf("  -------------------------------------------------------------------------------------------------------------\n");
  printf("  | parameter          value           description                                                    default |\n");
  printf("  -------------------------------------------------------------------------------------------------------------\n");
	printf("   <input>: \n");
	printf("     %s--reads%s           %sSTRING%s          FASTA/FASTQ reads file                                         %smandatory%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[0;32m","\033[0m");
	printf("     %s--ref%s             %sSTRING,STRING%s   FASTA reference file, index file                               %smandatory%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[0;32m","\033[0m");
    printf("                                        (ex. --ref /path/to/file1.fasta,/path/to/index1)\n");
    printf("                                        If passing multiple reference files, separate \n");
    printf("                                        them using the delimiter ':',\n");
    printf("                                        (ex. --ref /path/to/file1.fasta,/path/to/index1:/path/to/file2.fasta,path/to/index2)\n\n");
	printf("   <output>:\n");
	printf("     %s--aligned%s         %sSTRING%s          aligned reads base file name\n","\033[1m","\033[0m","\033[4m","\033[0m");
  printf("                                         (appropriate extension will be added)\n");
	printf("     %s--other%s           %sSTRING%s          rejected reads base file name\n","\033[1m","\033[0m","\033[4m","\033[0m");
  printf("                                         (appropriate extension will be added)\n");
	printf("     %s--fastx%s           %sFLAG%s            output FASTA/FASTQ file                                        %soff%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
  printf("                                         (for aligned and/or rejected reads)\n");
  printf("     %s--sam%s             %sFLAG%s            output SAM alignment                                           %soff%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
  printf("                                         (for aligned reads only)\n");
    printf("     %s--SQ%s              %sFLAG%s            add SQ tags to the SAM file                                    %soff%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
  printf("     %s--blast%s           %sFLAG%s            output BLAST-like alignment                                    %soff%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
  printf("                                         (for aligned reads only)\n");
    printf("     %s--log%s             %sFLAG%s            output overall statistics                                      %soff%s\n\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
#ifdef OTU_MAP
    printf("     %s--otu_map%s         %sFLAG%s            output OTU map                                                 %soff%s\n\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
#endif
	printf("   For alignments (with --sam or --blast options):\n\n");
  printf("     %s--feeling-lucky%s   %sFLAG%s            report the first alignment per read reaching E-value           %soff%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
	printf("       %sor%s\n","\033[31m","\033[0m");
	printf("     %s--num_alignments%s  %sINT%s             report first INT alignments per read reaching E-value          %s-1%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
    printf("                                        (--num_alignments 0 signifies all alignments will be output)\n");
	printf("       %sor%s (default)\n","\033[31m","\033[0m");
	printf("     %s--best%s            %sINT%s             report single best alignment per read reaching E-value         %s2%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
  printf("                                         from alignments of INT best candidate reference sequences\n");
  printf("                                         (ex. --best 2: find all alignments for the first 2\n");
  printf("                                         best matching reference sequences and report the\n");
  printf("                                         the single best alignment; --best %s0%s signifies\n","\033[4m","\033[0m");
    printf("                                         all highest scoring reference sequences will be searched) \n\n");
	printf("   <options>:\n");
#ifdef NOMASK_option
    printf("     %s--no-mask%s         %sFLAG%s            do not mask low occurrence (L/2)-mers when searching           %son%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
    printf("                                       for seeds of length L\n");
#endif
  printf("     %s--paired_in%s       %sFLAG%s            both paired-end reads go in --aligned fasta/q file             %soff%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
    printf("                                         (interleaved reads only, see Section 4.2.4 of User Manual)\n");
  printf("     %s--paired_out%s      %sFLAG%s            both paired-end reads go in --other fasta/q file               %soff%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");\
    printf("                                         (interleaved reads only, see Section 4.2.4 of User Manual)\n");
	printf("     %s--match %s          %sINT%s             SW score (positive integer) for a match                        %s2%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
	printf("     %s--mismatch%s        %sINT%s             SW score (negative integer) for a mismatch                     %s-3%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
	printf("     %s--gap_open%s        %sINT%s             SW score (positive integer) for introducing a gap               %s5%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
	printf("     %s--gap_ext%s         %sINT%s             SW score (positive integer) for extending a gap                %s2%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
    printf("     %s-N%s                %sINT%s             SW score for ambiguous letters (N's)                           %sscored as --mismatch%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
	printf("     %s-F%s                %sFLAG%s            search only the forward strand                                 %soff%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
	printf("     %s-R%s                %sFLAG%s            search only the reverse-complementary strand                   %soff%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
	printf("     %s-a%s                %sINT%s             number of threads to use                                       %s1%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
	printf("     %s-e%s                %sDOUBLE%s          E-value                                                        %s1%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
#ifdef id_cov
    printf("     %s--id%s              %sDOUBLE%s          minimum %%id to keep an alignment (the alignment must           %s0%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
    printf("                                         still pass the E-value threshold)\n");
    printf("     %s--coverage%s        %sDOUBLE%s          minimum %%query coverage to keep an alignment (the              %s0%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
    printf("                                         alignment must still pass the E-value threshold)\n");
#endif
    /// RAM cannot support 1GB default
	if ( 1073741824/pagesize_gv > maxpages_gv/2)
  printf("     %s-m%s                %sINT%s             INT Mbytes for loading the reads into memory               %s%lu%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m",((pagesize_gv*(maxpages_gv/2))/1048576),"\033[0m");
    /// RAM can support at least 1GB default
	else
  printf("     %s-m%s                %sINT%s             INT Mbytes for loading the reads into memory                   %s1024%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
  printf("                                        (maximum -m INT is %lu)\n",(((maxpages_gv/2)*pagesize_gv)/1048576));
	printf("     %s-v%s                %sFLAG%s            verbose                                                        %soff%s\n\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
	printf("   advanced <options>: (see SortMeRNA user manual for more details) \n");
	printf("    %s--passes%s           %sSTRING%s          values for seed skip lengths for Pass 1, 2 and 3               %sL,L/2,3%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
	printf("                                         must be in the form '%sINT%s,%sINT%s,%sINT%s', respectively\n","\033[4m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
  printf("                                         (L is the seed length set in ./indexdb_rna)\n");
	printf("    %s--edges%s            %sINT%s             number (or percent if INT followed by %% sign) of               %s4%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
  printf("                                         nucleotides to add to each edge of the read\n");
  printf("                                         prior to SW local alignment \n");
  printf("    %s--num_seeds%s        %sINT%s             number of seeds matched before searching                       %s2%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
  printf("                                         for candidate LIS \n");
  printf("    %s--full_search%s      %sFLAG%s            search for all 0-error and 1-error seed                        %soff%s\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
  printf("                                         matches in the index rather than stopping\n");
  printf("                                         after finding a 0-error match (<1%% gain in\n");
	printf("                                         sensitivity with up four-fold decrease in speed)\n");
    printf("    %s--pid%s              %sFLAG%s            add pid to output file names                                   %soff%s\n\n","\033[1m","\033[0m","\033[4m","\033[0m","\033[4m","\033[0m");
	printf("   help:\n");
	printf("     %s-h%s                %sFLAG%s            help\n","\033[1m","\033[0m","\033[4m","\033[0m");
	printf("     %s--version%s         %sFLAG%s            SortMeRNA version number\n\n","\033[1m","\033[0m","\033[4m","\033[0m");
    printf("   examples:\n");
    printf("     (1) Only filter rRNA\n");
    printf("         sortmerna --ref ref.fasta,ref_index --reads reads.fastq --aligned reads_aligned --other reads_rejected --fastx\n\n");
    printf("     (2) Only filter rRNA against multiple databases\n");
    printf("         sortmerna --ref bac.fasta,bac_index:euk.fasta,euk_index --reads reads.fastq --aligned reads_aligned --other reads_rejected --fastx\n\n");
    printf("     (3) Filter rRNA, output first alignment reaching E-value threshold and log file\n");
    printf("         sortmerna --ref ref.fasta,ref_index --reads reads.fastq --aligned reads_aligned --fastx --sam --feeling-lucky --log \n\n");
    printf("     (4) Search 10 candidate reference sequences for each read and output the single best alignment\n");
    printf("         sortmerna --ref bac.fasta,bac_index --reads reads.fastq --aligned reads_aligned --sam --best 10\n\n");
    printf("     (5) Output the first 10 alignments reaching E-value threshold\n");
    printf("         sortmerna --ref euk.fasta,euk_index --reads reads.fastq --aligned reads_aligned --sam --num_alignments 10\n\n");
    printf("     For more examples and user instructions, see the SortMeRNA v1.99 beta User Manual 2014 (distributed with this program)\n\n");

	exit(EXIT_FAILURE);
}//~printlist()


/*
 *
 * FUNCTION 	: main
 * PARAMETERS	: 
 * INPUT	: 
 * OUTPUT	: N/A
 *
 **************************************************************************************************************/
int 
main(int argc, 
     char** argv)
{
	/// parse the command line input
	int narg = 1;
	/// reads input file
	char* readsfile  = NULL;
	/// aligned reads output file
	char* ptr_filetype_ar = NULL;	
	/// rejected reads output file
	char* ptr_filetype_or = NULL;
    /// SW alignment parameters
	int32_t match = 0;
	int32_t mismatch = 0;
	int32_t gap_open = 0;
	int32_t gap_extension = 0;
    int32_t score_N = 0;
	bool match_set = false;
	bool mismatch_set = false;
	bool gap_open_set = false;
	bool gap_ext_set = false;
    bool full_search_set = false;
    bool passes_set = false;
    bool edges_set = false;
    bool match_ambiguous_N_gv = false;
    bool yes_SQ = false;
    bool best_gv_set = false;
    bool num_alignments_gv_set = false;
    
    /// vector of (FASTA file, index name) pairs for loading index
    vector< pair<string,string> > myfiles;
	/// skip lengths for pass 1, pass 2 and pass 3 in first step of sortmerna pipeline for each reference database searched
    vector< vector<uint32_t> > skiplengths;

  #ifdef __APPLE__
  int sz[2] = {CTL_HW, HW_MEMSIZE};
  u_int namelen = sizeof(sz)/sizeof(sz[0]);
  uint64_t size;
  size_t len = sizeof(size);
  if ( sysctl(sz,namelen,&size,&len,NULL,0) < 0 )
	{
	  fprintf(stderr,"\n  %sERROR%s: sysctl (main.cpp)\n","\033[0;31m","\033[0m");
	  exit(EXIT_FAILURE);
	}
  else
  {
	  maxpages_gv = size/pagesize_gv;
  }
  #else
  maxpages_gv = sysconf(_SC_PHYS_PAGES);
  #endif

	if ( argc == 1 )
	{
		verbose = true;
		welcome();
		fprintf(stderr,"  For help or more information on usage, type `./sortmerna %s-h%s'\n\n","\033[1m","\033[0m");
		exit(EXIT_FAILURE);
	}

	while ( narg < argc )
	{
	    switch( argv[narg][1] )
	    {

            /// options beginning with '--'
            case '-': {
					char* myoption = argv[narg];
					/// skip the '--'
					myoption+=2;

                    /// FASTA/FASTQ reads sequences
					if ( strcmp ( myoption, "reads" ) == 0 )
					{
						if ( argv[narg+1] == NULL ) 
						{
							fprintf(stderr,"\n  %sERROR%s: a path to a reads fasta/fastq file must be given after the option --reads\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
						else
						{
							if ( FILE *file = fopen(argv[narg+1], "r") )
							{
								readsfile = argv[narg+1];
								narg+=2;
								fclose(file);	
							}
							else
							{
								fprintf(stderr, "\n  %sERROR%s: the file %s could not be opened: %s.\n\n","\033[0;31m","\033[0m",argv[narg+1],strerror(errno));
								exit(EXIT_FAILURE);
							}
						}
					}
					/// FASTA reference sequences
					else if ( strcmp ( myoption, "ref") == 0 )
					{
						if ( argv[narg+1] == NULL ) 
						{
							fprintf(stderr,"\n  %sERROR%s: --ref must be followed by at least one entry (ex. --ref /path/to/file1.fasta,/path/to/index1)\n\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
                        /// path exists, check path
						else
						{
                            char *ptr = argv[narg+1];
                            while ( *ptr != '\0' )
                            {
                                /// get the FASTA file path + name
                                char fastafile[2000];
                                char *ptr_fastafile = fastafile;
                                /// the reference database FASTA file
                                while ( *ptr != ',' && *ptr != '\0' )
                                {
                                    *ptr_fastafile++ = *ptr++;
                                }
                                *ptr_fastafile = '\0';
                                if ( *ptr == '\0' )
                                {
                                    fprintf(stderr,"   %sERROR%s: the FASTA reference file name %s must be followed by an index name.\n\n","\033[0;31m","\033[0m",fastafile);
                                    exit(EXIT_FAILURE);
                                }
                                ptr++; //skip the ',' delimiter

                                
                                /// check FASTA file exists
                                if ( FILE *file = fopen(fastafile, "r") ) fclose(file);
                                else
                                {
                                    fprintf(stderr, "\n  %sERROR%s: the file %s could not be opened: %s.\n\n","\033[0;31m","\033[0m",fastafile,strerror(errno));
                                    exit(EXIT_FAILURE);
                                }
                                
                                
                                /// get the index path + name
                                char indexfile[2000];
                                char *ptr_indexfile = indexfile;
                                /// the reference database index name
                                while ( *ptr != ':' && *ptr != '\0') *ptr_indexfile++ = *ptr++;
                                *ptr_indexfile = '\0';
                                if ( *ptr != '\0' ) ptr++; //skip the ':' delimiter
                                
                                /// check the directory where to write the index exists
                                char dir[500];
                                char *ptr_end = strrchr( indexfile, '/');
                                if ( ptr_end != NULL )
                                {
                                    memcpy( dir, indexfile, (ptr_end-indexfile) );
                                    dir[(int)(ptr_end-indexfile)] = '\0';
                                }
                                else
                                {
                                    strcpy( dir, "./" );
                                }
                                
                                if ( DIR *dir_p = opendir(dir) ) closedir(dir_p);
                                else
                                {
                                    if ( ptr_end != NULL )
                                        fprintf(stderr,"\n  %sERROR%s: the directory %s for writing index '%s' could not be opened. The full directory path must be provided (ex. no '~'). \n\n","\033[0;31m","\033[0m",dir,ptr_end+1);
                                    else
                                        fprintf(stderr,"\n  %sERROR%s: the directory %s for writing index '%s' could not be opened. The full directory path must be provided (ex. no '~'). \n\n","\033[0;31m","\033[0m",dir,indexfile);
                                    
                                    exit(EXIT_FAILURE);
                                }
                                
                                /// check index file names are distinct
                                for ( int i = 0; i < (int)myfiles.size(); i++ )
                                {
                                    if ( (myfiles[i].first).compare(fastafile) == 0 )
                                    {
                                        fprintf(stderr, "\n  %sWARNING%s: the FASTA file %s has been entered twice in the list. It will be searched twice. \n\n","\033[0;33m","\033[0m",fastafile);
                                    }
                                    else if ( (myfiles[i].second).compare(indexfile) == 0 )
                                    {
                                        fprintf(stderr, "\n  %sWARNING%s: the index name %s has been entered twice in the list. It will be searched twice.\n\n","\033[0;33m","\033[0m",indexfile);
                                    }
                                }
                                
                                myfiles.push_back(pair<string,string>(fastafile,indexfile));
                                
                            }//~while (*ptr != '\0')
                            
                            narg+=2;

                        }//~else
					}
#ifdef chimera
                    /// output chimeric sequences to another file
                    else if ( strcmp ( myoption, "chimera" ) == 0 )
                    {
                        if ( chimeraout_gv )
                        {
                            fprintf(stderr,"\n  %sERROR%s: --chimera has already been set\n","\033[0;31m","\033[0m");
                            exit(EXIT_FAILURE);
                        }
                        else
                        {
                            chimeraout_gv = true;
                            narg++;
                        }
                    }
#endif
					/// the name of output aligned reads
					else if ( strcmp ( myoption, "aligned" ) == 0 )
					{
						if ( argv[narg+1] == NULL )
						{
							fprintf(stderr,"\n  %sERROR%s: a filename must follow the option --aligned\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
						else
						{
							/// check if the directory where to write exists
							char dir[500];
							char *ptr = strrchr( argv[narg+1], '/');
							if ( ptr != NULL )
							{
								memcpy( dir, argv[narg+1], (ptr-argv[narg+1]) );
								dir[(int)(ptr-argv[narg+1])] = '\0';
							}
							else
							{
								strcpy( dir, "./" );
							}

							if ( DIR *dir_p = opendir(dir) )
							{
								ptr_filetype_ar = argv[narg+1];
								narg+=2;
								closedir(dir_p);
							} 
							else	
							{
								fprintf(stderr,"\n  %sERROR%s: the --aligned directory %s could not be opened: %s.\n\n","\033[0;31m","\033[0m",dir,strerror(errno));
								exit(EXIT_FAILURE);
							}
						}
					}
					/// the name of output rejected reads
					else if ( strcmp ( myoption, "other"  ) == 0 )
					{
						if ( argv[narg+1] == NULL )
						{
							fprintf(stderr,"\n  %sERROR%s: a filename must follow the option --other\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);	
						}
						else
						{
                                /// check if the directory where to write exists
								char dir[500];
								char *ptr = strrchr( argv[narg+1], '/');
								if ( ptr != NULL )
								{
									memcpy( dir, argv[narg+1], (ptr-argv[narg+1]) );
									dir[(int)(ptr-argv[narg+1])] = '\0';
								}
								else
								{
									strcpy( dir, "./" );
								}

								if ( DIR *dir_p = opendir(dir) )
								{
									ptr_filetype_or = argv[narg+1];
									narg+=2;
									closedir(dir_p);
								} 
								else	
								{
									fprintf(stderr,"\n  %sERROR%s: the --other directory %s could not be opened, please check it exists.\n\n","\033[0;31m","\033[0m",dir);
									exit(EXIT_FAILURE);
								}
						}
					}
                    /// output overall statistics file
                    else if ( strcmp ( myoption, "log" ) == 0 )
                    {
                        if ( logout_gv )
                        {
                            fprintf(stderr,"\n  %sERROR%s: --log has already been set once.\n","\033[0;31m","\033[0m");
                            exit(EXIT_FAILURE);
                        }
                        else
                        {
                            logout_gv = true;
                            narg++;
                        }
                    }
#ifdef OTU_MAP
                    /// output OTU map
                    else if ( strcmp ( myoption, "otu_map" ) == 0 )
                    {
                        if ( otumapout_gv )
                        {
                            fprintf(stderr,"\n  %sERROR%s: --otu_map has already been set once.\n","\033[0;31m","\033[0m");
                            exit(EXIT_FAILURE);
                        }
                        else
                        {
                            otumapout_gv = true;
                            narg++;
                        }
                    }
#endif
                    /// don't add pid to output files
                    else if ( strcmp ( myoption, "pid" ) == 0 )
                    {
                        if ( pid_gv )
                        {
                            fprintf(stderr,"\n  %sERROR%s: --pid has already been set once.\n","\033[0;31m","\033[0m");
                            exit(EXIT_FAILURE);
                        }
                        else
                        {
                            pid_gv = true;
                            narg++;
                        }
                    }
                    /// put both paired reads into --accept reads file
                    else if ( strcmp ( myoption, "paired_in"  ) == 0 )
                    {
                        if ( pairedin_gv )
                        {
                            fprintf(stderr,"\n  %sERROR%s: --paired_in has already been set once.\n","\033[0;31m","\033[0m");
                            exit(EXIT_FAILURE);
                        }
                        else if ( pairedout_gv )
                        {
                            fprintf(stderr,"\n  %sERROR%s: --paired_out has been set, please choose one or the other, or use the default option.\n","\033[0;31m","\033[0m");
                            exit(EXIT_FAILURE);
                        }
                        else
                        {
                            pairedin_gv = true;
                            narg++;
                        }
                    }
                    /// put both paired reads into --other reads file
                    else if ( strcmp ( myoption, "paired_out"  ) == 0 )
                    {
                        if ( pairedout_gv )
                        {
                            fprintf(stderr,"\n  %sERROR%s: --paired_out has already been set once.\n","\033[0;31m","\033[0m");
                            exit(EXIT_FAILURE);
                        }
                        else if ( pairedin_gv )
                        {
                            fprintf(stderr,"\n %sERROR%s: --paired_in has been set, please choose one or the other, or use the default option.\n","\033[0;31m","\033[0m");
                            exit(EXIT_FAILURE);
                        }
                        else
                        {
                            pairedout_gv = true;
                            narg++;
                        }
                    }
					/// the score for a match
					else if ( strcmp ( myoption, "match" ) == 0 )
					{
						if (argv[narg+1] == NULL)
						{
							fprintf(stderr,"\n  %sERROR%s: --match requires a positive integer as input (ex. --match 2).\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
						/// set match
						if ( !match_set )
						{
							match = atoi(argv[narg+1]);      
							narg+=2;
							match_set = true;
						}
						else
						{
							fprintf(stderr,"\n  %sERROR%s: --match has been set twice, please verify your choice\n\n","\033[0;31m","\033[0m");
							printlist();
						} 
					}
					/// the score for a mismatch
					else if ( strcmp ( myoption, "mismatch" ) == 0 )
					{
						if (argv[narg+1] == NULL)
						{
							fprintf(stderr,"\n  %sERROR%s: --mismatch requires a negative integer input (ex. --mismatch -2)\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
						/// set mismatch
						if ( !mismatch_set )
						{
							mismatch = atoi(argv[narg+1]);  
							if ( mismatch > 0 )
							{
								fprintf(stderr,"\n  %sERROR%s: --mismatch requires a negative integer input (ex. --mismatch -2)\n","\033[0;31m","\033[0m");
								exit(EXIT_FAILURE);
							}    
							narg+=2;
							mismatch_set = true;
						}
						else
						{
							printf("\n  %sERROR%s: --mismatch has been set twice, please verify your choice\n\n","\033[0;31m","\033[0m");
							printlist();
						} 
					}
					/// the score for a gap
					else if ( strcmp ( myoption, "gap_open" ) == 0 )
					{
						if (argv[narg+1] == NULL)
						{
							fprintf(stderr,"\n  %sERROR%s: --gap_open requires a positive integer as input (ex. --gap_open 5)\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
						/// set gap open
						if ( !gap_open_set )
						{
							gap_open = atoi(argv[narg+1]); 
							if ( gap_open < 0 )	
							{
								fprintf(stderr,"\n  %sERROR%s: --gap_open requires a positive integer as input (ex. --gap_open 5)\n","\033[0;31m","\033[0m");
								exit(EXIT_FAILURE);
							}     
							narg+=2;
							gap_open_set = true;
						}
						else
						{
							printf("\n  %sERROR%s: --gap_open has been set twice, please verify your choice\n\n","\033[0;31m","\033[0m");
							printlist();
						} 
					}
					/// the score for a gap extension
					else if ( strcmp ( myoption, "gap_ext" ) == 0 )
					{
						if (argv[narg+1] == NULL)
						{
							fprintf(stderr,"\n  %sERROR%s: --gap_ext requires a positive integer as input (ex. --gap_ext 2)\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
						/// set gap extend
						if ( !gap_ext_set )
						{
							gap_extension = atoi(argv[narg+1]);   
							if ( gap_extension < 0 )	
							{
								fprintf(stderr,"\n  %sERROR%s: --gap_ext requires a positive integer as input (ex. --gap_ext 2)\n","\033[0;31m","\033[0m");
								exit(EXIT_FAILURE);
							}    
							narg+=2;
							gap_ext_set = true;
						}
						else
						{
							fprintf(stderr,"\n  %sERROR%s: --gap_ext has been set twice, please verify your choice\n\n","\033[0;31m","\033[0m");
							printlist();
						} 
					}
					/// number of seed hits before searching for candidate LCS
					else if ( strcmp ( myoption, "num_seeds" ) == 0 )
					{
						if (argv[narg+1] == NULL)
						{
							fprintf(stderr,"\n  %sERROR%s: --num_seeds requires a positive integer as input (ex. --num_seeds 6)\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
						/// set number of seeds
						if ( seed_hits_gv < 0 )
						{
							char* end = 0;
							seed_hits_gv = (int)strtol(argv[narg+1],&end,10); /// convert to integer
							if ( seed_hits_gv <= 0 )	
							{
								fprintf(stderr,"\n  %sERROR%s: --num_seeds requires a positive integer (>0) as input (ex. --num_seeds 6)\n","\033[0;31m","\033[0m");
								exit(EXIT_FAILURE);
							}    
							narg+=2;
						}
						else
						{
							fprintf(stderr,"\n  %sERROR%s: --num_seeds has been set twice, please verify your choice\n\n","\033[0;31m","\033[0m");
							printlist();
						} 
					}
#ifdef NOMASK_option
                    /// turn off masking of low-occurring L/2-mers for seed of length L
					else if ( strcmp ( myoption, "no-mask"  ) == 0 )
					{
						if ( nomask_gv )
						{
							fprintf(stderr,"\n  %sERROR%s: --no-mask has already been set once.\n\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
						else
						{
							nomask_gv = true;
							narg++;
						}
					}
#endif
					/// output all hits in FASTX format
					else if ( strcmp ( myoption, "fastx"  ) == 0 )
					{
						if ( fastxout_gv )
						{
							fprintf(stderr,"\n  %sERROR%s: --fastx has already been set once.\n\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
						else
						{
							fastxout_gv = true;
							narg++;
						}
					}
					/// output all hits in SAM format
					else if ( strcmp ( myoption, "sam"  ) == 0 )
					{
						if ( samout_gv )
						{
							fprintf(stderr,"\n  %sERROR%s: --sam has already been set once.\n\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
						else
						{
							samout_gv = true;
							narg++;
						}
					}
					/// output all hits in BLAST format 
					else if ( strcmp ( myoption, "blast"  ) == 0 )
					{
						if ( blastout_gv )
						{
							fprintf(stderr,"\n  %sERROR%s: --blast has already been set once.\n\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
						else
						{
							blastout_gv = true;
							narg++;
						}
					}
					/// output best alignment as predicted by the longest increasing subsequence
					else if ( strcmp ( myoption, "best" ) == 0 )
					{	
						if (argv[narg+1] == NULL)
						{
							fprintf(stderr,"\n  %sERROR%s: --best [INT] requires an integer (>=0) as input (ex. --best 2) (note: 0 signifies to search all high scoring reference sequences).\n\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
						/// best_gv has already been set
						else if ( best_gv_set )
                        {
 							fprintf(stderr,"\n  %sERROR%s: --best [INT] has been set twice, please verify your choice.\n\n","\033[0;31m","\033[0m");
							printlist();
                        }
						else
                        {
							best_gv = atoi(argv[narg+1]);
                            if ( best_gv < 0 )
							{
								fprintf(stderr,"\n  %sERROR%s: --best [INT] must be >= 0 (0 signifies to search all high scoring reference sequences).\n\n","\033[0;31m","\033[0m");
								exit(EXIT_FAILURE);
							} 
	 						narg+=2;
                            best_gv_set = true;
						}
					}
					/// output all alignments
					else if ( strcmp ( myoption, "num_alignments" ) == 0 )
					{	
						
						if (argv[narg+1] == NULL)
						{
							fprintf(stderr,"\n  %sERROR%s: --num_alignments [INT] requires an integer (>=0) as input (ex. --num_alignments 2) (note: 0 signifies to output all alignments).\n\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);						
						}
						/// --num_alignments [INT] has already been set
						else if ( num_alignments_gv_set )
                        {
 							fprintf(stderr,"\n  %sERROR%s:--num_alignments [INT] has been set twice, please verify your command parameters.\n\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
                        }
                        /// set number of alignments to output reaching the E-value
						else
                        {
							num_alignments_gv = atoi(argv[narg+1]);
                            if ( num_alignments_gv < 0 )
							{
								fprintf(stderr,"\n  %sERROR%s: --num_alignments [INT] must be >= 0 (0 signifies to output all alignments).\n\n","\033[0;31m","\033[0m");
								exit(EXIT_FAILURE);
							}
	 						narg+=2;
                            num_alignments_gv_set = true;
						}
					}
					/// output the first alignment reaching E-value score
					else if ( strcmp ( myoption, "feeling-lucky" ) == 0 )
					{	
						/// --feeling-lucky is already set
						if ( feeling_lucky_gv )
						{
							fprintf(stderr,"\n  %sERROR%s: --feeling-lucky has already been set once.\n\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);						
						}
						else
						{
							feeling_lucky_gv = true;
							narg++;
						} 
					}
					/// number of nucleotides to add to each edge of an alignment region before extension
					else if ( strcmp ( myoption, "edges" ) == 0 )
					{	
						/// --edges is already set
						if ( edges_set )
						{
							fprintf(stderr,"\n  %sERROR%s: --edges INT%% has already been set once.\n\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);						
						}						
						else 
						{
						  char *end = 0;
							char* test = strpbrk(argv[narg+1], "%"); /// find if % sign exists
							if ( test != NULL )
								as_percent_gv = true;

							edges_gv = (int)strtol(argv[narg+1],&end,10); /// convert to integer

							if ( edges_gv < 1 || edges_gv > 10 )
							{
								fprintf(stderr,"\n  %sERROR%s: --edges [INT] requires a positive integer between 0-10 as input (ex. --edges 4).\n","\033[0;31m","\033[0m");
								exit(EXIT_FAILURE);
							}

							narg+=2;
						}
					}
					/// execute full index search for 0-error and 1-error seed matches
					else if ( strcmp ( myoption, "full_search"  ) == 0 )
					{
						if ( full_search_set )	
						{
							fprintf(stderr,"\n  %sERROR%s: --full_search has been set twice, please verify your choice.\n\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
						else
						{
							full_search_set = true;
							full_search_gv = true;
							narg++;
						}
					}
                    /// do not output SQ tags in the SAM file
					else if ( strcmp ( myoption, "SQ"  ) == 0 )
					{
						if ( yes_SQ )
						{
							fprintf(stderr,"\n  %sERROR%s: --SQ has been set twice, please verify your choice.\n\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
						else
						{
							yes_SQ = true;
							narg++;
						}
					}
					/// --passes option
					else if ( strcmp ( myoption, "passes" ) == 0 )
					{
						if ( passes_set )
						{
							fprintf(stderr,"\n  %sERROR%s: --passes has been set twice, please verify your choice.\n\n","\033[0;31m","\033[0m");
							exit(EXIT_FAILURE);
						}
						/// set passes
						else 
						{
                            vector<uint32_t> skiplengths_v;
                            char *end = 0;
                            int32_t t = (int)strtol(strtok(argv[narg+1], ","),&end,10);
                            if ( t > 0 ) skiplengths_v.push_back(t);
                            else
							{
								fprintf(stderr,"\n  %sERROR%s: all three integers in --passes INT,INT,INT must contain positive integers where 0<INT<(shortest read length).\n\n","\033[0;31m","\033[0m");
								exit(EXIT_FAILURE);
							}
                            t = (int)strtol(strtok(NULL, ","),&end,10);
                            if ( t > 0 ) skiplengths_v.push_back(t);
                            else
							{
								fprintf(stderr,"\n  %sERROR%s: all three integers in --passes INT,INT,INT must contain positive integers where 0<INT<(shortest read length).\n\n","\033[0;31m","\033[0m");
								exit(EXIT_FAILURE);
							}
                            t = (int)strtol(strtok(NULL, ","),&end,10);
                            if ( t > 0 ) skiplengths_v.push_back(t);
                            else
							{
								fprintf(stderr,"\n  %sERROR%s: all three integers in --passes INT,INT,INT must contain positive integers where 0<INT<(shortest read length).\n\n","\033[0;31m","\033[0m");
								exit(EXIT_FAILURE);
							}
 
                            skiplengths.push_back(skiplengths_v);
							narg+=2;
							passes_set = true;
						}
					}
#ifdef id_cov
                    else if ( strcmp (myoption, "id" ) == 0 )
                    {
                        /// %id
                        if ( align_id < 0 )
                        {
                            sscanf(argv[narg+1],"%lf",&align_id);
                            narg+=2;
                        }
                        else
                        {
                            fprintf(stderr,"\n  %sERROR%s: --id has been set twice, please verify your command parameters.\n\n","\033[0;31m","\033[0m");
                            exit(EXIT_FAILURE);
                        }
                    }
                    else if ( strcmp (myoption, "coverage" ) == 0 )
                    {
                        /// %coverage
                        if ( align_cov < 0 )
                        {
                            sscanf(argv[narg+1],"%lf",&align_cov);
                            narg+=2;
                        }
                        else
                        {
                            fprintf(stderr,"\n  %sERROR%s: --coverage has been set twice, please verify your command parameters.\n\n","\033[0;31m","\033[0m");
                            exit(EXIT_FAILURE);
                        }
                    }
#endif
					/// the version number
					else if ( strcmp ( myoption, "version"  ) == 0 )
					{
						fprintf(stderr,"\n  SortMeRNA version %s\n\n",version_num);
						exit(EXIT_SUCCESS);
					}
					else
					{
						fprintf(stderr,"\n  %sERROR%s: option --%s is not an option.\n\n","\033[0;31m","\033[0m",myoption);
						printlist();	
					}
			  }
									break;
		case 'a': { 
				/// the number of cpus has been set twice
				if ( numcpu_gv == -1 )
				{
					numcpu_gv = atof(argv[narg+1]);      
					narg+=2;
				}
				else
				{
					printf("\n  %sERROR%s: -a has been set twice, please verify your command parameters.\n\n","\033[0;31m","\033[0m");
					exit(EXIT_FAILURE);
				} 
			  }    			
									break;
		case 'e': { 
				/// E-value
                if ( argv[narg+1] == NULL )
                {
                    fprintf(stderr,"\n  %sERROR%s: -e requires a positive double as input (ex. --e 1e-5)\n","\033[0;31m","\033[0m");
                    exit(EXIT_FAILURE);
                }
				if ( evalue < 0 )
				{
					sscanf(argv[narg+1],"%lf",&evalue);
                    if ( evalue < 0 )
                    {
                        fprintf(stderr,"\n  %sERROR%s: -e requires a positive double as input (ex. --e 1e-5)\n","\033[0;31m","\033[0m");
                        exit(EXIT_FAILURE);
                    }
					narg+=2;
				}
				else
				{
					fprintf(stderr,"\n  %sERROR%s: -e has been set twice, please verify your command parameters.\n\n","\033[0;31m","\033[0m");
					exit(EXIT_FAILURE);
				} 
			  }    			
									break;
		case 'F': {
				/// only forward strand
				if ( !forward_gv )
				{
					  forward_gv = true; 
					  narg++;
				}
				else 
				{	
					  fprintf(stderr,"\n  %sERROR%s: flag -F has been set more than once, please check your command parameters.\n","\033[0;31m","\033[0m");
					  exit(EXIT_FAILURE);
				}
				}        	
								              break;
		case 'R': {
				/// only reverse strand
				if ( !reverse_gv )
				{
				   reverse_gv = true; 
				   narg++;
				}
				else 
				{
				    fprintf(stderr,"\n  %sERROR%s: flag -R has been set more than once, please check your command parameters.\n","\033[0;31m","\033[0m");
				    exit(EXIT_FAILURE);
				}
			  }	   			   
							   		break;
		case 'h': {	
				/// help
				printlist();
			  }
									break;

		case 'v': {
                /// turn on verbose
				verbose = true;
				narg++;
			  }
									break;
		case 'N': {
				/// match ambiguous N's
				if ( !match_ambiguous_N_gv )
				{
                      match_ambiguous_N_gv = true;
                      score_N = atoi(argv[narg+1]);
					  narg+=2;
				}
				else 
				{	
					  fprintf(stderr,"\n  %sERROR%s: flag -N has been set more than once, please check your command parameters.\n","\033[0;31m","\033[0m");
					  exit(EXIT_FAILURE);
				}
		}        	
								   break;
		case 'm': { 
				/// set the map_size_gv variable
				if ( !map_size_set_gv )
				{
                    /// RAM limit for mmap'ing reads in megabytes
					unsigned long long int _m = strtol( argv[narg+1], NULL, 10 );
                    unsigned long long int pages_asked = (unsigned long long int)(_m*1048576)/pagesize_gv;
                    
                    /// RAM limit exceeds available resources
                    if ( pages_asked > maxpages_gv/2 )
                    {
                        int max_ram = (maxpages_gv*pagesize_gv)/1048576;
                        fprintf(stderr,"\n  %sERROR%s: -m INT must not exceed %d (Mbyte).\n\n","\033[0;31m","\033[0m",max_ram);
                        exit(EXIT_FAILURE);
                    }
                    /// set RAM limit
					if ( _m != 0 )
					{ 
                        map_size_gv*=pages_asked;
						narg+=2;
						map_size_set_gv = true;
					}
					else
					{
                        fprintf(stderr, "\n  %sERROR%s: -m value must be a positive integer value (in Mbyte).\n\n","\033[0;31m","\033[0m");
                        exit(EXIT_FAILURE);
					}
				}
				else
				{
					fprintf(stderr,"\n  %sERROR%s: -m has been set twice, please verify your command parameters.\n\n","\033[0;31m","\033[0m");
					exit(EXIT_FAILURE);
				} 
			  }    			
									break;

		default : {
				fprintf(stderr,"\n  %sERROR%s: '%c' is not one of the options.\n","\033[0;31m","\033[0m",argv[narg][1]);
				printlist();
			  } 
	    }//~switch
	}//~while ( narg < argc )



	/// ERROR messages *******

	/// check mandatory input
	if ( (readsfile == NULL) || myfiles.empty() )
	{
		fprintf(stderr,"\n  %sERROR%s: a reads file (--reads reads.fasta) and a reference sequence file (--ref /path/to/file1.fasta,/path/to/index1) are mandatory input.\n\n","\033[0;31m","\033[0m");
		printlist();
	}
    
    if ( !fastxout_gv && (pairedin_gv || pairedout_gv) )
    {
        eprintf("\n  %sERROR%s: options --paired_in and --paired_out must be accompanied by option --fastx.\n","\033[0;31m","\033[0m");
        eprintf("  These flags are for FASTA and FASTQ output files, for maintaining paired reads together.\n");
        exit(EXIT_FAILURE);
    }
    
	/// aligned reads output
	if ( ptr_filetype_ar == NULL )
	{
		if ( fastxout_gv || blastout_gv || samout_gv )
		{
			eprintf("\n  %sERROR%s: options --fastx, --blast and --sam must be accompanied by option --aligned STRING.\n\n","\033[0;31m","\033[0m");
			exit(EXIT_FAILURE);
		}
	}
	else if ( !fastxout_gv && !blastout_gv && !samout_gv )
	{
		eprintf("\n  %sERROR%s: option --aligned STRING has been set but no output format has been chosen (fastx/sam/blast).\n\n","\033[0;31m","\033[0m");
		exit(EXIT_FAILURE);
	}
    
	/// non-aligned reads output
	if ( ptr_filetype_or != NULL )
	{
		if ( !fastxout_gv && (blastout_gv || samout_gv) )
		{
			eprintf("\n  %sERROR%s: option --other STRING can only be used together with the --fastx option.\n\n","\033[0;31m","\033[0m");
            exit(EXIT_FAILURE);
		}
	}
    
    /// if feeling-lucky was chosen, check an alignment format has also been chosen
    if ( feeling_lucky_gv && !(blastout_gv || samout_gv) )
    {
        eprintf("\n  %sERROR%s: '--feeling-lucky' has been set but no alignment format has been chosen ('--blast' or '--sam').\n\n","\033[0;31m","\033[0m");
        exit(EXIT_FAILURE);
    }
    
	/// if best alignment was chosen, check an alignment format has also been chosen
	if ( (best_gv > -1) && !(blastout_gv || samout_gv) )
	{
        eprintf("\n  %sERROR%s: '--best' has been set but no alignment format has been chosen ('--blast' or '--sam').\n\n","\033[0;31m","\033[0m");
        exit(EXIT_FAILURE);
	}
    
	/// if all alignment was chosen, check an alignment format has also been chosen
	if ( (num_alignments_gv > 0) && !(blastout_gv || samout_gv) )
	{
		eprintf("\n  %sERROR%s: '--num_alignments [INT]' has been set but no alignment format has been chosen ('--blast' or '--sam').\n\n","\033[0;31m","\033[0m");
        exit(EXIT_FAILURE);
	}

	/// check gap extend score < gap open score
	if ( gap_extension > gap_open )
	{
		fprintf(stderr,"\n  %sERROR%s: --gap_ext must be less than --gap_open.\n\n","\033[0;31m","\033[0m");
		exit(EXIT_FAILURE);
	}
    
    if ( feeling_lucky_gv )
    {
        /// only one alignment (hence reference sequence) can be observed with option --feeling-lucky
        if (best_gv > -1)
        {
            fprintf(stderr,"\n  %sERROR%s: --feeling-lucky cannot be set with --best [INT].\n\n","\033[0;31m","\033[0m");
            exit(EXIT_FAILURE);
        }
        
        /// only one alignment can be observed with option --feeling-lucky
        if (num_alignments_gv > -1)
        {
            fprintf(stderr,"\n  %sERROR%s: --feeling-lucky cannot be set with --num_alignments [INT].\n\n","\033[0;31m","\033[0m");
            exit(EXIT_FAILURE);
        }
    }
    
    /// only one of these options is allowed (--best outputs one alignment, --num_alignments outputs > 1 alignments)
    if ( (best_gv > -1) && (num_alignments_gv > -1) )
    {
        fprintf(stderr,"\n  %sERROR%s: --best [INT] and --num_alignments [INT] cannot be set together, please choose one.\n","\033[0;31m","\033[0m");
        fprintf(stderr,"  (--best [INT] will search INT highest scoring reference sequences and output a single best alignment, whereas --num_alignments [INT] will output the first INT alignments).\n\n");
        exit(EXIT_FAILURE);
    }
    
    
    if (nomask_gv)
    {
        eprintf("\n  %sWARNING%s: the option '--no-mask' is not currently available, this flag has no effect on the results (L/2-mers will not be masked).\n","\033[0;31m","\033[0m");
    }
    
    
#ifdef chimera
    /// TEMPORARY for chimera
    if ( chimeraout_gv )
    {
        if (ptr_filetype_ar == NULL)
        {
            fprintf(stderr,"  %sERROR%s: a root filename name (--aligned STRING) for output needs to be provided with --chimera.\n\n","\033[0;31m","\033[0m");
            exit(EXIT_FAILURE);
        }
        //else if ( !samout_gv )
        //{
        //    fprintf(stderr,"  %sERROR%s: --sam output format must be selected with --chimera.\n\n","\033[0;31m","\033[0m");
        //    exit(EXIT_FAILURE);
        //}
    }
#endif






	/// the list of arguments is correct, welcome the user!
	welcome();
	

	/// if neither strand was selected for search, search both
	if ( !forward_gv && !reverse_gv )
	{
		forward_gv = true;
		reverse_gv = true;
	} 
	if ( numcpu_gv  < 0 ) numcpu_gv = 1;

	/// Gumbel parameters
	if ( evalue < 0.0 ) evalue = 1;

	/// SW alignment parameters
	if ( !match_set ) match = 2;
	if ( !mismatch_set ) mismatch = -3;
	if ( !gap_open_set ) gap_open = 5;
	if ( !gap_ext_set ) gap_extension = 2;
    if ( !match_ambiguous_N_gv ) score_N = mismatch;

	if ( !map_size_set_gv )
	{ 
		/// the maximum memory allowed for reads is less than 1GB
	  if ( 1073741824 > maxpages_gv*pagesize_gv/2 ) map_size_gv = maxpages_gv*pagesize_gv/2;
		/// we set the memory to 1GB
      else map_size_gv = 1073741824;
	}
    
    /// output single best alignment from 10 best candidate hits (default)
    if ( (best_gv == -1) && (num_alignments_gv == -1) && !feeling_lucky_gv ) best_gv = 2;
     
    /// only FASTA/FASTQ output, run in feeling-lucky mode (filtering mode)
    if ( fastxout_gv && !(blastout_gv || samout_gv) ) feeling_lucky_gv = true;

	/// default number of seed hits before searching for candidate LIS
	if ( seed_hits_gv < 0 ) seed_hits_gv = 2;

	/// default number of nucleotides to add to each edge of an alignment region before extension
	if ( edges_gv < 0 ) edges_gv = 4;

	/// activate heuristic for stopping search (of 1-error matches) after finding 0-error match
	if ( !full_search_set ) full_search_gv = false;
    
    /// default %id to keep alignment
    if ( align_id < 0 ) align_id = 0;
    
    /// default %query coverage to keep alignment
    if ( align_cov < 0 ) align_cov = 0;
    
    /// 3. For each window, traverse in parallel the Burst trie/reverse and LEV(k), outputting all reads with edit distance <= k between the window.
    paralleltraversal( readsfile,
                        ptr_filetype_ar,
                        ptr_filetype_or,
                        match,
                        mismatch,
                        gap_open,
                        gap_extension,
                        score_N,
                        skiplengths,
                        argc,
                        argv,
                        yes_SQ,
                        myfiles);
        


	
  	return 0;

}//~main()
