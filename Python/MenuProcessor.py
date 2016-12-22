import string
import collections
import sys
import os.path

###############################################################################
## Globals
###############################################################################
from enum import Enum
class scriptState(Enum):
    INITIAL_STATE = 0
    MENU_HEADER   = 1
    NEW_ROW       = 2
    NEW_CELL      = 3
    CONTINUE_CELL = 4
    SCRIPT_END    = 5

ByteWriter = collections.namedtuple('ByteWriter', ['used', 'value'])
gClientHostId = 0
gScriptState = scriptState.INITIAL_STATE
gCellPercentageSum = 0
gBytesWritten = 0

# Constants
KEY_TOKEN_LIST = ['MENU_HEADER','NEW_ROW', 'NEW_CELL_LEFT', 'NEW_CELL_RIGHT', 
                  'NEW_CELL_CENTER', 'COND_START', 'COND_END', 'TEXT_STATIC', 
                  'TEXT_DYNAMIC', 'BUTTON', 'CHECK_BOX', 'DROP_SELECT',
                  'EDIT_NUMBER', 'TIME_BOX', 'SCRIPT_END']

MAX_BUFFER_SIZE = 64

###############################################################################
## Cleanly exits
###############################################################################
def exitMenuProcessor():
    global fout
    fout.close()
    fout = open('.\TestMenuOut.txt', 'w')
    fout.close()
    print("Fail")
    exit()

###############################################################################
## Looks for duplicate tokens in the list
##   Prints an error and exits if duplicate tokens are found
###############################################################################
def checkDuplicateTokens(tokenList):
    listLen = len(tokenList)
    for x0 in range(listLen-1):
        for x1 in range(x0+1, listLen):
            if (tokenList[x0] == tokenList[x1]):
                print("Error -- Matching tokens -- ")
                print(tokenList[x0])
                exitMenuProcessor()

###############################################################################
## Processes a line and removes any comments - '#' starts a comment
###############################################################################
def commentRemover(line):
    line = line.split('#')[0]
    return line.strip()
    
###############################################################################
## This takes a single string (line) and converts it into a list of tokens
##   where each token in the list is a single word execpt for words grouped 
##   inside ""
###############################################################################
def createLineTokenList(line):
    done = False
    s0 = 0
    s1 = 0
    tokenList = [None]
    while(not(done)):
        s0 = line.find("\"", s0)
        if (s0 != -1):
            s1 = line.find("\"", s0+1)
            if (s1 == -1):
                print("Error -- Mismatched quotes on line -- ")
                print(line)
                exitMenuProcessor()
            tokenList += line[:s0].split()
            if (s1-s0 <= 1):
                print("Error -- Empty string on line -- ")
                print(line)
                exitMenuProcessor()
            tokenList.append(line[s0:s1+1])
            s0 = s1+1
            s1 = s0
        else:
            done = True;
    tokenList += line[s1:].split()
    tokenList = list(filter(None, tokenList))
    return tokenList

###############################################################################
## Prints error when you didn't have the correct number of tokens
###############################################################################
def checkTokenCountMismatch(numCurTokens, numExpectedTokens, line):
    if (numCurTokens != numExpectedTokens):
        print("Error -- Line didn't have correct number of tokens -- ")
        print(line)
        exitMenuProcessor()

###############################################################################
## Tracks total bytes in memory buffer
###############################################################################
def increaseBytesWritten():
    global gBytesWritten
    gBytesWritten = gBytesWritten + 1

