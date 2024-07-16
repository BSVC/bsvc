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
# Updatded for Python 3 and GNU GNU Binutils 2.42 (latest as of 2024).
#   This version was tested with several assembler and C sources linked.
#
# * USUAL DISCLAIMER... NO WARRANTY FOR ANY PURPOSE. *
#


import sys
import string
import regex

#-----------------------------------------------------------------
debug=0
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
			print ("Processing %s" % file_name)
		try:
			mfile=open(file_name,"r")
		except IOError:
			print ("ERROR: [map] Can't open file %s for reading" % file_name)
			return 1

		state=0
		while(1):
			try:
				s=mfile.readline()
			except IOError:
				print ("ERROR: [map] cannot read file %s" % file_name)
			if(s==""):
				break
			if(s=="**LINK EDITOR MEMORY MAP**\n"):
				state=1
				if(debug>=16):
					print ("map: link editor memory map found")
				break
			# new version
			if(s=="Linker script and memory map\n"):
				state=2
				if(debug>=16):
					print ("map: link editor memory map found")
				break

		if(state==0):
			print ("ERROR: [map] link editor memory map not found")
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
					print ("ERROR: [map] cannot read file %s" % file_name)
				if(s==""):
					break
				if(s=="\n"):
					# ignore blank lines
					continue

				i1=string.find(s,"from")
				if(i1>=0):
					x=string.split(s)
					if(len(x)<2):
						print ("Warning: bad \"from\" instruction (1)" )
						continue
					i2=string.find(x[1],"(")
					if(i2==-1):
						print ("Warning: bad \"from\" instruction (2)")
						continue
					module=x[1][0:i2]
					if(debug>=16):
						print ("Module =", module)
					i3=string.find(x[1],")")
					if(i3==-1):
						print ("Warning: bad \"from\" instruction (3)")
						continue
					segment=x[1][i2+1:i3]
					if(debug>=16):
						print ("Segment =", segment)
					continue

				i1=string.find(s,"output")
				if(i1==0):
					if(debug>=16):
						print ("\"output\" line ignored" )
					continue

				i1=string.find(s,"section")
				if(i1==0):
					if(debug>=16):
						print ("\"section\" line ignored" )
					continue

				i1=string.find(s,".data")
				if(i1>=0):
					if(debug>=16):
						print ("\".data\" line ignored" )
					continue

				i1=string.find(s,".text")
				if(i1>=0):
					if(debug>=16):
						print ("\".text\" line ignored" )
					continue

				i1=string.find(s,".bss")
				if(i1>=0):
					if(debug>=16):
						print ("\".bss\" line ignored" )
					continue

				i1=string.find(s,"LOAD")
				if(i1>=0):
					if(debug>=16):
						print ("\"LOAD\" line ignored")
					continue

				x=string.split(s)
				if(len(x)!=2):
					if(debug>=6):
						print ("line ignored: \"%s\"" % s[0:-1])
					continue

				try:
					address=string.atoi(x[0],16)
				except ValueError:
					if(debug>=6):
						print ("not an address \"%s\", line ignored" % x[1])
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
					print ("ERROR: [map] cannot read file %s" % file_name)
				##print "READ:", s,
				if(s==""):
					break
				if(s=="\n"):
					# ignore blank lines
					continue

				i1=s.find("LOAD")
				if(i1>=0):
					continue
				i1=s.find("OUTPUT")
				if(i1>=0):
					continue
				i1=s.find("COMMON")
				if(i1>=0):
					continue
				i1=s.find(".comment")
				if(i1>=0):
					continue

				z=s.split()

				if(z[0][0]=='.'):
					if(((z[0]==".data") or (z[0]==".bss")) and (len(z)>=4)):
						current_segment=1
						##print "DATA FOR MODULE", z[3]
						continue
					elif (len(z) >= 4):
						current_segment=2
						##print "TEXT FOR MODULE", z[3]
						continue

				if((len(z)==2) and (current_segment!=0)):
					try:
						address=int(z[0], 16)
					except ValueError:
						if(debug>=6):
							print ("not an address \"%s\", line ignored" % z[1])
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

