/**
 * ESP32 Kindlelaser Max 260W - Laser Control Implementation
 * Controle de potência PWM, segurança e presets de corte de metal
 *
 * IMPORTANTE - Segurança:
 * - O laser NUNCA liga sem fluxo de água
 * - O laser DESLIGA imediatamente se a tampa abrir
 * - O botão de emergência (E-STOP) desliga TUDO
 * - Potência máxima limitada a 99% (protege o tubo)
 * - Tempo máximo de corte contínuo: 5 minutos
 *
 * Conexões:
 *   ESP32 GPIO 25 (PWM) → Conversor 3.3V→5V → PSU Laser (IN)
 *   ESP32 GPIO 26       → Optoacoplador → PSU Laser (ENABLE)
 *   ESP32 GPIO 14       → Relé → Solenoide ar comprimido
 *   ESP32 GPIO 12       → Relé → Exaustor
 *   ESP32 GPIO 33       ← Sensor fluxo de água (pull-up)
 *   ESP32 GPIO 32       ← Chave de tampa (pull-up)
 *   ESP32 GPIO 39       ← Botão E-STOP (pull-up)
 *   ESP32 GPIO 35       ← Divisor NTC (ADC)
 */

#include "laser_control.h"

LaserControl::LaserControl()
    : _power(POWER_DEFAULT),
      _speed(SPEED_DEFAULT),
      _laserEnabled(false),
      _airAssistActive(false),
      _exhaustActive(false),
      _presetIndex(0),
      _mode(MODE_IDLE),
      _waterFlowOK(false),
      _lidClosed(false),
      _estopOK(false),
      _waterTemp(20.0f),
      _lastWaterPulse(0),
      _lastSafetyCheck(0),
      _cuttingStartTime(0),
      _totalCuttingTime(0),
      _warmupStart(0)
{
    memset(_errorMsg, 0, sizeof(_errorMsg));
}

void LaserControl::begin() {
    // Configure output pins
    pinMode(LASER_ENABLE_PIN, OUTPUT);
    pinMode(AIR_ASSIST_PIN, OUTPUT);
    pinMode(EXHAUST_FAN_PIN, OUTPUT);

    // All outputs OFF at startup
    digitalWrite(LASER_ENABLE_PIN, LOW);
    digitalWrite(AIR_ASSIST_PIN, LOW);
    digitalWrite(EXHAUST_FAN_PIN, LOW);

    // Configure PWM for laser power
    ledcSetup(LASER_PWM_CHANNEL, LASER_PWM_FREQ, LASER_PWM_RESOLUTION);
    ledcAttachPin(LASER_PWM_PIN, LASER_PWM_CHANNEL);
    ledcWrite(LASER_PWM_CHANNEL, 0); // Start with 0 power

    // Configure safety input pins
    pinMode(WATER_FLOW_PIN, INPUT_PULLUP);
    pinMode(LID_SWITCH_PIN, INPUT_PULLUP);
    pinMode(ESTOP_PIN, INPUT_PULLUP);

    // Configure temperature ADC
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    pinMode(TEMP_SENSOR_PIN, INPUT);

    // Initial safety check
    checkSafety();

    Serial.println("[LASER] Kindlelaser Max 260W controller initialized");
    Serial.printf("[LASER] PWM: %d Hz, %d-bit resolution\n", LASER_PWM_FREQ, LASER_PWM_RESOLUTION);
    Serial.printf("[LASER] Power: %d%%, Speed: %d mm/s\n", _power, _speed);
}

void LaserControl::update() {
    unsigned long now = millis();

    // Safety check at regular intervals
    if (now - _lastSafetyCheck >= SAFETY_CHECK_MS) {
        _lastSafetyCheck = now;
        checkSafety();
    }

    // Warmup mode timing
    if (_mode == MODE_WARMUP) {
        if (now - _warmupStart >= LASER_WARMUP_MS) {
            if (isSafeToOperate()) {
                setMode(MODE_READY);
                Serial.println("[LASER] Warmup complete - READY");
            }
        }
    }

    // Check maximum continuous cutting time
    if (_mode == MODE_CUTTING && _cuttingStartTime > 0) {
        if (now - _cuttingStartTime >= MAX_CONTINUOUS_CUT) {
            laserOff();
            snprintf(_errorMsg, sizeof(_errorMsg), "Tempo max corte! Pausa obrigatoria");
            setMode(MODE_PAUSED);
            Serial.println("[LASER] Max cutting time reached - paused");
        }
    }
}