###############################################################################
## Writes a number to the CA6 output
###############################################################################
def writeBoundsCheckedNumber(val, minVal, maxVal, bits, byteWriter):

    try:
        val = int(val)
    except ValueError:
        print("Error -- Input not an int  -- ")
        print(line)
        exitMenuProcessor()
        
    if ((val < minVal) or (val > maxVal)):
        print("Error -- Value out of range  -- ")
        print(line)
        exitMenuProcessor()

    if ((bits < 1) or (bits >32)):
        print("Error -- Value of bits is out of range  -- ")
        print(line)
        exitMenuProcessor()

    status = True
    packBitsLeft = bits
    bitsUsed = byteWriter.used
    bitsVal  = byteWriter.value

    while (status):
        bitsInCurDstByte = 8 - bitsUsed
        bitsToPack = min(bitsInCurDstByte, packBitsLeft)
        bitsVal = (bitsVal | (val << bitsUsed)) & 0xff
        bitsUsed = bitsUsed + bitsToPack
        val = val >> bitsToPack
        if (bitsUsed == 8):  # When byte is full write it's value
            fout.write(str(bitsVal) + ",")
            increaseBytesWritten()
            bitsUsed = 0;
            bitsVal = 0;
        packBitsLeft = packBitsLeft - bitsToPack
        status = (packBitsLeft != 0) # Continue if you can't write all the bits to current byte

    byteWriter = ByteWriter(bitsUsed, bitsVal)
    return byteWriter

###############################################################################
## Writes a string to the CA6 output
###############################################################################
def writeString(val):
    if (val[0] != "\""):
        print("Error -- String did not start with  -- ")
        print(line)
        exitMenuProcessor()
    if (val[-1] != "\""):
        print("Error -- String did not end with  -- ")
        print(line)
        exitMenuProcessor()
        
    val = val[1:-1] # remove the starting and ending quotes from the string
    if (len(val) >= MAX_BUFFER_SIZE-1): #-1 is for null terminator
        print("Error -- String too long -- ")
        print(line)
        exitMenuProcessor()

    for x0 in val:
        fout.write("'" + x0 +"',")
        increaseBytesWritten()

    fout.write("0,")
    increaseBytesWritten()

###############################################################################
## Writes a line size to the CA6 output
###############################################################################
def writeSize(val, strVal):
    if (strVal):
        if (strVal[0] != "\""):
            print("Error -- String did not start with  -- ")
            print(line)
            exitMenuProcessor()
        if (strVal[-1] != "\""):
            print("Error -- String did not end with  -- ")
            print(line)
            exitMenuProcessor()
                
        strVal = strVal[1:-1] # remove the starting and ending quotes from the string
        if (len(strVal) >= MAX_BUFFER_SIZE-1): #-1 is for null terminator
            print("Error -- String too long -- ")
            print(line)
            exitMenuProcessor()
        
        val = val + len(strVal) + 1

    if ((val >= 256) or (val < 0)):
        print("Error -- Invalid packet size -- ")
        print(line)
        exitMenuProcessor()
    
    fout.write("  " + str(val) + ",")
    increaseBytesWritten()
    
###############################################################################
## Checks the current state to make sure tokens are being added in the 
##  correct place in the script file
###############################################################################
def scriptStateCheck(newState, lineine, cellPercentage=0):
    global gScriptState
    global gCellPercentageSum
    
    if (newState == scriptState.SCRIPT_END):
        gScriptState = newState
        if ((gCellPercentageSum != 0) and (gCellPercentageSum != 100)):
            print("Error -- Invalid cell sum at end (should be 0 or 100) -- ")
            print(gCellPercentageSum)
            print(line)
            exitMenuProcessor()
        return
    
    if (gScriptState == scriptState.INITIAL_STATE):
        if (newState == scriptState.MENU_HEADER):
            gScriptState = newState
        else:
            print("Error -- Invalid state after INITIAL_STATE -- ")
            print(line)
            exitMenuProcessor()
    elif (gScriptState == scriptState.MENU_HEADER):
        if (newState == scriptState.NEW_ROW):
            gScriptState = newState
        else:
            print("Error -- Invalid state after MENU_HEADER -- ")
            print(line)
            exitMenuProcessor()
    elif (gScriptState == scriptState.NEW_ROW):
        if ((gCellPercentageSum != 0) and (gCellPercentageSum != 100)):
            print("Error -- Invalid cell sum (should be 0 or 100) -- ")
            print(gCellPercentageSum)
            print(line)
            exitMenuProcessor()
        if (newState == scriptState.NEW_CELL):
            gCellPercentageSum = cellPercentage
            gScriptState = newState
        else:
            print("Error -- Invalid state after NEW_ROW -- ")
            print(line)
            exitMenuProcessor()
    elif (gScriptState == scriptState.NEW_CELL):
        if (newState == scriptState.NEW_ROW):
            gScriptState = newState
        elif (newState == scriptState.NEW_CELL):
            gCellPercentageSum = gCellPercentageSum + cellPercentage
            gScriptState = newState
        elif (newState == scriptState.CONTINUE_CELL):
            gScriptState = newState
        else:
            print("Error -- Invalid state after NEW_CELL -- ")
            print(line)
            exitMenuProcessor()
    elif (gScriptState == scriptState.CONTINUE_CELL):
        if (newState == scriptState.NEW_ROW):
            gScriptState = newState
        elif (newState == scriptState.NEW_CELL):
            gCellPercentageSum = gCellPercentageSum + cellPercentage
            gScriptState = newState
        elif (newState == scriptState.CONTINUE_CELL):
            gScriptState = newState
        else:
            print("Error -- Invalid state after CONTINUE_CELL -- ")
            print(line)
            exitMenuProcessor()
    elif (gScriptState == scriptState.SCRIPT_END):
        print("Error -- Invalid state of SCRIPT_END -- ")
        print(line)
        exitMenuProcessor()
    else:
        print("Error -- Invalid old state error -- ")
        print(line)
        exitMenuProcessor()


