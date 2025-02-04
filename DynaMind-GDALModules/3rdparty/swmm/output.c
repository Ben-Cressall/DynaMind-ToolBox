//-----------------------------------------------------------------------------
//   output.c
//
//   Project:  EPA SWMM5
//   Version:  5.0
//   Date:     6/19/07   (Build 5.0.010)
//             2/4/08    (Build 5.0.012)
//             1/21/09   (Build 5.0.014)
//             4/10/09   (Build 5.0.015)
//             6/22/09   (Build 5.0.016)
//   Author:   L. Rossman
//
//   Binary output file access functions.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h>
#include "headers.h"
#include <math.h>


// Definition of 4-byte integer, 4-byte real and 8-byte real types             //(5.0.014 - LR)
#define INT4  int
#define REAL4 float
#define REAL8 double

enum InputDataType {INPUT_TYPE_CODE, INPUT_AREA, INPUT_INVERT, INPUT_MAX_DEPTH,
                    INPUT_OFFSET, INPUT_LENGTH};

//-----------------------------------------------------------------------------
//  Shared variables    
//-----------------------------------------------------------------------------
static INT4      IDStartPos;           // starting file position of ID names
static INT4      InputStartPos;        // starting file position of input data
static INT4      OutputStartPos;       // starting file position of output data
static INT4      BytesPerPeriod;       // bytes saved per simulation time period
static INT4      NsubcatchResults;     // number of subcatchment output variables
static INT4      NnodeResults;         // number of node output variables
static INT4      NlinkResults;         // number of link output variables
static INT4      NumSubcatch;          // number of subcatchments reported on  //(5.0.014 - LR)
static INT4      NumNodes;             // number of nodes reported on          //(5.0.014 - LR)
static INT4      NumLinks;             // number of links reported on          //(5.0.014 - LR)
static REAL4     SysResults[MAX_SYS_RESULTS];    // values of system output vars.

//-----------------------------------------------------------------------------
//  Exportable variables (shared with report.c)
//-----------------------------------------------------------------------------
REAL4*           SubcatchResults;
REAL4*           NodeResults;
REAL4*           LinkResults;

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static void output_openOutFile(void);
static void output_saveID(char* id, FILE* file);
static void output_saveSubcatchResults(double reportTime, FILE* file);
static void output_saveNodeResults(double reportTime, FILE* file);
static void output_saveLinkResults(double reportTime, FILE* file);

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//  output_open                   (called by swmm_start in swmm5.c)
//  output_end                    (called by swmm_end in swmm5.c)
//  output_close                  (called by swmm_close in swmm5.c)
//  output_saveResults            (called by swmm_step in swmm5.c)
//  output_checkFileSize          (called by swmm_report)                      //(5.0.015 - LR)
//  output_readDateTime           (called by routines in report.c)
//  output_readSubcatchResults    (called by report_Subcatchments)
//  output_readNodeResults        (called by report_Nodes)
//  output_readLinkResults        (called by report_Links)


//=============================================================================

