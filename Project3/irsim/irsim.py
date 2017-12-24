import sys, os, platform
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import ui_mainwindow, resources_rc

__version__ = "1.0.2"

class IRSyntaxError(Exception): pass
class DuplicatedLabelError(Exception): pass
class UndefinedLabelError(Exception): pass
class DuplicatedVariableError(Exception): pass
class CurrentFunctionNoneError(Exception): pass

class IRSim(QMainWindow, ui_mainwindow.Ui_MainWindow):
	def __init__(self, parent=None):
		super(IRSim, self).__init__(parent)
		
		# Some brushes
		self.cyanBrush = QBrush(Qt.cyan)
		self.yellowBrush = QBrush(Qt.yellow)
		self.noBrush = QBrush()
		
		# Read the UI design from Qt Designer file
		self.setupUi(self)
		self.rowLabel = QLabel()
		self.rowLabel.setFrameStyle(QFrame.StyledPanel|QFrame.Sunken)
		self.statusBar().addPermanentWidget(self.rowLabel)
		
		# Initialize some values
		self.initialize()
		
		# Connect the signals with desired slots
		self.connect(self.actionQuit, SIGNAL('triggered()'), self.close)
		self.connect(self.action_Open, SIGNAL('triggered()'), self.fileOpen)
		self.connect(self.actionRun, SIGNAL('triggered()'), self.run)
		self.connect(self.actionStop, SIGNAL('triggered()'), self.stop)
		self.connect(self.actionStep, SIGNAL('triggered()'), self.step)
		self.connect(self.actionAbout, SIGNAL('triggered()'), self.helpAbout)
		self.connect(self.codeList, SIGNAL('currentRowChanged(int)'), self.updateRowLabel)

	# Open an existing IR file
	def fileOpen(self):
		if not self.okToContinue():
			return
		dir = '.' if self.filename is None else os.path.dirname(self.filename)
		fname = unicode(QFileDialog.getOpenFileName(self,
				"IR Simulator - Open IR file", dir,
				"IR files (*.ir)"))
		if fname:
			self.initialize()
			self.loadFile(fname)
			
	# Initialize the VM settings
	def initialize(self):
		self.filename = None
		self.ip = -1
		self.entranceIP = -1
		self.pauseRunning = False
		self.offset = 0
		self.instrCnt = 0
		self.codes = list()
		self.mem = None
		self.functionDict = dict()
		self.currentFunction = None
		self.symTable = dict()
		self.labelTable = dict()
		self.callStack = list()
		self.argumentStack = list()
		self.codeList.clear()
		self.watchTable.setRowCount(0)
		self.watchTable.clearContents()
		self.console.clear()
		self.updateStatus(None)
	
	# Load the contents of the file
	def loadFile(self, fname):
		fp = open(fname, 'r')
		lineno = 0
		for line in fp:
			if line.isspace(): continue
			if self.sanity_check(line, lineno):
				self.codeList.addItem(line.strip().replace('\t', ' '))
			else:
				break
			lineno += 1
		else:
			self.filename = fname
			self.lineno = lineno
		fp.close()
		if self.entranceIP == -1:
			QMessageBox.critical(self, 'Error', 'Cannot find program entrance. Please make sure the \'main\' function does exist.')
		if (self.filename is None) or (not self.labelCheck()) or (self.offset > 0x100000) or (self.entranceIP == -1):
			self.updateStatus('Loading failed.')
			self.initialize()
			return
		
		self.mem = [0] * 262144
		self.displayWatchTable()
		self.updateStatus('File loaded successfully.')
		
	# Run the codes
	def run(self):
		self.stop()
		self.ip = self.entranceIP
		while True:
			if self.ip < 0 or self.ip >= len(self.codes):
				error_code = 3
				break
			code = self.codes[self.ip]
			error_code = self.execute_code(code)		# error_code : [0=nextIR; 1=finish; 2=memAccessError; 3=IPError]
			if error_code > 0:
				break
			self.ip += 1
		if error_code == 1:
			QMessageBox.information(self, 'Finish', 'Program has exited gracefully.\nTotal instructions = %d' % self.instrCnt)
			self.updateStatus('Simulation OK. Instruction count = %d' % self.instrCnt)
		elif error_code == 2:
			QMessageBox.critical(self, 'Error', 'An error occurred at line %d: Illegal memory access. \nIf this message keeps popping out, please reload the source file' % (self.ip+1))
			self.updateStatus('Simulation failed: Memory access out of bound.')
		elif error_code == 3:
			QMessageBox.warning(self, 'Warning', 'Program Counter goes out of bound. The running program will be terminated instantly.')
			self.updateStatus('Simulation failed: PC error.')
		self.watchTable.setRowCount(0)
		self.displayWatchTable()
		self.ip = -1
		
	# Stop the running
	def stop(self):
		if self.ip != -1:
			self.codeList.item(self.ip).setBackground(self.noBrush)
		self.ip = -1
		self.instrCnt = 0
		self.pauseRunning = False
		self.watchTable.setRowCount(0)
		self.mem = [0] * 262144
		self.callStack = list()
		self.argumentStack = list()
		self.displayWatchTable()
		self.codeList.setCurrentRow(-1)
		self.console.clear()
		
	# Step by step
	def step(self):
		if self.ip == -1:
			self.stop()
			self.pauseRunning = True
			self.ip = self.entranceIP - 1
		else:
			self.codeList.item(self.ip).setBackground(self.noBrush)
		self.ip += 1
		if self.ip < 0 or self.ip >= len(self.codes):
			QMessageBox.warning(self, 'Warning', 'Program Counter goes out of bound. The running program will be terminated instantly.')
			self.updateStatus('Simulation failed: PC error.')
			self.ip = -1
			self.pauseRunning = False
			return
		code = self.codes[self.ip]
		error_code = self.execute_code(code)
		if error_code == 1:
			QMessageBox.information(self, 'Finish', 'Program has exited gracefully.\nTotal instructions = %d' % self.instrCnt)
			self.updateStatus('Simulation OK. Instruction count = %d' % self.instrCnt)
			self.ip = -1
			self.pauseRunning = False
		elif error_code == 2:
			QMessageBox.critical(self, 'Error', 'An error occurred at line %d: Illegal memory access' % (self.ip+1))
			self.updateStatus('Simulation failed: Memory access out of bound')
			self.ip = -1
			self.pauseRunning = False
		else:
			self.codeList.item(self.ip).setBackground(self.cyanBrush)
			self.codeList.setCurrentRow(self.ip)
		self.watchTable.setRowCount(0)
		self.displayWatchTable()
		
	# Display the ABOUT dialog
	def helpAbout(self):
		QMessageBox.about(self, "About IR Simulator",
				"""<b>IR Simulator</b> v {0}
				<p>Copyright &copy; 2012 Grieve. 
				All rights reserved.
				<p>This application can be used to simulate the IR 
				designed for the Compiler course here at NJU.
				<p>Python {1} - Qt {2} - PyQt {3} on {4}""".format(
				__version__, platform.python_version(),
				QT_VERSION_STR, PYQT_VERSION_STR,
				platform.system()))
	
	# When current file is running, warn the user when he want to open a new file
	def okToContinue(self):
		if self.ip != -1:
			reply = QMessageBox.question(self,
					'IR Simulator - Warning',
					'You are running a file currently. Stop the running and proceed?',
					QMessageBox.Yes|QMessageBox.No)
			if reply == QMessageBox.No:
				return False
			self.ip = -1;
		return True

	# Update status bar, window title and enable actions
	def updateStatus(self, message):
		if message: self.statusBar().showMessage(message, 5000)
		if self.filename is None:
			self.setWindowTitle('IR Simulator')
			self.actionRun.setEnabled(False)
			self.actionStop.setEnabled(False)
			self.actionStep.setEnabled(False)
		else:
			self.setWindowTitle('IR Simulator - {0}'.format(os.path.basename(self.filename)))
			self.actionRun.setEnabled(True)
			self.actionStop.setEnabled(True)
			self.actionStep.setEnabled(True)
			
	# Update selected line in codeList
	def updateRowLabel(self):
		row = self.codeList.currentRow()
		if row >= 0:
			self.rowLabel.setText('Line %d' % (row+1))
		else:
			self.rowLabel.setText('')
			
	# Check the syntax of the line and update symbol&label table
	def sanity_check(self, code, lineno):
		strs = code.split()
		relops = [ '>', '<', '>=', '<=', '==', '!=' ]
		arithops = [ '+', '-', '*', '/' ]
		try:
			if strs[0] == 'LABEL' or strs[0] == 'FUNCTION':
				if len(strs) != 3 or strs[2] != ':':
					raise IRSyntaxError
				if strs[1] in self.labelTable:
					raise DuplicatedLabelError
				self.labelTable[strs[1]] = lineno
				if strs[1] == 'main':
					if strs[0] == 'LABEL': raise IRSyntaxError
					self.entranceIP = lineno
				if strs[0] == 'FUNCTION':
					self.currentFunction = strs[1]
					self.functionDict[strs[1]] = list()
				self.codes.append(('LABEL', strs[1]))
			else:
				if self.currentFunction == None:
					raise CurrentFunctionNoneError
				if strs[0] == 'GOTO':
					if len(strs) != 2:
						raise IRSyntaxError
					self.codes.append(('GOTO', strs[1]))
				elif strs[0] == 'RETURN' or strs[0] == 'READ' or strs[0] == 'WRITE' or strs[0] == 'ARG' or strs[0] == 'PARAM':
					if len(strs) != 2:
						raise IRSyntaxError
					if (strs[0] == 'READ' or strs[0] == 'PARAM') and (not strs[1][0].isalpha()):
						raise IRSyntaxError
					self.tableInsert(strs[1])
					self.codes.append((strs[0], strs[1]))
				elif strs[0] == 'DEC':
					if len(strs) != 3 or (int(strs[2]) % 4 != 0):
						raise IRSyntaxError
					if strs[1] in self.symTable:
						raise DuplicatedVariableError
					self.tableInsert(strs[1], int(strs[2]), True)
					self.codes.append('DEC')
				elif strs[0] == 'IF':
					if len(strs) != 6 or strs[4] != 'GOTO' or strs[2] not in relops:
						raise IRSyntaxError
					self.tableInsert(strs[1])
					self.tableInsert(strs[3])
					self.codes.append(('IF', strs[1], strs[2], strs[3], strs[5]))
				else:
					if strs[1] != ':=' or len(strs) < 3:
						raise IRSyntaxError
					if strs[0][0] == '&' or strs[0][0] == '#':
						raise IRSyntaxError
					self.tableInsert(strs[0])
					if strs[2] == 'CALL':
						if len(strs) != 4:
							raise IRSyntaxError
						self.codes.append(('CALL', strs[0], strs[3]))
					elif len(strs) == 3:
						self.tableInsert(strs[2])
						self.codes.append(('MOV', strs[0], strs[2]))
					elif len(strs) == 5 and strs[3] in arithops:
						self.tableInsert(strs[2])
						self.tableInsert(strs[4])
						self.codes.append(('ARITH', strs[0], strs[2], strs[3], strs[4]))
					else:
						raise IRSyntaxError
		except (IRSyntaxError, ValueError):
			QMessageBox.critical(self, 'Error', 'Syntax error at line %d:\n\n%s' % (lineno+1, code))
			return False
		except DuplicatedLabelError:
			QMessageBox.critical(self, 'Error', 'Duplicated label %s at line %d:\n\n%s' % (strs[1], lineno+1, code))
			return False
		except DuplicatedVariableError:
			QMessageBox.critical(self, 'Error', 'Duplicated variable %s at line %d:\n\n%s' % (strs[1], lineno+1, code))
			return False
		except CurrentFunctionNoneError:
			QMessageBox.critical(self, 'Error', 'Line %d does not belong to any function:\n\n%s' % (lineno+1, code))
			return False
		return True

	# Check if there is undefined label
	def labelCheck(self):
		try:
			for i in range(self.lineno):
				code = unicode(self.codeList.item(i).text())
				strs = code.split()
				if strs[0] == 'GOTO':
					if strs[1] not in self.labelTable:
						raise UndefinedLabelError
				elif strs[0] == 'IF':
					if strs[5] not in self.labelTable:
						raise UndefinedLabelError
				elif len(strs) > 2 and strs[2] == 'CALL':
					if strs[3] not in self.labelTable:
						raise UndefinedLabelError
		except UndefinedLabelError:
			QMessageBox.critical(self, 'Error', 'Undefined label at line %d:\n\n%s' % (i+1, code))
			return False
		return True

	# Insert variables into symTable
	def tableInsert(self, var, size=4, array=False):
		if var.isdigit():
			raise IRSyntaxError
		if var[0] == '&' or var[0] == '*': var = var[1:]
		elif var[0] == '#':
			test = int(var[1:])
			return
		if var in self.symTable:return
		self.functionDict[self.currentFunction].append(var)
		if self.currentFunction == 'main':
			self.symTable[var] = self.offset, size, array
			self.offset += size
		else:
			self.symTable[var] = -1, size, array
		
	# Get Value from a variable
	def getValue(self, var):
		if var[0] == '#':
			return int(var[1:])
		elif var[0] == '&':
			return self.symTable[var[1:]][0]
		elif var[0] == '*':
			return self.mem[self.mem[self.symTable[var[1:]][0] / 4] / 4]
		else:
			return self.mem[self.symTable[var][0]/ 4]

	# Populate watchTable from symTable
	def displayWatchTable(self):
		for row, key in enumerate(self.symTable.keys()):
			self.watchTable.insertRow(row)
			item = QTableWidgetItem(key)
			item.setTextAlignment(Qt.AlignRight|Qt.AlignVCenter)
			self.watchTable.setItem(row, 0, item)
			if self.symTable[key][0] >= 0:
				item = QTableWidgetItem(str(self.symTable[key][0]))
			else:
				item = QTableWidgetItem('N/A')
			item.setTextAlignment(Qt.AlignRight|Qt.AlignVCenter)
			self.watchTable.setItem(row, 1, item)
			if self.symTable[key][0] < 0:
				item = QTableWidgetItem('N/A')
			elif self.symTable[key][1] > 4:
				strs = str(self.mem[(self.symTable[key][0]/4) : (self.symTable[key][0]/4+self.symTable[key][1]/4)])
				item = QTableWidgetItem(strs)
			else :
				item = QTableWidgetItem(str(self.mem[self.symTable[key][0] / 4]))
				
			item.setTextAlignment(Qt.AlignRight|Qt.AlignVCenter)
			self.watchTable.setItem(row, 2, item)
		self.watchTable.sortItems(0)

	# Executed a single line
	def execute_code(self, code):
		#print 'Excecuting code: ' , code
		self.instrCnt += 1
		try:
			if code[0] == 'READ':
				result, ok = QInputDialog.getInteger(self, 'IR Simulator - Read', 'Please enter an integral value for %s' % code[1], 0)
				if ok:
					self.mem[self.symTable[code[1]][0] / 4] = result
			elif code[0] == 'WRITE':
				self.console.append(str(self.getValue(code[1])))
			elif code[0] == 'GOTO':
				self.ip = self.labelTable[code[1]]
			elif code[0] == 'IF':
				value1 = self.getValue(code[1])
				value2 = self.getValue(code[3])
				if eval(str(value1) + code[2] + str(value2)):
					self.ip = self.labelTable[code[4]]
			elif code[0] == 'MOV':
				value = self.getValue(code[2])
				if code[1][0] == '*':
					self.mem[self.mem[self.symTable[code[1][1:]][0] / 4] / 4] = value
				else:
					self.mem[self.symTable[code[1]][0] / 4] = value
			elif code[0] == 'ARITH':
				value1 = self.getValue(code[2])
				value2 = self.getValue(code[4])
				self.mem[self.symTable[code[1]][0] / 4] = eval(str(value1) + code[3] + str(value2))
			elif code[0] == 'RETURN':
				if len(self.callStack) == 0:
					return 1
				else:
					returnValue = self.getValue(code[1])
					stackItem = self.callStack.pop()
					self.ip = stackItem[0]
					for key in stackItem[2].keys():
						self.symTable[key] = stackItem[2][key]
					self.offset = stackItem[3]
					self.mem[self.symTable[stackItem[1]][0] / 4] = returnValue
			elif code[0] == 'CALL':
				oldAddrs = dict()
				oldOffset = self.offset
				for key in self.functionDict[code[2]]:
					oldAddrs[key] = self.symTable[key]
					self.symTable[key] = self.getNewAddr(self.symTable[key][1]), self.symTable[key][1], self.symTable[key][2]
				self.callStack.append((self.ip, code[1], oldAddrs, oldOffset))
				self.ip = self.labelTable[code[2]]
			elif code[0] == 'ARG':
				self.argumentStack.append(self.getValue(code[1]))
			elif code[0] == 'PARAM':
				self.mem[self.symTable[code[1]][0] / 4] = self.argumentStack.pop()
			# Not finished yet
		except IndexError:
			return 2
		return 0
		
	def getNewAddr(self, size):
		ret = self.offset
		self.offset = self.offset + size
		return ret

app = QApplication(sys.argv)
app.setOrganizationName('Nanjing University')
app.setApplicationName('IR Simulator')
app.setWindowIcon(QIcon(':/icon.png'))
app.setStyle('Plastique')
form = IRSim()
form.show()
app.exec_()