###############################################################################
## This group of functions processes each line from the text file and converts
##   it into the CA6 progmem code
###############################################################################
def procMenuHeader(tokenList, line):
    scriptStateCheck(scriptState.MENU_HEADER, line)
    byteWriter = ByteWriter(0,0)
    checkTokenCountMismatch(len(tokenList), 4, line)
    writeSize(4, tokenList[3])
    fout.write("PID_MENU_HEADER,")
    increaseBytesWritten()
    byteWriter = writeBoundsCheckedNumber(tokenList[1], 0, 255, 8, byteWriter);
    byteWriter = writeBoundsCheckedNumber(tokenList[2], 0, 255, 8, byteWriter);
    writeString(tokenList[3])
    writeLineComment(tokenList, -1)

def procNewRow(tokenList, line):
    scriptStateCheck(scriptState.NEW_ROW, line)
    checkTokenCountMismatch(len(tokenList), 1, line)
    writeSize(2, None)
    fout.write("PID_NEW_ROW,")
    increaseBytesWritten()
    writeLineComment(tokenList, -1)

def procNewCellLeft(tokenList, line):
    scriptStateCheck(scriptState.NEW_CELL, line, int(tokenList[1]))
    byteWriter = ByteWriter(0,0)
    checkTokenCountMismatch(len(tokenList), 2, line)
    writeSize(3, None)
    fout.write("PID_NEW_CELL_LEFT,")
    increaseBytesWritten()
    byteWriter = writeBoundsCheckedNumber(tokenList[1], 0, 100, 8, byteWriter);
    writeLineComment(tokenList, -1)

def procNewCellRight(tokenList, line):
    scriptStateCheck(scriptState.NEW_CELL, line, int(tokenList[1]))
    byteWriter = ByteWriter(0,0)
    checkTokenCountMismatch(len(tokenList), 2, line)
    writeSize(3, None)
    fout.write("PID_NEW_CELL_RIGHT,")
    increaseBytesWritten()
    byteWriter = writeBoundsCheckedNumber(tokenList[1], 0, 100, 8, byteWriter);
    writeLineComment(tokenList, -1)

def procNewCellCenter(tokenList, line):
    scriptStateCheck(scriptState.NEW_CELL, line, int(tokenList[1]))
    byteWriter = ByteWriter(0,0)
    checkTokenCountMismatch(len(tokenList), 2, line)
    writeSize(3, None)
    fout.write("PID_NEW_CELL_CENTER,")
    increaseBytesWritten()
    byteWriter = writeBoundsCheckedNumber(tokenList[1], 0, 100, 8, byteWriter);
    writeLineComment(tokenList, -1)

