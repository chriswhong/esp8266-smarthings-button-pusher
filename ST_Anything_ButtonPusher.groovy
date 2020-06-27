/**
 *  ST_Anything_ButtonPusher.groovy
 *  ST_Anything Device controller hack that gets rid of the child devices and is customized for a single device
 *  Based on Parent_ST_Anything_Ethernet.groovy
 *
**/

metadata {
	definition (name: "ST_Anything_ButtonPusher", namespace: "cwhong", author: "Chris Whong") {
        capability "Signal Strength"
        capability "Presence Sensor"  //used to determine is the Arduino microcontroller is still reporting data or not
		capability "Switch"
        capability "Refresh"

        command "on"
	}

    simulator {
    }

    // Preferences
	preferences {
    	input "ip", "text", title: "Arduino IP Address", description: "IP Address in form 192.168.1.226", required: true, displayDuringSetup: true
		input "port", "text", title: "Arduino Port", description: "port in form of 8090", required: true, displayDuringSetup: true
		input "mac", "text", title: "Arduino MAC Addr", description: "MAC Address in form of 02A1B2C3D4E5", required: true, displayDuringSetup: true
		input "timeOut", "number", title: "Timeout in Seconds", description: "Arduino max time (try 900)", range: "120..*", required: true, displayDuringSetup:true
	}

	// Tile Definitions
	tiles (scale: 2){
    	multiAttributeTile(name:"switch", type: "lighting", width: 6, height: 4, canChangeIcon: true){
			tileAttribute ("device.switch", key: "PRIMARY_CONTROL") {
				attributeState "on", label: 'Calling...', icon: "st.locks.lock.unlocked", backgroundColor: "#00A0DC"
				attributeState "off", label: 'Call', action: "on", icon: "st.locks.lock.locked", backgroundColor: "#ffffff"
			}
		}


        standardTile("refresh", "device.switch", width: 2, height: 2, inactiveLabel: false, decoration: "flat") {
			state "default", label:'', action:"refresh.refresh", icon:"st.secondary.refresh"
		}

		standardTile("presence", "device.presence", width: 2, height: 2, canChangeBackground: true) {
			state "present", labelIcon:"st.presence.tile.present", backgroundColor:"#53a7c0"
			state "not present", labelIcon:"st.presence.tile.not-present", backgroundColor:"#ebeef2"
		}

        valueTile("rssi", "device.rssi", width: 2, height: 2) {
			state("rssi", label:'RSSI ${currentValue}', unit:"",
				backgroundColors:[
					[value: -30, color: "#006600"],
					[value: -45, color: "#009900"],
					[value: -60, color: "#99cc00"],
					[value: -70, color: "#ff9900"],
					[value: -90, color: "#ff0000"]
				]
			)
		}

        main "switch"
		details(["switch", "presence", "rssi", "refresh"])
   	}
}

def on() {
	sendData("on")
}

def off() {
	sendData("off")
}

// parse events into attributes
def parse(String description) {
	log.debug "Parsing '${description}'"
	def msg = parseLanMessage(description)
	def headerString = msg.header

	if (!headerString) {
		//log.debug "headerstring was null for some reason :("
    }

	def bodyString = msg.body

	if (bodyString) {
        log.debug "Parsing: $bodyString"
    	def parts = bodyString.split(" ")
    	def name  = parts.length>0?parts[0].trim():null
    	def value = parts.length>1?parts[1].trim():null

		def nameparts = name.split("\\d+", 2)
		def namebase = nameparts.length>0?nameparts[0].trim():null
        def namenum = name.substring(namebase.length()).trim()

        def results = []

        if (device.currentValue("presence") != "present") {
            sendEvent(name: "presence", value: "present", isStateChange: true, descriptionText: "New update received from Arduino device")
        }

		if (timeOut != null) {
            runIn(timeOut, timeOutArduino)
        } else {
           	log.info "Using 900 second default timeout.  Please set the timeout setting appropriately and then click save."
           	runIn(900, timeOutArduino)
        }

		if (name.startsWith("rssi")) {
			log.debug "In parse: RSSI name = ${name}, value = ${value}"
           	results = createEvent(name: name, value: value, displayed: false)
            log.debug results
			return results
        }

        if (name.startsWith("relaySwitch")) {
           	results = createEvent(name: "switch", value: value)
            log.debug results
			return results
        }

        results = createEvent(name: name, value: value)
        log.debug results
        return results
	}
}

private getHostAddress() {
    def ip = settings.ip
    def port = settings.port

    log.debug "Using ip: ${ip} and port: ${port} for device: ${device.id}"
    return ip + ":" + port
}


def sendData(String value) {
    sendEthernet("relaySwitch ${value}")
}

def sendEthernet(message) {
	log.debug "Executing 'sendEthernet' ${message}"
	if (settings.ip != null && settings.port != null) {
        sendHubCommand(new physicalgraph.device.HubAction(
            method: "POST",
            path: "/${message}?",
            headers: [ HOST: "${getHostAddress()}" ]
        ))
    }
    else {
        state.alertMessage = "ST_Anything Parent Device has not yet been fully configured. Click the 'Gear' icon, enter data for all fields, and click 'Done'"
        runIn(2, "sendAlert")
    }
}

def refresh() {
	log.debug "Executing 'refresh()'"
	sendEthernet("refresh")
}

def installed() {
	log.debug "Executing 'installed()'"
    if ( device.deviceNetworkId =~ /^[A-Z0-9]{12}$/)
    {
    }
    else
    {
        log.info "Device has not yet been fully configured. Click the 'Gear' icon, enter data for all fields, and click 'Done'"
        //state.alertMessage = "ST_Anything Parent Device has not yet been fully configured. Click the 'Gear' icon, enter data for all fields, and click 'Done'"
        //runIn(2, "sendAlert")
    }
}

def initialize() {
	log.debug "Executing 'initialize()'"
}

def updated() {
	if (!state.updatedLastRanAt || now() >= state.updatedLastRanAt + 5000) {
		state.updatedLastRanAt = now()
		log.debug "Executing 'updated()'"
    	runIn(3, "updateDeviceNetworkID")

        log.debug "Hub IP Address = ${device.hub.getDataValue("localIP")}"
        log.debug "Hub Port = ${device.hub.getDataValue("localSrvPortTCP")}"

        //Schedule inactivity timeout
        log.info "Device inactivity timer started for ${timeOut} seconds"
        runIn(timeOut, timeOutArduino)

	}
	else {
		//log.trace "updated(): Ran within last 5 seconds so aborting."
	}
}

def updateDeviceNetworkID() {
	log.debug "Executing 'updateDeviceNetworkID'"
    def formattedMac = mac.toUpperCase()
    formattedMac = formattedMac.replaceAll(":", "")
    if(device.deviceNetworkId!=formattedMac) {
        log.debug "setting deviceNetworkID = ${formattedMac}"
        device.setDeviceNetworkId("${formattedMac}")
	}
    //Need deviceNetworkID updated BEFORE we can create Child Devices
	//Have the Arduino send an updated value for every device attached.  This will auto-created child devices!
	refresh()
}

private sendAlert() {
   sendEvent(
      descriptionText: state.alertMessage,
	  eventType: "ALERT",
	  name: "childDeviceCreation",
	  value: "failed",
	  displayed: true,
   )
}

def timeOutArduino() {
    //If the timeout expires before being reset, mark this Parent Device as 'not present' to allow action to be taken
    log.info "No update received from Arduino device in past ${timeOut} seconds"
    sendEvent(name: "presence", value: "not present", isStateChange: true, descriptionText: "No update received from Arduino device in past ${timeOut} seconds")
}
