/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: NT35510 8080驱动芯片，IO模拟驱动.
 * Author: leon sun
 * Create: 2022-11-24
 ************************************************************************************************/


#include "drv_nt35510.h"
#include "stdio.h"
#include "mhscpu.h"
#include "cmsis_os.h"
//#include "test_image_480_800.h"
//#include "test_image.h"
#include "user_memory.h"
#include "drv_parallel8080.h"
#include "user_delay.h"


#define NT35510_SET_CMD_X               0x2A00
#define NT35510_SET_CMD_Y               0x2B00
#define NT35510_SET_CMD_WRAM            0x2C00

static void Nt35510WriteReg(uint16_t reg);
static void Nt35510WriteData(uint8_t data);
//static void Parallel8080WriteToReg(uint16_t reg, uint8_t value);
static void Nt35510WriteToReg16(uint16_t reg, uint8_t value);
static void Nt35510SetWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd);
static void Nt35510WriteRam(void);
static void Nt35510InitSequence(void);


void Nt35510Init(void)
{
    Parallel8080Init();
    UserDelay(50);
    Parallel8080Reset();   //初始化之前复位
    Nt35510InitSequence();
}


static void Nt35510WriteReg(uint16_t reg)
{
    PARALLEL_8080_CS_CLR;
    LCDI_Write(LCD, LCDI_CMD, reg >> 8);
    LCDI_Write(LCD, LCDI_CMD, reg & 0xFF);
    PARALLEL_8080_CS_SET;
}


static void Nt35510WriteData(uint8_t data)
{
    PARALLEL_8080_CS_CLR;
    LCDI_Write(LCD, LCDI_DAT, data);
    PARALLEL_8080_CS_SET;
}


static void Nt35510WriteToReg16(uint16_t reg, uint8_t value)
{
    Nt35510WriteReg(reg);
    Nt35510WriteData(value);
    Nt35510WriteData(value);
}


static void Nt35510SetWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd)
{
    //Different from other registers, the coordinate registers need to be written in 16bit mode.
    Nt35510WriteToReg16(NT35510_SET_CMD_X, xStart >> 8);
    Nt35510WriteToReg16(NT35510_SET_CMD_X + 1, xStart);
    Nt35510WriteToReg16(NT35510_SET_CMD_X + 2, xEnd >> 8);
    Nt35510WriteToReg16(NT35510_SET_CMD_X + 3, xEnd);
    Nt35510WriteToReg16(NT35510_SET_CMD_Y, yStart >> 8);
    Nt35510WriteToReg16(NT35510_SET_CMD_Y + 1, yStart);
    Nt35510WriteToReg16(NT35510_SET_CMD_Y + 2, yEnd >> 8);
    Nt35510WriteToReg16(NT35510_SET_CMD_Y + 3, yEnd);

    Nt35510WriteRam();      //Writing GRAM mode.
}


static void Nt35510WriteRam(void)
{
    Nt35510WriteReg(NT35510_SET_CMD_WRAM);
}


bool Nt35510Busy(void)
{
    return Parallel8080Busy();
}


void Nt35510Clear(uint16_t color)
{
    uint32_t i;
    uint8_t *ramImg;
    uint32_t startTick, endTick;

    while (Parallel8080Busy());
    ramImg = EXT_MALLOC(NT35510_HEIGHT * NT35510_WIDTH * 2);
    if (ramImg == NULL) {
        printf("clear malloc err\r\n");
        return;
    }
    for (i = 0; i < NT35510_WIDTH * NT35510_HEIGHT ; i++) {
        ramImg[i * 2] = color >> 8;
        ramImg[i * 2 + 1] = color;
    }
    Nt35510SetWindow(0, 0, NT35510_WIDTH - 1, NT35510_HEIGHT - 1);
    startTick = osKernelGetTickCount();
    Parallel8080SendDmaData(ramImg, NT35510_HEIGHT * NT35510_WIDTH * 2);
    while (Parallel8080Busy());
    endTick = osKernelGetTickCount();
    EXT_FREE(ramImg);
    printf("frame tick=%d\r\n", endTick - startTick);
}


