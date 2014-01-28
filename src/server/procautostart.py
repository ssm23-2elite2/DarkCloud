import os
import sys 
import time
import datetime

cmd = " ".join( sys.argv[1:] )

while 1:
	try:
		st = datetime.datetime.now()

		os.system( cmd )

		et = datetime.datetime.now()
		elapsed = (et-st).seconds * 1000000 + (et-st).microseconds
		
		if elapsed < 500000:    # 0.5s
			break
	except:
		time.sleep( 2 ) 

