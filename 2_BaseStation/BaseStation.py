import time
import serial
import datetime
import requests
import sys

webAPIServer="http://62.28.41.174:8082/"
#webAPIServer="http://localhost/ValmeiralWEBAPI1"
#webAPIServer="http://192.168.137.1/ValmeiralWEBAPI1"

####################################################################
#	Code to handler the keypress handling
####################################################################
#import thread

#try:
#    from msvcrt import getch  # try to import Windows version
#except ImportError:
#    def getch():   # define non-Windows version
#        import sys, tty, termios
#        fd = sys.stdin.fileno()
#        old_settings = termios.tcgetattr(fd)
#        try:
#            tty.setraw(sys.stdin.fileno())
#            ch = sys.stdin.read(1)
#        finally:
#            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
#        return ch

#char = None
 
#def keypress():
#    global char
#    char = getch()

#thread.start_new_thread(keypress, ())

####################################################################
#	Classes
####################################################################
class SMSMessage():
	def __init_(self):
		self.sender = ""
		self.receivedDate = datetime.datetime.now()
		self.message = ""

class DeviceData():
    def __init_(self):
        self.deviceID = ""
        self.latitude = ""
        self.longitude = ""


####################################################################
#	Functions
####################################################################
def getMessageID(newMessage):
	lines = newMessage.split(",", 2)
	if (len(lines) > 0):
		#print "First Line=" + lines[0]
		return lines[0].replace("+CMGL: ","")
	else:
		return 0

def uploadMessage(newMessage):

    if newMessage.message.startswith("API"):
        return postHiveDataToWebApi(newMessage)
    else:
        return postDeviceDataToWebApi(newMessage)

    print "Message was uploaded"

def processMessage(messageLines):
	try:
		# print "Processing message:"
		# print "linecount=" + str(len(messageLines))
		
		lineCount = len(messageLines)
			
		if (lineCount < 5):
			print "The message is not ok"
			return

		header = messageLines[1].replace('\n', ' ').replace('\r', '')
		headerValues = header.split(",", 5)
		
		phoneNumber = ""
		messageDate = ""
		if (len(headerValues) > 4):
			phoneNumber = headerValues[1]
			messageDate = headerValues[3] + "," + headerValues[4]
			
		
		receivedDateTime = datetime.datetime.now()
		
		message = ""
		for index in range(2, lineCount - 2):
			message+=messageLines[index]
		
		newMessage = SMSMessage()
		newMessage.sender = phoneNumber
		newMessage.receivedDate = messageDate
		newMessage.message = message
		
		print ""	
		print "-------------"
		print "New message:"
		print "-------------"
		print "PhoneNumber=" + newMessage.sender
		print "Message Date=" + messageDate
		print "Received Date=" + str(newMessage.receivedDate)
		print "Message:"
		print newMessage.message

		#Uploading the new message to the webservice
		return uploadMessage(newMessage)
		
	except Exception, e:
		print "Error occurred in processMessage(): " + str(e)
		return False
#End Funcion processMessage
def deleteMessage(phone, messageID):
	phone.write(b'AT+CMGD=' + messageID.encode() + b'\r')
	time.sleep(1)
	
	print "Message was deleted"
	
def initializePhoneForMessages(phone):
    if phone is None:
        return

    phone.write(b'AT\r')
    time.sleep(0.5)

    phone.write(b'AT+CMGF=1\r')
    time.sleep(0.5)

def getNewMessages(phone):
	#clear the input
	phone.flushInput() 

	#phone.write(b'AT+CMGL="REC UNREAD"\r') ##Get the messages unread
	phone.write(b'AT+CMGL="ALL"\r') ##Get all messages
	time.sleep(5)
	
	newMessages = []
	while phone.inWaiting() > 0:
		line = phone.readline()
		shortLine = line.replace('\n', ' ').replace('\r', '')
		if (shortLine.startswith("+CMGL", 0, len(line))):
			newMessages.append(shortLine)
			
	return newMessages

