function [y,I,Q]=Qam(x,Kbase,fs,fb,fc)
T=length(x)/fb;
m=fs/fb;
nn=length(x);
dt=1/fs;
t=0:dt:T-dt;
I=x(1:2:nn-1);[I,In]=two2four(I,4*m);
Q=x(2:2:nn);[Q,Qn]=two2four(Q,4*m);
if Kbase == 2
    I=bshape(I,fs,fb/4);Q=bshape(Q,fs,fb/4);
end
y=I.*cos(2*pi*fc*t)-Q.*sin(2*pi*fc*t);