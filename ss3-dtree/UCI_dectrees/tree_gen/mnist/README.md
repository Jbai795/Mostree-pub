# handwrittendigit-recognition
Hand written digit recognition with SVM, Decision Tree and Random Forests

MNIST dataset is already given under Datasets folder. 

svmfile.py directly works with svm on mnist dataset. 
decisiontreefile.py works with decision tree on mnist dataset.
randomforestsfile.py works with random forests on mnist dataset.

In source codes, some of the lines are commented. Especially different kernels which
works less successful are commented. You can uncomment them in order to test the system
with different kernels.

For classification with decision tree, the decision tree is drawn in the end. In order 
to see it more precisely, please zoom in because decision tree is a bit huge.

Loader of the mnist dataset is taken from: https://github.com/sorki/python-mnist
Its usage:
from mnist import MNIST
mndata = MNIST('./dir_with_mnist_data_files')
mndata.load_training()
mndata.load_testing()

Even I put the dataset in git, you can reach it via this link: http://yann.lecun.com/exdb/mnist/

There are comments in source code, and it is enough for general understanding.

