/* File: GCALab.c
 *
 * Author: David J. Warne (david.warne@qut.edu.au)
 * 
 * School of Electrical Engineering and Computer Science
 * Faculty of Science and Engineering
 * Queensland University of Technology
 * 
 * Date Created: 18/04/2012
 * Last Modified: 18/04/2012
 *
 * Version History:
 *       v 0.01 (18/04/2012) - i. Initial Version...  was bored at work and 
 *                             thought I should write a main program for my research.
 *       v 0.02 (19/04/2012) - i. Implemented and tested Commandline parsing
 *                             ii. starting to implement Batch mode
 *       v 0.04 (04/10/2012) - i. Re-implementing main processing loop entirely
 *                                this is a major restructure.
 *
 * Description: Main Program for Graph Cellular Automata generation, simulation,
 *              analysis and Visualisation.
 *
 * TODO List:
 *		1. test commandline parsing - done (v 0.02)
 *		2. implement batch mode first
 *		3. use ptheads to implement a main loop plus compute threads
 * 		4. implement text mode first, then re-implement batch mode, then graphics
 * 		5. carefully separated main engine from the mode type
 * Known Issues:
 *
 *==============================================================================
 */

#include "GCALab.h"

/*global array of workspace addresses*/
GCALab_WS **GCALab_Global;
unsigned int GCALab_numWS;
unsigned char GCALab_mode;


/* main(): entry point of the GCALab Program
 */
int main(int argc, char **argv)
{
	CL_Options* CL_opt;
	char rc;
	
	rc = GCALab_Init(argc,argv,&CL_opt);
	GCALab_HandleErr(rc);
	/*Start up in either batch or interactive mode*/
	switch(GCALab_mode)
	{
		case GCALAB_GRAPHICS_MODE:
			GCALab_GraphicsMode(CL_opt);
			break;
		case GCALAB_TEXT_MODE:
			GCALab_TextMode(CL_opt);
			break;
		case GCALAB_BATCH_MODE:
			GCALab_BatchMode(CL_opt);
			break;
		default:
			fprintf(stderr,"ERROR: You Should not see this!\n");
			exit(1);
			break;
	}
	
	free(CL_opt);
	pthread_exit(NULL);
} 

/* GCALab_TextMode(): starts a GCALab session in text-only mode
 */
void GCALab_TextMode(CL_Options* opts)
{
	char **userinput;
	char* cmd;
	char rc;
	unsigned int cur_ws;
	GCALab_SplashScreen();
	
	while(1)
	{
		/*get user input*/
		userinput = GCALab_CommandPrompt();
		/*ensure input is valid*/
		rc = GCALab_TestPointer((void*)userinput);
		GCALab_HandleErr(rc);
		
		cmd = userinput[0];

		if (!strcmp(cmd,"nwork"))
		{
			int lim;
			lim = atoi(userinput[1]);
			rc = GCALab_NewWorkSpace(lim);
			GCALab_HandleErr(rc);
			cur_ws = GCALab_numWS - 1;
			printf("New Workspace create! ID = %d\n",cur_ws);
			/*for ()*/
		}
		else if(!strcmp(cmd,"printwork"))
		{
			rc = GCALab_PrintWorkSpace(cur_ws);
			GCALab_HandleErr(rc);
		}
		else if (!strcmp(cmd,"listallwork"))
		{
			rc = GCALab_ListWorkSpaces();
			GCALab_HandleErr(rc);
		}
		else if (!strcmp(cmd,"help"))
		{
			rc = GCALab_PrintHelp();
			GCALab_HandleErr(rc);
		}
		else if (!strcmp(cmd,"chwork"))
		{
			unsigned int new_ws = (unsigned int)atoi(userinput[1]);
			rc = GCALab_ValidWSId(new_ws);
			GCALab_HandleErr(rc);
		}
		else if (!strcmp(cmd,"q"))
		{
			unsigned int cmd_code;
			unsigned int target;
			char ** params;
			cmd_code = GCALab_GetCommandCode(userinput[1]);
			target = (unsigned int)atoi(userinput[2]);
			params = userinput+3;
			rc = GCALab_QueueCommand(cur_ws,cmd_code,target,(void*)params);	
			GCALab_HandleErr(rc);
		}
		else if (!strcmp(cmd,"dq"))
		{
			unsigned int ind;
			ind = (unsigned int)atoi(userinput[1]);
			rc = GCALab_DequeueCommand(cur_ws,ind);
			GCALab_HandleErr(rc);
		}
		else if (!strcmp(cmd,"execq"))
		{
			rc = GCALab_ProcessCommandQueue(cur_ws);
			GCALab_HandleErr(rc);
		}
		else if (!strcmp(cmd,"stopq"))
		{
			rc = GCALab_PauseCommandQueue(cur_ws);
			GCALab_HandleErr(rc);
		}
		else if(!strcmp(cmd,"quit"))
		{
			GCALab_ShutDown(GCALAB_SUCCESS);
		}
		else
		{
			fprintf(stderr,"Unknown Command [%s], type help for guidence\n",cmd);
		}

	}
}

