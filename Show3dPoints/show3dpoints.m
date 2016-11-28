clear all
close all
clc

pts = load('3dpoints.txt');
figure
plot3(pts(:, 1), pts(:, 2), pts(:, 3));