def processReceivedMessages(phone, newMessages):
	#clear the input
	phone.flushInput() 
	
	#Process a single message for testing
	# messageID=str(16)
	# phone.write(b'AT+CMGR='+ messageID.encode() + b'\r')
	# time.sleep(1)
	
	# print "New Message:"
	# messageLines=[]
	# while phone.inWaiting() > 0:
		# line=phone.readline()
		# messageLines.append(line)
	
	# processMessage(messageLines)
	
	#Process the new messages
	for index in range(len(newMessages)):
		messageHeader = newMessages[index]
		messageID = getMessageID(messageHeader)

		if (messageID > 0):
			phone.write(b'AT+CMGR=' + messageID.encode() + b'\r')
			time.sleep(1)
			messageLines = []
			while phone.inWaiting() > 0:
				line = phone.readline()
				messageLines.append(line)
			
			result = processMessage(messageLines)
			if (result == True):
				deleteMessage(phone, messageID)

def openSerial():
	try:
		print "Opening Serial..."
		phone = serial.Serial("/dev/ttyAMA0",  115200, timeout=5)
		time.sleep(0.5)

		# #flush input buffer, discarding all its contents
		phone.flushInput() 

		# #flush output buffer, aborting current output
		# #and discard all that is in buffer
		phone.flushOutput()     
		
		if not phone.isOpen():
			print "Serial is not open."
			return None
		else:
			return phone
	except Exception, e:
		print "Error openin serial: " + str(e)
		return None	
		
#def extractWeightFromMessage(newMessage):
#    message = newMessage.message

#    return 20.5

#def extractDeviceDataFromMessage(newMessage):
#    message = newMessage.message

#    data = DeviceData()
#    data.deviceID = "JA01"
#    data.latitude = "41.207249"
#    data.longitude = "-8.282890"
    
#    return data

def postHiveDataToWebApi(newMessage):

    #print "requesting token from webapi"

    loginData = {"grant_type": "password", "username": "basestationuser", "password":"User_123" }
    resp = requests.post(webAPIServer + '/Token',
	    data=loginData, headers={'Content-Type':'application/json'})

    token = ""

    if resp.status_code != 200:
        print ('ERROR POST /token/ {}'.format(resp.text))
    else:    
        token = resp.json()["access_token"]

    #print('Access Token: {}'.format(token))

    if token == "":
        print "Error: Token is invalid"
        return False

    requestData = {"token": token}
    resp = requests.get(webAPIServer + '/api/hivedata/0',
	    headers={'Authorization': 'Bearer ' + token})

    if resp.status_code != 200:
        print "Could not create a new HiveData record"
        return False

    hiveData = resp.json()

    if hiveData is None:
        print "Could not create a new HiveData record"
        return False

    now = time.strftime("%c")

    #weight=extractWeightFromMessage(newMessage)
        
    #hiveData["apiaryID"] = 0
    #hiveData["hiveID"] = 0
    hiveData["dataDate"] = newMessage.receivedDate
    hiveData["dataMessage"] = newMessage.message
    #hiveData["weight"] = weight

    print ""
    #print "HiveData:"
    #print hiveData
    print "Posting new Data"
    resp = requests.post(webAPIServer + '/api/hivedata',
    data=hiveData, headers={'Authorization': 'Bearer ' + token})
    print "Finished Posting Hive Data"

    return True

def postDeviceDataToWebApi(newMessage):

    #print "requesting token from webapi"

    loginData = {"grant_type": "password", "username": "basestationuser", "password":"User_123" }
    resp = requests.post(webAPIServer + '/Token',
        data=loginData, headers={'Content-Type':'application/json'})

    token = ""

    if resp.status_code != 200:
        print ('ERROR POST /token/ {}'.format(resp.text))
    else:    
        token = resp.json()["access_token"]

    #print('Access Token: {}'.format(token))

    if token == "":
        print "Error: Token is invalid"
        return False

    requestData = {"token": token}
    resp = requests.get(webAPIServer + '/api/devicedata/0',
        headers={'Authorization': 'Bearer ' + token})

    if resp.status_code != 200:
        print "Could not create a new DeviceData record"
        return False

    deviceData = resp.json()

    if deviceData is None:
        print "Could not create a new DeviceData record"
        return False

    now = time.strftime("%c")

    #data=extractDeviceDataFromMessage(newMessage)

    #deviceData["deviceID"] = 0
    #deviceData["deviceCode"] = data.deviceID
    deviceData["dataDate"] = newMessage.receivedDate
    deviceData["dataMessage"] = newMessage.message
    #deviceData["latitude"] = data.latitude
    #deviceData["longitude"] = data.longitude

    print ""
    #print "DeviceData:"
    #print deviceData
    print "Posting new Data"
    resp = requests.post(webAPIServer + '/api/devicedata',
	    data=deviceData, headers={'Authorization': 'Bearer ' + token})
    
    print "Finished Posting device Data"

    return True