/* GCALab_TestPointer(): returns an error if the pointer is null
 */
char GCALab_TestPointer(void* ptr)
{
	if (!ptr)
		return GCALAB_MEM_ERROR;
	else
		return GCALAB_SUCCESS;
}

/* GCALab_ValidWSId(): tests if the given workspace id maps to a real workspace
 */
char GCALab_ValidWSId(unsigned int ws_id)
{
	/*for now simple*/
	if (ws_id >= GCALab_numWS)
		return GCALAB_INVALID_WS_ERROR;
	else
		return GCALAB_SUCCESS;
}

/* GCALab_HandleErr(): tests if the given returen code indicates an error,
 *                     if the error is fatal then it aborts.
 */
void GCALab_HandleErr(char rc)
{
	if (rc <= 0)
	{
		/*for now we consider all errors fatal*/
		fprintf(stderr,"GCALab: An error occurred for which GCALab is not smart enought to recover... Aborting [%x]\n",rc);
		pthread_exit((void*)1);
	}
}

/* GCALab_Init(): sets up GCALab with default settings, unless
 *                overridden via start-up commands
 */
char GCALab_Init(int argc,char **argv,CL_Options **opts)
{
	GCALab_Global = (GCALab_WS **)malloc(GCALAB_MAX_WORKSPACES*sizeof(GCALab_WS*));
	if(!(GCALab_Global))
	{
		return GCALAB_FATAL_ERROR;
	}

	opts[0] = ParseCommandLineArgs(argc,argv);
	if (!(opts[0]))
	{
		return GCALAB_CL_PARSE_ERROR;
	}
	GCALab_numWS = 0;
	GCALab_mode = (opts[0])->mode;
	return GCALAB_SUCCESS;
}

/* GCALab_InteractiveMode(): Runs GCALab interactively
 */
void GCALab_GraphicsMode(CL_Options* opts)
{
}

/* GCALab_BatchMode(): Runs Commands is batch mode
 * TODO: support a Job queue
 */
