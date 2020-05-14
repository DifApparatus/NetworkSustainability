clear all;
clc;

dir = pwd;
pktDelivery = jsondecode(fileread(strcat(dir,'/pktDelivery.json')));

timeSent = pktDelivery.General_0_20200511_17_26_47_2629.vectors(1).time;
pktSent = pktDelivery.General_0_20200511_17_26_47_2629.vectors(1).value;

timeRec = pktDelivery.General_0_20200511_17_26_47_2629.vectors(2).time;
pktRec = pktDelivery.General_0_20200511_17_26_47_2629.vectors(2).value;

pktSent = pktSent - pktSent(1);
pktRec = pktRec - pktRec(1);
timeSent = timeSent - timeSent(1);
timeRec = timeRec - timeRec(1);

dt=100;%seconds
t = 0:dt:max(timeRec(length(timeRec)),timeSent(length(timeSent)));

averSent = zeros(size(t));
timeSentFrameArray = {};
averRec = zeros(size(t));
timeRecFrameArray = {};
for i=1:length(t)-2
    pktAverSent = mean(pktSent(timeSent>=t(i)&timeSent<t(i+1)));
    averSent(i)=ceil(pktAverSent);
    timeSentFrame = timeSent(timeSent>=t(i) & timeSent<t(i+1))';
    timeSentFrameArray = [timeSentFrameArray;timeSentFrame];
    
    pktAverRec = mean(pktRec(timeRec>=t(i)&timeRec<t(i+1)));
    averRec(i)=ceil(pktAverRec);
    timeRecFrame = timeRec(timeRec>=t(i) & timeRec<t(i+1))';
    timeRecFrameArray = [timeRecFrameArray;timeRecFrame];
end

deliveryRate = averRec./averSent;
Qnes = 0.8;
Q = deliveryRate/Qnes;
deliveryRateFlags = Q<1;
Q_fal = (1 - Q).*deliveryRateFlags;
Q_fal_aver = mean(rmmissing(Q_fal));

plot(t,1-Q_fal);

%%%%%% Resilience %%%%%
R = 1 - Q_fal_aver*sum(deliveryRateFlags)/length(deliveryRateFlags);
