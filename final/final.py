#!/usr/bin/env python
# coding: utf-8
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

capture = cv2.VideoCapture(0)
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
    cv2.imwrite("D:/snapshot/snapshot-%d.png" % fileno, frame)     # save frame as JPEG file
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
bucket = 'knujongp1'

filenames = os.listdir('D:/snapshot')
for filename in filenames:
    full_filename = os.path.join('D:/snapshot', filename)
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


def visual():
   df = pd.read_csv('output.txt',names=['timeclass','sex','age','emotion'],header=None)

   cnt = len(df['timeclass']) #반복문을 돌리기 위함.
   cnttime = df['timeclass'][cnt-1]+1
  
   index_time=list()
   cnt_p = list(0 for _ in range(cnttime))
   cnt_n = list(0 for _ in range(cnttime))
   cnt_normal = list(0 for _ in range(cnttime))
   positive_age = list(0 for _ in range(9))
   negative_age = list(0 for _ in range(9))
   normal_age = list(0 for _ in range(9))
   labels = ['10','20','30','40','50','60','70','80','90']
   

   for i in range(cnttime):
     index_time.append(i+1)
   for i in range(cnt):
     if(df['emotion'][i] == 'Normal'):
       index_normal=df['timeclass'][i]
       cnt_normal[index_normal]+=1
       #연령index저장
       index_normal_age = df['age'][i]//10-1
       normal_age[index_normal_age]+=1
      
     if(df['emotion'][i] == 'Positive'):
       index_positive = df['timeclass'][i]
       cnt_p[index_positive]+=1

       index_positive_age = df['age'][i]//10-1
       positive_age[index_positive_age]+=1
     if(df['emotion'][i] == 'Negative'):
       index_negative = df['timeclass'][i]
       cnt_n[index_negative]+=1

       index_negative_age = df['age'][i]//10-1
       negative_age[index_negative_age]+=1

   plt.plot(index_time, cnt_p, color = 'r')
   plt.plot(index_time, cnt_n, color = 'g')
   plt.plot(index_time, cnt_normal, color='b')
   plt.title('time-emotion graph',fontsize = 20)
   plt.xlabel('$time$', fontsize = 10)
   plt.ylabel('Number Of People', fontsize = 10)
   plt.legend(('Positive','Negative','Normal'), fontsize = 10)
   plt.show()

   df_age = pd.DataFrame({'Positive':positive_age,'Normal':normal_age,'Negative':negative_age},index=labels)
   ax = df_age.plot.bar(rot=0)
   ax.set_title('Age-emotional graph',fontsize = 20)
   ax.set_ylabel('$emotion$',fontsize = 10)
   ax.set_xlabel('$ages$',fontsize = 10)
   plt.show()


# 전체 감정 별 성별 분포도
   nor_mcnt = 0
   nor_fcnt = 0
   p_mcnt = 0
   p_fcnt = 0
   n_mcnt = 0
   n_fcnt = 0
   for i in range(cnt):
       if(df['emotion'][i] == 'Normal'):
           if(df['sex'][i] == 'M'):
               nor_mcnt += 1
           if(df['sex'][i] == 'F'):
               nor_fcnt += 1
       if(df['emotion'][i] == 'Positive'):
           if(df['sex'][i] == 'M'):
               p_mcnt += 1
           if(df['sex'][i] == 'F'):
               p_fcnt += 1
       if(df['emotion'][i] == 'Negative'):
           if(df['sex'][i] == 'M'):
               n_mcnt += 1
           if(df['sex'][i] == 'F'):
               n_fcnt += 1

   male_cnt = [p_mcnt,nor_mcnt,n_mcnt]
   female_cnt = [p_fcnt,nor_fcnt,n_fcnt]

   index_mf = ['Positive','Normal','Negative']
   df_mf = pd.DataFrame({'male':male_cnt,
                        'female':female_cnt}, index=index_mf)
   ax = df_mf.plot.bar(rot=0)
   plt.title("Entire Emotion",fontsize=20)
   plt.xlabel("$Emotion$")
   plt.ylabel("$Number-of-People$")
   plt.show()




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
        visual() 