char GCALab_BatchMode(CL_Options* opts)
{
/*	GraphCellularAutomaton *GCA;
	CellularAutomatonParameters *params;
	char rc;
*/	/*allocate memory the params struct*/
/*	if (!(params = (CellularAutomatonParameters*)malloc(sizeof(CellularAutomatonParameters))))
	{
		return MEM_ERROR;
	}
*/	
	/*Create or load CA and Topology*/
/*	if (opts->load_CA)
	{
*/		/*read file*/
		
/*		if (opts->GenTopology)
		{
*/			/*generate top using given params*/
/*		}
		else
		{
*/			/*use loaded topology*/
/*		}
	}
	else if (opts->GenCA)
	{
*/		/*two special cases*/
/*		if (opts->ECA)
		{
*/			/*create given 1-d ECA rule*/
/*			GCA = CreateECA(opts->N,opts->k,opts->rule);
*/			/*set initial condition if not already specifieds*/
/*			if (!(opts->ICtype == EXPLICIT_IC_TYPE))
			{
				SetCAIC(GCA,NULL,opts->ICtype);
			}
		}
		else if (opts->GOL)
		{
*/			/*create given Game of life instance*/
/*		}
		else
		{
			if (opts->GenTopology)
			{
*/				/*generate top using given params*/
/*			}
			else
			{
*/				/*just assume 1-d ring*/
/*			}
		}
	}
		
	if (opts->save_CA)
	{
*/		/*write CA params and topology to file*/
/*		if((rc = GCALAB_saveCA(opts->CAOutputFilename,GCA)) <= 0)
		{
			fprintf(stderr,"%d\n",rc);
			return rc; 
		}
	}
	
*/	/*simulation stuff*/
/*	if (opts->do_ShannonE || opts->do_WordE || opts->do_InputE || opts->do_Pr || opts->do_G)
	{
*/		/*this is an analysis run, don't run a ordinary simulation*/
		
/*		if (opts->do_ShannonE)
		{
			float S;
			unsigned int i,samples;
			char buff[255];
			samples = opts->numSamples;
			S = 0.0;
			for (i=0;i<samples;i++)
			{
				SetCAIC(GCA,NULL,NOISE_IC_TYPE);
				S += ShannonEntropy(GCA,opts->end_t,NULL);
			}
			S /= (float)samples;	
			if (opts->save_CA)
			{
*/				/*append data*/
/*				sprintf(buff,"Shannon Entropy (%d samples, %d timesteps)",samples,opts->end_t);
				GCALAB_appendData(opts->CAOutputFilename,buff,(void*)&S,1,FLOAT32);
			}
		}
		
		if (opts->do_WordE)
		{
			float W;
			unsigned int i,samples;
			char buff[255];
			samples = opts->numSamples;
			W = 0.0;
			for (i=0;i<samples;i++)
			{
				SetCAIC(GCA,NULL,NOISE_IC_TYPE);
				W += WordEntropy(GCA,opts->end_t,NULL);
			}
			W /= (float)samples;	
			if (opts->save_CA)
			{
*/				/*append data*/
/*				sprintf(buff,"Word Entropy (%d samples, %d timesteps)",samples,opts->end_t);
				GCALAB_appendData(opts->CAOutputFilename,buff,(void*)&W,1,FLOAT32);
			
			}
		}
		
		if (opts->do_InputE)
		{
			if (opts->save_CA)
			{
*/				/*append data*/
/*			}
		}
		
		if (opts->do_Pr)
		{
			unsigned int *counts;
			unsigned int *temp;
			int size;
			char buff[255];
*/		/*	probs = ComputeExactProbs(GCA);*/
/*			size = (GCA->params->N)*(GCA->params->s);
			counts = (unsigned int*)malloc(size*sizeof(unsigned int));
			memset((void*)counts,0,size*sizeof(unsigned int));
*/			/*get counts within the IC range*/
/*			SumCAImages(GCA,counts,opts->range,0);
			if (opts->save_CA)
			{
*/				/*append data*/
/*				unsigned char s;
				for (s=0;s<opts->numStates;s++)
				{
					sprintf(buff,"Cell State Counts (s = %d)",s);
					temp = &(counts[((unsigned int)s)*GCA->params->N]);
					GCALAB_appendData(opts->CAOutputFilename,buff,(void*)temp,GCA->params->N,UINT32);
				}
			}
			free(counts);
			free(temp);
		}

		if (opts->do_R)
		{
			unsigned int *counts;
			int size;
			int i;
			char buff[255];
			unsigned int *temp;
			chunk* ics;
			
			size = (GCA->params->N)*(GCA->params->s);
			counts = (unsigned int*)malloc(size*sizeof(unsigned int));
			memset((void*)counts,0,size*sizeof(unsigned int));
			ics = (chunk*)malloc((opts->numSamples)*sizeof(chunk));
			
			for (i=0;i<opts->numSamples;i++)
			{
				ics[i] = rand();
			}

			SumCAImages(GCA,counts,ics,opts->numSamples);
			if (opts->save_CA)
			{
*/				/*append data*/
/*				unsigned char s;
				for (s=0;s<opts->numStates;s++)
				{
					sprintf(buff,"Cell State Counts (s = %d)",s);
					temp = &(counts[((unsigned int)s)*GCA->params->N]);
					GCALAB_appendData(opts->CAOutputFilename,buff,(void*)temp,GCA->params->N,UINT32);
				}
			}
			free(counts);
			free(temp);
			free(ics);
		}
	}
	else
	{
		int i;
		char buff[255];
*/		/*set initial condition if not already specifieds*/
/*		if (!(opts->ICtype == EXPLICIT_IC_TYPE))
		{
			SetCAIC(GCA,NULL,opts->ICtype);
		}
		
*/		/*just get the spatio-temopral pattern*/
/*		CASimTSteps(GCA,opts->end_t);
		
		if (opts->save_CA)
		{
*/			/*append data*/
/*			if (GCA->t > GCA->params->WSIZE)
			{
*/				/*for(i=GCA->params->WSIZE-1;i>=0;i--)
				{
					sprintf(buff,"t = %d",GCA->t-i);
					GCALAB_appendData(opts->CAOutputFilename,buff,(void*)(GCA->st_pattern[i]),GCA->size,CHUNK);	
				}*/
/*				for (i=0;i<=GCA->t;i++)
				{
					sprintf(buff,"t = %d",i);
					GCALAB_appendData(opts->CAOutputFilename,buff,(void*)(GCA->st_pattern[i]),GCA->size,CHUNK);	
				}
			}
			else
			{
				for(i=0;i<=GCA->t;i++)
				{
					sprintf(buff,"t = %d",i);
					GCALAB_appendData(opts->CAOutputFilename,buff,(void*)(GCA->st_pattern[i]),GCA->size,CHUNK);	
				}
			}
		}
	}
	
	if (opts->do_Z)
	{
*/		/*relatively Cheap*/
/*		if (opts->save_CA)
		{
*/			/*append data*/
/*		}
	}
	
	if (opts->do_Lambda)
	{
*/		/*trivial*/
/*		if (opts->save_CA)
		{
*/			/*append data*/
/*		}
	}
	return 0;	
*/
}




