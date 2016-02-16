import json
import urllib2
import datetime
import dateutil.parser as dateparser
from dateutil import tz
import paho.mqtt.client as mqtt

#Variables
mqtt_user="USERNAME"
mqtt_pass="PASSWORD"
mqtt_host="HOST"
mqtt_port="PORT"
facebook_access_token="TOKEN"
is_homicide=0

#Setup mqtt client
mqttc = mqtt.Client()
mqttc.username_pw_set(mqtt_user, mqtt_pass)
mqttc.connect(mqtt_host, mqtt_port)

#Get latest BPD Facebook posts
url = "https://graph.facebook.com/v2.5/58771761955/posts?access_token="+facebook_access_token
response = urllib2.urlopen(url)
data = response.read()
values = json.loads(data)

#Dates
utc_zone = tz.gettz('UTC')
est_zone = tz.gettz('EST')
today = datetime.date.today()

for i in values["data"]:
    created_time = dateparser.parse(i["created_time"])
    created_time = created_time.replace(tzinfo=utc_zone)
    created_time = created_time.astimezone(est_zone)
    created_time = datetime.datetime.date(created_time)
    if created_time == today:
	if i.get("message") is not None: #check for posts actually containing a message (photo only posts will fail)
	    homicide = ['District\nHomicide', 'District Homicide'] #the two strings I've seen the BPD post
	    for phrase in homicide:
		message = i.get("message")
                #is_homicide=1
		if phrase in message:
                    is_homicide=1
                    print message

if is_homicide:
    mqttc.publish("homicide", "1")
    print "homicide"
else:
    mqttc.publish("homicide", "0")
    print "no homicide"
