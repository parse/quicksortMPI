

timing = [
67.5            255.5           579.25          1104.4300200000    1709.7732680000			
18.2089560000   69              157.5013620000  264.6242420000     789.9681250000
5.25            20.0181380000   41.2632820000   72.5228400000      121.0492110000
1.8208240000    6.0845500000    12.3536760000   20.9815160000      32.2654600000
0.9772460000    2.5014570000    5.0548730000    7.1549150000       15.6029320000
0.7680510000    1.6032780000    2.4814770000    3.7904030000       5.5097070000
];

nprocs = [1 2 4 8 16 32]; 
nelements = [10000000 20000000 30000000 40000000 50000000];

speedup = zeros(6,5);
speedup(:,1) = timing(1,1)./timing(:,1);
speedup(:,2) = timing(1,2)./timing(:,2);
speedup(:,3) = timing(1,3)./timing(:,3);
speedup(:,4) = timing(1,4)./timing(:,4);
speedup(:,5) = timing(1,5)./timing(:,5);

figure(1);
plot(nprocs,timing(:,1), 'r-');
title('Quicksort on MPI');
xlabel('Number of processes'); 
ylabel('Time in seconds');
legend('n = 10 000 000', 'Location','SouthEast');

figure(2);
plot(nprocs,timing(:,2), 'r-');
title('Quicksort on MPI');
xlabel('Number of processes'); 
ylabel('Time in seconds');
legend('n = 20 000 000', 'Location','SouthEast');

figure(3); 
plot(nprocs,timing(:,3), 'r-');
title('Quicksort on MPI');
xlabel('Number of threads'); 
ylabel('Time in seconds');
legend('n = 30 000 000', 'Location','SouthEast');

figure(4);
plot(nprocs,timing(:,4), 'r-');
title('Quicksort on MPI');
xlabel('Number of processes'); 
ylabel('Time in seconds');
legend('n = 40 000 000', 'Location','SouthEast');

figure(5);
plot(nprocs,timing(:,5), 'r-');
title('Quicksort on MPI');
xlabel('Number of processes'); 
ylabel('Time in seconds');
legend('n = 50 000 000', 'Location','SouthEast');

figure(6);
plot(nprocs,speedup(:,1), 'g--');
hold on;
plot(nprocs,speedup(:,2), 'r:');
plot(nprocs,speedup(:,3), 'b-');
plot(nprocs,speedup(:,4), 'b-.');
plot(nprocs,speedup(:,5), 'r-');
title('Quicksort on MPI Speedup');
xlabel('Number of processes'); 
ylabel('Speedup');
legend('n = 10 000 000', 'n = 20 000 000', 'n = 30 000 000', 'n = 40 000 000', 'n = 50 000 000',  'Location','SouthEast');