unsigned int GCALab_GetCommandCode(char* cmd)
{
	if (!strcmp(cmd,"load"))
	{
		return GCALAB_LOAD;
	}
	else if(!strcmp(cmd,"save"))
	{
		return GCALAB_SAVE;
	}
	else if(!strcmp(cmd,"simulate"))
	{
		return GCALAB_SIMULATE;
	}
	else if (!strcmp(cmd,"gca"))
	{
		return GCALAB_GCA;
	}
	else if (!strcmp(cmd,"sample"))
	{
		return GCALAB_SAMPLE;
	}
	else if (!strcmp(cmd,"entropy"))
	{
		return GCALAB_ENTROPY;
	}
	else if (!strcmp(cmd,"lambda"))
	{
		return GCALAB_LAMBDA;
	}
	else if (!strcmp(cmd,"Z"))
	{
		return GCALAB_Z;
	}
	else if (!strcmp(cmd,"G"))
	{
		return GCALAB_GDENSE;
	}

}

/* GCALab_Worker(): Processing thread attached to a workspace
 */
void * GCALab_Worker(void *params)
{
	unsigned char ws_id;
	unsigned char exit,error;
	ws_id = *((unsigned int*)params);

	exit = 0;
	error = 0;
	while(!exit)
	{
		unsigned int state,cmd,target;
		
		state = GCALab_GetState(ws_id);
		switch(state)
		{
			case GCALAB_WS_STATE_ERROR:
				exit = 1;
				error = 1;
				break;
			case GCALAB_WS_STATE_PAUSED:
				break;
			case GCALAB_WS_STATE_PROCESSING:
				/*the next command*/

				/*if empty set to idle state*/
				/*otherwise process*/
				break;
			case GCALAB_WS_STATE_IDLE:
				/*check if a command is available*/
				/*if so then set state to processing*/
				break;
		}
	}
	pthread_exit((void*)((long long)error));
}


/* GCALab_PopCommandQueue(): gets the next command from the command queue of the given
 *                           workspace
 */
char GCALab_PopCommandQueue(unsigned char ws_id,unsigned int *cmd, unsigned int *trgt,void *params)
{
	/*get a lock on this workspace*/
	/*pthread_mutex_lock();

	pthread_mutex_unlock();*/
}


/* PrintsAbout(): Prints Author and affiliation information
 */
void GCALab_PrintAbout(void)
{
	fprintf(stdout,"The Graph Cellular Automata Lab is a multi-threaded, flexible, analysis tool forthe exploration of cellular automata defined on graphs.\n");
	fprintf(stdout,"Version: %f\n",GCALAB_VERSION);
	fprintf(stdout,"Author: David J. Warne\n");
	fprintf(stdout,"School: School of Electrical Engineering and Computer Science, The Queensland University of Technology, Brisbane, Australia\n");
	fprintf(stdout,"Contact: david.warne@qut.edu.au\n");
	fprintf(stdout,"website: coming soon!\n");
	return;
}

/* GCALab_PrintLicense(): prints user license summary
 */
void GCALab_PrintLicense(void)
{
	fprintf(stdout,"GCALab v %f  Copyright (C) 2012  David J. Warne\n This program comes with ABSOLUTELY NO WARRANTY.\n This is free software, and you are welcome to redistribute it\n under certain conditions;\n",GCALAB_VERSION);
	return;
}

/* GCALab_SplashScreen(): startup screen
 */