// ============================================
// Power Control
// ============================================
void LaserControl::setPower(uint8_t percent) {
    if (percent > POWER_MAX) percent = POWER_MAX;
    if (percent < LASER_MIN_POWER_PCT && percent != 0) percent = LASER_MIN_POWER_PCT;
    _power = percent;
    if (_laserEnabled) {
        applyPWM();
    }
    Serial.printf("[LASER] Power set: %d%% (%dW)\n", _power, getPowerWatts());
}

void LaserControl::adjustPower(int delta) {
    int newPower = (int)_power + delta;
    if (newPower < LASER_MIN_POWER_PCT) newPower = LASER_MIN_POWER_PCT;
    if (newPower > POWER_MAX) newPower = POWER_MAX;
    setPower((uint8_t)newPower);
}

uint8_t LaserControl::getPower() const {
    return _power;
}

uint16_t LaserControl::getPowerWatts() const {
    return (uint16_t)((uint32_t)_power * LASER_MAX_POWER_W / 100);
}

// ============================================
// Speed Control
// ============================================
void LaserControl::setSpeed(uint8_t mmPerSec) {
    if (mmPerSec < SPEED_MIN) mmPerSec = SPEED_MIN;
    if (mmPerSec > SPEED_MAX) mmPerSec = SPEED_MAX;
    _speed = mmPerSec;
    Serial.printf("[LASER] Speed set: %d mm/s\n", _speed);
}

void LaserControl::adjustSpeed(int delta) {
    int newSpeed = (int)_speed + delta;
    if (newSpeed < SPEED_MIN) newSpeed = SPEED_MIN;
    if (newSpeed > SPEED_MAX) newSpeed = SPEED_MAX;
    setSpeed((uint8_t)newSpeed);
}

uint8_t LaserControl::getSpeed() const {
    return _speed;
}

// ============================================
// Laser Enable/Disable
// ============================================
void LaserControl::laserOn() {
    if (!isSafeToOperate()) {
        Serial.println("[LASER] RECUSADO - condições de segurança não atendidas");
        return;
    }

    _laserEnabled = true;
    _cuttingStartTime = millis();

    // Enable air assist automatically
    if (!_airAssistActive) {
        airAssistOn();
    }

    // Enable exhaust
    if (!_exhaustActive) {
        exhaustOn();
    }

    // Apply PWM and enable
    applyPWM();
    digitalWrite(LASER_ENABLE_PIN, HIGH);

    setMode(MODE_CUTTING);
    Serial.printf("[LASER] ON - %d%% (%dW) @ %d mm/s\n", _power, getPowerWatts(), _speed);
}

void LaserControl::laserOff() {
    _laserEnabled = false;

    // Immediately disable laser output
    digitalWrite(LASER_ENABLE_PIN, LOW);
    ledcWrite(LASER_PWM_CHANNEL, 0);

    // Update cutting time
    if (_cuttingStartTime > 0) {
        _totalCuttingTime += (millis() - _cuttingStartTime);
        _cuttingStartTime = 0;
    }

    if (_mode == MODE_CUTTING) {
        setMode(MODE_READY);
    }

    Serial.println("[LASER] OFF");
}

void LaserControl::emergencyStop() {
    // Immediately disable everything
    _laserEnabled = false;
    digitalWrite(LASER_ENABLE_PIN, LOW);
    ledcWrite(LASER_PWM_CHANNEL, 0);
    digitalWrite(AIR_ASSIST_PIN, LOW);
    _airAssistActive = false;

    if (_cuttingStartTime > 0) {
        _totalCuttingTime += (millis() - _cuttingStartTime);
        _cuttingStartTime = 0;
    }

    snprintf(_errorMsg, sizeof(_errorMsg), "PARADA DE EMERGENCIA!");
    setMode(MODE_ERROR);
    Serial.println("[LASER] *** EMERGENCY STOP ***");
}

bool LaserControl::isLaserOn() const {
    return _laserEnabled;
}

