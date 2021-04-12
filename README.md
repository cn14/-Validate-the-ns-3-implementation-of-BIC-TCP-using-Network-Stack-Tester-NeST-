# Validate the ns-3 implementation of BIC TCP using Network Stack Tester (NeST)

Brief: ​Binary Increase Congestion control (BIC) is a precursor to CUBIC and is targeted for
Long Fat Networks.    
It uses a binary search algorithm to find the optimal value of congestion
window (​cwnd​).   
In this project, the aim is to validate ns-3 BIC implementation by comparing
the results obtained from it to those obtained by emulating Linux BIC in NeST.   
Required experience:​ C and C++   
Bonus experience:​ Knowledge of BIC and TCP implementation in ns-3   
Difficulty:​ Moderate
Recommended Reading:   
● NeST (Link: ​https://gitlab.com/nitk-nest/nest​)    
● Linux kernel code (Link:https://elixir.bootlin.com/linux/v5.11-rc7/source/net/ipv4/tcp_bic.c​)   
● BIC Paper (Link: ​https://ieeexplore.ieee.org/abstract/document/1354672/​)   
● ns-3 code for BIC (Path: ns-3.xx/src/internet/model/tcp-bic.{h, cc})   


## Result plots:
https://github.com/cn14/Validate-the-ns-3-implementation-of-BIC-TCP-using-Network-Stack-Tester-NeST/blob/main/results.pdf   
Note:Sometimes github not loading pdf.please download pdf to see results.


