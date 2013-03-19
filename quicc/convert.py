#! /usr/bin/python

import sys
import re

class State:
	INIT = 1
	NAMESPACE = 2
	STRUCT = 3
	REG = 4

state = State.INIT

names = []

prefix = ''

PREFIX = ''

base = ''

namespace = ''

header_guard = ''

structOffsets = ''

isUnion = 0

currentOffsetInStruct = 0

regSizeInBits = 0

typeSizes = {}

class Instance:
	def __init__(self, type, designator, offset, size):
		self.type = type
		self.designator = designator
		self.offset = offset
		self.size = size

	def __lt__(self, other):
		return self.offset < other.offset

instances = []

def updatePrefix():
	global prefix
	global PREFIX
	if len(names) > 0:
		prefix = ''
		PREFIX = ''
		for n in names:
			if len(n) > 0:
				if len(prefix) > 0:
					prefix = prefix + '_' + n.lower()
					PREFIX = PREFIX + '_' + n.upper()
				else:
					prefix = n.lower()
					PREFIX = n.upper()
	else:
		prefix = ''
		PREFIX = ''

def addName(name):
	names.append(name)
	updatePrefix()

def delName():
	names.pop()
	updatePrefix()

def changeState(name, newState):
	global state
	global currentOffsetInStruct
	global instances
	global typeSizes
	if newState == State.NAMESPACE:
		if state != State.INIT:
			raise
	elif newState == State.STRUCT:
		doClose = 0
		if state == State.REG:
			delName()
			doClose = 1
		elif state == State.STRUCT:
			doClose = 1
		elif state != State.NAMESPACE:
			raise
		if doClose:
			print '} %s;\n' % (prefix)
			offsets = re.split(',\s+', structOffsets)
			type = prefix
			typeSize = currentOffsetInStruct
			typeSizes [type] = typeSize
			if len(offsets) > 0 and offsets [0] != '':
				index = 0
				m = re.match('=([0-9]+)', offsets [0])
				if m:
					index = int(m.group(1))
					offsets = offsets [1:]
				if len(offsets) == 1:
					offsetValue = int(offsets [0], 16)
					if len(base) > 0:
						print '#define %s (*(volatile %s *) (%s + 0x%02x))' % (PREFIX, type, base, offsetValue)
						instances.append(Instance(type, names [-1].lower(), offsetValue, typeSize))
					else:
						print '#define %s (*(volatile %s *) (0x%02x))' % (PREFIX, type, offsetValue)
				else:
					for offset in offsets:
						addName(str(index))
						offsetValue = int(offset, 16)
						if len(base) > 0:
							print '#define %s (*(volatile %s *) (%s + 0x%02x))' % (PREFIX, type, base, offsetValue)
							instances.append(Instance(type, names [-2].lower() + '_' + str(index), offsetValue, typeSize))
						else:
							print '#define %s (*(volatile %s *) (0x%02x))' % (PREFIX, type, offsetValue)
						delName()
						index = index + 1
				print
			delName()
		currentOffsetInStruct = 0
	elif newState == State.REG:
		if state == State.REG:
			delName()
		elif state != State.STRUCT:
			raise
	else:
		raise
	state = newState
	addName(name)

def openNamespace(name, addr):
	global base
	global namespace
	global header_guard
	changeState(name, State.NAMESPACE)
	namespace = name
	if len(name) > 0:
		header_guard = name.upper() + '_REGS_H';
		print '#ifndef %s\n#define %s\n\n#include <bsp/utility.h>\n' % (header_guard, header_guard)
	if len(name) > 0:
		addName('base')
		base = PREFIX
		delName()
		print '#define %s 0x%02x\n' % (base, int(addr, 16))

def openStruct(name, offsets, newIsUnion):
	global structOffsets
	global isUnion
	changeState(name, State.STRUCT)
	structOffsets = offsets
	isUnion = newIsUnion
	if newIsUnion == '':
		print 'typedef struct {'
	else:
		print 'typedef union {'

def printReserved(offset, delta):
	wordSize = '8'
	if delta % 4 == 0:
		delta = delta / 4
		wordSize = '32'
	elif delta % 2 == 0:
		delta = delta / 2
		wordSize = '16'
	if delta == 1:
		print '\tuint%s_t reserved_%02x;' % (wordSize, offset)
	else:
		print '\tuint%s_t reserved_%02x [%i];' % (wordSize, offset, delta)

