import urllib.request
import json
import datetime

import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import numpy as np
import math


url = 'https://slack.com/api/conversations.history?token=[TOKEN]&channel=[CHANNEL]&pretty=1&limit='+str(6*12)

# ============  SlackからJsonを取得  ============
req = urllib.request.Request(url)
with urllib.request.urlopen(req) as res:
    body = res.read()

# ============  Jsonをパース  ============
parse_text=json.loads(body)

# ============  (X,Y) のデータを準備  ============
x=[]; y=[]
for i in parse_text["messages"]:
	x.insert(0,datetime.datetime.fromtimestamp(float(i["ts"])))
	y.insert(0,float(i["attachments"][0]["text"].split(' ')[1]))

# ============  プロット  ============
ax = plt.subplot()
ax.plot(x, y, marker=".")

xfmt = mdates.DateFormatter("%H")
xloc = mdates.HourLocator()
ax.xaxis.set_major_locator(xloc)
ax.xaxis.set_major_formatter(xfmt)

ax.set_xlim(x[0], x[-1])
ax.set_ylim([10,40])
plt.xlabel("Time [hour]")
plt.ylabel("Tmp [℃]")

ax.grid(True)

plt.show()