def procCondStart(tokenList, line):
    scriptStateCheck(scriptState.CONTINUE_CELL, line)
    global gClientHostId
    byteWriter = ByteWriter(0,0)
    checkTokenCountMismatch(len(tokenList), 3, line)
    writeSize(4, None)
    fout.write("PID_COND_START,")
    increaseBytesWritten()
    byteWriter = writeBoundsCheckedNumber(gClientHostId, 0, 255, 8, byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[1], 0, 1, 4, byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[2], 0, 1, 4, byteWriter)
    gClientHostId = writeLineComment(tokenList, gClientHostId)

def procCondEnd(tokenList, line):
    scriptStateCheck(scriptState.CONTINUE_CELL, line)
    checkTokenCountMismatch(len(tokenList), 1, line)
    writeSize(2, None)
    fout.write("PID_COND_END,")
    increaseBytesWritten()
    writeLineComment(tokenList, -1)
    
def procTextStatic(tokenList, line):
    scriptStateCheck(scriptState.CONTINUE_CELL, line)
    checkTokenCountMismatch(len(tokenList), 2, line)
    writeSize(2, tokenList[1])
    fout.write("PID_TEXT_STATIC,")
    increaseBytesWritten()
    writeString(tokenList[1])
    writeLineComment(tokenList, -1)

def procTextDynamic(tokenList, line):
    scriptStateCheck(scriptState.CONTINUE_CELL, line)
    global gClientHostId
    byteWriter = ByteWriter(0,0)
    checkTokenCountMismatch(len(tokenList), 2, line)
    writeSize(3, tokenList[1])
    fout.write("PID_TEXT_DYNAMIC,")
    increaseBytesWritten()
    byteWriter = writeBoundsCheckedNumber(gClientHostId, 0, 255, 8, byteWriter)
    writeString(tokenList[1])
    gClientHostId = writeLineComment(tokenList, gClientHostId)

def procButton(tokenList, line):
    scriptStateCheck(scriptState.CONTINUE_CELL, line)
    global gClientHostId
    byteWriter = ByteWriter(0,0)
    checkTokenCountMismatch(len(tokenList), 3, line)
    writeSize(4, None)
    fout.write("PID_BUTTON,")
    increaseBytesWritten()
    byteWriter = writeBoundsCheckedNumber(gClientHostId, 0, 255, 8, byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[1], 0, 1, 4, byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[2], 0, 1, 4, byteWriter)
    gClientHostId = writeLineComment(tokenList, gClientHostId)

def procCheckBox(tokenList, line):
    scriptStateCheck(scriptState.CONTINUE_CELL, line)
    global gClientHostId
    byteWriter = ByteWriter(0,0)
    checkTokenCountMismatch(len(tokenList), 2, line)
    writeSize(4, None)
    fout.write("PID_CHECK_BOX,")
    increaseBytesWritten()
    byteWriter = writeBoundsCheckedNumber(gClientHostId, 0, 255, 8, byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[1], 0, 1, 8, byteWriter)
    gClientHostId = writeLineComment(tokenList, gClientHostId)

def procDropSelect(tokenList, line):
    scriptStateCheck(scriptState.CONTINUE_CELL, line)
    global gClientHostId
    byteWriter = ByteWriter(0,0)
    checkTokenCountMismatch(len(tokenList), 3, line)
    writeSize(4, tokenList[2])
    fout.write("PID_DROP_SELECT,")
    increaseBytesWritten()
    byteWriter = writeBoundsCheckedNumber(gClientHostId, 0, 255, 8, byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[1], 0, 255, 8, byteWriter)
    writeString(tokenList[2])
    gClientHostId = writeLineComment(tokenList, gClientHostId)

def procEditNumber(tokenList, line):
    scriptStateCheck(scriptState.CONTINUE_CELL, line)
    global gClientHostId
    byteWriter = ByteWriter(0,0)
    checkTokenCountMismatch(len(tokenList), 6, line)
    writeSize(16, None)
    fout.write("PID_EDIT_NUMBER,")
    increaseBytesWritten()
    byteWriter = writeBoundsCheckedNumber(gClientHostId, 0, 255, 8, byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[1], 0, 8, 4, byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[2], 0, 8, 4, byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[3], 0, 99999999, 32, byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[4], 0, 99999999, 32, byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[5], 0, 99999999, 32, byteWriter)
    gClientHostId = writeLineComment(tokenList, gClientHostId)
    
    if (int(tokenList[1])+int(tokenList[2]) > 8): #doing this check after initial checks above
        print("Error -- digits_before_decimal and digits_after_decimal can't exceed 8 -- ")
        print(line)
        exitMenuProcessor()

