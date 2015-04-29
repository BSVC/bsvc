#
# gal.py : GNU Assembler listing
#
# DJ, September 1996
#
# Make a concatenation of listing files produced
# by the GNU assembler in order to build
# a complete listing suitable for BSVC.
# In order to get function addresses quite right,
# the mapping of the linker is used.
# Usage: python gal.py {mapping} {listing files}
#
# This version addresses the 'binutils-2.7' new (?) map format.
#
# * USUAL DISCLAIMER... NO WARRANTY FOR ANY PURPOSE. *
#

import sys
import string
import regex

#-----------------------------------------------------------------
debug=10

#-----------------------------------------------------------------
class c_function:
	def __init__(self, name, address):
		self.name=name
		self.address=address

#-----------------------------------------------------------------
class c_variable:
	def __init__(self, name, address):
		self.name=name
		self.address=address
				
#-----------------------------------------------------------------
class c_map:
	def __init__(self):
		self.functions=[]
		self.variables=[]
	   
	def load(self, file_name):
		global debug
		if(debug>=1):
			print "Processing %s" % file_name
		try:
			mfile=open(file_name,"r")
		except IOError:
			print "ERROR: [map] Can't open file %s for reading" % file_name
			return 1
		
		state=0
		while(1):
			try:
				s=mfile.readline()
			except IOError:
				print "ERROR: [map] cannot read file %s" % file_name
			if(s==""):
				break
			if(s=="**LINK EDITOR MEMORY MAP**\n"):
				state=1
				if(debug>=16):
					print "map: link editor memory map found"
				break
			# new version
			if(s=="Linker script and memory map\n"):
				state=2
				if(debug>=16):
					print "map: link editor memory map found"
				break

		if(state==0):
			print "ERROR: [map] link editor memory map not found"
			try:
				mfile.close()
			except IOError:
				pass
			return 2

		if(state==1):
			while(1):
				try:
					s=mfile.readline()
				except IOError:
					print "ERROR: [map] cannot read file %s" % file_name
				if(s==""):
					break
				if(s=="\n"):
					# ignore blank lines
					continue
			
				i1=string.find(s,"from")
				if(i1>=0):
					x=string.split(s)
					if(len(x)<2):
						print "Warning: bad \"from\" instruction (1)" 
						continue
					i2=string.find(x[1],"(")
					if(i2==-1):
						print "Warning: bad \"from\" instruction (2)"
						continue		
					module=x[1][0:i2]
					if(debug>=16):
						print "Module =", module
					i3=string.find(x[1],")")
					if(i3==-1):
						print "Warning: bad \"from\" instruction (3)"
						continue		
					segment=x[1][i2+1:i3]
					if(debug>=16):
						print "Segment =", segment
					continue
									
				i1=string.find(s,"output")
				if(i1==0):
					if(debug>=16):
						print "\"output\" line ignored" 
					continue
	
				i1=string.find(s,"section")
				if(i1==0):
					if(debug>=16):
						print "\"section\" line ignored" 
					continue
	
				i1=string.find(s,".data")
				if(i1>=0):
					if(debug>=16):
						print "\".data\" line ignored" 
					continue
	
				i1=string.find(s,".text")
				if(i1>=0):
					if(debug>=16):
						print "\".text\" line ignored" 
					continue
	
				i1=string.find(s,".bss")
				if(i1>=0):
					if(debug>=16):
						print "\".bss\" line ignored" 
					continue
	
				i1=string.find(s,"LOAD")
				if(i1>=0):
					if(debug>=16):
						print "\"LOAD\" line ignored"
					continue
							
				x=string.split(s)
				if(len(x)!=2):
					if(debug>=6):
						print "line ignored: \"%s\"" % s[0:-1]
					continue
				
				try:
					address=string.atoi(x[0],16)
				except ValueError:
					if(debug>=6):
						print "not an address \"%s\", line ignored" % x[1]
					continue
						
				name=x[1]
				##print "Name=%s, Address=%#x" % (name,address)
				if(segment==".text"):
					f=c_function(name,address)
					self.functions.append(f)
				elif (segment==".data"):	
					v=c_variable(name,address)
					self.variables.append(v)

		if(state==2):
			current_segment=0
			while(1):
				try:
					s=mfile.readline()
				except IOError:
					print "ERROR: [map] cannot read file %s" % file_name
				##print "READ:", s,	
				if(s==""):
					break
				if(s=="\n"):
					# ignore blank lines
					continue

				i1=string.find(s,"LOAD")
				if(i1>=0):
					continue
				i1=string.find(s,"OUTPUT")
				if(i1>=0):
					continue
				i1=string.find(s,"COMMON")
				if(i1>=0):
					continue
				
				
				z=string.split(s)
				
				if((z[0]==".data") and (len(z)>=4)):
					current_segment=1
					##print "DATA FOR MODULE", z[3]
					continue

				if((z[0]==".bss") and (len(z)>=4)):
					current_segment=1
					##print "BSS FOR MODULE", z[3]
					continue

				if((z[0]==".text") and (len(z)>=4)):
					current_segment=2
					##print "TEXT FOR MODULE", z[3]
					continue
				
				if((len(z)==2) and (current_segment!=0)):
					try:
						address=string.atoi(z[0], 16)
					except ValueError:
						if(debug>=6):
							print "not an address \"%s\", line ignored" % z[1]
						continue
					
					if(current_segment==2):
						f=c_function(z[1],address)
						self.functions.append(f)
						##print "FUNCTION:", z[1], "at", address
					elif (current_segment==1):
						v=c_variable(z[1],address)
						self.variables.append(v)
						##print "VARIABLE:", z[1], "at", address
							
									
		try:
			mfile.close()
		except IOError:
			pass
		
		return 0

	def find_function(self, n):
		f=None
		for i in self.functions:
			if(i.name==n):
				f=i
				break
		return f
			
	def find_variable(self, n):
		v=None
		for i in self.variables:
			if(i.name==n):
				v=i
				break
		return v
			
