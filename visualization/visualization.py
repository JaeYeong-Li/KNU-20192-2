#Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#PDX-License-Identifier: MIT-0 (For details, see https://github.com/awsdocs/amazon-rekognition-developer-guide/blob/master/LICENSE-SAMPLECODE.)
# -*- coding: utf-8 -*-

from operator import itemgetter
from pandas import DataFrame
import boto3
import json
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
sns.set(style='whitegrid')


def info(i, flag):
        name = 'normal-'+str(i)+'.png'
        photo = name
        bucket = 'jeongjiu'
        client = boto3.client('rekognition')
        age = 0

        response = client.detect_faces(
            Image={'S3Object': {'Bucket': bucket, 'Name': photo}}, Attributes=['ALL'])

        print('Detected faces for ' + photo)
        for faceDetail in response['FaceDetails']:
            print('The detected face is between ' + str(faceDetail['AgeRange']['Low'])
                  + ' and ' + str(faceDetail['AgeRange']['High']) + ' years old')
            print('Here are the other attributes:')
            #print(str(faceDetail['Emotions'][0]))
            #print(len(faceDetail['Emotions']))
            data = sorted(faceDetail['Emotions'],
                          key=itemgetter('Confidence'), reverse=True)
            for d in data:
             print(d)
            age = (faceDetail['AgeRange']['Low'] +
                   faceDetail['AgeRange']['High'])//2
            age = (age//10)*10
            f.write(str(i)+',')
            f.write(str(faceDetail['Gender']['Value'][0])+',')
            f.write(str(age)+',')
            if (data[0]['Type'] == 'HAPPY' and data[0]['Confidence'] >= 90):
                f.write('Positive')
            elif (data[0]['Type'] == 'HAPPY' and data[1]['Type'] == 'CALM'):
                f.write('Positive')
            elif (data[0]['Type'] == 'CALM' and data[1]['Type'] == 'SAD'):
                f.write('Normal')
            elif(data[0]['Type'] == 'CALM' and data[1]['Type'] == 'CONFUSED'):
                f.write('Normal')
            elif(data[0]['Type'] == 'CALM' and data[1]['Type'] == 'HAPPY'):
                f.write('Normal')
            elif(data[0]['Type'] == 'SAD' and data[1]['Type'] == 'CALM'):
                f.write('Negative')
            elif(data[0]['Type'] == 'SAD' and data[1]['Type'] == 'ANGRY'):
                f.write('Negative')
            elif (data[0]['Type'] == 'ANGRY' and data[0]['Confidence'] >= 90):
                f.write('Negative')
            elif (data[0]['Type'] == 'SAD' and data[0]['Confidence'] >= 90):
                f.write('Negative')
            elif (data[0]['Type'] == 'DISGUSTED' and data[0]['Confidence'] >= 90):
                f.write('Negative')
            else:
                f.write('Normal')

            f.write('\n')

        #print(str(faceDetail['Gender']['Value']))
        #for i in range(len(faceDetail['Emotions'])):
        #  if(first < faceDetail['Emotions'][i]['Confidence']):
        #     firstnum = i
        #print(json.dumps(faceDetail['Gender'], indent=4, sort_keys=True))


def visual():
   df = pd.read_csv('output.txt',names=['timeclass','sex','age','emotion'],header=None)
   print(df)
   cnt = len(df['timeclass']) #반복문을 돌리기 위함.
   cnttime = df['timeclass'][cnt-1]+1
   #print(cnt)
   #print(cnttime)
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
  #  print(cnt_normal)
  #  print(cnt_p)
  #  print(cnt_n)
   plt.plot(index_time, cnt_p, color = 'r')
   plt.plot(index_time, cnt_n, color = 'g')
   plt.plot(index_time, cnt_normal, color='b')
   plt.title('time-emotion graph',fontsize = 20)
   plt.xlabel('$time$', fontsize = 10)
   plt.ylabel('Number Of People', fontsize = 10)
   plt.legend(('Positive','Negative','Normal'), fontsize = 10)
   plt.show()
#    print(normal_age)
#    print(positive_age)
#    print(negative_age)

   df_age = pd.DataFrame({'Positive':positive_age,'Normal':normal_age,'Negative':negative_age},index=labels)
   ax = df_age.plot.bar(rot=0)
   ax.set_title('Age-emotional graph',fontsize = 20)
   ax.set_ylabel('$emotion$',fontsize = 10)
   ax.set_xlabel('$ages$',fontsize = 10)
   plt.show()


  
   #특정시간에서 감정 분포도 차트

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
    f.close()
    visual()
    # try:
    #     while(True):
    #         info(i, flag)
    #         i += 1
    # except:
    #     f.close()
    #     visual()