void LaserControl::testPulse(uint16_t durationMs) {
    if (!isSafeToOperate()) {
        Serial.println("[LASER] Test pulse RECUSADO - segurança");
        return;
    }

    Serial.printf("[LASER] Test pulse: %dms @ %d%%\n", durationMs, _power);

    // Brief pulse for alignment/testing
    applyPWM();
    digitalWrite(LASER_ENABLE_PIN, HIGH);
    delay(durationMs);
    digitalWrite(LASER_ENABLE_PIN, LOW);
    ledcWrite(LASER_PWM_CHANNEL, 0);
}

// ============================================
// Air Assist
// ============================================
void LaserControl::airAssistOn() {
    _airAssistActive = true;
    digitalWrite(AIR_ASSIST_PIN, HIGH);
    Serial.println("[LASER] Air assist ON");
}

void LaserControl::airAssistOff() {
    // Don't allow turning off air during cutting
    if (_mode == MODE_CUTTING) {
        Serial.println("[LASER] Air assist cannot be disabled during cutting");
        return;
    }
    _airAssistActive = false;
    digitalWrite(AIR_ASSIST_PIN, LOW);
    Serial.println("[LASER] Air assist OFF");
}

void LaserControl::toggleAirAssist() {
    if (_airAssistActive) airAssistOff();
    else airAssistOn();
}

bool LaserControl::isAirAssistOn() const {
    return _airAssistActive;
}

// ============================================
// Exhaust Fan
// ============================================
void LaserControl::exhaustOn() {
    _exhaustActive = true;
    digitalWrite(EXHAUST_FAN_PIN, HIGH);
    Serial.println("[LASER] Exhaust fan ON");
}

void LaserControl::exhaustOff() {
    if (_mode == MODE_CUTTING) {
        Serial.println("[LASER] Exhaust cannot be disabled during cutting");
        return;
    }
    _exhaustActive = false;
    digitalWrite(EXHAUST_FAN_PIN, LOW);
    Serial.println("[LASER] Exhaust fan OFF");
}

void LaserControl::toggleExhaust() {
    if (_exhaustActive) exhaustOff();
    else exhaustOn();
}

bool LaserControl::isExhaustOn() const {
    return _exhaustActive;
}

// ============================================
// Safety Monitoring
// ============================================
bool LaserControl::isSafeToOperate() const {
    return _waterFlowOK && _lidClosed && _estopOK && isTempOK();
}

bool LaserControl::isWaterFlowOK() const {
    return _waterFlowOK;
}

bool LaserControl::isLidClosed() const {
    return _lidClosed;
}

bool LaserControl::isEstopOK() const {
    return _estopOK;
}

float LaserControl::getWaterTemp() const {
    return _waterTemp;
}

bool LaserControl::isTempOK() const {
    return _waterTemp < WATER_TEMP_MAX;
}

bool LaserControl::isTempWarning() const {
    return _waterTemp >= WATER_TEMP_WARNING && _waterTemp < WATER_TEMP_MAX;
}

const char* LaserControl::getErrorMessage() const {
    return _errorMsg;
}

// ============================================
// Material Presets
// ============================================
void LaserControl::loadPreset(uint8_t index) {
    if (index >= PRESET_COUNT) index = 0;
    _presetIndex = index;

    const CuttingPreset& preset = CUTTING_PRESETS[index];
    _power = preset.power;
    _speed = preset.speed;

    if (preset.airAssist && !_airAssistActive) {
        airAssistOn();
    }

    if (_laserEnabled) {
        applyPWM();
    }

    Serial.printf("[LASER] Preset: %s | %d%% | %d mm/s | Gas: %s\n",
                  preset.name, preset.power, preset.speed, preset.gas);
}

void LaserControl::nextPreset() {
    uint8_t next = (_presetIndex + 1) % PRESET_COUNT;
    loadPreset(next);
}

void LaserControl::prevPreset() {
    uint8_t prev = (_presetIndex == 0) ? PRESET_COUNT - 1 : _presetIndex - 1;
    loadPreset(prev);
}

uint8_t LaserControl::getCurrentPresetIndex() const {
    return _presetIndex;
}

const CuttingPreset& LaserControl::getCurrentPreset() const {
    return CUTTING_PRESETS[_presetIndex];
}

// ============================================
// Mode
// ============================================
LaserMode LaserControl::getMode() const {
    return _mode;
}

