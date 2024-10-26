```
#define VL53L8A1_I2C_INIT               BSP_I2C4_Init
#define VL53L8A1_I2C_DEINIT             BSP_I2C4_DeInit
#define VL53L8A1_I2C_WRITEREG           BSP_I2C4_WriteReg16
#define VL53L8A1_I2C_READREG            BSP_I2C4_ReadReg16
#define VL53L8A1_GETTICK                BSP_GetTick
```

//PF1 --- SCL

//PF2 --- SDA







传感器初始化

```
static int32_t VL53L8CX_Probe(uint32_t Instance)
```



