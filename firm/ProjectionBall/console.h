/*
 *  	console.h
 *
 *  Copyright (c) 2023
 *  K.Watanabe, Crescentt
 *  Released under the MIT license
 *  http://opensource.org/licenses/mit-license.php
 *
 */
#ifndef CONSOLE_H
#define CONSOLE_H

void ConsoleGetString();
void OnUartRx();
bool IsResetEnable();
void ClearBuffer();

#endif