void GCALab_SplashScreen(void)
{
	switch(GCALab_mode)
	{
		case GCALAB_TEXT_MODE:
			fprintf(stdout,"Welcome to Graph Cellular Automata Lab!\n");
			GCALab_PrintAbout();
			GCALab_PrintLicense();
			fprintf(stdout,"Type \'help\' to get started.\n");
			break;
		case GCALAB_BATCH_MODE:
			GCALab_PrintLicense();
			break;
		case GCALAB_GRAPHICS_MODE:
			break;
	}
	return;
}

/* PrintUsage(): Prints the help menu
 */
void GCALab_PrintUsage()
{
	printf("CGALab - Options");
	printf("\t [-i,--interactive]\n\t\t : start in interactive mode\n");
	printf("\t [-b,--batch]\n\t\t : start in batch mode\n");
	printf("\t [-l,--load] filename\n\t\t : load *.gca file\n");			
	printf("\t [-o,--save] filename\n\t\t : save results in *.gca file\n");
	printf("\t [-E,--simulate] startT endT\n\t\t : Specify a the time interval to run the CA over\n");
	printf("\t [-m,--topology] g N k\n\t\t : Create a Graph CA with topology genus g of N cells with k neighbourhood\n");	
	printf("\t [-g,--GOL] N\n\t\t : Create Conway's Game of Life with N cells\n");			
	printf("\t [-e,--ECA] r N\n\t\t : Create elementary CA rule r of N cells\n");			
	printf("\t [-w,--window-size] size\n\t\t : Specify temporal window size\n");			
	printf("\t [-t,--ruletype] ruletype\n\t\t : Specify the type of the given rule\n");			
	printf("\t [-r,--rule] code\n\t\t : the given rule interpreted by the -t option\n");			
	printf("\t [-s,--num-States]\n\t\t : number of possible states per cell\n");			
	printf("\t [-I,--Input-Entropy]\n\t\t : Compute Input Entropy\n");			
	printf("\t [-S,--Shannon-Entropy]\n\t\t : Compute Shannon Entropy\n");			
	printf("\t [-W,--Word-Entropy]\n\t\t : Compute Word Entropy\n");			
	printf("\t [-L,--Lambda-paramater\n\t\t : Compute Langton's Lambda parameter\n");
	printf("\t [-Z,--Z-parameter]\n\t\t : Compute Weunsche's Z parameter\n");
	printf("\t [-G,--G-density]\n\t\t : Compute Garden-of-Eden Density\n");
	printf("\t [-P,--Exact-Probs] l u\n\t\t : Compute Exact Proabitities over given range\n");
	printf("\t [-R,--Random-Sample-Number] n\n\t\t : Specify n Random initial conditions\n");
}

/* GCALab_CommandPrompt(): Prompts the user to enter a command in text mode, the
 *                         users input string is then tokenised with whitespace as the delimiter.
 */
char ** GCALab_CommandPrompt(void)
{
	char buffer[256];
	int i,j,k;
	int len;
	int argc;
	char** argv;
	char c;
	
	i = 0;
	
	fprintf(stdout,">>");
	fflush(stdout);
	do {
		c = fgetc(stdin);
		buffer[i] = c;
		i++;
	} while(c != '\n' && i <= 255);
	len = i;
	buffer[len-1] = '\0';
	/*count spaces*/
	argc = 0;
	for (i=1;i<len;i++)
	{
		if (((buffer[i] == ' ') || (buffer[i] == '\0')) && (buffer[i-1] != ' '))		
		{
			argc++;
		}
	}
	/*allocate memory for the commands*/
	if (!(argv = (char **)malloc(argc*sizeof(char*))))
	{
		return NULL;
	}

	for (i=0;i<argc;i++)
	{
		if (!(argv[i] = (char*)malloc(128*sizeof(char))))
		{
			return NULL;
		}
	}

	j = 0;
	k = 0;
	for (i=0;i<len-1;i++)
	{
		if (buffer[i] != ' ')
		{
			argv[j][k] = buffer[i];
			k++;
			if (buffer[i+1] == ' ' || buffer[i+1] == '\0')
			{
				argv[j][k] = '\0';
				fprintf(stderr,"%s\n",argv[j]);
				k = 0;
				j++;
			}
		}
	}

	return argv;
}

/* PrintOptions(): Prints selected options
 */