int output_open()
//
//  Input:   none
//  Output:  returns an error code
//  Purpose: writes basic project data to binary output file.
//
{
    int   nPolluts = Nobjects[POLLUT];
    int   j;
    int   m;
    INT4  k;
    REAL4 x;
    REAL8 z;

    // --- open binary output file
    output_openOutFile();
    if ( ErrorCode ) return ErrorCode;

    // --- subcatchment results consist of Rainfall, Snowdepth, Losses, Runoff, 
    //     GW Flow, GW Elev, and Washoff
    NsubcatchResults = MAX_SUBCATCH_RESULTS - 1 + nPolluts;

    // --- node results consist of Depth, Head, Volume, Lateral Inflow,
    //     Total Inflow, Overflow and Quality
    NnodeResults = MAX_NODE_RESULTS - 1 + nPolluts;

    // --- link results consist of Depth, Flow, Velocity, Froude No.,
    //     Capacity and Quality
    NlinkResults = MAX_LINK_RESULTS - 1 + nPolluts;

    // --- get number of objects reported on                                   //(5.0.014 - LR)
    NumSubcatch = 0;
    NumNodes = 0;
    NumLinks = 0;
    for (j=0; j<Nobjects[SUBCATCH]; j++) if (Subcatch[j].rptFlag) NumSubcatch++;
    for (j=0; j<Nobjects[NODE]; j++) if (Node[j].rptFlag) NumNodes++;
    for (j=0; j<Nobjects[LINK]; j++) if (Link[j].rptFlag) NumLinks++;


    BytesPerPeriod = sizeof(REAL8)                                             //(5.0.014 - LR)
        + NumSubcatch * NsubcatchResults * sizeof(REAL4)                       //(5.0.014 - LR)
        + NumNodes * NnodeResults * sizeof(REAL4)                              //(5.0.014 - LR)
        + NumLinks * NlinkResults * sizeof(REAL4)                              //(5.0.014 - LR)
        + MAX_SYS_RESULTS * sizeof(REAL4);
    Nperiods = 0;

    SubcatchResults = NULL;
    NodeResults = NULL;
    LinkResults = NULL;
    SubcatchResults = (REAL4 *) calloc(NsubcatchResults, sizeof(REAL4));
    NodeResults = (REAL4 *) calloc(NnodeResults, sizeof(REAL4));
    LinkResults = (REAL4 *) calloc(NlinkResults, sizeof(REAL4));
    if ( !SubcatchResults || !NodeResults || !LinkResults )
    {
        report_writeErrorMsg(ERR_MEMORY, "");
        return ErrorCode;
    }

    fseek(Fout.file, 0, SEEK_SET);
    k = MAGICNUMBER;
    fwrite(&k, sizeof(INT4), 1, Fout.file);   // Magic number
    k = VERSION;
    fwrite(&k, sizeof(INT4), 1, Fout.file);   // Version number
    k = FlowUnits;
    fwrite(&k, sizeof(INT4), 1, Fout.file);   // Flow units
    k = NumSubcatch;                                                           //(5.0.014 - LR)
    fwrite(&k, sizeof(INT4), 1, Fout.file);   // # subcatchments
    k = NumNodes;                                                              //(5.0.014 - LR)
    fwrite(&k, sizeof(INT4), 1, Fout.file);   // # nodes
    k = NumLinks;                                                              //(5.0.014 - LR) 
    fwrite(&k, sizeof(INT4), 1, Fout.file);   // # links
    k = Nobjects[POLLUT];
    fwrite(&k, sizeof(INT4), 1, Fout.file);   // # pollutants

    // --- save ID names of subcatchments, nodes, links, & pollutants          //(5.0.014 - LR)
    IDStartPos = ftell(Fout.file);
    for (j=0; j<Nobjects[SUBCATCH]; j++)
    {
        if ( Subcatch[j].rptFlag ) output_saveID(Subcatch[j].ID, Fout.file);
    }
    for (j=0; j<Nobjects[NODE];     j++)
    {
        if ( Node[j].rptFlag ) output_saveID(Node[j].ID, Fout.file);
    }
    for (j=0; j<Nobjects[LINK];     j++)
    {
        if ( Link[j].rptFlag ) output_saveID(Link[j].ID, Fout.file);
    }
    for (j=0; j<Nobjects[POLLUT];   j++) output_saveID(Pollut[j].ID, Fout.file);

    // --- save codes of pollutant concentration units
    for (j=0; j<Nobjects[POLLUT]; j++)
    {
        k = Pollut[j].units;
        fwrite(&k, sizeof(INT4), 1, Fout.file);
    }

    InputStartPos = ftell(Fout.file);

    // --- save subcatchment area
    k = 1;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_AREA;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    for (j=0; j<Nobjects[SUBCATCH]; j++)
    {
         if ( !Subcatch[j].rptFlag ) continue;                                 //(5.0.014 - LR)
         SubcatchResults[0] = (REAL4)(Subcatch[j].area * UCF(LANDAREA));
         fwrite(&SubcatchResults[0], sizeof(REAL4), 1, Fout.file);
    }

    // --- save node type, invert, & max. depth
    k = 3;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_TYPE_CODE;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_INVERT;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_MAX_DEPTH;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    for (j=0; j<Nobjects[NODE]; j++)
    {
        if ( !Node[j].rptFlag ) continue;                                      //(5.0.014 - LR)
        k = Node[j].type;
        NodeResults[0] = (REAL4)(Node[j].invertElev * UCF(LENGTH));
        NodeResults[1] = (REAL4)(Node[j].fullDepth * UCF(LENGTH));
        fwrite(&k, sizeof(INT4), 1, Fout.file);
        fwrite(NodeResults, sizeof(REAL4), 2, Fout.file);
    }

    // --- save link type, offsets, max. depth, & length
    k = 5;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_TYPE_CODE;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_OFFSET;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_OFFSET;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_MAX_DEPTH;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = INPUT_LENGTH;
    fwrite(&k, sizeof(INT4), 1, Fout.file);

    for (j=0; j<Nobjects[LINK]; j++)
    {
        if ( !Link[j].rptFlag ) continue;                                      //(5.0.014 - LR)
        k = Link[j].type;
        if ( k == PUMP )
        {
            for (m=0; m<4; m++) LinkResults[m] = 0.0f;
        }
        else
        {
            LinkResults[0] = (REAL4)(Link[j].offset1 * UCF(LENGTH));           //(5.0.012 - LR)
            LinkResults[1] = (REAL4)(Link[j].offset2 * UCF(LENGTH));           //(5.0.012 - LR)
            if ( Link[j].direction < 0 )
            {
                x = LinkResults[0];
                LinkResults[0] = LinkResults[1];
                LinkResults[1] = x;
            }
            if ( k == OUTLET ) LinkResults[2] = 0.0f;
            else LinkResults[2] = (REAL4)(Link[j].xsect.yFull * UCF(LENGTH));
            if ( k == CONDUIT )
            {
                m = Link[j].subIndex;
                LinkResults[3] = (REAL4)(Conduit[m].length * UCF(LENGTH));
            }
            else LinkResults[3] = 0.0f;
        }
        fwrite(&k, sizeof(INT4), 1, Fout.file);
        fwrite(LinkResults, sizeof(REAL4), 4, Fout.file);
    }

    // --- save number & codes of subcatchment result variables
    k = NsubcatchResults;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = SUBCATCH_RAINFALL;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = SUBCATCH_SNOWDEPTH;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = SUBCATCH_LOSSES;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = SUBCATCH_RUNOFF;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = SUBCATCH_GW_FLOW;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = SUBCATCH_GW_ELEV;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    for (j=0; j<nPolluts; j++)
    {
        k = SUBCATCH_WASHOFF + j;
        fwrite(&k, sizeof(INT4), 1, Fout.file);
    }

    // --- save number & codes of node result variables
    k = NnodeResults;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = NODE_DEPTH;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = NODE_HEAD;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = NODE_VOLUME;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = NODE_LATFLOW;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = NODE_INFLOW;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = NODE_OVERFLOW;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    for (j=0; j<nPolluts; j++)
    {
        k = NODE_QUAL + j;
        fwrite(&k, sizeof(INT4), 1, Fout.file);
    }

    // --- save number & codes of link result variables
    k = NlinkResults;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = LINK_FLOW;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = LINK_DEPTH;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = LINK_VELOCITY;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = LINK_FROUDE;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = LINK_CAPACITY;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    for (j=0; j<nPolluts; j++)
    {
        k = LINK_QUAL + j;
        fwrite(&k, sizeof(INT4), 1, Fout.file);
    }

    // --- save number & codes of system result variables
    k = MAX_SYS_RESULTS;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    for (k=0; k<MAX_SYS_RESULTS; k++) fwrite(&k, sizeof(INT4), 1, Fout.file);

    // --- save starting report date & report step
    //     (if reporting start date > simulation start date then               //(5.0.014 - LR)
    //      make saved starting report date one reporting period               //(5.0.014 - LR)
    //      prior to the date of the first reported result)                    //(5.0.014 - LR)
    z = (double)ReportStep/86400.0;                                            //(5.0.014 - LR)
    if ( StartDateTime + z > ReportStart ) z = StartDateTime;                  //(5.0.014 - LR)
    else                                                                       //(5.0.014 - LR)
    {                                                                          //(5.0.014 - LR)
        z = floor((ReportStart - StartDateTime)/z) - 1.0;                      //(5.0.014 - LR)
        z = StartDateTime + z*(double)ReportStep/86400.0;                      //(5.0.014 - LR)
    }                                                                          //(5.0.014 - LR)
    fwrite(&z, sizeof(REAL8), 1, Fout.file);
    k = ReportStep;
    if ( fwrite(&k, sizeof(INT4), 1, Fout.file) < 1)
    {
        report_writeErrorMsg(ERR_OUT_WRITE, "");
        return ErrorCode;
    }
    OutputStartPos = ftell(Fout.file);
    if ( Fout.mode == SCRATCH_FILE ) output_checkFileSize();                   //(5.0.015 - LR)
    return ErrorCode;
}

