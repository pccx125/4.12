#include"main.h"
/*��ص�ѹ��⣬������⣬ѹ�����������ADת����ʹ��DMA��ADC�Ĵ�����ֱֵ�Ӵ��䵽�ڴ�������*/

__IO uint16_t ADCConvertedValue[12];//3��ͨ�����ֱ�ȡ�Ĵ�ȡƽ��ֵ


void ADC_GPIO_Configuration(void)
{
  GPIO_InitTypeDef      GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  //PC0 ��Ϊģ��ͨ��10��������    �������                 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //PC2 ��Ϊģ��ͨ��12��������    ��ص�ѹ���                    
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);	
  //PC3 ��Ϊģ��ͨ��13��������       ѹ�������� 
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
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDRESS;//Դ��ַ��ADC�Ĵ�����ַ
	DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)ADCConvertedValue;//Ŀ�ĵ�ַ��Ϊ�洢ADC���ݵ������ַ
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;//��������Ƶ��ڴ�
	DMA_InitStructure.DMA_BufferSize = 12;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//����һ�����ݺ��豸��ַ��ֹ����
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//��������һ�����ݺ�Ŀ���ڴ��ַ����
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //DMA�����ݳߴ磬HalfWord����Ϊ16λ
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode =DMA_Mode_Circular;         //������ѭ��ģʽ
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;//DMA���ȼ���
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);	
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);//DMA��������ж�
  DMA_Cmd(DMA2_Stream0, ENABLE);


  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//������ת��ģʽ
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC1 Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;//ģ��ת��������ɨ��ģʽ����ͨ����
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; //��������ת��ģʽ
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//ADC�ⲿ���أ��ر�״̬����ת������������
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//ADC��������룬ADC����λΪ12λ���Ҷ���
  ADC_InitStructure.ADC_NbrOfConversion = 3;//ת����ADCͨ������ĿΪ3
  ADC_Init(ADC1, &ADC_InitStructure);
	
	
  /* ADC1 regular  configuration *************************************/
	  /* ����ADC1��4��������ͨ�����������ǵ�ת��˳��Ͳ���ʱ�� ����ԭ��    ������Ϊ15cycles��ԭ����3��������ʱ��������*/
// 		ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);  //��ѹ���
//    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 2, ADC_SampleTime_3Cycles);//ѹ�����
// 	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 3, ADC_SampleTime_3Cycles);//�������
  ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_15Cycles);  //��ѹ���
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 2, ADC_SampleTime_15Cycles);//ѹ�����
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 3, ADC_SampleTime_15Cycles);//�������
 /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
	
	ADC_SoftwareStartConv(ADC1);   //�������ADCת��
}


