# -*- coding: UTF-8 -*-
import pandas as pd
from sklearn.preprocessing import LabelEncoder

import pydotplus
from six import StringIO
# from sklearn.externals.six import StringIO
#It will be in pandas 0.22 module as after upgradation of version , it was removed i.e. sklearn.external.six .

def cleanoutdata(dataset):#数据清洗
    for row in dataset:
        for column in row:
            if column == '?' or column=='':
                dataset.remove(row)
                break

if __name__ == '__main__':
    with open('processed.cleveland.data', 'r') as fr:                                        #加载文件
        heartDisease = [inst.strip().split(',') for inst in fr.readlines()]        #处理文件
        cleanoutdata(heartDisease)        
    heartDisease_target = []                                                        #提取每组数据的类别，保存在列表里
    for each in heartDisease:
        heartDisease_target.append(each[-1])

    heartDiseaseLabels = ['age', 'sex', 'cp', 'trestbps', 'chol', 'fbs', 'restecg',
	'thalach', 'exang', 'oldpeak', 'slope', 'ca', 'thal']            #特征标签       
    heartDisease_list = []                                                        #保存heartDisease数据的临时列表
    heartDisease_dict = {}                                                        #保存heartDisease数据的字典，用于生成pandas
    for each_label in heartDiseaseLabels:                                            #提取信息，生成字典
        for each in heartDisease:
            heartDisease_list.append(each[heartDiseaseLabels.index(each_label)])
        heartDisease_dict[each_label] = heartDisease_list
        heartDisease_list = []
    # print(heartDisease_dict)                                                        #打印字典信息
    heartDisease_pd = pd.DataFrame(heartDisease_dict)                                    #生成pandas.DataFrame
    print(heartDisease_pd)                                                        #打印pandas.DataFrame
    le = LabelEncoder()                                                        #创建LabelEncoder()对象，用于序列化            
    for col in heartDisease_pd.columns:                                            #为每一列序列化
        heartDisease_pd[col] = le.fit_transform(heartDisease_pd[col])
    print(heartDisease_pd)
