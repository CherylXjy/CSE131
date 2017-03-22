int x;
void foobar()
{
	x = 5;
	bool lol = true;
	if(lol)
	{
		float z = 0.0;
		switch(z) {
			case 0.0: z = 1.5;
			case 1.0: z = 2.5;
			case 2.0: return;
			default: lol = false;
		}	

		for(z = 0.0; z < 5.0; z++)
		{
			x = 2;
			return;
		}

	}

	{
		z = 0.5;
	}
}