def checkForNewMessagesReceived(phone):
    try:
        if phone is None:
            return

        #GET new messages
        newMessages = getNewMessages(phone)
    
        #Process the new messages received
        processReceivedMessages(phone, newMessages)
    except Exception, e:
        print "error occurred 'checkForNewMessagesReceived': " + str(e)

def sendSMSMessage(message, phone):
    if message is None:
        return False

    if phone is None:
        return False

    recipient=message["phoneNumber"]
    messageText=message["messageText"]

    print "Send SMS Message"
    print recipient
    print messageText
    
    phone.write(b'AT+CMGS="' + recipient.encode() + b'"\r')
    time.sleep(0.5)
    phone.write(messageText.encode() + b"\r")
    time.sleep(0.5)
    phone.write(b'\x1a\r')
    time.sleep(0.5)

    return True

def sendMessagesFromServer(phone):
    loginData = {"grant_type": "password", "username": "basestationuser", "password":"User_123" }
    resp = requests.post(webAPIServer + '/Token',
	    data=loginData, headers={'Content-Type':'application/json'})

    token = ""

    if resp.status_code != 200:
        print ('ERROR POST /token/ {}'.format(resp.text))
    else:    
        token = resp.json()["access_token"]

    #print('Access Token: {}'.format(token))

    if token == "":
        print "Error: Token is invalid"
        return 

    requestData = {"token": token}
    resp = requests.get(webAPIServer + '/api/devicecontactmessage',
	    headers={'Authorization': 'Bearer ' + token})

    if resp.status_code != 200:
        print "Could not get the new messages to send"
        return

    messages = resp.json()

    if messages is None:
        print "Messages to send are null"
        return

    if len(messages)==0:
        return
    
    for message in messages:
        print "Sending Message:"
        print message["messageText"]
        print "To number: "
        print message["phoneNumber"]

        res=sendSMSMessage(message, phone)
        print res
        if res==True:
            print "Message was sent"
            now = time.strftime("%c")
            message["messageSent"]=True
            message["messageSentDate"]=now
            s=webAPIServer + "/api/devicecontactmessage/%d" % (message["id"])
            resp = requests.put(s, data=message, headers={"Authorization": "Bearer " + token})
            print "Posted update to server"
        else:
            print "Message was not sent"

    print "Finished sending new messages"

def checkForNewMessagesToSend(phone):
    try:
        #Get and send new messages to send
        sendMessagesFromServer(phone)
    except Exception, e:
        print "error occurred 'checkForNewMessagesToSend': " + str(e)

def sendTestDataToServer():
    newMessage = SMSMessage()
    newMessage.sender = "914057256"
    newMessage.receivedDate = datetime.datetime.now()
    #newMessage.message = "API;1;1;12.5"
    newMessage.message = "JA01,15/12/12,11:06:09,7.7,136.3,xxxxx-xxxVV-V[http://maps.google.com/maps?hl=en&q=41.210006,-8.282471]"
    #newMessage.message = "SOS:JA01,15/12/12,11:06:09,7.7,136.3,xxxxx-xxxVV-V[http://maps.google.com/maps?hl=en&q=41.210006,-8.282471]"

    uploadMessage(newMessage)

####################################################################
#	Main program
####################################################################



##Open serial port
phone = openSerial()

if phone is None:
    print "Serial is not open."
    #testServer()
    #time.sleep(10)
    #exit()

try:
    #Initialize the phone for receiving SMSs
    initializePhoneForMessages(phone)
    
    print "Server:"
    print webAPIServer
    print "--------------------------"
    print "Listening for new messages"
    print "--------------------------"
    print "Press <ctrl+c> to quit"
    print "--------------------------"

    while (1):
        #if char is not None:
        #    if char=="q":
        #        print "Exiting program"
        #        break
        
        #sendTestDataToServer()

        checkForNewMessagesReceived(phone)
       
        checkForNewMessagesToSend(phone)

        time.sleep(5)

except Exception, e:
	print "error occurred: " + str(e)
	exit()

if phone is not None:
    print "Closing Serial..."	
    phone.close()

    if not phone.isOpen():
        print "Serial is closed."
    else:
        print "Serial is still open."
