
void setup()
{
	Serial.begin(38400);
}

void rgb(uint16_t r, uint16_t g, uint16_t b)
{
	Serial.write(r&0xff);
	Serial.write(r>>8);
	Serial.write(g&0xff);
	Serial.write(g>>8);
	Serial.write(b&0xff);
	Serial.write(b>>8);
}

void loop()
{
	uint16_t i, j, c;
	#define N_LEDS (9)
	uint8_t counter[N_LEDS];

	for (i=0; i<N_LEDS; i++)
		counter[i] = 0;

	for(;;)
	{
		for (j=32; j>0; j--)
		{
			for (i=0; i<N_LEDS; i++)
			{
				if (counter[i] == 0)
					rgb(j,0,0);
				else if (counter[i] <= i)
					rgb(0,j,j);
				else
					rgb(0,0,0);
			}
			Serial.flush();
			delay(2);
		}

		for (i=0; i<N_LEDS; i++)
		{
			counter[i] += 1;
			if (counter[i] > i+2)
				counter[i] = 0;
		}

		for (i=0; i<N_LEDS; i++)
			rgb(0,0,0);

		Serial.flush();
		delay(100);
	}
}
