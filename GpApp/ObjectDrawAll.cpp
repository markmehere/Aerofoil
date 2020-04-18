
//============================================================================
//----------------------------------------------------------------------------
//								ObjectDrawAll.c
//----------------------------------------------------------------------------
//============================================================================

#include "BitmapImage.h"
#include "Externs.h"
#include "Environ.h"
#include "RectUtils.h"


extern	Rect		localRoomsDest[], movieRect;
extern	short		localNumbers[];
extern	short		numLights, tvWithMovieNumber;
extern	Boolean		tvOn;


//==============================================================  Functions
//--------------------------------------------------------------  DrawARoomsObjects

void DrawARoomsObjects (short neighbor, Boolean redraw)
{
	objectType	thisObject;
	Rect		whoCares, itsRect, rectA, rectB, testRect;
	short		i, legit, dynamicNum, n;
	short		floor, suite, room, obj;
	char		wasState;
	Boolean		isLit;
	
	if (localNumbers[neighbor] == kRoomIsEmpty)
		return;
	
	testRect = houseRect;
	ZeroRectCorner(&testRect);
	isLit = (numLights > 0);
	
	for (i = 0; i < kMaxRoomObs; i++)
	{
		dynamicNum = -1;
		legit = -1;
		
		if (IsThisValid(localNumbers[neighbor], i))
		{
			thisObject = (*thisHouse)->rooms[localNumbers[neighbor]].objects[i];
			switch (thisObject.what)
			{
				case kObjectIsEmpty:
				break;
				
				case kFloorVent:
				case kCeilingVent:
				case kFloorBlower:
				case kCeilingBlower:
				case kSewerGrate:
				case kLeftFan:
				case kRightFan:
				case kGrecoVent:
				case kSewerBlower:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawSimpleBlowers(thisObject.what, &itsRect);
				break;
				
				case kTaper:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					if (isLit)
						DrawSimpleBlowers(thisObject.what, &itsRect);
					if (neighbor == kCentralRoom)
					{
						const short h = itsRect.left + 10;
						const short v = itsRect.top + 7;
						if (redraw)
							ReBackUpFlames(localNumbers[neighbor], i, h, v);
						else
							AddCandleFlame(localNumbers[neighbor], i, 
									h, v);
					}
					else
					{
						QSetRect(&rectA, 0, 0, 16, 15);
						QOffsetRect(&rectA, itsRect.left + 10 - 8, itsRect.top + 7 - 15);
						rectB = localRoomsDest[kCentralRoom];
						rectB.top -= kFloorSupportTall;
						rectB.bottom += kFloorSupportTall;
						if (!SectRect(&rectA, &rectB, &whoCares))
						{
							const short h = itsRect.left + 10;
							const short v = itsRect.top + 7;
							if (redraw)
								ReBackUpFlames(localNumbers[neighbor], i, h, v);
							else
								AddCandleFlame(localNumbers[neighbor], i, 
										h, v);
						}
					}
				}
				break;
				
				case kCandle:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					if (isLit)
						DrawSimpleBlowers(thisObject.what, &itsRect);
					if (neighbor == kCentralRoom)
					{
						const short h = itsRect.left + 14;
						const short v = itsRect.top + 7;
						if (redraw)
							ReBackUpFlames(localNumbers[neighbor], i, h, v);
						else
							AddCandleFlame(localNumbers[neighbor], i, 
									h, v);
					}
					else
					{
						QSetRect(&rectA, 0, 0, 16, 15);
						QOffsetRect(&rectA, itsRect.left + 14 - 8, itsRect.top + 7 - 15);
						rectB = localRoomsDest[kCentralRoom];
						rectB.top -= kFloorSupportTall;
						rectB.bottom += kFloorSupportTall;
						if (!SectRect(&rectA, &rectB, &whoCares))
						{
							const short h = itsRect.left + 14;
							const short v = itsRect.top + 7;
							if (redraw)
								ReBackUpFlames(localNumbers[neighbor], i, h, v);
							else
								AddCandleFlame(localNumbers[neighbor], i, 
										h, v);
						}
					}
				}
				break;
				
				case kStubby:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					if (isLit)
						DrawSimpleBlowers(thisObject.what, &itsRect);
					if (neighbor == kCentralRoom)
					{
						const short h = itsRect.left + 9;
						const short v = itsRect.top + 7;
						if (redraw)
							ReBackUpFlames(localNumbers[neighbor], i, h, v);
						else
							AddCandleFlame(localNumbers[neighbor], i, 
									h, v);
					}
					else
					{
						QSetRect(&rectA, 0, 0, 16, 15);
						QOffsetRect(&rectA, itsRect.left + 9 - 8, itsRect.top + 7 - 15);
						rectB = localRoomsDest[kCentralRoom];
						rectB.top -= kFloorSupportTall;
						rectB.bottom += kFloorSupportTall;
						if (!SectRect(&rectA, &rectB, &whoCares))
						{
							const short h = itsRect.left + 9;
							const short v = itsRect.top + 7;
							if (redraw)
								ReBackUpFlames(localNumbers[neighbor], i, h, v);
							else
								AddCandleFlame(localNumbers[neighbor], i, 
										h, v);
						}
					}
				}
				break;
				
				case kTiki:
				{
					GetObjectRect(&thisObject, &itsRect);
					OffsetRectRoomRelative(&itsRect, neighbor);
					const short h = itsRect.left + 10;
					const short v = itsRect.top - 9;
					if (isLit)
						DrawTiki(&itsRect, playOriginV + VerticalRoomOffset(neighbor));
					if (redraw)
						ReBackUpTikiFlames(localNumbers[neighbor], i, h, v);
					else
						AddTikiFlame(localNumbers[neighbor], i, h, v);
				}
				break;
				
				case kBBQ:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					const short h = itsRect.left + 16;
					const short v = itsRect.top + 9;
					if (isLit)
						DrawPictSansWhiteObject(thisObject.what, &itsRect);
					if (redraw)
						ReBackUpBBQCoals(localNumbers[neighbor], i, h, v);
					else
						AddBBQCoals(localNumbers[neighbor], i, h, v);
				}
				break;
				
				case kInvisBlower:
				case kLiftArea:
				break;
				
				case kTable:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (isLit)
					DrawTable(&itsRect, playOriginV);
				break;
				
				case kShelf:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (isLit)
					DrawShelf(&itsRect);
				break;
				
				case kCabinet:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawCabinet(&itsRect);
				break;
				
				case kFilingCabinet:
				case kOzma:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawPictObject(thisObject.what, &itsRect);
				break;
				
				case kWasteBasket:
				case kMilkCrate:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawSimpleFurniture(thisObject.what, &itsRect);
				break;
				
				case kCounter:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawCounter(&itsRect);
				break;
				
				case kDresser:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (isLit)
					DrawDresser(&itsRect);
				break;
				
				case kDeckTable:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (isLit)
					DrawDeckTable(&itsRect, playOriginV);
				break;
				
				case kStool:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (isLit)
					DrawStool(&itsRect, playOriginV + VerticalRoomOffset(neighbor));
				break;
				
				case kInvisObstacle:
				break;
				
				case kManhole:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					AddTempManholeRect(&itsRect);
					if (isLit)
						DrawPictSansWhiteObject(thisObject.what, &itsRect);
				}
				break;
				
				case kInvisBounce:
				break;
				
				case kRedClock:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					if (redraw)
						legit = ReBackUpSavedMap(&itsRect, localNumbers[neighbor], i);
					else
						legit = BackUpToSavedMap(&itsRect, localNumbers[neighbor], i);
					if (legit != -1)
						DrawRedClock(&itsRect);
				}
				break;
				
				case kBlueClock:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					if (redraw)
						legit = ReBackUpSavedMap(&itsRect, localNumbers[neighbor], i);
					else
						legit = BackUpToSavedMap(&itsRect, localNumbers[neighbor], i);
					if (legit != -1)
						DrawBlueClock(&itsRect);
				}
				break;
				
				case kYellowClock:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					if (redraw)
						legit = ReBackUpSavedMap(&itsRect, localNumbers[neighbor], i);
					else
						legit = BackUpToSavedMap(&itsRect, localNumbers[neighbor], i);
					if (legit != -1)
						DrawYellowClock(&itsRect);
				}
				break;
				
				case kCuckoo:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					if (redraw)
						legit = ReBackUpSavedMap(&itsRect, localNumbers[neighbor], i);
					else
						legit = BackUpToSavedMap(&itsRect, localNumbers[neighbor], i);
					if (legit != -1)
					{
						DrawCuckoo(&itsRect);
						short pendulumH = itsRect.left + 4;
						short pendulumV = itsRect.top + 46;
						if (redraw)
							ReBackUpPendulum(localNumbers[neighbor], i, pendulumH, pendulumV);
						else
							AddPendulum(localNumbers[neighbor], i, 
									itsRect.left + 4, itsRect.top + 46);
					}
				}
				break;
				
				case kPaper:
				case kBattery:
				case kBands:
				case kHelium:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					if (redraw)
						legit = ReBackUpSavedMap(&itsRect, localNumbers[neighbor], i);
					else
						legit = BackUpToSavedMap(&itsRect, localNumbers[neighbor], i);
					if (legit != -1)
						DrawSimplePrizes(thisObject.what, &itsRect);
				}
				break;
				
				case kGreaseRt:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (thisObject.data.c.state)		// standing
				{
					if (SectRect(&itsRect, &testRect, &whoCares))
					{
						if (redraw)
							dynamicNum = ReBackUpGrease(localNumbers[neighbor], i, itsRect.left, itsRect.top);
						else
							dynamicNum = AddGrease(localNumbers[neighbor], i, 
									itsRect.left, itsRect.top, 
									thisObject.data.c.length, true);
						if (dynamicNum != -1)
							DrawGreaseRt(&itsRect, thisObject.data.c.length, true, false);
					}
				}
				else								// fallen
				{
					Boolean isDynamic = false;
					if (redraw)
						FixupFallenGrease(localNumbers[neighbor], i, itsRect.left, itsRect.top, &isDynamic);
					DrawGreaseRt(&itsRect, thisObject.data.c.length, false, !isDynamic);
				}
				break;
				
				case kGreaseLf:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (thisObject.data.c.state)
				{
					if (SectRect(&itsRect, &testRect, &whoCares))
					{
						if (redraw)
							dynamicNum = ReBackUpGrease(localNumbers[neighbor], i, itsRect.left, itsRect.top);
						else
							dynamicNum = AddGrease(localNumbers[neighbor], i, 
									itsRect.left, itsRect.top, 
									thisObject.data.c.length, false);
						if (dynamicNum != -1)
							DrawGreaseLf(&itsRect, thisObject.data.c.length, true, false);
					}
				}
				else
				{
					Boolean isDynamic = false;
					if (redraw)
						FixupFallenGrease(localNumbers[neighbor], i, itsRect.left, itsRect.top, &isDynamic);
					DrawGreaseLf(&itsRect, thisObject.data.c.length, false, !isDynamic);
				}
				break;
				
				case kFoil:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					if (redraw)
						legit = ReBackUpSavedMap(&itsRect, localNumbers[neighbor], i);
					else
						legit = BackUpToSavedMap(&itsRect, localNumbers[neighbor], i);
					if (legit != -1)
						DrawFoil(&itsRect);
				}
				break;
				
				case kInvisBonus:
				case kSlider:
				break;
				
				case kStar:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					if (redraw)
						legit = ReBackUpSavedMap(&itsRect, localNumbers[neighbor], i);
					else
						legit = BackUpToSavedMap(&itsRect, localNumbers[neighbor], i);
					if (legit != -1)
					{
						if (redraw)
							ReBackUpStar(localNumbers[neighbor], i, itsRect.left, itsRect.top);
						else
							AddStar(localNumbers[neighbor], i, itsRect.left, 
									itsRect.top);
						DrawSimplePrizes(thisObject.what, &itsRect);
					}
				}
				break;
				
				case kSparkle:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					if (neighbor == kCentralRoom)
					{
						rectA = itsRect;
						QOffsetRect(&rectA, -playOriginH, -playOriginV);
						dynamicNum = AddDynamicObject(kSparkle, &rectA, &thisObject, 
								localNumbers[neighbor], i, thisObject.data.c.state, redraw);
					}
				}
				break;
				
				case kUpStairs:
				case kDoorInLf:
				case kDoorInRt:
				case kWindowInLf:
				case kWindowInRt:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
					DrawPictSansWhiteObject(thisObject.what, &itsRect);
				break;
				
				case kDownStairs:
				case kDoorExRt:
				case kDoorExLf:
				case kWindowExRt:
				case kWindowExLf:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
					DrawPictObject(thisObject.what, &itsRect);
				break;
				
				case kMailboxLf:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				DrawMailboxLeft(&itsRect, playOriginV + VerticalRoomOffset(neighbor));
				break;
				
				case kMailboxRt:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				DrawMailboxRight(&itsRect, playOriginV + VerticalRoomOffset(neighbor));
				break;
				
				case kFloorTrans:
				case kCeilingTrans:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
					DrawSimpleTransport(thisObject.what, &itsRect);
				break;
				
				case kInvisTrans:
				case kDeluxeTrans:
				break;
				
				case kLightSwitch:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					ExtractFloorSuite(thisObject.data.e.where, &floor, &suite);
					room = GetRoomNumber(floor, suite);
					obj = (short)thisObject.data.e.who;
					DrawLightSwitch(&itsRect, GetObjectState(room, obj));
				}
				dynamicNum = masterObjects[i].hotNum;
				break;
				
				case kMachineSwitch:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					ExtractFloorSuite(thisObject.data.e.where, &floor, &suite);
					room = GetRoomNumber(floor, suite);
					obj = (short)thisObject.data.e.who;
					DrawMachineSwitch(&itsRect, GetObjectState(room, obj));
				}
				dynamicNum = masterObjects[i].hotNum;
				break;
				
				case kThermostat:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					ExtractFloorSuite(thisObject.data.e.where, &floor, &suite);
					room = GetRoomNumber(floor, suite);
					obj = (short)thisObject.data.e.who;
					DrawThermostat(&itsRect, GetObjectState(room, obj));
				}
				dynamicNum = masterObjects[i].hotNum;
				break;
				
				case kPowerSwitch:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					ExtractFloorSuite(thisObject.data.e.where, &floor, &suite);
					room = GetRoomNumber(floor, suite);
					obj = (short)thisObject.data.e.who;
					DrawPowerSwitch(&itsRect, GetObjectState(room, obj));
				}
				dynamicNum = masterObjects[i].hotNum;
				break;
				
				case kKnifeSwitch:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					ExtractFloorSuite(thisObject.data.e.where, &floor, &suite);
					room = GetRoomNumber(floor, suite);
					obj = (short)thisObject.data.e.who;
					DrawKnifeSwitch(&itsRect, GetObjectState(room, obj));
				}
				dynamicNum = masterObjects[i].hotNum;
				break;
				
				case kInvisSwitch:
				dynamicNum = masterObjects[i].hotNum;
				break;
				
				case kTrigger:
				case kLgTrigger:
				case kSoundTrigger:
				break;
				
				case kCeilingLight:
				case kLightBulb:
				case kTableLamp:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawSimpleLight(thisObject.what, &itsRect);
				break;
				
				case kTrunk:
				case kBooks:
				case kHipLamp:
				case kDecoLamp:
				case kGuitar:
				case kCinderBlock:
				case kFlowerBox:
				case kFireplace:
				case kBear:
				case kVase1:
				case kVase2:
				case kRug:
				case kChimes:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawPictSansWhiteObject(thisObject.what, &itsRect);
				break;
				
				case kCustomPict:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawCustPictSansWhite(thisObject.data.g.height, &itsRect);
				break;
				
				case kFlourescent:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawFlourescent(&itsRect);
				break;
				
				case kTrackLight:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawTrackLight(&itsRect);
				break;
				
				case kInvisLight:
				break;
				
				case kShredder:
				case kCDs:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawSimpleAppliance(thisObject.what, &itsRect);
				break;
				
				case kToaster:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					DrawSimpleAppliance(thisObject.what, &itsRect);
					if (neighbor == kCentralRoom)
					{
						rectA = itsRect;
						QOffsetRect(&rectA, -playOriginH, -playOriginV);
						dynamicNum = AddDynamicObject(kToaster, &rectA, &thisObject, 
								localNumbers[neighbor], i, thisObject.data.g.state, redraw);
					}
				}
				break;
				
				case kMacPlus:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					DrawMacPlus(&itsRect, thisObject.data.g.state, isLit);
					rectA = itsRect;
					QOffsetRect(&rectA, -playOriginH, -playOriginV);
					dynamicNum = AddDynamicObject(kMacPlus, &rectA, &thisObject, 
							localNumbers[neighbor], i, thisObject.data.g.state, redraw);
				}
				break;
				
				case kTV:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