void Nt35510Draw(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t *colors)
{
    uint32_t bytes;
    Nt35510SetWindow(xStart, yStart, xEnd, yEnd);
    bytes = (yEnd - yStart + 1) * (xEnd - xStart + 1) * 2;
    Parallel8080SendDmaData((uint8_t *)colors, bytes);
}


static void Nt35510InitSequence(void)
{
//************* NT35510初始化**********//
    Nt35510WriteReg(0xF000);
    Nt35510WriteData(0x55);
    Nt35510WriteReg(0xF001);
    Nt35510WriteData(0xAA);
    Nt35510WriteReg(0xF002);
    Nt35510WriteData(0x52);
    Nt35510WriteReg(0xF003);
    Nt35510WriteData(0x08);
    Nt35510WriteReg(0xF004);
    Nt35510WriteData(0x01);
    //# AVDD: manual); Nt35510WriteData(
    Nt35510WriteReg(0xB600);
    Nt35510WriteData(0x34);
    Nt35510WriteReg(0xB601);
    Nt35510WriteData(0x34);
    Nt35510WriteReg(0xB602);
    Nt35510WriteData(0x34);

    Nt35510WriteReg(0xB000);
    Nt35510WriteData(0x0D);//09
    Nt35510WriteReg(0xB001);
    Nt35510WriteData(0x0D);
    Nt35510WriteReg(0xB002);
    Nt35510WriteData(0x0D);
    //# AVEE: manual); Nt35510WriteData( -6V
    Nt35510WriteReg(0xB700);
    Nt35510WriteData(0x24);
    Nt35510WriteReg(0xB701);
    Nt35510WriteData(0x24);
    Nt35510WriteReg(0xB702);
    Nt35510WriteData(0x24);

    Nt35510WriteReg(0xB100);
    Nt35510WriteData(0x0D);
    Nt35510WriteReg(0xB101);
    Nt35510WriteData(0x0D);
    Nt35510WriteReg(0xB102);
    Nt35510WriteData(0x0D);
    //#Power Control for
    //VCL
    Nt35510WriteReg(0xB800);
    Nt35510WriteData(0x24);
    Nt35510WriteReg(0xB801);
    Nt35510WriteData(0x24);
    Nt35510WriteReg(0xB802);
    Nt35510WriteData(0x24);

    Nt35510WriteReg(0xB200);
    Nt35510WriteData(0x00);

    //# VGH: Clamp Enable); Nt35510WriteData(
    Nt35510WriteReg(0xB900);
    Nt35510WriteData(0x24);
    Nt35510WriteReg(0xB901);
    Nt35510WriteData(0x24);
    Nt35510WriteReg(0xB902);
    Nt35510WriteData(0x24);

    Nt35510WriteReg(0xB300);
    Nt35510WriteData(0x05);
    Nt35510WriteReg(0xB301);
    Nt35510WriteData(0x05);
    Nt35510WriteReg(0xB302);
    Nt35510WriteData(0x05);

    ///Nt35510WriteReg(0xBF00); Nt35510WriteData(0x01);

    //# VGL(LVGL):
    Nt35510WriteReg(0xBA00);
    Nt35510WriteData(0x34);
    Nt35510WriteReg(0xBA01);
    Nt35510WriteData(0x34);
    Nt35510WriteReg(0xBA02);
    Nt35510WriteData(0x34);
    //# VGL_REG(VGLO)
    Nt35510WriteReg(0xB500);
    Nt35510WriteData(0x0B);
    Nt35510WriteReg(0xB501);
    Nt35510WriteData(0x0B);
    Nt35510WriteReg(0xB502);
    Nt35510WriteData(0x0B);
    //# VGMP/VGSP:
    Nt35510WriteReg(0xBC00);
    Nt35510WriteData(0X00);
    Nt35510WriteReg(0xBC01);
    Nt35510WriteData(0xA3);
    Nt35510WriteReg(0xBC02);
    Nt35510WriteData(0X00);
    //# VGMN/VGSN
    Nt35510WriteReg(0xBD00);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xBD01);
    Nt35510WriteData(0xA3);
    Nt35510WriteReg(0xBD02);
    Nt35510WriteData(0x00);
    //# VCOM=-0.1
    Nt35510WriteReg(0xBE00);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xBE01);
    Nt35510WriteData(0x63);//4f
    //  VCOMH+0x01;
    //#R+
    Nt35510WriteReg(0xD100);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD101);
    Nt35510WriteData(0x37);
    Nt35510WriteReg(0xD102);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD103);
    Nt35510WriteData(0x52);
    Nt35510WriteReg(0xD104);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD105);
    Nt35510WriteData(0x7B);
    Nt35510WriteReg(0xD106);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD107);
    Nt35510WriteData(0x99);
    Nt35510WriteReg(0xD108);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD109);
    Nt35510WriteData(0xB1);
    Nt35510WriteReg(0xD10A);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD10B);
    Nt35510WriteData(0xD2);
    Nt35510WriteReg(0xD10C);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD10D);
    Nt35510WriteData(0xF6);
    Nt35510WriteReg(0xD10E);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD10F);
    Nt35510WriteData(0x27);
    Nt35510WriteReg(0xD110);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD111);
    Nt35510WriteData(0x4E);
    Nt35510WriteReg(0xD112);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD113);
    Nt35510WriteData(0x8C);
    Nt35510WriteReg(0xD114);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD115);
    Nt35510WriteData(0xBE);
    Nt35510WriteReg(0xD116);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD117);
    Nt35510WriteData(0x0B);
    Nt35510WriteReg(0xD118);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD119);
    Nt35510WriteData(0x48);
    Nt35510WriteReg(0xD11A);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD11B);
    Nt35510WriteData(0x4A);
    Nt35510WriteReg(0xD11C);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD11D);
    Nt35510WriteData(0x7E);
    Nt35510WriteReg(0xD11E);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD11F);
    Nt35510WriteData(0xBC);
    Nt35510WriteReg(0xD120);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD121);
    Nt35510WriteData(0xE1);
    Nt35510WriteReg(0xD122);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD123);
    Nt35510WriteData(0x10);
    Nt35510WriteReg(0xD124);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD125);
    Nt35510WriteData(0x31);
    Nt35510WriteReg(0xD126);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD127);
    Nt35510WriteData(0x5A);
    Nt35510WriteReg(0xD128);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD129);
    Nt35510WriteData(0x73);
    Nt35510WriteReg(0xD12A);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD12B);
    Nt35510WriteData(0x94);
    Nt35510WriteReg(0xD12C);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD12D);
    Nt35510WriteData(0x9F);
    Nt35510WriteReg(0xD12E);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD12F);
    Nt35510WriteData(0xB3);
    Nt35510WriteReg(0xD130);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD131);
    Nt35510WriteData(0xB9);
    Nt35510WriteReg(0xD132);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD133);
    Nt35510WriteData(0xC1);
    //#G+
    Nt35510WriteReg(0xD200);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD201);
    Nt35510WriteData(0x37);
    Nt35510WriteReg(0xD202);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD203);
    Nt35510WriteData(0x52);
    Nt35510WriteReg(0xD204);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD205);
    Nt35510WriteData(0x7B);
    Nt35510WriteReg(0xD206);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD207);
    Nt35510WriteData(0x99);
    Nt35510WriteReg(0xD208);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD209);
    Nt35510WriteData(0xB1);
    Nt35510WriteReg(0xD20A);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD20B);
    Nt35510WriteData(0xD2);
    Nt35510WriteReg(0xD20C);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD20D);
    Nt35510WriteData(0xF6);
    Nt35510WriteReg(0xD20E);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD20F);
    Nt35510WriteData(0x27);
    Nt35510WriteReg(0xD210);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD211);
    Nt35510WriteData(0x4E);
    Nt35510WriteReg(0xD212);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD213);
    Nt35510WriteData(0x8C);
    Nt35510WriteReg(0xD214);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD215);
    Nt35510WriteData(0xBE);
    Nt35510WriteReg(0xD216);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD217);
    Nt35510WriteData(0x0B);
    Nt35510WriteReg(0xD218);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD219);
    Nt35510WriteData(0x48);
    Nt35510WriteReg(0xD21A);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD21B);
    Nt35510WriteData(0x4A);
    Nt35510WriteReg(0xD21C);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD21D);
    Nt35510WriteData(0x7E);
    Nt35510WriteReg(0xD21E);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD21F);
    Nt35510WriteData(0xBC);
    Nt35510WriteReg(0xD220);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD221);
    Nt35510WriteData(0xE1);
    Nt35510WriteReg(0xD222);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD223);
    Nt35510WriteData(0x10);
    Nt35510WriteReg(0xD224);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD225);
    Nt35510WriteData(0x31);
    Nt35510WriteReg(0xD226);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD227);
    Nt35510WriteData(0x5A);
    Nt35510WriteReg(0xD228);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD229);
    Nt35510WriteData(0x73);
    Nt35510WriteReg(0xD22A);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD22B);
    Nt35510WriteData(0x94);
    Nt35510WriteReg(0xD22C);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD22D);
    Nt35510WriteData(0x9F);
    Nt35510WriteReg(0xD22E);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD22F);
    Nt35510WriteData(0xB3);
    Nt35510WriteReg(0xD230);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD231);
    Nt35510WriteData(0xB9);
    Nt35510WriteReg(0xD232);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD233);
    Nt35510WriteData(0xC1);
    //#B+
    Nt35510WriteReg(0xD300);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD301);
    Nt35510WriteData(0x37);
    Nt35510WriteReg(0xD302);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD303);
    Nt35510WriteData(0x52);
    Nt35510WriteReg(0xD304);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD305);
    Nt35510WriteData(0x7B);
    Nt35510WriteReg(0xD306);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD307);
    Nt35510WriteData(0x99);
    Nt35510WriteReg(0xD308);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD309);
    Nt35510WriteData(0xB1);
    Nt35510WriteReg(0xD30A);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD30B);
    Nt35510WriteData(0xD2);
    Nt35510WriteReg(0xD30C);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD30D);
    Nt35510WriteData(0xF6);
    Nt35510WriteReg(0xD30E);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD30F);
    Nt35510WriteData(0x27);
    Nt35510WriteReg(0xD310);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD311);
    Nt35510WriteData(0x4E);
    Nt35510WriteReg(0xD312);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD313);
    Nt35510WriteData(0x8C);
    Nt35510WriteReg(0xD314);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD315);
    Nt35510WriteData(0xBE);
    Nt35510WriteReg(0xD316);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD317);
    Nt35510WriteData(0x0B);
    Nt35510WriteReg(0xD318);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD319);
    Nt35510WriteData(0x48);
    Nt35510WriteReg(0xD31A);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD31B);
    Nt35510WriteData(0x4A);
    Nt35510WriteReg(0xD31C);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD31D);
    Nt35510WriteData(0x7E);
    Nt35510WriteReg(0xD31E);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD31F);
    Nt35510WriteData(0xBC);
    Nt35510WriteReg(0xD320);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD321);
    Nt35510WriteData(0xE1);
    Nt35510WriteReg(0xD322);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD323);
    Nt35510WriteData(0x10);
    Nt35510WriteReg(0xD324);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD325);
    Nt35510WriteData(0x31);
    Nt35510WriteReg(0xD326);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD327);
    Nt35510WriteData(0x5A);
    Nt35510WriteReg(0xD328);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD329);
    Nt35510WriteData(0x73);
    Nt35510WriteReg(0xD32A);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD32B);
    Nt35510WriteData(0x94);
    Nt35510WriteReg(0xD32C);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD32D);
    Nt35510WriteData(0x9F);
    Nt35510WriteReg(0xD32E);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD32F);
    Nt35510WriteData(0xB3);
    Nt35510WriteReg(0xD330);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD331);
    Nt35510WriteData(0xB9);
    Nt35510WriteReg(0xD332);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD333);
    Nt35510WriteData(0xC1);

    //#R-///////////////////////////////////////////
    Nt35510WriteReg(0xD400);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD401);
    Nt35510WriteData(0x37);
    Nt35510WriteReg(0xD402);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD403);
    Nt35510WriteData(0x52);
    Nt35510WriteReg(0xD404);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD405);
    Nt35510WriteData(0x7B);
    Nt35510WriteReg(0xD406);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD407);
    Nt35510WriteData(0x99);
    Nt35510WriteReg(0xD408);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD409);
    Nt35510WriteData(0xB1);
    Nt35510WriteReg(0xD40A);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD40B);
    Nt35510WriteData(0xD2);
    Nt35510WriteReg(0xD40C);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD40D);
    Nt35510WriteData(0xF6);
    Nt35510WriteReg(0xD40E);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD40F);
    Nt35510WriteData(0x27);
    Nt35510WriteReg(0xD410);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD411);
    Nt35510WriteData(0x4E);
    Nt35510WriteReg(0xD412);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD413);
    Nt35510WriteData(0x8C);
    Nt35510WriteReg(0xD414);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD415);
    Nt35510WriteData(0xBE);
    Nt35510WriteReg(0xD416);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD417);
    Nt35510WriteData(0x0B);
    Nt35510WriteReg(0xD418);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD419);
    Nt35510WriteData(0x48);
    Nt35510WriteReg(0xD41A);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD41B);
    Nt35510WriteData(0x4A);
    Nt35510WriteReg(0xD41C);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD41D);
    Nt35510WriteData(0x7E);
    Nt35510WriteReg(0xD41E);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD41F);
    Nt35510WriteData(0xBC);
    Nt35510WriteReg(0xD420);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD421);
    Nt35510WriteData(0xE1);
    Nt35510WriteReg(0xD422);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD423);
    Nt35510WriteData(0x10);
    Nt35510WriteReg(0xD424);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD425);
    Nt35510WriteData(0x31);
    Nt35510WriteReg(0xD426);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD427);
    Nt35510WriteData(0x5A);
    Nt35510WriteReg(0xD428);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD429);
    Nt35510WriteData(0x73);
    Nt35510WriteReg(0xD42A);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD42B);
    Nt35510WriteData(0x94);
    Nt35510WriteReg(0xD42C);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD42D);
    Nt35510WriteData(0x9F);
    Nt35510WriteReg(0xD42E);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD42F);
    Nt35510WriteData(0xB3);
    Nt35510WriteReg(0xD430);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD431);
    Nt35510WriteData(0xB9);
    Nt35510WriteReg(0xD432);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD433);
    Nt35510WriteData(0xC1);

    //#G-//////////////////////////////////////////////
    Nt35510WriteReg(0xD500);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD501);
    Nt35510WriteData(0x37);
    Nt35510WriteReg(0xD502);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD503);
    Nt35510WriteData(0x52);
    Nt35510WriteReg(0xD504);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD505);
    Nt35510WriteData(0x7B);
    Nt35510WriteReg(0xD506);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD507);
    Nt35510WriteData(0x99);
    Nt35510WriteReg(0xD508);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD509);
    Nt35510WriteData(0xB1);
    Nt35510WriteReg(0xD50A);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD50B);
    Nt35510WriteData(0xD2);
    Nt35510WriteReg(0xD50C);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD50D);
    Nt35510WriteData(0xF6);
    Nt35510WriteReg(0xD50E);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD50F);
    Nt35510WriteData(0x27);
    Nt35510WriteReg(0xD510);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD511);
    Nt35510WriteData(0x4E);
    Nt35510WriteReg(0xD512);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD513);
    Nt35510WriteData(0x8C);
    Nt35510WriteReg(0xD514);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD515);
    Nt35510WriteData(0xBE);
    Nt35510WriteReg(0xD516);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD517);
    Nt35510WriteData(0x0B);
    Nt35510WriteReg(0xD518);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD519);
    Nt35510WriteData(0x48);
    Nt35510WriteReg(0xD51A);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD51B);
    Nt35510WriteData(0x4A);
    Nt35510WriteReg(0xD51C);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD51D);
    Nt35510WriteData(0x7E);
    Nt35510WriteReg(0xD51E);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD51F);
    Nt35510WriteData(0xBC);
    Nt35510WriteReg(0xD520);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD521);
    Nt35510WriteData(0xE1);
    Nt35510WriteReg(0xD522);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD523);
    Nt35510WriteData(0x10);
    Nt35510WriteReg(0xD524);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD525);
    Nt35510WriteData(0x31);
    Nt35510WriteReg(0xD526);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD527);
    Nt35510WriteData(0x5A);
    Nt35510WriteReg(0xD528);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD529);
    Nt35510WriteData(0x73);
    Nt35510WriteReg(0xD52A);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD52B);
    Nt35510WriteData(0x94);
    Nt35510WriteReg(0xD52C);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD52D);
    Nt35510WriteData(0x9F);
    Nt35510WriteReg(0xD52E);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD52F);
    Nt35510WriteData(0xB3);
    Nt35510WriteReg(0xD530);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD531);
    Nt35510WriteData(0xB9);
    Nt35510WriteReg(0xD532);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD533);
    Nt35510WriteData(0xC1);
    //#B-///////////////////////////////
    Nt35510WriteReg(0xD600);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD601);
    Nt35510WriteData(0x37);
    Nt35510WriteReg(0xD602);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD603);
    Nt35510WriteData(0x52);
    Nt35510WriteReg(0xD604);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD605);
    Nt35510WriteData(0x7B);
    Nt35510WriteReg(0xD606);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD607);
    Nt35510WriteData(0x99);
    Nt35510WriteReg(0xD608);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD609);
    Nt35510WriteData(0xB1);
    Nt35510WriteReg(0xD60A);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD60B);
    Nt35510WriteData(0xD2);
    Nt35510WriteReg(0xD60C);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xD60D);
    Nt35510WriteData(0xF6);
    Nt35510WriteReg(0xD60E);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD60F);
    Nt35510WriteData(0x27);
    Nt35510WriteReg(0xD610);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD611);
    Nt35510WriteData(0x4E);
    Nt35510WriteReg(0xD612);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD613);
    Nt35510WriteData(0x8C);
    Nt35510WriteReg(0xD614);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xD615);
    Nt35510WriteData(0xBE);
    Nt35510WriteReg(0xD616);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD617);
    Nt35510WriteData(0x0B);
    Nt35510WriteReg(0xD618);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD619);
    Nt35510WriteData(0x48);
    Nt35510WriteReg(0xD61A);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD61B);
    Nt35510WriteData(0x4A);
    Nt35510WriteReg(0xD61C);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD61D);
    Nt35510WriteData(0x7E);
    Nt35510WriteReg(0xD61E);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD61F);
    Nt35510WriteData(0xBC);
    Nt35510WriteReg(0xD620);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xD621);
    Nt35510WriteData(0xE1);
    Nt35510WriteReg(0xD622);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD623);
    Nt35510WriteData(0x10);
    Nt35510WriteReg(0xD624);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD625);
    Nt35510WriteData(0x31);
    Nt35510WriteReg(0xD626);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD627);
    Nt35510WriteData(0x5A);
    Nt35510WriteReg(0xD628);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD629);
    Nt35510WriteData(0x73);
    Nt35510WriteReg(0xD62A);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD62B);
    Nt35510WriteData(0x94);
    Nt35510WriteReg(0xD62C);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD62D);
    Nt35510WriteData(0x9F);
    Nt35510WriteReg(0xD62E);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD62F);
    Nt35510WriteData(0xB3);
    Nt35510WriteReg(0xD630);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD631);
    Nt35510WriteData(0xB9);
    Nt35510WriteReg(0xD632);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xD633);
    Nt35510WriteData(0xC1);

    //#Enable Page0
    Nt35510WriteReg(0xF000);
    Nt35510WriteData(0x55);
    Nt35510WriteReg(0xF001);
    Nt35510WriteData(0xAA);
    Nt35510WriteReg(0xF002);
    Nt35510WriteData(0x52);
    Nt35510WriteReg(0xF003);
    Nt35510WriteData(0x08);
    Nt35510WriteReg(0xF004);
    Nt35510WriteData(0x00);
    //# RGB I/F Setting
    Nt35510WriteReg(0xB000);
    Nt35510WriteData(0x08);
    Nt35510WriteReg(0xB001);
    Nt35510WriteData(0x05);
    Nt35510WriteReg(0xB002);
    Nt35510WriteData(0x02);
    Nt35510WriteReg(0xB003);
    Nt35510WriteData(0x05);
    Nt35510WriteReg(0xB004);
    Nt35510WriteData(0x02);
    //## SDT:
    Nt35510WriteReg(0xB600);
    Nt35510WriteData(0x08);
    Nt35510WriteReg(0xB500);
    Nt35510WriteData(0x50);//0x6b ???? 480x854       0x50 ???? 480x800

    //## Gate EQ:
    Nt35510WriteReg(0xB700);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xB701);
    Nt35510WriteData(0x00);

    //## Source EQ:
    Nt35510WriteReg(0xB800);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xB801);
    Nt35510WriteData(0x05);
    Nt35510WriteReg(0xB802);
    Nt35510WriteData(0x05);
    Nt35510WriteReg(0xB803);
    Nt35510WriteData(0x05);

    //# Inversion: Column inversion (NVT)
    Nt35510WriteReg(0xBC00);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xBC01);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xBC02);
    Nt35510WriteData(0x00);

    //# BOE's Setting(default)
    Nt35510WriteReg(0xCC00);
    Nt35510WriteData(0x03);
    Nt35510WriteReg(0xCC01);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0xCC02);
    Nt35510WriteData(0x00);

    //# Display Timing:
    Nt35510WriteReg(0xBD00);
    Nt35510WriteData(0x01);
    Nt35510WriteReg(0xBD01);
    Nt35510WriteData(0x84);
    Nt35510WriteReg(0xBD02);
    Nt35510WriteData(0x07);
    Nt35510WriteReg(0xBD03);
    Nt35510WriteData(0x31);
    Nt35510WriteReg(0xBD04);
    Nt35510WriteData(0x00);

    Nt35510WriteReg(0xBA00);
    Nt35510WriteData(0x01);

    Nt35510WriteReg(0xFF00);
    Nt35510WriteData(0xAA);
    Nt35510WriteReg(0xFF01);
    Nt35510WriteData(0x55);
    Nt35510WriteReg(0xFF02);
    Nt35510WriteData(0x25);
    Nt35510WriteReg(0xFF03);
    Nt35510WriteData(0x01);

    Nt35510WriteReg(0x3500);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0x3600);
    Nt35510WriteData(0x00);
    Nt35510WriteReg(0x3A00);
    Nt35510WriteData(0x55);  ////55=16?/////66=18?
    Nt35510WriteReg(0x1100);
    UserDelay(120);
    Nt35510WriteReg(0x2900);
    Nt35510WriteReg(0x2C00);
    //设置LCD属性参数
    //LCD_direction(USE_HORIZONTAL);      //设置LCD显示方向
    //Nt35510WriteToReg(0x3600, 0x00);
    Nt35510WriteToReg16(0x3600, (1 << 6) | (1 << 0));

    //LCD_BL=1;//点亮背光
}

