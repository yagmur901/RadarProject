/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

I2S_HandleTypeDef hi2s3;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */

// buradaki pv = private variables. bu bloğun içinde global değişkenler olur.


uint32_t sure = 0;		//(u- unsigned, int, 32_t - işlemciye 32 bitlik = 4 byte yer ayır deriz(ms cinsinden old. için büyük yer ayırttık.)).

volatile uint16_t mesafe = 0; // volatile = her an değişebilir demek.

//volatile yazılmasaydı compiler, değişkeni cpu registerında tutar, ram e yazmaz.
//sensor mesafeyi hesaplar, registerdaki değer güncellenir ama ram e yazılmadığı için live exp. hep 0 görünür.




/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2S3_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM3_Init(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



#include <stdio.h> //prinf kullanabilmek için

int _write(int file, char *ptr, int len) { // override ediyor.
	//normalde printf yazıyı sadece formatlar ama yazıyı ekrana basamaz
	//bunun için arka planda _write isimli çok alt seviye bir komutunu çağırır.
	//char *ptr = gönderilecek karakter dizisinin hafızadaki başlangıç adresini tutar. (pointer)
	//len = gönderilecek yazının kaç harf (byte) old. tutar.


	for(int i = 0; i < len; i++) {
		ITM_SendChar((*ptr++));
		// donanımsal debug modülünün komutu, veriyi st-link kablosu üzerinden cube ide konsoluna fırlatır
		//kelime bitene kadar pointerdaki adresteki harfi alır gönderir ve sonra sonraki harfe geçer.
	}
	return len; // len kadar karakter başarıyla gönderildi demek
}


void delay_us (uint16_t us) //direkt HAL_Delay(1); yazılsa bile çok uzun sürer ondan mikrosaniye cinsinden
{
    __HAL_TIM_SET_COUNTER(&htim3, 0); // kronometreyi sıfırla
    while (__HAL_TIM_GET_COUNTER(&htim3) < us); // istenen süre kadar bekle (şart bozulunca döngüden çıkar)
}





/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();
  MX_USB_HOST_Init();
  MX_TIM4_Init();
  MX_TIM3_Init();


  /* USER CODE BEGIN 2 */







  //tim4  ch1 - servoya bağlı pd12 pini
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1); // TIM4'un 1. kanalından PWM sinyalini baslat


  // pulse width = 1500 (1500 mikro saniye = 1.5 ms iken motor 90 derecede olur = tam ortası)
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 1500); // motora başlangıç noktasına gitmesi söylenir.
  HAL_Delay(1000); // oraya gitmesi ve beklemesi için 1 saniye zaman tanı


  HAL_TIM_Base_Start(&htim3);               // ms kronometresi çalışmaya hazır
  // bu base start komutunu en başta çalıştırmazsak kronometre uykuda kalır.
  // arkaplanda rcc = reset and clock control, clock enable bit = 1 (enerji artık timer3 e ulaşır.)
  // enerji geldikten sonra hemen saymaya başlamaz, başlatan fiziksel tetik:
  // timer3'ün içindeki cr1 (control register1)'in içindeki cen(counter enable) bitini 0dan 1e çeker.


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


    int aci = 0;

    uint32_t zaman_asimi = 0; // dongu sonsuza kadar kilitlenmesin diye güvenlik için

  while (1)
  {



/*
	  // Sadece servoya git-gel hareketi yaptırmak için:

	  // motoru yaklaşık 45 dereceden 135 dereceye (1000'den 2000'e) (güvenli sınırlar) dogru tarat => tepe sinyali 0.5 ms ise 0 derece; tepe sinyali 1.5 ms ise 90 derece; tepe sinyali 2.5 ms ise 180 derece;
	  	  for ( aci = 500; aci <= 2500; aci += 20)
	  	  {
	  		  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, aci); // motora aci degerini gonder
	  		  HAL_Delay(30); // hareketin tamamlanmasi icin kisa bir sure bekle (hizi belirler)
	  	  }

	  	  HAL_Delay(500); // 180 dereceye ulasınca yarım saniye bekle

	  	  // Motoru 180 dereceden 0 dereceye dogru geri tarat
	  	  for ( aci = 2500; aci >= 500; aci -= 20)
	  	  {
	  		  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, aci); // motora aci degerini gonder
	  		  HAL_Delay(30);
	  	  }

	  	  HAL_Delay(500); // 0 dereceye ulasinca yarim saniye bekle
*/






	  /*

	  ///Ultrasonik sensörden veri alabilmek için (Motor sabitken/kilitli.):


      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 1500); // motoru merkezde kilitli tut
      // sensörü tetikleme = kare dalga biçimi için (kare dalga olmasaydı sensor kendi yankısıyla eski yankıları karıştırırdı.):
	  HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET); // elektiriği aç
	  delay_us(10);												// 10 ms açık tut
	  HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET); // elektriği kes


	  while (HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_RESET) { // sinyalin 1 olmasının yani echonun başlamasını bekle

	  	zaman_asimi++;
	       	if (zaman_asimi > 500000) break; // sinyalin uzun süre gelmemesi durumu (güvenlik kilidi gibi düşün)
        	}

	        if (zaman_asimi <= 500000) { //sinyal zaman asımsız basarıyla geldiyse

	        	__HAL_TIM_SET_COUNTER(&htim3, 0); //kronometreyi sıfırla

				zaman_asimi = 0;

				while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET) {
					zaman_asimi++;
					if(zaman_asimi > 500000) break;
				}


			sure = __HAL_TIM_GET_COUNTER(&htim3);
			mesafe = sure * 0.0343 / 2; //cm olarak hesaplama formulu (ses hızına göre)

	        } else {
	        	mesafe = 999; // sinyal hiç gelmedi hatası (direkt donmasındansa 999 yazdırsın ki ne olduğunu anlayabilelim.)
	        }

	        printf("Mesafe : %d cm\r\n", mesafe); //SWV konsoluna yazdırmak için.
	        HAL_Delay(200);

*/

	  // Servo 0-180 arasını git-gel şeklinde tararken ultrasonikten veri alma ve yazdırma:

	  for (aci = 0; aci<= 180; aci +=90) { //ileri tarama!!!!! 0dan 180e



		  //motoru yeni açıyla gönder
		  uint32_t pwm_value = 500+ (aci * 2000) / 180;
		  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pwm_value);

		  HAL_Delay(500); //motorun fiziksel olarak hedefe varması için bekle



		  //yukarıda yazdığım ultrasonik sensor kodunun aynısı:


	      	  	  	// sensörü tetikleme = kare dalga biçimi için (kare dalga olmasaydı sensor kendi yankısıyla eski yankıları karıştırırdı.):
		 	        HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET); // elektiriği aç
		 	        delay_us(10);												// 10 ms açık tut
		 	        HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET); // elektriği kes


		 	        while (HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_RESET) { // sinyalin 1 olmasının yani echonun başlamasını bekle

		 	        	zaman_asimi++;
		 	        	if (zaman_asimi > 500000) break; // sinyalin uzun süre gelmemesi durumu

		 	        }

		 	        if (zaman_asimi <= 500000) { //sinyal zaman asımsız basarıyla geldiyse

		 	        	__HAL_TIM_SET_COUNTER(&htim3, 0); //kronometreyi sıfırla

		 				zaman_asimi = 0;

		 				while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET) {
		 					zaman_asimi++;
		 					if(zaman_asimi > 500000) break;
		 				}

		 				sure = __HAL_TIM_GET_COUNTER(&htim3); // süre kaç onu al
		 				mesafe = sure * 0.0343 / 2; //cm olarak
		 	        } else {
		 	        	mesafe = 999; // sinyal hiç gelmedi hatası
		 	        }



		 	        printf("<Aci: %d, Mesafe: %d>\r\n", aci, mesafe);

	  }



	  for (aci = 180; aci >= 0; aci -= 90) { //geri taramak için!!!!! 180den 0a


		  //motoru yeni açıyla gönder
		  uint32_t pwm_value = 500 + (aci *2000) /180;
		  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pwm_value);


		  HAL_Delay(500); // motorun hedefe varması için bekle

		  //sensör kodumuz (geriye dönerken de aynı, for döngüsünün içinde olduğundan tekrar yazıyorum.)



		  // sensoru tetikle // kare dalga biçiimi için
		  		 	        HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET); // elektiriği aç
		  		 	        delay_us(10);												// 10 ms açık tut
		  		 	        HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET); // elektriği kes


		  		 	        while (HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_RESET) { // sinyalin 1 olmasının yani echonun başlamasını bekle

		  		 	        	zaman_asimi++;
		  		 	        	if (zaman_asimi > 500000) break; // sinyalin uzun süre gelmemesi durumu

		  		 	        }

		  		 	        if (zaman_asimi <= 500000) { //sinyal zaman asımsız basarıyla geldiyse

		  		 	        	__HAL_TIM_SET_COUNTER(&htim3, 0); //kronometreyi sıfırla

		  		 				zaman_asimi = 0; // ikinci döngü için güvenlik sayacını sıfırla

		  		 				while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET) {
		  		 				// sensor sesin geri donmesini beklerken echo pinini 1de (set) tutar
		  		 				// ses cisme çarpıp senore geri dondugu an pin 0a düşer.
		  		 				// echo pini 1 old. sürece = ses havadayken burada bekle, 0 oldugu an donguden cıkarız.
		  		 					zaman_asimi++;
		  		 					if(zaman_asimi > 500000) break;
		  		 					}

		  		 					sure = __HAL_TIM_GET_COUNTER(&htim3);
		  		 					mesafe = sure * 0.0343 / 2; //cm olarak
		  		 	        	} else {
		  		 	        		mesafe = 999; // sinyal hiç gelmedi hatası
		  		 	        	}



		  		 	        printf("<Aci: %d, Mesafe: %d>\r\n", aci, mesafe);

	  }


    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2S3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S3_Init(void)
{

  /* USER CODE BEGIN I2S3_Init 0 */

  /* USER CODE END I2S3_Init 0 */

  /* USER CODE BEGIN I2S3_Init 1 */

  /* USER CODE END I2S3_Init 1 */
  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_96K;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S3_Init 2 */

  /* USER CODE END I2S3_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 83;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 83;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 19999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_I2C_SPI_GPIO_Port, CS_I2C_SPI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LD3_Pin|LD5_Pin|LD6_Pin|Audio_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : CS_I2C_SPI_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_I2C_SPI_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PDM_OUT_Pin */
  GPIO_InitStruct.Pin = PDM_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(PDM_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : TRIG_Pin */
  GPIO_InitStruct.Pin = TRIG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TRIG_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ECHO_Pin */
  GPIO_InitStruct.Pin = ECHO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ECHO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CLK_IN_Pin */
  GPIO_InitStruct.Pin = CLK_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(CLK_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD3_Pin LD5_Pin LD6_Pin Audio_RST_Pin */
  GPIO_InitStruct.Pin = LD3_Pin|LD5_Pin|LD6_Pin|Audio_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MEMS_INT2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
