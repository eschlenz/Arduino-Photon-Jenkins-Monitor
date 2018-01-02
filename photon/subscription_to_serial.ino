#define SUBSCRIPTION_TOPIC "your_subscription_topic"

void setup() {
    Particle.subscribe(SUBSCRIPTION_TOPIC, onEvent, MY_DEVICES);
    Serial1.begin(38400);
    Serial1.println("INIT");
}

void loop() {
}

void onEvent(const char *event, const char *data) {
  if (data) {
    Serial1.println(data);
  } else {
    Serial1.println("NULL");
  }
}

