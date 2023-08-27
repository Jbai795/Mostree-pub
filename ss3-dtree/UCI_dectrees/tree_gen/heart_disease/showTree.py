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
	with open('processed.cleveland.data', 'r') as fr:										#加载文件
		heartDisease = [inst.strip().split(',') for inst in fr.readlines()]		#处理文件
		cleanoutdata(heartDisease)
	heartDisease_target = []														#提取每组数据的类别，保存在列表里
	for each in heartDisease:
		heartDisease_target.append(each[-1])
	# print(heartDisease_target)

	heartDiseaseLabels = ['age', 'sex', 'cp', 'trestbps', 'chol', 'fbs', 'restecg',
	'thalach', 'exang', 'oldpeak', 'slope', 'ca', 'thal']			#特征标签
	heartDisease_list = []														#保存heartDisease数据的临时列表
	heartDisease_dict = {}														#保存heartDisease数据的字典，用于生成pandas
	for each_label in heartDiseaseLabels:											#提取信息，生成字典
		for each in heartDisease:
			heartDisease_list.append(each[heartDiseaseLabels.index(each_label)])
		heartDisease_dict[each_label] = heartDisease_list
		heartDisease_list = []
	# print(heartDisease_dict)														#打印字典信息
	heartDisease_pd = pd.DataFrame(heartDisease_dict)									#生成pandas.DataFrame
	# print(heartDisease_pd)														#打印pandas.DataFrame
	le = LabelEncoder()														#创建LabelEncoder()对象，用于序列化
	for col in heartDisease_pd.columns:											#序列化
		heartDisease_pd[col] = le.fit_transform(heartDisease_pd[col])
	# print(heartDisease_pd)														#打印编码信息
	
	# clf = tree.DecisionTreeClassifier()						#创建DecisionTreeClassifier()类
	clf = tree.DecisionTreeClassifier(splitter = 'best', max_depth = 4)						#创建DecisionTreeClassifier()类
	# clf = tree.DecisionTreeClassifier(splitter = 'best', max_depth = 20, min_samples_split = 20, min_samples_leaf = 5, min_weight_fraction_leaf = 0.2, max_leaf_nodes = 20, )						#创建DecisionTreeClassifier()类
	clf = clf.fit(heartDisease_pd.values.tolist(), heartDisease_target)					#使用数据，构建决策树
	tree.export_graphviz(clf, out_file = "heart_disease")
	# dot_data = StringIO()
	# tree.export_graphviz(clf, out_file = dot_data,							#绘制决策树
	# 					feature_names = heartDisease_pd.keys(),
	# 					class_names = clf.classes_,
	# 					filled=True, rounded=True,
	# 					special_characters=True)
	# graph = pydotplus.graph_from_dot_data(dot_data.getvalue())
	# graph.write_pdf("tree.pdf")