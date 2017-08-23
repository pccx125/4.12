#include"main.h"
/*电池电压检测，电流检测，压力传感器检测AD转换，使用DMA把ADC寄存器的值直接传输到内存数组中*/

__IO uint16_t ADCConvertedValue[12];//3个通道，分别取四次取平均值


void ADC_GPIO_Configuration(void)
{
  GPIO_InitTypeDef      GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  //PC0 作为模拟通道10输入引脚    电流检测                 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //PC2 作为模拟通道12输入引脚    电池电压检测                    
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);	
  //PC3 作为模拟通道13输入引脚       压力传感器 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  
}
/**
  * @brief  ADC1 channel13 with DMA configuration
  * @param  None
  * @retval None
  */

void ADC_DMA_Config(void)
{
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;
 

  /* Enable ADC1, DMA2 and GPIO clocks ****************************************/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 , ENABLE);
	ADC_DeInit();
  DMA_DeInit(DMA2_Stream0);
  /* DMA2 Stream0 channel2 configuration **************************************/
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDRESS;//源地址，ADC寄存器地址
	DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)ADCConvertedValue;//目的地址，为存储ADC数据的数组地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;//由外设搬移到内存
	DMA_InitStructure.DMA_BufferSize = 12;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//接收一次数据后，设备地址禁止后移
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//开启接收一次数据后，目标内存地址后移
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //DMA搬数据尺寸，HalfWord就是为16位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode =DMA_Mode_Circular;         //工作在循环模式
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;//DMA优先级高
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);	
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);//DMA传输完成中断
  DMA_Cmd(DMA2_Stream0, ENABLE);


  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立的转换模式
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC1 Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;//模数转换工作在扫描模式（多通道）
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; //开启连续转换模式
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//ADC外部开关，关闭状态，即转换不受外界决定
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//ADC数据左对齐，ADC数据位为12位，右对齐
  ADC_InitStructure.ADC_NbrOfConversion = 3;//转换的ADC通道的数目为3
  ADC_Init(ADC1, &ADC_InitStructure);
	
	
  /* ADC1 regular  configuration *************************************/
	  /* 设置ADC1的4个规则组通道，设置它们的转化顺序和采样时间 采样原理    采样改为15cycles，原来是3，导致有时采样出错*/
// 		ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);  //电压检测
//    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 2, ADC_SampleTime_3Cycles);//压力检测
// 	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 3, ADC_SampleTime_3Cycles);//电流检测
  ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_15Cycles);  //电压检测
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 2, ADC_SampleTime_15Cycles);//压力检测
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 3, ADC_SampleTime_15Cycles);//电流检测
 /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
	
	ADC_SoftwareStartConv(ADC1);   //软件启动ADC转换
}