void PrintOptions(CL_Options* opts)
{
	printf("mode: %d\n",opts->mode);
	printf("load: %u\n",opts->load_CA);
	printf("save: %u\n",opts->save_CA);
	printf("In file: %s\n",opts->CAInputFilename);
	printf("Out file: %s\n",opts->CAOutputFilename);
	printf("Gen Topology: %u\n",opts->GenTopology);
	printf("Gen CA: %u\n",opts->GenCA);
	printf("N: %u\n",opts->N);
	printf("k: %u\n",opts->k);
	printf("GOL: %u\n",opts->GOL);
	printf("ECA: %u\n",opts->ECA);
	printf("S: %u\n",opts->numStates);
	printf("type: %u\n",opts->ruletype);
	printf("rule: %u\n",opts->rule);
	printf("W: %u\n",opts->windowSize);
	printf("t: [%u,%u] \n",opts->start_t,opts->end_t);
	printf("vis: %u\n",opts->visualise);
	printf("SE: %u\n",opts->do_ShannonE);
	printf("WE: %u\n",opts->do_WordE);
	printf("IE: %u\n",opts->do_InputE);
	printf("L: %u\n",opts->do_Lambda);
	printf("Z: %u\n",opts->do_Z);
	printf("G: %u\n",opts->do_G);
	printf("Pr: %u\n",opts->do_Pr);
	printf("range: [%u,%u]\n",opts->range[0],opts->range[1]);
	printf("IC: %u\n",opts->ICtype);
}

/* InitCL_Options(): Initialises the options struct with defaults values
 */
void InitCL_Options(CL_Options* opts)
{
	opts->mode = GCALAB_DEFAULT_MODE;
	opts->load_CA = 0;
	opts->save_CA = 0;
	opts->CAInputFilename = "";
	opts->CAOutputFilename = "";
	opts->GenTopology = 0;
	opts->GenCA = 1;
	opts->N = 512;
	opts->k = 3;
	opts->GOL = 0;
	opts->ECA = 1;
	opts->numStates = 2;
	opts->ruletype = DEFAULT_RULE_TYPE;
	opts->ICtype = DEFAULT_IC_TYPE;
	opts->rule = 110;
	opts->windowSize = 1024;
	opts->start_t = 0;
	opts->end_t = 1024;
	opts->visualise = 1;
	opts->do_ShannonE = 0;
	opts->do_WordE = 0;
	opts->do_InputE = 0;
	opts->do_Lambda = 0;
	opts->do_Z = 0;
	opts->do_G = 0;
	opts->do_Pr = 0;
	opts->do_R = 0;
	opts->range[0] = 0;
	opts->range[1] = 2;
	opts->numSamples = 1000;
}

/*PrintError(): Print Error messages*/
/*void PrintError(char err_code,void *err_data)
{
	switch (err_code)
	{
		case MEM_ERROR:
			fprintf(stderr,"ERROR: Memory allocation failed! code [%d]\n",err_code);
			break;
		case INVALID_OPTION:
			fprintf(stderr,"ERROR: Inavild use of option -%s! code [%d]\n",(char*)err_data,err_code);
			break;
		case UNKNOWN_OPTION:
			fprintf(stderr,"ERROR: Unknown option %s! code [%d]\n",(char*)err_data,err_code);
			break;
		default:
			fprintf(stderr,"ERROR: Dunno what happened though? Weird...\n");
			break;
	}
}*/

/* ParseCommandLineArgs(): parses the command line arguments and returns a struct
 *                         of user options. 
 */