def columns(file):
	# determine columns. we look for somethig like
	#   18 0008 4EB9 0000		jsr sub
	#0     6    11   16  20

	hex_sym = set("0123456789ABCDEF")

	while True:
		s = file.readline()
		if not s:
			print("Can't determine columns")
			return [6, 11, 16, 20]

		if (regex.match(r"[ ]+([0-9]+)[ ]([0-9a-fA-F]+)[ ]([0-9a-fA-F]+)[ ]([0-9a-fA-F]+)(.*)",s)):
			i = 0;
			while s[i].isspace():
				i += 1
			while s[i].isnumeric():
				i += 1
			while s[i].isspace():
				i += 1
			addr = i + 1;

			while s[i].upper() in hex_sym:
				i += 1
			while s[i].isspace():
				i += 1
			data1 = i + 1

			while s[i].upper() in hex_sym:
				i += 1
			while s[i].isspace():
				i += 1
			data2 = i + 1

			while s[i].upper() in hex_sym:
				i += 1
			rest = i + 1

			return [addr, data1, data2, rest]

#-----------------------------------------------------------------
def write_lst(ofile, s, address, cols):
	x = s.split()
	if len(x) < 1:
		return
	ln = x[0]

	if address >= 0:
		ns = "%0*x" % (cols[1]-cols[0]-1, address)
	else:
		ns = ' ' * (cols[0]-2)

	ns += ' '

	for i in range(cols[1]-1, cols[3]-1):
		if i < len(s) and not s[i] in ['\r', '\n']:
			ns += s[i]
		else:
			ns += ' '

	ns += "%*s" % (cols[1]-cols[0]-1, ln)

	if len(s) >= cols[3]-1:
		ns += s[cols[3]-1:]
	else:
		ns += '\n'

	ofile.write(ns)

#-----------------------------------------------------------------

