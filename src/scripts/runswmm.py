#!/usr/bin/python3

import glob, os, sys, shutil
import multiprocessing
import subprocess
import tempfile
import time
import smtplib
import argparse
from email.mime.text import MIMEText

EMAIL = "christian.mikovits@uibk.ac.at"
SMTP = "smtp.uibk.ac.at"

def create_argparser():
    parser = argparse.ArgumentParser(description='runs swmm5 processes in parallel\n')
    parser.add_argument('-p', '--procs', type=int, default=multiprocessing.cpu_count()-1, dest='max_procs',
    					help='how many processors should be used,\n\t default and maximum is number of processors - 1')
    parser.add_argument('-e', '--email', default=EMAIL, dest='email',
    					help='email address for the finish notification')
    parser.add_argument('-s', '--smtp', default=SMTP, dest='smtp',
    					help='smtp server to send the message')
    parser.add_argument('-r', '--readdir', default=os.getcwd(), dest='readdir',
    					help='sets the ')
    parser.add_argument('-w', '--workdir', default=os.getcwd(), dest='workdir',
    					help='sets the simulation and report directory')
    parser.add_argument('--clean', dest='cleanworkdir', action='store_true',
    					help='')
    parser.add_argument('--resume', dest='resume', action='store_true',
    					help='')
    parser.add_argument('--run', dest='run', action='store_true',
    					help='')

    return parser

def execswmm(swmm5bin, simname, cursimdir, perc):
	binextender = "./"
	if sys.platform == 'win32':
		binextender == ""

	cmd = (binextender+swmm5bin,simname+".inp",simname+".rep")
	print("%s%%: Processing %s" % (perc, simname))
	sp = subprocess.call(cmd,cwd=cursimdir,stdout=subprocess.DEVNULL)
	donefile = os.path.join(cursimdir,simname+".done")
	open(donefile, 'x').close()
	return 0
	
def execswmm_parallel(params):
	return execswmm(*params)

def sendmessage():
	msg = MIMEText("")
	msg['Subject'] = "runswmm just finished"
	msg['From'] = EMAIL
	msg['To'] = EMAIL
	c = smtplib.SMTP(SMTP)
	c.send_message(msg)
	c.quit()
	
def create_simulations(readdir, workdir, swmm5bin):

	inpdir = os.path.join(readdir,"inpfiles")
	datdir = os.path.join(readdir,"datfiles")
	simdirs = os.path.join(workdir,"simdirs")
	swmm5loc = os.path.join(readdir,swmm5bin)
	
	inpfiles = glob.glob(inpdirs+"/*inp")

	if len(inpfiles) == 0:
		print("no inputfiles found")
		sys.exit(0)
	
	datfiles = glob.glob(datdirs+"/*dat")
	if len(datfiles) == 0:
		print("no rainfiles found")
		sys.exit(0)

	simulations = list()
	
	for inpfile in inpfiles:
		for datfile in datfiles:
			inpname=os.path.split(inpfile)[1].split('.')[0]
			datname=os.path.split(datfile)[1].split('.')[0]
			simname=inpname+'_'+datname
		
			cursimdir = os.path.join(simdirs,simname)
			curswmmbin = os.path.join(cursimdir,swmm5bin)
			curinpfile = os.path.join(cursimdir,simname+".inp")
			curdatfile = os.path.join(cursimdir,"rain.dat")
			currepfile = os.path.join(cursimdir,simname+".rep")
			
			if args.resume == True:
				donefile = os.path.join(cursimdir,simname+".done")
				if os.path.isfile(donefile):
					continue
			try:
				shutil.rmtree(cursimdir)
			except:
				pass
			os.mkdir(cursimdir)
			shutil.copy(swmm5loc,cursimdir)
			shutil.copyfile(inpfile,curinpfile)
			shutil.copyfile(datfile,curdatfile)
			
			siminfo = (simname,cursimdir)
			simulations.append(siminfo)

	return simulations
	
def cleanworkdir(workdir):
	try:
		shutil.rmtree(workdir)
	except:
		pass
	
	os.mkdir(workdir)
	
	return 0

def main():

	parser = create_argparser()
	args = parser.parse_args()
	
	readdir = args.readdir
	workdir = args.workdir
	max_procs = args.max_procs
	
	swmm5bin = "swmm5"
	if sys.platform == 'win32':
		swmm5bin = "swmm5.exe"

	if args.run != True:
		print(args)
		sys.exit()
		
	if args.cleanworkdir == True:
		cleanworkdir(workdir)
		
	pool = multiprocessing.Pool(processes=max_procs)

		
	simulations = create_simulations(inputdir, workdir, swmm5bin)

	totaliterations=len(simulations)
	
	print("Starting a total of %s itarations using %s of %s CPUs" % (totaliterations,max_procs,multiprocessing.cpu_count()))
				
	for index, params in enumerate(simulations):
		perc = int(index/totaliterations*100)
		params = [(swmm5bin, params[0], params[1], perc)]
		
		res = pool.map_async(execswmm_parallel, params)
			
	if totaliterations == 0:
		sys.exit("Nothing to do")
	
	res.get()
	print("FINISHED")
	sendmessage()

if __name__ == "__main__":
	main()
