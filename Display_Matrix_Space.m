%Matlab code for analysis and processing data
file=importdata('Your_CSV_File.csv'); %importing csv file
for i=1:length(file) %for loop from 1 to the end of file length
   m=file(i); 
   m=cell2mat(m); %convert cell array to ordinary array
   m=str2num(m); %convert character array or string to numeric array
   m=str2num(m);
   figure(i)     %figure for every iteration in the loop
   mymatrix(m); %function for making and showing matrix of space
end
close all 