//=============================================================================

////  New function added for release 5.0.015.  ////                            //(5.0.015 - LR)

void  output_checkFileSize()
//
//  Input:   none
//  Output:  none
//  Purpose: checks if the size of the binary output file will be too big
//           to access using an integer file pointer variable.
//
{
    if ( RptFlags.subcatchments != NONE ||
         RptFlags.nodes != NONE ||
         RptFlags.links != NONE )
    {
        if ( (double)OutputStartPos + (double)BytesPerPeriod * TotalDuration
             / 1000.0 / (double)ReportStep >= (double)MAXFILESIZE )
        {
            report_writeErrorMsg(ERR_FILE_SIZE, "");
        }
    }
}


//=============================================================================

void output_openOutFile()
//
//  Input:   none
//  Output:  none
//  Purpose: opens a project's binary output file.
//
{
    // --- close output file if already opened
    if (Fout.file != NULL) fclose(Fout.file); 

    // --- else if file name supplied then set file mode to SAVE
    else if (strlen(Fout.name) != 0) Fout.mode = SAVE_FILE;

    // --- otherwise set file mode to SCRATCH & generate a name
    else
    {
        Fout.mode = SCRATCH_FILE;
        getTmpName(Fout.name);
    }

    // --- try to open the file
    if ( (Fout.file = fopen(Fout.name, "w+b")) == NULL)
    {
        writecon(FMT14);
        ErrorCode = ERR_OUT_FILE;
    }
}

