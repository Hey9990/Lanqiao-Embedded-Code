
/* i2c.h添加的声明 */
void eeprom_write(uint8_t *EEPROM_String, uint8_t addr, uint8_t num);
void eeprom_read(uint8_t *EEPROM_String, uint8_t addr, uint8_t num);
void mcp4017_write(uint8_t data);
void mcp4017_read(uint8_t *data);




/* i2c.c需要添加的自定义函数 */

/**
 * @brief 向EEPROM写入数据
 * 
 * @param EEPROM_String 指向要写入EEPROM的数据的指针
 * @param addr EEPROM内存中的起始地址
 * @param num 要写入的数据字节数
 */
void eeprom_write(uint8_t *EEPROM_String, uint8_t addr, uint8_t num)
{
    I2CStart();               // 启动I2C通信
    I2CSendByte(0xa0);        // 发送EEPROM写命令（器件地址 + 写操作位）
    I2CWaitAck();             // 等待EEPROM的ACK信号
    
    I2CSendByte(addr);        // 发送要写入的EEPROM起始地址
    I2CWaitAck();             // 等待EEPROM的ACK信号
    
    while(num--)              // 逐字节写入数据
    {
        I2CSendByte(*EEPROM_String++);  // 发送当前字节的数据
        I2CWaitAck();                   // 等待EEPROM的ACK信号
        delay1(200);                    // 短暂延时，确保数据写入
    }
    I2CStop();                // 停止I2C通信
    HAL_Delay(5);             // 延时5ms，确保数据已写入EEPROM
}

/**
 * @brief 从EEPROM读取数据
 * 
 * @param EEPROM_String 指向存储读取数据的缓冲区的指针
 * @param addr EEPROM内存中的起始地址
 * @param num 要读取的数据字节数
 */
void eeprom_read(uint8_t *EEPROM_String, uint8_t addr, uint8_t num)
{
    I2CStart();               // 启动I2C通信
    I2CSendByte(0xa0);        // 发送EEPROM写命令（器件地址 + 写操作位）
    I2CWaitAck();             // 等待EEPROM的ACK信号
    
    I2CSendByte(addr);        // 发送要读取的EEPROM起始地址
    I2CWaitAck();             // 等待EEPROM的ACK信号
    
    I2CStart();               // 再次启动I2C通信，准备读操作
    I2CSendByte(0xa1);        // 发送EEPROM读命令（器件地址 + 读操作位）
    I2CWaitAck();             // 等待EEPROM的ACK信号
    
    while(num--)              // 逐字节读取数据
    {
        *EEPROM_String++ = I2CReceiveByte();  // 接收当前字节的数据
        if(num)
            I2CSendAck();      // 如果还需要继续读取，发送ACK信号
        else
            I2CSendNotAck();   // 如果已读取完毕，发送非ACK信号
    }
    I2CStop();                // 停止I2C通信
}

/**
 * @brief 向MCP4017写入数据
 * 
 * @param data 要写入MCP4017的数值
 */
void mcp4017_write(uint8_t data)
{
    I2CStart();               // 启动I2C通信
    I2CSendByte(0x5e);        // 发送MCP4017的写命令（器件地址 + 写操作位）
    I2CWaitAck();             // 等待MCP4017的ACK信号
    I2CSendByte(data);        // 发送要写入的数据
    I2CWaitAck();             // 等待MCP4017的ACK信号
    I2CStop();                // 停止I2C通信
}

/**
 * @brief 从MCP4017读取数据
 * 
 * @param data 指向存储读取数据的变量的指针
 */
void mcp4017_read(uint8_t *data)
{
    I2CStart();               // 启动I2C通信
    I2CSendByte(0x5f);        // 发送MCP4017的读命令（器件地址 + 读操作位）
    I2CWaitAck();             // 等待MCP4017的ACK信号
    *data = I2CReceiveByte(); // 接收来自MCP4017的数据
    I2CSendNotAck();          // 发送非ACK信号，结束读操作
    I2CStop();                // 停止I2C通信
}