def procTimeBox(tokenList, line):
    scriptStateCheck(scriptState.CONTINUE_CELL, line)
    global gClientHostId
    byteWriter = ByteWriter(0,0)
    checkTokenCountMismatch(len(tokenList), 13, line)
    writeSize(11, None)
    fout.write("PID_TIME_BOX,")
    increaseBytesWritten()
    byteWriter = writeBoundsCheckedNumber(gClientHostId, 0, 255, 8, byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[1],  0, 1,   1,  byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[2],  0, 1,   1,  byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[3],  0, 1,   1,  byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[4],  0, 1,   1,  byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[5],  0, 1,   1,  byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[6],  0, 1,   1,  byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[7],  0, 999, 10, byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[8],  0, 59,  6,  byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[9],  0, 59,  6,  byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[10], 0, 999, 10, byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[11], 0, 999, 10, byteWriter)
    byteWriter = writeBoundsCheckedNumber(tokenList[12], 0, 999, 10, byteWriter)
    byteWriter = writeBoundsCheckedNumber(0            , 0, 1,   6,  byteWriter) #unused
    gClientHostId = writeLineComment(tokenList, gClientHostId)

def procScriptEnd(tokenList, line):
    scriptStateCheck(scriptState.SCRIPT_END, line)
    checkTokenCountMismatch(len(tokenList), 1, line)
    writeSize(2, None)
    fout.write("PID_SCRIPT_END,")
    increaseBytesWritten()
    writeLineComment(tokenList, -1)

FUNC_LIST = [procMenuHeader, procNewRow, procNewCellLeft, procNewCellRight, 
             procNewCellCenter, procCondStart, procCondEnd, procTextStatic,
             procTextDynamic, procButton, procCheckBox, procDropSelect,
             procEditNumber, procTimeBox, procScriptEnd]

###############################################################################
## Outputs a comment for the current line of tokens
###############################################################################
def writeLineComment(tokenList, clientHostId):
    global fout
    fout.write("  // ")
    x0 = 0
    for token in tokenList:
        x0 = x0+1
        fout.write(token)
        if (x0 < len(tokenList)):
            fout.write(" ")

    if (clientHostId >= 0):
        fout.write("  **gClientHostId_" + str(clientHostId) + "**")
    fout.write("\n")
    return clientHostId+1
    
###############################################################################
## The central hub to process a line from the input file, validate it, and
##    then output the reduced version to the output file
###############################################################################
def processLine(line):
    line = commentRemover(line)
    
    if (len(line) == 0):
        return 

    tokenList = createLineTokenList(line)

    if (KEY_TOKEN_LIST.count(tokenList[0]) == 0):
        print("Error -- Token not in token list -- ")
        print(tokenList[0])
        exitMenuProcessor()

    x0 = KEY_TOKEN_LIST.index(tokenList[0])
    FUNC_LIST[x0](tokenList, line)

###############################################################################
## main()
###############################################################################
global fout

if (len(sys.argv) != 2):
    print("PARAM ERROR - Must pass in one parameter with filename.")
    sys.exit(0)
if ((len(sys.argv[1]) < 5) or (sys.argv[1][-4:] != '.txt')):
    print("PARAM ERROR - Need a valid filename ending in .txt")
    sys.exit(0)
outFileName = '.\\' + sys.argv[1][:-4] + 'Out.txt'
inFileName = sys.argv[1]

if (os.path.isfile(inFileName) == False):
    print("ERROR - File does not exists")
    sys.exit(0)

fin = open(inFileName, 'r')
fout = open(outFileName, 'w')

checkDuplicateTokens(KEY_TOKEN_LIST)

fout.write("  const uint8 sData[] PROGMEM = {\n");

for line in fin.readlines():
    processLine(line)

fout.write("  };  // Total Bytes = ")
fout.write(str(gBytesWritten))
fout.write("\n")
fout.close()
fin.close()
print("Success!")