//=============================================================================

void output_saveResults(double reportTime)
//
//  Input:   reportTime = elapsed simulation time (millisec)
//  Output:  none
//  Purpose: writes computed results for current report time to binary file.
//
{
    int i;
    DateTime reportDate = getDateTime(reportTime);
    REAL8 date;

    if ( reportDate < ReportStart ) return;
    for (i=0; i<MAX_SYS_RESULTS; i++) SysResults[i] = 0.0f;
    date = reportDate;
    fwrite(&date, sizeof(REAL8), 1, Fout.file);
    if (Nobjects[SUBCATCH] > 0)
        output_saveSubcatchResults(reportTime, Fout.file);
    if (Nobjects[NODE] > 0)
        output_saveNodeResults(reportTime, Fout.file);
    if (Nobjects[LINK] > 0)
        output_saveLinkResults(reportTime, Fout.file);
    fwrite(SysResults, sizeof(REAL4), MAX_SYS_RESULTS, Fout.file);
    if ( Foutflows.mode == SAVE_FILE && RouteModel != NO_ROUTING )             //(5.0.014 - LR)
        iface_saveOutletResults(reportDate, Foutflows.file);
    Nperiods++;
}

//=============================================================================

void output_end()
//
//  Input:   none
//  Output:  none
//  Purpose: writes closing records to binary file.
//
{
    INT4 k;
    fwrite(&IDStartPos, sizeof(INT4), 1, Fout.file);
    fwrite(&InputStartPos, sizeof(INT4), 1, Fout.file);
    fwrite(&OutputStartPos, sizeof(INT4), 1, Fout.file);
    k = Nperiods;
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = (INT4)error_getCode(ErrorCode);
    fwrite(&k, sizeof(INT4), 1, Fout.file);
    k = MAGICNUMBER;
    if (fwrite(&k, sizeof(INT4), 1, Fout.file) < 1)
    {
        report_writeErrorMsg(ERR_OUT_WRITE, "");
    }
}

//=============================================================================

void output_close()
//
//  Input:   none
//  Output:  none
//  Purpose: frees memory used for accessing the binary file.
//
{
    FREE(SubcatchResults);
    FREE(NodeResults);
    FREE(LinkResults);
}

//=============================================================================

void output_saveID(char* id, FILE* file)
//
//  Input:   id = name of an object
//           file = ptr. to binary output file
//  Output:  none
//  Purpose: writes an object's name to the binary output file.
//
{
    INT4 n = strlen(id);
    fwrite(&n, sizeof(INT4), 1, file);
    fwrite(id, sizeof(char), n, file);
}

//=============================================================================