#ifdef COMPILEQT
					if ((thisMac.hasQT) && (hasMovie) && (neighbor == kCentralRoom))
					{
						whoCares = tvScreen1;
						ZeroRectCorner(&whoCares);
						OffsetRect(&whoCares, itsRect.left + 17, itsRect.top + 10);
						movieRect = (*theMovie.m_animPackage->GetFrame(0))->GetRect();
						CenterRectInRect(&movieRect, &whoCares);
						theMovie.m_renderRect = movieRect;
						theMovie.m_constrainRect = whoCares;

						if (!tvInRoom)
							tvOn = thisObject.data.g.state;
					}
#endif
					DrawTV(&itsRect, thisObject.data.g.state, isLit);
					rectA = itsRect;
					QOffsetRect(&rectA, -playOriginH, -playOriginV);
					dynamicNum = AddDynamicObject(kTV, &rectA, &thisObject, 
							localNumbers[neighbor], i, thisObject.data.g.state, redraw);
					if (!redraw)
					{
#ifdef COMPILEQT
						if ((thisMac.hasQT) && (hasMovie) && (neighbor == kCentralRoom) && 
								(!tvInRoom))
						{
							tvWithMovieNumber = dynamicNum;
							tvInRoom = true;
						}
#endif
					}
				}
				break;
				
				case kCoffee:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					DrawCoffee(&itsRect, thisObject.data.g.state, isLit);
					rectA = itsRect;
					QOffsetRect(&rectA, -playOriginH, -playOriginV);
					dynamicNum = AddDynamicObject(kCoffee, &rectA, &thisObject, 
							localNumbers[neighbor], i, thisObject.data.g.state, redraw);
				}
				break;
				
				case kOutlet:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					if (isLit)
						DrawOutlet(&itsRect);
					rectA = itsRect;
					QOffsetRect(&rectA, -playOriginH, -playOriginV);
					dynamicNum = AddDynamicObject(kOutlet, &rectA, &thisObject, 
							localNumbers[neighbor], i, thisObject.data.g.state, redraw);
				}
				break;
				
				case kVCR:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					DrawVCR(&itsRect, thisObject.data.g.state, isLit);
					rectA = itsRect;
					QOffsetRect(&rectA, -playOriginH, -playOriginV);
					dynamicNum = AddDynamicObject(kVCR, &rectA, &thisObject, 
							localNumbers[neighbor], i, thisObject.data.g.state, redraw);
				}
				break;
				
				case kStereo:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					DrawStereo(&itsRect, isPlayMusicGame, isLit);
					rectA = itsRect;
					QOffsetRect(&rectA, -playOriginH, -playOriginV);
					dynamicNum = AddDynamicObject(kStereo, &rectA, &thisObject, 
							localNumbers[neighbor], i, thisObject.data.g.state, redraw);
				}
				break;
				
				case kMicrowave:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					DrawMicrowave(&itsRect, thisObject.data.g.state, isLit);
					rectA = itsRect;
					QOffsetRect(&rectA, -playOriginH, -playOriginV);
					dynamicNum = AddDynamicObject(kMicrowave, &rectA, &thisObject, 
							localNumbers[neighbor], i, thisObject.data.g.state, redraw);
				}
				break;
				
				case kBalloon:
				if (neighbor == kCentralRoom)
				{
					GetObjectRect(&thisObject, &itsRect);
					OffsetRectRoomRelative(&itsRect, neighbor);
					QOffsetRect(&itsRect, -playOriginH, -playOriginV);
					dynamicNum = AddDynamicObject(kBalloon, &itsRect, &thisObject, 
							localNumbers[neighbor], i, thisObject.data.h.state, redraw);
				}
				break;
				
				case kCopterLf:
				if (neighbor == kCentralRoom)
				{
					GetObjectRect(&thisObject, &itsRect);
					OffsetRectRoomRelative(&itsRect, neighbor);
					QOffsetRect(&itsRect, -playOriginH, -playOriginV);
					dynamicNum = AddDynamicObject(kCopterLf, &itsRect, &thisObject, 
							localNumbers[neighbor], i, thisObject.data.h.state, redraw);
				}
				break;
				
				case kCopterRt:
				if (neighbor == kCentralRoom)
				{
					GetObjectRect(&thisObject, &itsRect);
					OffsetRectRoomRelative(&itsRect, neighbor);
					QOffsetRect(&itsRect, -playOriginH, -playOriginV);
					dynamicNum = AddDynamicObject(kCopterRt, &itsRect, &thisObject, 
							localNumbers[neighbor], i, thisObject.data.h.state, redraw);
				}
				break;
				
				case kDartLf:
				if (neighbor == kCentralRoom)
				{
					GetObjectRect(&thisObject, &itsRect);
					OffsetRectRoomRelative(&itsRect, neighbor);
					QOffsetRect(&itsRect, -playOriginH, -playOriginV);
					dynamicNum = AddDynamicObject(kDartLf, &itsRect, &thisObject, 
							localNumbers[neighbor], i, thisObject.data.h.state, redraw);
				}
				break;
				
				case kDartRt:
				if (neighbor == kCentralRoom)
				{
					GetObjectRect(&thisObject, &itsRect);
					OffsetRectRoomRelative(&itsRect, neighbor);
					QOffsetRect(&itsRect, -playOriginH, -playOriginV);
					dynamicNum = AddDynamicObject(kDartRt, &itsRect, &thisObject, 
							localNumbers[neighbor], i, thisObject.data.h.state, redraw);
				}
				break;
				
				case kBall:
				if (neighbor == kCentralRoom)
				{
					GetObjectRect(&thisObject, &itsRect);
					OffsetRectRoomRelative(&itsRect, neighbor);
					QOffsetRect(&itsRect, -playOriginH, -playOriginV);
					dynamicNum = AddDynamicObject(kBall, &itsRect, &thisObject, 
							localNumbers[neighbor], i, thisObject.data.h.state, redraw);
				}
				break;
				
				case kDrip:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					DrawDrip(&itsRect);
					if (neighbor == kCentralRoom)
					{
						rectA = itsRect;
						QOffsetRect(&rectA, -playOriginH, -playOriginV);
						dynamicNum = AddDynamicObject(kDrip, &rectA, &thisObject, 
								localNumbers[neighbor], i, thisObject.data.h.state, redraw);
					}
				}
				break;
				
				case kFish:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
				{
					DrawFish(thisObject.what, &itsRect);
					if (neighbor == kCentralRoom)
					{
						rectA = itsRect;
						QOffsetRect(&rectA, -playOriginH, -playOriginV);
						dynamicNum = AddDynamicObject(kFish, &rectA, &thisObject, 
								localNumbers[neighbor], i, thisObject.data.h.state, redraw);
					}
				}
				break;
				
				case kCobweb:
				case kCloud:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawPictWithMaskObject(thisObject.what, &itsRect);
				break;
				
				case kMirror:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawMirror(&itsRect);
				if (neighbor == kCentralRoom)
				{
					InsetRect(&itsRect, 4, 4);
					AddToMirrorRegion(&itsRect);
				}
				break;
				
				case kMousehole:
				case kFaucet:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawSimpleClutter(thisObject.what, &itsRect);
				break;
				
				case kFlower:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawFlower(&itsRect, thisObject.data.i.pict);
				break;
				
				case kWallWindow:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if (SectRect(&itsRect, &testRect, &whoCares))
					DrawWallWindow(&itsRect);
				break;
				
				case kCalendar:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawCalendar(&itsRect);
				break;
				
				case kBulletin:
				GetObjectRect(&thisObject, &itsRect);
				OffsetRectRoomRelative(&itsRect, neighbor);
				if ((SectRect(&itsRect, &testRect, &whoCares)) && isLit)
					DrawBulletin(&itsRect);
				break;
				
			}
		}

		for (n = 0; n < numMasterObjects; n++)
		{
			if ((masterObjects[n].objectNum == i) && 
					(masterObjects[n].roomNum == localNumbers[neighbor]))
				masterObjects[n].dynaNum = dynamicNum;
		}
	}
}

