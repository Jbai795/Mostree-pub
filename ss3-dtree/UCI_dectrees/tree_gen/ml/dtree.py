#!/usr/bin/env python3
# -*-coding:utf-8 -*-

import numpy as np
import pandas as pd
import os
from sklearn import metrics, preprocessing, tree
from sklearn.model_selection import train_test_split
from sklearn.model_selection import GridSearchCV
import joblib
import pickle
from loaders import *

if __name__ == '__main__':

  datasets = [
    # load_wdbc_breast_cancer_data(WDBC_BREAST_CANCER_DATA),
    # load_simple_breast_cancer_data(SIMPLE_BREAST_CANCER_DATA),
    # load_audiology_data(AUDIOLOGY_TRAIN_DATA, AUDIOLOGY_TEST_DATA),
    #load_nursery_data(NURSERY_DATA),
    # load_credit_data(CREDIT_DATA),
    # load_ecg_data(ECG_DATA),
    # load_housing_data(HOUSING_DATA),
     load_spambase_data(SPAMBASE_DATA),
  ]

  np.seterr(all='raise')

  # PRINT_TREES=False
  PRINT_TREES=True
  if PRINT_TREES:
    import io
    import pydotplus

  for idx, (X_train, X_test, Y_train, Y_test) in enumerate(datasets):

    clf = tree.DecisionTreeClassifier(max_depth=17)
    print(clf)
    clf.fit(X_train, Y_train)

    # joblib.dump(clf,'clf.pkl')
    # f = open('clf.pkl', 'rb')
    # data = pickle.load(f)
    # f.close()
    # print(data)  # show file

    print("metrics on training data")
    print(metrics.classification_report(Y_train, clf.predict(X_train)))
    print()

    print("metrics on testing data")
    print(metrics.classification_report(Y_test, clf.predict(X_test)))

    if PRINT_TREES:
      dot_data = io.StringIO()
      tree.export_graphviz(clf, out_file = "spambase.dot")
      #graph = pydotplus.graph_from_dot_data(dot_data.getvalue())
      #graph.write_dot("dtree-%d.dot" % (idx))
      #graph.write_pdf("dtree-%d.pdf" % (idx))