#-----------------------------------------------------------------
def process_listing(map, ofile, ifile, ifn):
	global debug
	
	print "Processing %s" % ifn
	state=0 #state says if addresses must be converted or not
	while(1):
		s=ifile.readline()

		if(s==""):
			break
		i1=string.find(s,"****")
		if(i1>=0):
			# c source (most likely!?)
			ofile.write(s)
			continue
		i1=string.find(s,".stab")
		if(i1>=0):
			# c stabs - ignore them
			###ofile.write(s)
			continue
		i1=string.find(s,"68K GAS")
		if(i1>=0):
			# header of listing (page number) - ignore them
			###ofile.write(s)
			continue
		i1=string.find(s,"DEFINED SYMBOLS")
		i2=string.find(s,"UNDEFINED SYMBOLS")
		if((i1>=0)or(i2>=0)):
			# at the end of the listing there are 2 lists: defined
			# and undefined symbols...stop there! (not any more
			# interesting stuff)
			###ofile.write(s)
			break
		
		# .globl line is "{line number} .globl {symbol name}"
		i1=string.find(s,".globl")
		if(i1==-1):
			i1=string.find(s,".global")
		if(i1>=0):
			state=0
			x=string.split(s)
			if(len(x)<2):
				if(debug>=5):
					print "Warning: malformed .globl", x,
				ofile.write(s)
				continue
			
			if(debug>=6):
				print "\tanalysing %s, line %s" % (x[2],x[0])
			f=map.find_function(x[2])
			if(f!=None):
				if(debug>=15):
					print "Function %s at %#x found" % (f.name,f.address)
				state=1
				delta=0
				###ofile.write("* FUNCTION %s at %#04x\n" % (f.name,f.address))
		 	v=map.find_variable(x[2])
			if(v!=None):
				if(debug>=15):
					print "Variable %s at %#x found" % (v.name,v.address)
				state=3
				delta=0	
				###ofile.write("* VARIABLE %s at %#04x\n" % (v.name,v.address))
			if((f==None)and(v==None)and(debug>=1)):
				print "Warning: global %s not in map!" % x[2]
										
		if(state==1):
			# the first time, inside a function			
			result=regex.match("[ ]+\([0-9]+\) \([0-9a-fA-F]+\) \([0-9a-fA-F]+\)\(.*\)",s)
			if(result>=0):
				###print "MATCH", s,
				x=string.split(s)
				# old adress is x[1]
				try:
					delta=string.atoi(x[1],16)
				except ValueError:
					print "ERROR: Bad address %s line %s" % (x[1],x[0])
					break
				new_address=hex(f.address)
				ofile.write("%s %s " % (x[0],new_address[2:]))
			   	for e in x[2:]:
					ofile.write(e+" ")				
				ofile.write("\n")
				state=2
			else:
				ofile.write(s)
				
		elif(state==2):
			# inside a function, when delta is known
			result=regex.match("[ ]+\([0-9]+\)[ ]+\([0-9a-fA-F]+\)[ ]+\([0-9a-fA-F]+\)[ ]+\(.*\)",s)
			if(result>=0):
				###print "MATCH", s,
				x=string.split(s)
				# old adress is x[1]
				try:
					new_address=hex(string.atoi(x[1],16)-delta+f.address)
				except ValueError:
					print "ERROR: Bad address %s line %s" % (x[1],x[0])
					break
				ofile.write("%s %s " % (x[0],new_address[2:]))
			   	for e in x[2:]:
					ofile.write(e+" ")				
				ofile.write("\n")
			else:
				###print "DONT MATCH", s,
				ofile.write(s)			
						
		elif (state==3):
			# variable definition
			result=regex.match("[ ]+\([0-9]+\) \([0-9a-fA-F]+\) \([0-9a-fA-F]+\)\(.*\)",s)
			if(result>=0):
				###print "MATCH", s,
				x=string.split(s)
				# old adress is x[1]
				new_address=hex(v.address)
				ofile.write("%s %s " % (x[0],new_address[2:]))
			   	for e in x[2:]:
					ofile.write(e+" ")				
				ofile.write("\n")
				#a var. is defined in 1 line: as soon as there
				# is a match, return to state 0
				state=0 
			else:
				ofile.write(s)
		else:
			###print "????", s,
			ofile.write(s)
	
	
#-----------------------------------------------------------------
def main(argv):
	if(len(argv)<2):
		print "usage %s map-file listing-files" % argv[0]
		sys.exit(-1)

	map=c_map()
	if(map.load(argv[1])):
		sys.exit(1)
		
	index=string.rfind(argv[1],'.')
	n=argv[1][0:index]+".LIS"
		
	try:
		ofile=open(n,"w")
	except IOError:
		print "ERROR: Can't open file %s for writing" % n
		sys.exit(2)
		
	for lf in argv[2:]:
		try:
			ifile=open(lf,"r")
		except IOError:
			print "ERROR: Can't open file %s for reading" % lf
			ofile.close()
			sys.exit(2)

		process_listing(map,ofile,ifile,lf)

		try:
			ifile.close()
		except IOError:
			pass

	try:
		ofile.close()
	except IOError:
		pass
			
#-----------------------------------------------------------------
main(sys.argv)
#-----------------------------------------------------------------
