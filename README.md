# EL334 Project: Digital Stopwatch & High-Frequency PWM Melody Player (FRDM-K64F)

ระบบนาฬิกาจับเวลาดิจิทัลความละเอียด 1 วินาที (`0000`–`9999`) พร้อมเครื่องเล่นทำนองเพลง *Jingle Bells* ด้วยสัญญาณ PWM ความถี่สูง พัฒนาด้วยภาษา C ระดับรีจิสเตอร์ (Bare-Metal Register Programming) บนบอร์ด **NXP FRDM-K64F** ร่วมกับ **Multifunctional Shield**

---

## 🎯 คุณสมบัติหลักของระบบ (Key Features)
* **Exact 1-Second Digital Stopwatch:** นับเวลาทีละ `1.00 วินาที` แม่นยำจากฐานเวลา `PIT0` (`0.01s` Base Tick ที่ Bus Clock `20.97152 MHz`) แสดงผลบนจอ 7-Segment 4 หลักโดยเริ่มจากหลักหน่วยทางฝั่งขวาสุด
* **High-Frequency PWM Melody Player:** สร้างเสียงทำนองเพลง *Jingle Bells* (4 ท่อนวนซ้ำ) ด้วยโมดูล `FTM0_CH6` ในย่านความถี่ `2.0 – 5.5 kHz` ร่วมกับการปรับ Duty Cycle และตัดเสียงเงียบสนิทด้วยโหมด High-Impedance (`Hi-Z`) ในช่วงพักโน้ต
* **Concurrent Multitasking Architecture:** แยกตัวหารเวลาของนาฬิกาจับเวลา (`STOPWATCH_SPEED = 100`) และจังหวะเพลง (`MELODY_SPEED = 12`) ออกจากกันอย่างอิสระ ทำให้จับเวลาและเล่นเพลงพร้อมกันได้โดยหน้าจอไม่กระพริบหรือหน่วง
* **Non-Blocking Timestamp Debounce:** ป้องกันปัญหาหน้าสัมผัสปุ่มกระเด้งด้วยเงื่อนไขเวลา `200 ms` ภายใน `PORTB_IRQHandler` โดยไม่ใช้ลูปหน่วงเวลา (Blocking Delay)

---

## 🕹️ วิธีใช้งานปุ่มกดและสถานะไฟ (Controls & Indicators)

| ปุ่มกด / อุปกรณ์ | ขาพอร์ต (FRDM-K64F) | หน้าที่การทำงาน |
| :--- | :--- | :--- |
| **Switch S1** | `PTB3` | **Start / Pause:** เริ่มจับเวลาหรือหยุดชั่วคราว พร้อมเสียงบี๊บความถี่สูง (`4,500 Hz`) ตอบรับเมื่อกด |
| **Switch S2** | `PTB10` | **Reset:** รีเซ็ตค่าเวลาเป็น `0000` และหยุดการทำงาน |
| **Switch S3** | `PTB11` | **Toggle Melody:** เปิด / ปิดการเล่นทำนองเพลง *Jingle Bells* |
| **LED D1 – D4** | `PTD1, PTD3, PTD2, PTD0` | `D2` ติดสว่างขณะกำลังนับเวลา, `D4` กระพริบทุก 1 วินาที, `D1` ติดเมื่อหยุดทำงาน |
| **7-Segment (74HC595)** | `PTC3, PTC6/PTB23, PTC12/PTA0` | แสดงผลตัวเลข `0000` – `9999` (นับจากหลักหน่วยฝั่งขวาไปซ้าย) |
| **Active Buzzer** | `PTA1` (`FTM0_CH6`) | ขับเสียงสัญญาณตอบรับและทำนองเพลง PWM ความถี่สูง |

---

## 📁 โครงสร้างไฟล์ภายใน Repository (Project Structure)

โค้ดถูกออกแบบในรูปแบบ **Modular C Architecture** แยกไฟล์ส่วนหัว (`.h`) และไฟล์การทำงาน (`.c`) ตามแต่ละโมดูลฮาร์ดแวร์ เพื่อให้คอมไพล์บน **Keil uVision MDK-ARM** ได้ทันที:

```text
├── main.c                  # ลูปหลักควบคุมการสแกน 7-Segment และการตัดเสียง Buzzer (Hi-Z)
├── config.h                # ศูนย์รวมค่าคงที่เวลา (10ms Base Tick), ขาพอร์ต และตัวแปรส่วนกลาง
├── led.c / led.h           # โมดูลควบคุมสถานะไฟ LED D1–D4 (Active-Low)
├── buzzer_pwm.c / .h       # โมดูลสร้างคลื่น PWM ความถี่สูง (FTM0_CH6) และตารางโน้ตเพลง
├── seven_segment.c / .h    # โมดูลขับไอซี Shift Register 74HC595 แสดงผล 4 หลัก
├── switches_irq.c / .h     # โมดูลรับ External Interrupt (PORTB) และ Non-blocking Debounce
├── timer_pit.c / .h        # โมดูลฐานเวลา PIT0 (0.01s) และตัวหารความเร็วอิสระ
├── Full code.c             # ซอร์สโค้ดฉบับรวมทั้งหมดในไฟล์เดียว (Single-File Reference)
└── EL334_Project_Report_FRDM_K64F.docx  # เล่มรายงานโครงงานฉบับสมบูรณ์พร้อมไดอะแกรมและผังงาน
