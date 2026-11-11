#include "app_sensor.h"

#define AI_BUFFER_SIZE 5
static int temp_history[AI_BUFFER_SIZE] = {0};
static int history_index = 0;
static int history_count = 0;

// Biến đếm thời gian giữ cảnh báo (Latch)
static int anomaly_latch_count = 0;
#define ANOMALY_LATCH_SAMPLES 6  // Giữ cảnh báo trong 6 mẫu (3 giây)

void Sensor_Init(void) {
    printf("[SENSOR] Khoi tao LM35 hoan tat.\r\n");
    anomaly_latch_count = 0;
    history_count = 0;
    history_index = 0;
}

int Sensor_ReadTemperature(void) {
    uint32_t adc_val = 0;
    uint32_t voltage_mv = 0;
    int temp_c = 0;

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK) {
        HAL_ADC_Stop(&hadc1);
        return TEMP_ERROR;
    }
    adc_val = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    if (adc_val < 10 || adc_val > 4085) {
        return TEMP_ERROR;
    }

    voltage_mv = (adc_val * 3300) / 4095;
    temp_c = voltage_mv / 10;
    return temp_c;
}

int Sensor_DetectAnomaly(int new_temp) {
    if (new_temp == TEMP_ERROR) return 0;

    // Nếu đang trong thời gian giữ cảnh báo, giảm bộ đếm và trả về Anomaly
    if (anomaly_latch_count > 0) {
        anomaly_latch_count--;
        return 1;
    }

    // Khởi tạo bộ đệm
    if (history_count < AI_BUFFER_SIZE) {
        temp_history[history_index] = new_temp;
        history_index = (history_index + 1) % AI_BUFFER_SIZE;
        history_count++;
        return 0;
    }

    // Tính trung bình
    int sum = 0;
    for (int i = 0; i < AI_BUFFER_SIZE; i++) {
        sum += temp_history[i];
    }
    int avg_temp = sum / AI_BUFFER_SIZE;

    // Cập nhật mẫu mới
    temp_history[history_index] = new_temp;
    history_index = (history_index + 1) % AI_BUFFER_SIZE;

    // So sánh độ lệch (Ngưỡng 2 độ C)
    int delta = new_temp - avg_temp;
    if (delta < 0) delta = -delta;

    if (delta > 2) {                                  // Hạ ngưỡng xuống 2
        anomaly_latch_count = ANOMALY_LATCH_SAMPLES;  // Bắt đầu giữ cảnh báo
        return 1;
    }

    return 0;
}