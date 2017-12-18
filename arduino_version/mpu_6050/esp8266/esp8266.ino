
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);   \
  void operator=(const TypeName&) 

class I2CPins {
 public:
  virtual ~I2CPins() {}
  virtual void SdaSet(bool val) = 0;
  virtual bool SdaGet() const = 0;
  virtual void SclSet(bool val) = 0;
  virtual bool SclGet() const = 0;
};

class I2CPinsEsp8266 final: public I2CPins {
 public:
  DISALLOW_COPY_AND_ASSIGN(I2CPinsEsp8266);

  static void Setup() {
    GetInstance();
  }
  
  static I2CPins& GetInstance() {
    static I2CPinsEsp8266 instance;
    return instance;   
  }

  void SdaSet(bool val) override {
    digitalWrite(kSdaPin, val ? HIGH : LOW);
  }
  
  bool SdaGet() const override {
    return false;
  }
  
  void SclSet(bool val) override {
    digitalWrite(kSclPin, val ? HIGH : LOW);  
  }
  
  bool SclGet() const override {
    return false;
  }
  
 private:
  static constexpr const int kSdaPin = 5;
  static constexpr const int kSclPin = 5;
   
  I2CPinsEsp8266() {
    pinMode(kSdaPin, OUTPUT);
    pinMode(kSclPin, OUTPUT);
  }
};

class I2CProtocol {
 public:
  I2CProtocol(I2CPins& pins): pins_(pins) {}

  
 private:
  I2CPins& pins_;
}

void setup() {
  // put your setup code here, to run once:
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
