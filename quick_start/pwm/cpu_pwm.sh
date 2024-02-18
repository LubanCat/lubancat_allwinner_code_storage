#!/bin/bash

tempstart=55  # 风扇启动的温度阈值，单位为摄氏度
period=1000000 #PWM周期
cpu_temp1=65 #节点1
cpu_temp1_pwm=50 #节点1占空比,%
cpu_temp2=75 #节点2
cpu_temp2_pwm=80  #节点2占空比,%
cpu_temp3=85 #节点3
cpu_temp3_pwm=100 #节点3占空比,%

#将pwm1导出到用户空间
echo 1 > /sys/class/pwm/pwmchip0/export
#设置pwm周期
sudo echo $period > /sys/class/pwm/pwmchip0/pwm1/period
#设置PWM周期中“ON”的初始时间
echo 0 > /sys/class/pwm/pwmchip0/pwm1/duty_cycle
#设置pwm极性
echo "normal" > /sys/class/pwm/pwmchip0/pwm1/polarity
#使能pwm
echo 1 > /sys/class/pwm/pwmchip0/pwm1/enable

while true; do
   temp=$(cat /sys/class/thermal/thermal_zone0/temp)
   temp=$((temp/1000))  # 将温度转换为摄氏度
   
   if ((temp > tempstart)); then
      echo 1 > /sys/class/pwm/pwmchip0/pwm1/enable
      if ((temp < cpu_temp1)); then
         cpu_pwm1=$(( cpu_temp1_pwm*(temp-tempstart)/(cpu_temp1-tempstart)*period/100 ))
	      echo $cpu_pwm1 > /sys/class/pwm/pwmchip0/pwm1/duty_cycle
         # echo "CPU Temp: ${temp}℃ CPU PWM1: ${cpu_pwm1}"
      elif ((temp > cpu_temp1))&&((temp < cpu_temp2)); then
         cpu_pwm2=$(( ( (cpu_temp2_pwm-cpu_temp1_pwm)*(temp-cpu_temp1)/(cpu_temp2-cpu_temp1)+cpu_temp1_pwm)*period/100 ))
         echo $cpu_pwm2 > /sys/class/pwm/pwmchip0/pwm1/duty_cycle
         # echo "CPU Temp: ${temp}℃ CPU PWM2: ${cpu_pwm2}"
      elif ((temp > cpu_temp2))&&((temp < cpu_temp3)); then
         cpu_pwm3=$(( ( (cpu_temp3_pwm-cpu_temp2_pwm)*(temp-cpu_temp2)/(cpu_temp3-cpu_temp2)+cpu_temp2_pwm)*period/100 ))
         echo $cpu_pwm3 > /sys/class/pwm/pwmchip0/pwm1/duty_cycle
         # echo "CPU Temp: ${temp}℃ CPU PWM3: ${cpu_pwm3}"
      elif ((temp > cpu_temp3)); then
         cpu_pwm_end=$(( cpu_temp3_pwm*period/100 ))
         echo $cpu_pwm_end > /sys/class/pwm/pwmchip0/pwm1/duty_cycle
         # echo "CPU Temp: ${temp}℃ CPU PWM_END: ${cpu_pwm_end}"
      fi

   else
      echo 0 > /sys/class/pwm/pwmchip0/pwm1/duty_cycle
      echo 0 > /sys/class/pwm/pwmchip0/pwm1/enable
      # echo "CPU Temp: ${temp}℃ CPU风扇关闭"
   fi

   sleep 30
done
