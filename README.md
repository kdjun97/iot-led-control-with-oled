# 자동 전등 제어 + 센서 데이터 OLED 출력  

[작동 영상 : Youtube](https://www.youtube.com/watch?v=c-hRML66dDU)  
[참고 : 기술 블로그](https://kdjun97.github.io/iot/auto-led-control-with-oled/)  

`기능`  
1. 1초마다 LED가 깜빡거림.
1. 온습도 데이터를 1초마다 읽고 OLED에 출력
1. 조도 센서로 밝기를 측정 후, 어두워지면 전등에 불이 10초간 켜짐
1. 불이 켜지고 10초 카운트 중, 다시 불을 켰다가 끄면 10초 카운트 리셋됨