void output_saveSubcatchResults(double reportTime, FILE* file)
//
//  Input:   reportTime = elapsed simulation time (millisec)
//           file = ptr. to binary output file
//  Output:  none
//  Purpose: writes computed subcatchment results to binary file.
//
{
    int      j;
    double   f;
    double   area;
    REAL4    totalArea = 0.0f; 
    DateTime reportDate = getDateTime(reportTime);

    // --- update reported rainfall at each rain gage
    for ( j=0; j<Nobjects[GAGE]; j++ )
    {
        gage_setReportRainfall(j, reportDate);
    }

    // --- find where current reporting time lies between latest runoff times
    f = (reportTime - OldRunoffTime) / (NewRunoffTime - OldRunoffTime);

    // --- write subcatchment results to file
    for ( j=0; j<Nobjects[SUBCATCH]; j++)
    {
        // --- retrieve interpolated results for reporting time & write to file
        subcatch_getResults(j, f, SubcatchResults);
        if ( Subcatch[j].rptFlag )                                             //(5.0.014 - LR)
            fwrite(SubcatchResults, sizeof(REAL4), NsubcatchResults, file);    //(5.0.014 - LR)

        // --- update system-wide results
        area = Subcatch[j].area * UCF(LANDAREA);
        totalArea += (REAL4)area;
        SysResults[SYS_RAINFALL] +=
            (REAL4)(SubcatchResults[SUBCATCH_RAINFALL] * area);
        SysResults[SYS_SNOWDEPTH] +=
            (REAL4)(SubcatchResults[SUBCATCH_SNOWDEPTH] * area);
        SysResults[SYS_LOSSES] +=
            (REAL4)(SubcatchResults[SUBCATCH_LOSSES] * area);
        SysResults[SYS_RUNOFF] += (REAL4)SubcatchResults[SUBCATCH_RUNOFF];
    }

    // --- normalize system-wide results to catchment area
    if ( UnitSystem == SI ) f = (5./9.) * (Temp.ta - 32.0);
    else f = Temp.ta;
    SysResults[SYS_TEMPERATURE] = (REAL4)f;
    SysResults[SYS_EVAPORATION] = (REAL4)(Evap.rate * UCF(EVAPRATE));
    SysResults[SYS_RAINFALL]  /= totalArea;
    SysResults[SYS_SNOWDEPTH] /= totalArea;
    SysResults[SYS_LOSSES]    /= totalArea;
}

//=============================================================================

void output_saveNodeResults(double reportTime, FILE* file)
//
//  Input:   reportTime = elapsed simulation time (millisec)
//           file = ptr. to binary output file
//  Output:  none
//  Purpose: writes computed node results to binary file.
//
{
    extern TRoutingTotals StepFlowTotals;  // defined in massbal.c
    int j;

    // --- find where current reporting time lies between latest routing times
    double f = (reportTime - OldRoutingTime) /
               (NewRoutingTime - OldRoutingTime);

    // --- write node results to file
    for (j=0; j<Nobjects[NODE]; j++)
    {
        // --- retrieve interpolated results for reporting time & write to file
        node_getResults(j, f, NodeResults);
        if ( Node[j].rptFlag )                                                 //(5.0.014 - LR)
            fwrite(NodeResults, sizeof(REAL4), NnodeResults, file);            //(5.0.014 - LR)

        // --- update system-wide storage volume                               //(5.0.012 - LR)
        //SysResults[SYS_FLOODING] += NodeResults[NODE_OVERFLOW];              //(5.0.012 - LR)
        SysResults[SYS_STORAGE] += NodeResults[NODE_VOLUME];
        //if ( Node[j].degree == 0 )                                           //(5.0.012 - LR)
        //{                                                                    //(5.0.012 - LR)
        //    SysResults[SYS_OUTFLOW] += NodeResults[NODE_INFLOW];             //(5.0.012 - LR)
        //}                                                                    //(5.0.012 - LR)
    }

    // --- update system-wide flows                                            //(5.0.012 - LR)
    SysResults[SYS_FLOODING] = (REAL4) (StepFlowTotals.flooding * UCF(FLOW));  //(5.0.012 - LR)
    SysResults[SYS_OUTFLOW]  = (REAL4) (StepFlowTotals.outflow * UCF(FLOW));   //(5.0.012 - LR)
    SysResults[SYS_DWFLOW] = (REAL4)(StepFlowTotals.dwInflow * UCF(FLOW));
    SysResults[SYS_GWFLOW] = (REAL4)(StepFlowTotals.gwInflow * UCF(FLOW));
    SysResults[SYS_IIFLOW] = (REAL4)(StepFlowTotals.iiInflow * UCF(FLOW));
    SysResults[SYS_EXFLOW] = (REAL4)(StepFlowTotals.exInflow * UCF(FLOW));
    SysResults[SYS_INFLOW] = SysResults[SYS_RUNOFF] +
                             SysResults[SYS_DWFLOW] +
                             SysResults[SYS_GWFLOW] +
                             SysResults[SYS_IIFLOW] +
                             SysResults[SYS_EXFLOW];
}

