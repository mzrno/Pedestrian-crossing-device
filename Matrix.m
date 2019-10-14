function Matrix=mymatrix(Vector) %input an array vector
angle=Vector(1:2:(length(Vector))); %store every value in odd position of array into variable
distance=Vector(2:2:(length(Vector))); %store every value in even position of array into variable
x=distance.*cos(angle*pi/180); %calculate x coordinate
y=distance.*sin(angle*pi/180); %calculate y coordinate
X=round(x./10)+(abs(round(min(x./10)))+2); %devide every x coordinate with 10 and round the value
Y=round(y./10)+(abs(round(min(y./10)))+2); %devide every y coordinate with 10 and round the value
[val,ind]=find(angle<360); %find every value and index for angle lower than 360
Matrix=zeros(max(abs(Y))+1,max(abs(X))+1); %make a matrix of zeros
for i=min(abs(ind)):((max(abs(ind)+1))-1) 
    Matrix(abs(Y(i)),abs(X(i)))=1; %in specified X and Y coordinates of Matrix place value 1
end
Matrix=flip(Matrix);
imagesc(Matrix) %display matrix with scaled colors
title("Matrix of space")
pause
end