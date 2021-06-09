#include "combo.h"

#include "../mem.h"
#include "../random.h"

//Combo object functions
boolean Obj_Combo_Tick(Object *obj)
{
	Obj_Combo *this = (Obj_Combo*)obj;
	
	//Tick hit type
	if (this->hit_type != 0xFF && this->ht < 16)
	{
		//Get hit src and dst
		u8 clipp = 16;
		if (this->ht > 0)
			clipp = 16 - this->ht;
		
		RECT hit_src = {
			0,
			128 + (this->hit_type << 5),
			80,
			clipp << 1
		};
		RECT_FIXED hit_dst = {
			this->x - FIXED_DEC(8,1) - stage.camera.x,
			this->hy - FIXED_DEC(16,1) - stage.camera.y,
			FIXED_DEC(80,1),
			(FIXED_DEC(32,1) * clipp) >> 4
		};
		Stage_DrawTex(&stage.tex_hud0, &hit_src, &hit_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Apply gravity
		this->hy += this->hv;
		this->hv += FIXED_DEC(5,100);
	}
	
	//Increment hit type timer
	this->ht++;
	
	//Tick combo
	if (this->num[4] != 0xFF && this->ct < 16)
	{
		//Get hit src and dst
		u8 clipp = 16;
		if (this->ct > 0)
			clipp = 16 - this->ct;
		
		RECT combo_src = {
			80,
			128,
			80,
			clipp << 1
		};
		RECT_FIXED combo_dst = {
			this->x + FIXED_DEC(48,1) - stage.camera.x,
			this->cy - FIXED_DEC(16,1) - stage.camera.y,
			FIXED_DEC(60,1),
			(FIXED_DEC(24,1) * clipp) >> 4
		};
		Stage_DrawTex(&stage.tex_hud0, &combo_src, &combo_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Apply gravity
		this->cy += this->cv;
		this->cv += FIXED_DEC(3,100);
	}
	
	//Increment combo timer
	this->ct++;
	
	//Tick numbers
	if (this->numt < 16)
	{
		for (u8 i = 0; i < 5; i++)
		{
			u8 num = this->num[i];
			if (num == 0xFF)
				continue;
			
			//Get number src and dst
			u8 clipp = 16;
			if (this->numt > 0)
				clipp = 16 - this->numt;
			
			RECT num_src = {
				80  + ((num % 5) << 5),
				160 + ((num / 5) << 5),
				32,
				clipp << 1
			};
			RECT_FIXED num_dst = {
				this->x - FIXED_DEC(32,1) + (i * FIXED_DEC(16,1)) - FIXED_DEC(12,1) - stage.camera.x,
				this->numy[i] - FIXED_DEC(12,1) - stage.camera.y,
				FIXED_DEC(24,1),
				(FIXED_DEC(24,1) * clipp) >> 4
			};
			Stage_DrawTex(&stage.tex_hud0, &num_src, &num_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
			
			//Apply gravity
			this->numy[i] += this->numv[i];
			this->numv[i] += FIXED_DEC(3,100);
		}
	}
	
	//Increment number timer
	this->numt++;
	
	return this->numt >= 16 && this->ht >= 16 && this->ct >= 16;
}

void Obj_Combo_Free(Object *obj)
{
	(void)obj;
}

Obj_Combo *Obj_Combo_New(fixed_t x, fixed_t y, u8 hit_type, u16 combo)
{
	//Allocate new object
	Obj_Combo *this = (Obj_Combo*)Mem_Alloc(sizeof(Obj_Combo));
	if (this == NULL)
		return NULL;
	
	//Set object functions
	this->obj.tick = Obj_Combo_Tick;
	this->obj.free = Obj_Combo_Free;
	
	//Use set position
	this->x = x - FIXED_DEC(48,1);
	
	//Setup hit type
	if ((this->hit_type = hit_type) != 0xFF)
	{
		this->hy = y - FIXED_DEC(38,1);
		this->hv = -(FIXED_DEC(8,10) + RandomRange(0, FIXED_DEC(3,10)));
	}
	
	//Setup numbers
	if (combo != 0xFFFF)
	{
		//Initial numbers
		this->num[0] = this->num[1] = 0xFF;
		this->num[2] = this->num[3] = this->num[4] = 0; //MEH
		
		//Write numbers
		static const u16 dig[5] = {10000, 1000, 100, 10, 1};
		boolean hit = false;
		
		const u16 *digp = dig;
		for (u8 i = 0; i < 5; i++, digp++)
		{
			//Get digit value
			u8 v = 0;
			while (combo >= *digp)
			{
				combo -= *digp;
				v++;
			}
			
			//Write digit value
			if (v || hit)
			{
				hit = true;
				this->num[i] = v;
			}
		}
		
		//Initialize number positions
		for (u8 i = 0; i < 5; i++)
		{
			if (this->num[i] == 0xFF)
				continue;
			this->numy[i] = y;
			this->numv[i] = -(FIXED_DEC(7,10) + RandomRange(0, FIXED_DEC(18,100)));
		}
		
		//Setup combo
		this->cy = y;
		this->cv = -(FIXED_DEC(7,10) + RandomRange(0, FIXED_DEC(16,100)));
	}
	else
	{
		//Write null numbers
		this->num[0] = this->num[1] = this->num[2] = this->num[3] = this->num[4] = 0xFF;
	}
	
	//Initialize timers
	this->ht = -30;
	this->ct = -53;
	this->numt = -56;
	
	return this;
}
