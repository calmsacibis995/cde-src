/*
 * callback headers
 */
#ifndef CALLBACKS_H
#define CALLBACKS_H

/*
 * Callbacks
 */
void ClearCb(Widget, XtPointer, XtPointer);
void HelpCb(Widget, XtPointer, XtPointer);
void AboutCb(Widget, XtPointer, XtPointer);
void OnItemCb(Widget, XtPointer, XtPointer);
void NewCb(Widget, XtPointer, XtPointer);
void FSBCancelCb(Widget, XtPointer, XtPointer);
void OpenCb(Widget, XtPointer, XtPointer);
void OpenOkCb(Widget, XtPointer, XtPointer);
void ExitCb(Widget, XtPointer, XtPointer);
void ExposeCb(Widget, XtPointer, XtPointer);
void InputCb(Widget, XtPointer, XtPointer);
void SaveSessionCb(Widget, XtPointer, XtPointer);
void SaveAsCb(Widget, XtPointer, XtPointer);
void SaveAsCancelCb(Widget, XtPointer, XtPointer);
void SaveAsOkCb(Widget, XtPointer, XtPointer);
void DevicesCb(Widget, XtPointer, XtPointer);
void UpdateDevicesCb(Widget, XtPointer, XtPointer);
void OptionsCb(Widget, XtPointer, XtPointer);
void InfoCb(Widget, XtPointer, XtPointer);
void GenericCancelCb(Widget, XtPointer, XtPointer);
void OptionsSetCb(Widget, XtPointer, XtPointer);
void InfoDlogOkCb(Widget, XtPointer, XtPointer);
void OptionsDlogPaneCb(Widget, XtPointer, XtPointer);
void ProgScaleChangedCb(Widget, XtPointer, XtPointer);
void ProgScaleDragCb(Widget, XtPointer, XtPointer);
void OptionsChangedCb(Widget, XtPointer, XtPointer);
void OptionsCancelCb(Widget, XtPointer, XtPointer);
void OptionsResetCb(Widget, XtPointer, XtPointer);
void DndTransferCB(Widget, XtPointer, XtPointer);
void ShowWaveWindowCb(Widget ,XtPointer ,XtPointer );
void PlayCb(Widget, XtPointer, XtPointer);
void RecordCb(Widget, XtPointer, XtPointer);
void BlinkLightCb(XtPointer client_data, XtIntervalId *id);
void EnterButtonCb(Widget, XtPointer, XEvent *, Boolean *);
void LeaveButtonCb(Widget, XtPointer, XEvent *, Boolean *);
void ButtonDownWWCb(Widget, XtPointer, XEvent *, Boolean *);
void ButtonUpWWCb(Widget, XtPointer, XEvent *, Boolean *);
void ConfigNotifyWWCb(Widget, XtPointer, XEvent *, Boolean *);
void ChangeVolCb(Widget , XtPointer , XtPointer );
void WaveWindowRefreshCb(Widget, XtPointer, XtPointer);
void LevelMeterRefreshCb(Widget, XtPointer, XtPointer);
void EnterWaveWindowCb(Widget, XtPointer, XEvent *, Boolean *);
void LeaveWaveWindowCb(Widget, XtPointer, XEvent *, Boolean *);
void DrawLocationCb(Widget, XtPointer, XEvent *, Boolean *);
void MuteButtonCb(Widget, XtPointer, XtPointer);
void SetPositionCb(Widget, XtPointer, XtPointer);
void SetRecordValuesCb(Widget, XtPointer, XtPointer);

#endif /* CALLBACKS_H */