def addReg(name, newOffset, count, type):
	global currentOffsetInStruct
	global regSizeInBits
	changeState(name, State.REG)
	if newOffset != '':
		count = int(count)
		newOffsetValue = int(newOffset, 16)
		offsetValue = currentOffsetInStruct
		if isUnion:
			if newOffsetValue != 0:
				raise
		else:
			if newOffsetValue < offsetValue:
				raise
			delta = newOffsetValue - offsetValue
			if delta > 0:
				printReserved(offsetValue, delta)
		try:
			regSizeInBytes = int(type)
			regSizeInBits = 8 * regSizeInBytes
			type = 'uint' + str(regSizeInBits) + '_t'
		except ValueError:
			regSizeInBytes = typeSizes [type]
			regSizeInBits = 8 * regSizeInBytes
		if count > 1:
			print '\t%s %s [%i];' % (type, name.lower(), count)
		elif count < 0:
			print '\t%s %s [1];' % (type, name.lower())
		else:
			print '\t%s %s;' % (type, name.lower())
		offsetValue = newOffsetValue + regSizeInBytes * count
		if isUnion:
			if currentOffsetInStruct < offsetValue:
				currentOffsetInStruct = offsetValue
		else:
			currentOffsetInStruct = offsetValue

def addField(begin, last, name):
	addName(name)
	print '#define %s(val) BSP_BFLD%i(val, %s, %s)' % (PREFIX, regSizeInBits, begin, last)
	print '#define %s_GET(reg) BSP_BFLD%iGET(reg, %s, %s)' % (PREFIX, regSizeInBits, begin, last)
	print '#define %s_SET(reg, val) BSP_BFLD%iSET(reg, val, %s, %s)' % (PREFIX, regSizeInBits, begin, last)
	delName()

def addBit(bit, name):
	addName(name)
	print '#define %s BSP_BBIT%i(%s)' % (PREFIX, regSizeInBits, bit)
	delName()

for line in sys.stdin.readlines():
	m = re.match('(\\w*)\\{([0-9a-fA-F]+)\\}', line)
	if m:
		openNamespace(m.group(1), m.group(2))
		continue
	m = re.match('\\t(\\w+)(\\*?)\\{([^\\}]*)\\}', line)
	if m:
		openStruct(m.group(1), m.group(3), m.group(2))
		continue
	m = re.match('\\t{2}(\\w*)\\((\\w+)\\)\\[(-?[0-9]+)\\]\\{([0-9a-fA-F]*)\\}', line)
	if m:
		addReg(m.group(1), m.group(4), m.group(3), m.group(2))
		continue
	m = re.match('\\t{2}(\\w*)\\[(-?[0-9]+)\\]\\{([0-9a-fA-F]*)\\}', line)
	if m:
		addReg(m.group(1), m.group(3), m.group(2), 4)
		continue
	m = re.match('\\t{2}(\\w*)\\((\\w+)\\)\\{([0-9a-fA-F]*)\\}', line)
	if m:
		addReg(m.group(1), m.group(3), 1, m.group(2))
		continue
	m = re.match('\\t{2}(\\w*)\\{([0-9a-fA-F]*)\\}', line)
	if m:
		addReg(m.group(1), m.group(2), 1, 4)
		continue
	m = re.match('\\t{3}([0-9]+) ([0-9]+) (\\w+)', line)
	if m:
		addField(m.group(1), m.group(2), m.group(3))
		continue
	m = re.match('\\t{3}([0-9]+) (\\w+)', line)
	if m:
		addBit(m.group(1), m.group(2))
		continue
	raise

changeState('', State.STRUCT)

if len(instances) > 0:
	print 'typedef struct {'
	offset = 0
	for i in sorted(instances):
		newOffset = i.offset
		delta = newOffset - offset
		if delta > 0:
			printReserved(offset, delta)
		print '\t%s %s;' % (i.type, i.designator)
		offset = newOffset + i.size
	print '} ' + namespace.lower() + ';\n'
	print '#define %s (*(volatile %s *) (%s))\n' % (namespace.upper(), namespace.lower(), base)

if len(header_guard) > 0:
	print '#endif /* %s */' % (header_guard)
