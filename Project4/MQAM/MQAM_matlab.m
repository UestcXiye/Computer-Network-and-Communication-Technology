
function [info]=random_binary(N)
if naegin == 0
    N=10000;
end
for i=1:N
    temp=rand;
    if temp<0.5
        info(i)=0;
    else
        info(i)=1;
    end
end
I=x(1:2:nn-1);
[I,In]=two2four(1,4*m);
Q=x(2:2:nn);
[Q,Qn]=two2four(Q,4*m);
if Kbase == 2
    I=bshape(I,fs,fb/4);
    Q=bshape(Q,fs,fb/4);
end
y=I.*cos(2*pi*fc*t)-Q.*sin(2*pi*fc*t);
two2four.m
function [y,yn]=two2four(x,m)
T=[0 1;3 2];
n=length(x);
ii=1;
for i=1:2:n-1
    xi=x(i:i+1)+1;
    yn(ii)=T(xi(1),xi(2));
    ii=ii+1;
end
yn=yn-1.5;
y=yn;
for i=1:m-1
    y=[y;yn];
end
y=y(:)';
function c=constel(x,fs,fb,fc)
N=length(x);
m=2*fs/fb;
n=fs/fc;
i1=m-n;
i=1;
ph0=(i1-1)*2*pi/n;
while i<=N/m
    xi=x(i1:i1+n-1);
    y=2*fft(xi)/n;
    c(i)=y(2);
    i=i+1;
    i1=i1+m;
end
if nargout<1
    cmax=max(abs(c));
    ph=(0:5:360)*pi/180;
    plot(1.414*cos(ph),1.414*sin(ph),'c');
    hold on;
    for i=1:length(c)
        ph=ph0-angle(c(i));
        a=abs(c(i))/cmax*1.414;
        plot(a*cos(ph),a*sin(ph),'r*');
    end
    plot([-1.5 1.5],[0 0],'k:',[0 0],[-1.5 1.5],'k:');
    hold off;
    axis equal;
    axis([-1.5 1.5 -1.5 1.5]);
end