import cv2
import time
import boto3
import os
from operator import itemgetter
from pandas import DataFrame
import boto3
import json
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import logging
from botocore.exceptions import ClientError

capture = cv2.VideoCapture(1)
capture.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
capture.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)

init_time = time.time()
counter_timeout = init_time+1
counter = 1
fileno=0

while(capture.isOpened()):
 ret, frame = capture.read()

 if ret==True:
  if (time.time() > counter_timeout):
   print(counter)#time clock
   counter+=1
   counter_timeout+=1

   if(counter%3==0):#3초지났을때 캡처
    cv2.imwrite("./snapshot/snapshot-%d.png" % fileno, frame)     # save frame as JPEG file
    fileno+=1

  cv2.imshow('frame', frame)
  if cv2.waitKey(10) == 27:   #esc   
   break

 else:
  break
    # Release everything if job is finished
capture.release()
cv2.destroyAllWindows()




# S3 클라이언트 생성
s3 = boto3.client('s3')
# 업로드할 S3 버킷
bucket = 'kubinn'

filenames = os.listdir('C:/Users/user/Desktop/3학년2학기/snapshot')
for filename in filenames:
    full_filename = os.path.join('C:/Users/user/Desktop/3학년2학기/snapshot', filename)
    # 첫본째 매개변수 : 로컬에서 올릴 파일이름
    # 두번째 매개변수 : S3 버킷 이름
    # 세번째 매개변수 : 버킷에 저장될 파일 이름.
    s3.upload_file(full_filename, bucket, filename)
    os.remove(full_filename)





sns.set(style='whitegrid')

def info(i, flag):
   name = 'snapshot-' + str(i) + '.png'
   photo = name
   client = boto3.client('rekognition')
   age = 0

   response = client.detect_faces(Image={'S3Object': {'Bucket': bucket, 'Name': photo}}, Attributes=['ALL'])
   print('Detected faces for ' + photo)
   for faceDetail in response['FaceDetails']:
      print('The detected face is between ' + str(faceDetail['AgeRange']['Low'])+ ' and ' + str(faceDetail['AgeRange']['High']) + ' years old')
      print('Here are the other attributes:')
        # print(str(faceDetail['Emotions'][0]))
        # print(len(faceDetail['Emotions']))
      data = sorted(faceDetail['Emotions'],key=itemgetter('Confidence'),reverse=True)
    #   for d in data:
    #      print(d)
      age = (faceDetail['AgeRange']['Low'] + faceDetail['AgeRange']['High'])//2
      age = (age//10)*10
      f.write(str(i)+',')
      f.write(str(faceDetail['Gender']['Value'][0])+',')
      f.write(str(age)+',')
      if(data[0]['Type']=='HAPPY' and data[0]['Confidence']>=90):
         f.write('Positive')
      elif(data[0]['Type']=='HAPPY' and data[1]['Type']=='CALM'):
         f.write('Positive')
      elif(data[0]['Type']=='CALM' and data[1]['Type']=='SAD'):
         f.write('Normal')
      elif(data[0]['Type']=='CALM' and data[1]['Type']=='CONFUSED'):
         f.write('Normal')
      elif(data[0]['Type'] =='CALM' and data[1]['Type'] == 'HAPPY'):
         f.write('Normal')
      elif(data[0]['Type'] =='SAD' and data[1]['Type'] == 'ANGRY'):
         f.write('Negative')
      elif(data[0]['Type'] =='SAD' and data[1]['Type'] == 'CALM'):
         f.write('Negative')
      elif(data[0]['Type'] =='SAD' and data[1]['Type'] == 'ANGRY'):
         f.write('Negative')
      elif(data[0]['Type'] =='SAD' and data[0]['Confidence'] >= 90):
         f.write('Negative')
      else:
         f.write('Normal')
      f.write('\n')
   s3.delete_object(Bucket=bucket,Key=name)

   # print(str(faceDetail['Gender']['Value']))
   # for i in range(len(faceDetail['Emotions'])):
      # if(first < faceDetail['Emotions'][i]['Confidence']):
         # firstnum = i
   # print(json.dumps(faceDetail['Gender'], indent=4, sort_keys=True))


# def visual():
#    df = pd.read_csv('output.txt',names=['timeclass','sex','age','emotion'],header=None)
#    print(df)

#    cnt = len(df['timeclass']) #반복문을 돌리기 위함.
#    search = 12 #특정 시간을 지금 5로 둔 것임
#    # search_e = []

#    index_emo = list()
#    index_age = list()
#    index_sex = list()
#    index_age_p = list() #연령대별 positive
#    index_age_n = list() #연령대별 negative
#    index_age_normal = list() #연령대별 normal
#    searchcnt = 0

#    # print(index_emo)
#    # print(index_age)
#    # print(index_sex)
#    # print(index_age_p)
#    # print(index_age_n)

#    #특정 시간에 해당하는 사람들 출력
#    for i in range(cnt):
#       if df['timeclass'][i] == search:
#          index_emo.append(df['emotion'][i])
#          index_age.append(df['age'][i])
#          index_sex.append(df['sex'][i])

#          if df['emotion'][i] == 'Positive':
#             index_age_p.append(df['age'][i])

#          if df['emotion'][i] == 'Negative':
#             index_age_n.append(df['age'][i])

#          if df['emotion'][i] == 'Normal':
#             index_age_normal.append(df['age'][i])

#          searchcnt = searchcnt+1


#    # #dataframe에 넣기!!
#    data = {'emo' : index_emo, 'age': index_age}
#    data_p = {'age_p': index_age_p}
#    data_n = {'age_n': index_age_n}
#    data_normal = {'age_normal': index_age_normal}
#    # print(data)
#    # print(data_p)
#    # print(data_n)

#    frame = pd.DataFrame(data)
#    frame_p = pd.DataFrame(data_p)
#    frame_n = pd.DataFrame(data_n)
#    frame_normal = pd.DataFrame(data_normal)
#    # frame

#    # print(frame_p)

#    #감정 pie chart 생성
#    emo = frame['emo'].value_counts()
#    emo.plot.pie(subplots = True)
#    plt.show()

#    emo.value_counts()

#    # 특정시간에서의 연령별 반응(긍정/부정)분포도
#    #긍정_pie chart
#    age_positive = frame_p['age_p'].value_counts()
#    if not index_age_p:
#       print ("positve_empty")
#    else:
#       age_positive.plot.pie(subplots = True)
#       plt.show()

#    # age_positive.value_counts()

#    #부정_pie chart
#    age_negative = frame_n['age_n'].value_counts()
#    if not index_age_n:
#       print ("negative_empty")
#    else:
#       age_negative.plot.pie(subplots = True)
#       plt.show()


#    #normal pie chart
#    age_normal = frame_normal['age_normal'].value_counts()
#    if not index_age_normal:
#       print ("normal_empty")
#    else:
#       age_normal.plot.pie(subplots = True)
#       plt.show()



if __name__ == "__main__":
    i = 0
    flag = 0
    f = open('output.txt', mode='a', encoding='utf-8')

    try:
        while (True):
            info(i, flag)
            i += 1
    except:
        f.close()
        # visual()