CL_Options* ParseCommandLineArgs(int argc, char **argv)
{
	int i;
	CL_Options* CL_opt;
	
	if(!(CL_opt = (CL_Options*)malloc(sizeof(CL_Options))))
	{
		return NULL;
	}
	
	InitCL_Options(CL_opt);
	
	for (i=1;i<argc;i++)
	{
		/*short format options -abcd*/
		if (argv[i][0] == '-' && argv[i][1] != '-')
		{
			int j;
			char *optslist;
			j=1;
			optslist = argv[i];
			while (optslist[j] != '\0')
			{
				
				switch (optslist[j])
				{
					case 'i':
						CL_opt->mode = GCALAB_TEXT_MODE;
						break;
					case 'b':
						CL_opt->mode = GCALAB_BATCH_MODE;
						break;
					case 'l':
						if (optslist[j+1] == '\0')
						{
							CL_opt->load_CA = 1;
							CL_opt->CAInputFilename = argv[++i];	 
						}
						else
						{
						//	PrintError(INVALID_OPTION,(void *)optslist);
							GCALab_PrintUsage();
							return NULL;
						}
						break;
					case 'o':
						if (optslist[j+1] == '\0')
						{
							CL_opt->save_CA = 1;
							CL_opt->CAOutputFilename = argv[++i];	
						}
						else
						{
						//	PrintError(INVALID_OPTION,(void *)optslist);
							GCALab_PrintUsage();
							return NULL;
						}
						break;
					case 'E':
						if (optslist[j+1] == '\0')
						{
							CL_opt->start_t = (unsigned int)atoi(argv[++i]);
							CL_opt->end_t = (unsigned int)atoi(argv[++i]);
						}
						else
						{
						//	PrintError(INVALID_OPTION,(void *)optslist);
							GCALab_PrintUsage();
							return NULL;
						}
						break;
					case 'm':
						if (optslist[j+1] == '\0')
						{
							CL_opt->GenTopology = 1;
							CL_opt->type = argv[++i][0]; 
							CL_opt->N = (unsigned int)atoi(argv[++i]);
							CL_opt->k = (unsigned int)atoi(argv[++i]);
						}
						else
						{
						//	PrintError(INVALID_OPTION,(void *)optslist);
							GCALab_PrintUsage();
							return NULL;
						}
						break;
					case 'g':
						CL_opt->GenCA = 1;
						CL_opt->GOL = 1;
						CL_opt->N = (unsigned int)atoi(argv[++i]);
						break;
					case 'e':
						CL_opt->GenCA = 1;
						CL_opt->ECA = 1;
						CL_opt->rule = (unsigned int)atoi(argv[++i]);
						CL_opt->N = (unsigned int)atoi(argv[++i]);
						break;
					case 'w':
						if (optslist[j+1] == '\0')
						{
							CL_opt->GenCA = 1;
							CL_opt->windowSize = (unsigned int)atoi(argv[++i]);
						}
						else
						{
						//	PrintError(INVALID_OPTION,(void *)optslist);
							GCALab_PrintUsage();
							return NULL;
						}
						break;
					case 't':
						if (optslist[j+1] == '\0')
						{
							CL_opt->GenCA = 1;
							CL_opt->ruletype = (unsigned char)atoi(argv[++i]);
						}
						else
						{
						//	PrintError(INVALID_OPTION,(void *)optslist);
							GCALab_PrintUsage();
							return NULL;
						}
						break;
					case 'r':
						if (optslist[j+1] == '\0')
						{
							CL_opt->GenCA = 1;
							CL_opt->rule = (unsigned int)atoi(argv[++i]);
						}
						else
						{
						//	PrintError(INVALID_OPTION,(void *)optslist);
							GCALab_PrintUsage();
							return NULL;
						}
						break;
					case 's':
						if (optslist[j+1] == '\0')
						{
							CL_opt->GenCA = 1;
							CL_opt->numStates = (unsigned char)atoi(argv[++i]);
						}
						else
						{
						//	PrintError(INVALID_OPTION,(void *)optslist);
							GCALab_PrintUsage();
							return NULL;
						}
						break;
					case 'I':
						CL_opt->do_InputE = 1;
						break;
					case 'S':
						CL_opt->do_ShannonE = 1;
						break;
					case 'W':
						CL_opt->do_WordE = 1;
						break;
					case 'L':
						CL_opt->do_Lambda = 1;
						break;
					case 'Z':
						CL_opt->do_Z = 1;
						break;
					case 'G':
						CL_opt->do_G = 1;
						break;
					case 'P':
						CL_opt->do_Pr = 1;
						CL_opt->range[0] = (chunk)atoi(argv[++i]);
						CL_opt->range[1] = (chunk)atoi(argv[++i]);
						break;
					case 'R':
						CL_opt->numSamples = atoi(argv[++i]);
						break;
					default:
						//PrintError(UNKNOWN_OPTION,(void *)optslist);
						GCALab_PrintUsage();
						return NULL;
						break;
				}
				
				j++;
			}
		} /*long format options --some_option*/
		else if(argv[i][0] == '-' && argv[i][1] == '-')
		{
			if (!strcmp(argv[i],"--interactive"))
			{
				CL_opt->mode = GCALAB_TEXT_MODE;
			}
			else if (!strcmp(argv[i],"--batch"))
			{
				CL_opt->mode = GCALAB_BATCH_MODE;
			}
			else if (!strcmp(argv[i],"--load"))
			{
				CL_opt->load_CA = 1;
				CL_opt->CAInputFilename = argv[++i];	
			}
			else if(!strcmp(argv[i],"--save"))
			{
				CL_opt->save_CA = 1;
				CL_opt->CAOutputFilename = argv[++i];	
			}
			else if(!strcmp(argv[i],"--simulate"))
			{
				CL_opt->start_t = (unsigned int)atoi(argv[++i]);
				CL_opt->end_t = (unsigned int)atoi(argv[++i]);
			}
			else if(!strcmp(argv[i],"--topology"))
			{
				CL_opt->GenTopology = 1;
				CL_opt->type = argv[++i][0]; 
				CL_opt->N = (unsigned int)atoi(argv[++i]);
				CL_opt->k = (unsigned int)atoi(argv[++i]);
			}
			else if (!strcmp(argv[i],"--GOL"))
			{
				CL_opt->GenCA = 1;
				CL_opt->GOL = 1;
				CL_opt->N = (unsigned int)atoi(argv[++i]);
			}
			else if (!strcmp(argv[i],"--ECA"))
			{
				CL_opt->GenCA = 1;
				CL_opt->ECA = 1;
				CL_opt->rule = (unsigned int)atoi(argv[++i]);
				CL_opt->N = (unsigned int)atoi(argv[++i]);
			}
			else if (!strcmp(argv[i],"--window-size"))
			{
				CL_opt->GenCA = 1;
				CL_opt->windowSize = (unsigned int)atoi(argv[++i]);
			}
			else if(!strcmp(argv[i],"--ruletype"))
			{
				CL_opt->GenCA = 1;
				CL_opt->ruletype = (unsigned char)atoi(argv[++i]);
			}
			else if (!strcmp(argv[i],"--rule"))
			{
				CL_opt->GenCA = 1;
				CL_opt->rule = (unsigned int)atoi(argv[++i]);
			}
			else if (!strcmp(argv[i],"--num-States"))
			{
				CL_opt->GenCA = 1;
				CL_opt->numStates = (unsigned char)atoi(argv[++i]);
			}
			else if (!strcmp(argv[i],"--Input-Entropy"))
			{
				CL_opt->do_InputE = 1;
			}
			else if (!strcmp(argv[i],"--Shannon-Entropy"))
			{
				CL_opt->do_ShannonE = 1;
			}
			else if (!strcmp(argv[i],"--Word-Entropy"))
			{
				CL_opt->do_WordE = 1;
			}
			else if (!strcmp(argv[i],"--Lambda-parameter"))
			{
				CL_opt->do_Lambda = 1;
			}
			else if (!strcmp(argv[i],"--Z-parameter"))
			{
				CL_opt->do_Z = 1;
			}
			else if (!strcmp(argv[i],"--G-density"))
			{
				CL_opt->do_G = 1;
			}
			else if (!strcmp(argv[i],"--Exact-Probs"))
			{
				CL_opt->do_Pr = 1;
				CL_opt->range[0] = (chunk)atoi(argv[++i]);
				CL_opt->range[1] = (chunk)atoi(argv[++i]);
			}
			else if (!strcmp(argv[i],"--Random-Sample-Number"))
			{
				
				CL_opt->numSamples = atoi(argv[++i]);
			}
			else /*unknown option*/
			{
				//PrintError(INVALID_OPTION,(void *)argv[i]);
				GCALab_PrintUsage();
				return NULL;
			}
			
		}
	}
	
	return CL_opt;	
}

char GCALab_NewWorkSpace(int GCALimit)
{
	return 0;
}

char GCALab_QueueCommand(unsigned char ws_id,unsigned int command_id,unsigned int target_id,void *params)
{
	return 0;
}

char GCALab_DequeueCommand(unsigned char ws_id,unsigned int index)
{
	return 0;
}

char GCALab_ProcessCommandQueue(unsigned char ws_id)
{
	return 0;
}

char GCALab_PauseCommandQueue(unsigned char ws_id)
{
	return 0;
}

unsigned int GCALab_GetState(unsigned char ws_id)
{
	return 0;
}

void GCALab_SetState(unsigned char ws_id,unsigned int state)
{
	return;
}

/* GCALab_ShutDown(): Nicely and humainly kills the session
 */
void GCALab_ShutDown(char rc)
{
	/*TODO: check if any workspaces are in processing state*/
	/*if so then prompt the user*/
	pthread_exit((void*)(long long)rc);
}

void GCALab_PrintHelp(void)
{
	return;
}

char GCALab_PrintWorkSpace(unsigned char ws_id)
{
	return 0;
}

char GCALab_ListWorkSpaces(void)
{
	return 0;
}