//=============================================================================

void output_saveLinkResults(double reportTime, FILE* file)
//
//  Input:   reportTime = elapsed simulation time (millisec)
//           file = ptr. to binary output file
//  Output:  none
//  Purpose: writes computed link results to binary file.
//
{
    int j;
    double f;
    double z;

    // --- find where current reporting time lies between latest routing times
    f = (reportTime - OldRoutingTime) / (NewRoutingTime - OldRoutingTime);

    // --- write link results to file
    for (j=0; j<Nobjects[LINK]; j++)
    {
        // --- retrieve interpolated results for reporting time & write to file
        link_getResults(j, f, LinkResults);
        if ( Link[j].rptFlag )                                                 //(5.0.014 - LR)
            fwrite(LinkResults, sizeof(REAL4), NlinkResults, file);            //(5.0.014 - LR)

        // --- update system-wide results
        z = ((1.0-f)*Link[j].oldVolume + f*Link[j].newVolume) * UCF(VOLUME);
        SysResults[SYS_STORAGE] += (REAL4)z;
    }
}

//=============================================================================

void output_readDateTime(int period, DateTime* days)
//
//  Input:   period = index of reporting time period
//  Output:  days = date/time value
//  Purpose: retrieves the date/time for a specific reporting period
//           from the binary output file.
//
{
    INT4 bytePos = OutputStartPos + (period-1)*BytesPerPeriod;
    fseek(Fout.file, bytePos, SEEK_SET);
    *days = NO_DATE;
    fread(days, sizeof(REAL8), 1, Fout.file);
}

//=============================================================================

void output_readSubcatchResults(int period, int index)
//
//  Input:   period = index of reporting time period
//           index = subcatchment index
//  Output:  none
//  Purpose: reads computed results for a subcatchment at a specific time
//           period.
//
{
    INT4 bytePos = OutputStartPos + (period-1)*BytesPerPeriod;
    bytePos += sizeof(REAL8) + index*NsubcatchResults*sizeof(REAL4);
    fseek(Fout.file, bytePos, SEEK_SET);
    fread(SubcatchResults, sizeof(REAL4), NsubcatchResults, Fout.file);
}

//=============================================================================

void output_readNodeResults(int period, int index)
//
//  Input:   period = index of reporting time period
//           index = node index
//  Output:  none
//  Purpose: reads computed results for a node at a specific time period.
//
{
    INT4 bytePos = OutputStartPos + (period-1)*BytesPerPeriod;
    bytePos += sizeof(REAL8) + NumSubcatch*NsubcatchResults*sizeof(REAL4);     //(5.0.014 - LR)
    bytePos += index*NnodeResults*sizeof(REAL4);
    fseek(Fout.file, bytePos, SEEK_SET);
    fread(NodeResults, sizeof(REAL4), NnodeResults, Fout.file);
}

//=============================================================================

void output_readLinkResults(int period, int index)
//
//  Input:   period = index of reporting time period
//           index = link index
//  Output:  none
//  Purpose: reads computed results for a link at a specific time period.
//
{
    INT4 bytePos = OutputStartPos + (period-1)*BytesPerPeriod;
    bytePos += sizeof(REAL8) + NumSubcatch*NsubcatchResults*sizeof(REAL4);     //(5.0.014 - LR)
    bytePos += NumNodes*NnodeResults*sizeof(REAL4);                            //(5.0.014 - LR)
    bytePos += index*NlinkResults*sizeof(REAL4);
    fseek(Fout.file, bytePos, SEEK_SET);
    fread(LinkResults, sizeof(REAL4), NlinkResults, Fout.file);
    fread(SysResults, sizeof(REAL4), MAX_SYS_RESULTS, Fout.file);
}

//=============================================================================
    