def process_listing(map, ofile, ifile, ifn, cols):
	global debug

	c_src = False
	s = ""

	print ("Processing %s" % ifn)
	state=0 #state says if addresses must be converted or not
	while(1):
		last = s
		s = ifile.readline()

		if not s:
			return

		x = s.split()
		if len(x) > 0 and not x[0].isnumeric():
			continue	# line number first on source lines

		i1=s.find(".file")
		if(i1 >= 0):
			try:
				name = s.split()[2]
				ext = name[name.rfind('.')+1:-1].lower()
				if (ext in ['c', 'cpp', 'cc', 'ccx', 'h', 'hpp']):
					c_src = True;
			except:
				print("Bogus file name.")
			write_lst(ofile, s, -1, cols)
			continue

		if(s==""):
			continue
		i1=s.find("****")
		if(i1>=0):
			# c source (most likely!?)
			write_lst(ofile, s, -1, cols)
			continue
		i1=s.find(".stab")
		if(i1>=0):
			# c stabs - ignore them
			write_lst(ofile, s, -1, cols)
			continue
		i1=s.find("68K GAS")
		if(i1>=0):
			# header of listing (page number) - ignore them
			continue
		i1=s.find("DEFINED SYMBOLS")
		i2=s.find("UNDEFINED SYMBOLS")
		if((i1>=0)or(i2>=0)):
			# at the end of the listing there are 2 lists: defined
			# and undefined symbols...stop there! (not any more
			# interesting stuff)
			break

		# .globl line is "{line number} .globl {symbol name}"
		i1=s.find(".globl")
		if(i1==-1):
			i1=s.find(".global")
		if(i1>=0):
			state=0
			x=s.split()
			if(len(x)<2):
				if(debug>=5):
					print ("Warning: malformed .globl", x,)
				write_lst(ofile, s, -1, cols)
				continue

			if(debug>=6):
				print ("\tanalysing %s, line %s" % (x[2],x[0]))
			f=map.find_function(x[2])
			if(f!=None):
				if(debug>=15):
					print ("Function %s at %#x found" % (f.name,f.address))
				state=1
				delta=0
				###ofile.write("* FUNCTION %s at %#04x\n" % (f.name,f.address))
			v=map.find_variable(x[2])
			if(v!=None):
				if(debug>=15):
					print ("Variable %s at %#x found" % (v.name,v.address))
				state=3
				delta=0
				###ofile.write("* VARIABLE %s at %#04x\n" % (v.name,v.address))
			if((f==None)and(v==None)and(debug>=1)):
				print ("Warning: global %s not in map!" % x[2])

		if (not c_src):
			# if last line was a label and this line is recoginzed, then assembler source
			s0 = last.split();
			if (len(s0) >= 2 and s0[0].isnumeric()):
				if (s0[1][-1] == ':'):
					sym = s0[1][:-1]
				else:
					sym = s0[0]
				tmp = map.find_function(sym)
				if (tmp == None):
					tmp = map.find_variable(sym)
				if (tmp != None):
					result = regex.match(r"[ ]+([0-9]+)[ ]([0-9a-fA-F]+)[ ]([0-9a-fA-F]+)(.*)",s)
					if (result):
						f=tmp;
						state = 4
						x=s.split()
						delta=int(x[1],16)

		if(state==1):
			# the first time, inside a function
			result=regex.match(r"[ ]+([0-9]+) ([0-9a-fA-F]+) ([0-9a-fA-F]+)(.*)",s)
			if(result):
				###print "MATCH", s,
				x=s.split()
				# old adress is x[1]
				try:
					delta=int(x[1],16)
				except ValueError:
					print ("ERROR: Bad address %s line %s" % (x[1],x[0]))
					break
				write_lst(ofile, s, f.address, cols)
				state=2
			else:
				write_lst(ofile, s, -1, cols)

		elif(state==2):
			# inside a function, when delta is known
			result=regex.match(r"[ ]+([0-9]+)[ ]+([0-9a-fA-F]+)[ ]+([0-9a-fA-F]+)[ ]+(.*)",s)
			if(result):
				###print "MATCH", s,
				x=s.split()
				# old adress is x[1]
				try:
					new_address=int(x[1],16)-delta+f.address
				except ValueError:
					print ("ERROR: Bad address %s line %s" % (x[1],x[0]))
					break
				write_lst(ofile, s, new_address, cols)
			else:
				###print "DONT MATCH", s,
				write_lst(ofile, s, -1, cols)

		elif (state==3):
			# variable definition
			result=regex.match(r"[ ]+([0-9]+) ([0-9a-fA-F]+) ([0-9a-fA-F]+)(.*)",s)
			if(result):
				###print "MATCH", s,
				x=s.split()
				write_lst(ofile, s, v.address, cols)
				state=0
			else:
				write_lst(ofile, s, -1, cols)

		elif(state == 4):
			# in assembler file
			result = regex.match(r"[ ]+([0-9]+)[ ]([0-9a-fA-F]+)[ ]([0-9a-fA-F]+)(.*)", s)
			if(result):
				x = s.split()
				try:
					new_address = int(x[1], 16) - delta + f.address
				except ValueError:
					print ("ERROR: Bad address %s line %s" % (x[1], x[0]))
					break
				write_lst(ofile, s, new_address, cols)
			else:
				###print "DONT MATCH", s,
				write_lst(ofile, s, -1, cols)

		else:
			write_lst(ofile, s, -1, cols)


#-----------------------------------------------------------------
def main(argv):
	if(len(argv)<2):
		print ("usage %s map-file listing-files" % argv[0])
		sys.exit(-1)

	map=c_map()
	if(map.load(argv[1])):
		sys.exit(1)

	if(debug>=20):
		for x in map.functions:
			print("Function:", x.name, hex(x.address))
		for x in map.variables:
			print("Variable: ", x.name, hex(x.address))

	index=argv[1].rfind('.')
	n=argv[1][0:index]+".LIS"

	try:
		ofile=open(n,"w")
	except IOError:
		print ("ERROR: Can't open file %s for writing" % n)
		sys.exit(2)

	for lf in argv[2:]:
		try:
			ifile=open(lf,"r")
		except IOError:
			print ("ERROR: Can't open file %s for reading" % lf)
			ofile.close()
			sys.exit(2)

		cols = columns(ifile)
		ifile.seek(0,0)

		ofile.write("\n*** " + lf + " ***\n\n")

		process_listing(map,ofile,ifile,lf, cols)

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
