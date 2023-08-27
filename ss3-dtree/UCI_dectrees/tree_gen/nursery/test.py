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
    with open('nursery.data', 'r') as fr:                                        #加载文件
        nursery = [inst.strip().split(',') for inst in fr.readlines()]        #处理文件
        cleanoutdata(nursery)        
    nursery_target = []                                                        #提取每组数据的类别，保存在列表里
    for each in nursery:
        nursery_target.append(each[-1])

    nurseryLabels = ['parents', 'has_nurs', 'form', 'children', 'housing', 'finance'
	, 'social', 'health']            #特征标签       
    nursery_list = []                                                        #保存nursery数据的临时列表
    nursery_dict = {}                                                        #保存nursery数据的字典，用于生成pandas
    for each_label in nurseryLabels:                                            #提取信息，生成字典
        for each in nursery:
            nursery_list.append(each[nurseryLabels.index(each_label)])
        nursery_dict[each_label] = nursery_list
        nursery_list = []
    # print(nursery_dict)                                                        #打印字典信息
    nursery_pd = pd.DataFrame(nursery_dict)                                    #生成pandas.DataFrame
    print(nursery_pd)                                                        #打印pandas.DataFrame
    le = LabelEncoder()                                                        #创建LabelEncoder()对象，用于序列化            
    for col in nursery_pd.columns:                                            #为每一列序列化
        nursery_pd[col] = le.fit_transform(nursery_pd[col])
    print(nursery_pd)