const char* LaserControl::getModeString() const {
    switch (_mode) {
    case MODE_IDLE:     return "IDLE";
    case MODE_READY:    return "PRONTO";
    case MODE_CUTTING:  return "CORTANDO";
    case MODE_PAUSED:   return "PAUSADO";
    case MODE_ERROR:    return "ERRO";
    case MODE_WARMUP:   return "AQUECENDO";
    default:            return "---";
    }
}

// ============================================
// Statistics
// ============================================
unsigned long LaserControl::getCuttingTime() const {
    if (_mode == MODE_CUTTING && _cuttingStartTime > 0) {
        return millis() - _cuttingStartTime;
    }
    return 0;
}

unsigned long LaserControl::getTotalCuttingTime() const {
    unsigned long total = _totalCuttingTime;
    if (_mode == MODE_CUTTING && _cuttingStartTime > 0) {
        total += (millis() - _cuttingStartTime);
    }
    return total;
}

// ============================================
// Private Methods
// ============================================
void LaserControl::applyPWM() {
    // Convert percentage to PWM duty cycle
    // power 0-99% → duty 0-4095
    uint32_t duty = ((uint32_t)_power * LASER_PWM_MAX) / 100;
    ledcWrite(LASER_PWM_CHANNEL, duty);
}

float LaserControl::readTemperature() {
    // Read NTC thermistor via voltage divider
    int rawADC = analogRead(TEMP_SENSOR_PIN);
    if (rawADC <= 0) rawADC = 1;

    // Convert ADC to resistance
    float resistance = NTC_SERIES_R * ((4095.0f / (float)rawADC) - 1.0f);

    // Steinhart-Hart simplified (Beta equation)
    float steinhart = resistance / NTC_NOMINAL_R;
    steinhart = log(steinhart);
    steinhart /= NTC_BETA;
    steinhart += 1.0f / (NTC_NOMINAL_T + 273.15f);
    steinhart = 1.0f / steinhart;
    steinhart -= 273.15f;

    return steinhart;
}

void LaserControl::checkSafety() {
    // Read safety inputs (active LOW with pull-up)
    _waterFlowOK = (digitalRead(WATER_FLOW_PIN) == LOW);
    _lidClosed = (digitalRead(LID_SWITCH_PIN) == LOW);
    _estopOK = (digitalRead(ESTOP_PIN) == HIGH); // E-stop is NC (normally closed)

    // Read water temperature
    _waterTemp = readTemperature();

    // Check for critical safety violations while laser is active
    if (_laserEnabled) {
        if (!_waterFlowOK) {
            laserOff();
            snprintf(_errorMsg, sizeof(_errorMsg), "SEM FLUXO DE AGUA!");
            setMode(MODE_ERROR);
            Serial.println("[SAFETY] Water flow LOST - laser disabled!");
        } else if (!_lidClosed) {
            laserOff();
            snprintf(_errorMsg, sizeof(_errorMsg), "TAMPA ABERTA!");
            setMode(MODE_ERROR);
            Serial.println("[SAFETY] Lid OPEN - laser disabled!");
        } else if (!_estopOK) {
            emergencyStop();
        } else if (!isTempOK()) {
            laserOff();
            snprintf(_errorMsg, sizeof(_errorMsg), "TEMP AGUA: %.1fC (MAX %.0fC)", _waterTemp, WATER_TEMP_MAX);
            setMode(MODE_ERROR);
            Serial.printf("[SAFETY] Water temp too high: %.1f°C\n", _waterTemp);
        }
    }

    // Update mode based on safety state
    if (_mode == MODE_ERROR && isSafeToOperate()) {
        // Auto-recover from error if conditions restored
        snprintf(_errorMsg, sizeof(_errorMsg), "");
        setMode(MODE_IDLE);
        Serial.println("[SAFETY] Conditions restored - back to IDLE");
    } else if (_mode == MODE_IDLE && isSafeToOperate()) {
        setMode(MODE_READY);
    } else if (_mode == MODE_READY && !isSafeToOperate()) {
        setMode(MODE_IDLE);
    }
}

void LaserControl::setMode(LaserMode mode) {
    if (_mode != mode) {
        _mode = mode;
        Serial.printf("[LASER] Mode: %s\n", getModeString());
    }
}
