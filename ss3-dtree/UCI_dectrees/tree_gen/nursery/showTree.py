# -*- coding: UTF-8 -*-
from sklearn.preprocessing import LabelEncoder, OneHotEncoder
from six import StringIO
from sklearn import tree
import pandas as pd
import numpy as np
import pydotplus

def cleanoutdata(dataset):#数据清洗
    for row in dataset:
        for column in row:
            if column == '?' or column=='':
                dataset.remove(row)
                break


if __name__ == '__main__':
	with open('nursery.data', 'r') as fr:										#加载文件
		nursery = [inst.strip().split(',') for inst in fr.readlines()]		#处理文件
		cleanoutdata(nursery)
	nursery_target = []														#提取每组数据的类别，保存在列表里
	for each in nursery:
		nursery_target.append(each[-1])
	# print(nursery_target)

	nurseryLabels = ['parents', 'has_nurs', 'form', 'children', 'housing', 'finance'
	, 'social', 'health']			#特征标签
	nursery_list = []														#保存nursery数据的临时列表
	nursery_dict = {}														#保存nursery数据的字典，用于生成pandas
	for each_label in nurseryLabels:											#提取信息，生成字典
		for each in nursery:
			nursery_list.append(each[nurseryLabels.index(each_label)])
		nursery_dict[each_label] = nursery_list
		nursery_list = []
	# print(nursery_dict)														#打印字典信息
	nursery_pd = pd.DataFrame(nursery_dict)									#生成pandas.DataFrame
	# print(nursery_pd)														#打印pandas.DataFrame
	le = LabelEncoder()														#创建LabelEncoder()对象，用于序列化
	for col in nursery_pd.columns:											#序列化
		nursery_pd[col] = le.fit_transform(nursery_pd[col])
	# print(nursery_pd)														#打印编码信息

	clf = tree.DecisionTreeClassifier(max_depth = 4)						#创建DecisionTreeClassifier()类
	clf = clf.fit(nursery_pd.values.tolist(), nursery_target)	
	
	# 只需这一句就可以生成kiss文章里的decision tree的格式
	tree.export_graphviz(clf, out_file = "nursery")
	
	# 生成很全的树
	# dot_data = StringIO()

	# # tree.export_graphviz(clf, out_file = dot_data,							#绘制决策树
	# # 					feature_names = nursery_pd.keys(),
	# # 					class_names = clf.classes_,
	# # 					filled=True, rounded=True,
	# # 					special_characters=True)
	# graph = pydotplus.graph_from_dot_data(dot_data.getvalue())
	# graph.write_dot("tree.dot")
	# graph.write_pdf("tree.pdf")

	
	