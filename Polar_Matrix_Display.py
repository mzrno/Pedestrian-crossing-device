#Python code for analysis and processing data
import matplotlib.pyplot as plt
import numpy as np
plt.style.use('ggplot')
import csv
with open("Your_CSV_File.csv", "r") as csv_file:  #extracting data from csv file
    data = csv.reader(csv_file, delimiter=',')
    for lines in data:
      h=len(lines)
for k in range(h):
    d=lines[k]
    f=d.replace(",","")
    e=f.split()
    Z=[]
    for j in range(len(e)):
        Z.append(float(e[j]))
    MAT=np.asarray(Z)  #putting data from csv file to an array
    angle=np.array(MAT[range(0,len(MAT),2)]) #store every value in odd position of array into variable
    dist=np.array(MAT[range(1,len(MAT),2)])  #store every value in even position of array into variable
    x=dist*np.cos(angle*np.pi/180)  #calculate x coordinate
    y=dist*np.sin(angle*np.pi/180)  #calculate y coordinate
    X=np.round(x/10)+(np.abs(np.round(np.min(x/10)))+2)  #devide every x coordinate with 10 and round the value
    Y=np.round(y/10)+(np.abs(np.round(np.min(y/10)))+2)  #devide every y coordinate with 10 and round the value
    a=np.max(np.abs(Y))+2
    b=np.max(np.abs(X))+2
    A=np.zeros((int(a),int(b))) #make a matrix of zeros
    A=A.astype(int)
    for i in range(len(angle)):
        A[int(Y[i])][int(X[i])]=1  #in specified X and Y coordinates of Matrix place value 1
    A=np.flipud(A)
    print(A.shape)
    fig = plt.figure()
    ax = fig.add_subplot(111, polar=True)  #polar display of space
    c = ax.scatter((angle*np.pi/180),dist)
    fig.suptitle('Polar display', fontsize=14)
    fig.show()
    fig1 = plt.figure()
    ax1 = fig1.add_subplot(111)
    C=ax1.imshow(A, extent=[np.min(X), np.max(X), np.min(Y), np.max(Y)])  #display matrix with scaled colors
    fig1.suptitle('Matrix display', fontsize=14)
    fig1.show()