# 考勤人臉拍卡模式參考AttendanceMode



**接口流程:**
以POST的數據舉例子:

```
PostData =
{
    "MainComId": "6000014",
    "DeviceId": " 3003",
    "DeviceSerialNo": "SERIAL00000001231ss333",
    "EmployeeNo": "8",
    "AttendanceMode": -1,
    "Occur": 1628783438292,
    "PhysicalId": "3991852973",
    "Face": "1628783438.bmp",
    "FingerPrint": ""
}
```

由於設備是基於以人為本,所以,首要是POST EmployeeId 即係工號, 
考勤模式 AttendanceMode 來自程序預設的常量文檔, 例如定制海康拍卡設備 
則 HIK_CARD=1這個常量.
**數據結構體 PostData 賦值**
第一步: EmployeeId (必須的複製) , Occur 發生時間,例如HIKSDK返回時間提供的時間
第二步: PhysicalId  拍卡號(十進制) [Optional可選的]  來源硬件的Hex交叉翻譯為10十進制,
		具體視乎什麼廠家而定.
		例如: 海康廠家是向右交叉編譯,具體查一下網上的.
第四步: Face 人臉識別 (圖片名稱”1628783438292.jpg”先傳入) [Optional可選的] 
		當POST成功後,則上存圖片文件. 
第五步: FingerPrint 指紋 如果事件返回對象存在指紋數據,則補充該值, 填入哈希值,
		當POST成功後,則上存哈希加密的指紋文件. (指紋目前不確定數據的情況,當前是假設流程)



**參考來源的代碼文件 EnumCode\EnumBusiness.cs**

## 參考的常量值

```
/// <summary>
/// 複製自 [考勤與薪酬系統 DGX] 參考用途 切勿引用
/// 考勤與薪酬系統 (DGX) 對應拍卡模式
/// </summary>
public enum AttendanceModeDGX
{
    [EnumDisplayName("AttendanceMode_FACE_RECOGNITION")]
    FACE = -1,
    [EnumDisplayName("AttendanceMode_CAM_GUARD")]
    CAM_GUARD = 0,
    [EnumDisplayName("AttendanceMode_HIK_CARD")]
    HIK_CARD = 1,
    [EnumDisplayName("AttendanceMode_STANDARD_CARD")]
    STANDARD_CARD = 2,
    [EnumDisplayName("AttendanceMode_STANDARD_CARD_AND_FINGERPRINT")]
    STANDARD_CARD_AND_FINGERPRINT = 3,
    [EnumDisplayName("AttendanceMode_CIC_CARD")]
    CIC_CARD = 4,
    [EnumDisplayName("AttendanceMode_BAIDU_MAP")]
    BAIDU = 5,
    [EnumDisplayName("AttendanceMode_GOOGLE_MAP")]
    GOOGLE = 6,
    [EnumDisplayName("AttendanceMode_QQ_MAP")]
    QQ = 7,
    [EnumDisplayName("AttendanceMode_GPS_MAP")]
    GPS = 8,
    [EnumDisplayName("AttendanceMode_FINGERPRINT")]
    FINGERPRINT = 9,
    [EnumDisplayName("AttendanceMode_PASSWORD")]
    PASSWORD = 10,
    [EnumDisplayName("AttendanceMode_COMBINE_VERIFY")]
    COMBINE_VERIFY = 11,
    [EnumDisplayName("AttendanceMode_CARD_FINGERPRINT_PASSWD")]
    CARD_FINGERPRINT_PASSWD = 12,
    [EnumDisplayName("AttendanceMode_FACE_CARD_VERIFY")]
    FACE_CARD_VERIFY = 13,
    //手动调整 MANUAL ADJUSTMENT  FROM [AttRegularMode]
    [EnumDisplayName("AttRegularMode_WORK_ON")]
    WORK_ON = 994,
    [EnumDisplayName("AttRegularMode_WORK_OFF")]
    WORK_OFF = 995,
    [EnumDisplayName("AttRegularMode_LUNCH_START")]
    LUNCH_START = 996,
    [EnumDisplayName("AttRegularMode_LUNCH_END")]
    LUNCH_END = 997,
    [EnumDisplayName("AttRegularMode_OVERTIME_START")]
    OVERTIME_START = 998,
    [EnumDisplayName("AttRegularMode_OVERTIME_END")]
    OVERTIME_END = 999
}

/// <summary>
/// 設備類型 主要是指設備的 類型 其他實體設備直接以品牌大寫+設備型號大寫為設備名 例如 HIK_DS_KIT341BMW
/// 部分設備的類型引用AttendanceMode
/// 部分是虛擬設備 例如QQ MAP GOOGLE MAP 屬於GPS的虛擬設備
/// </summary>
public enum DeviceType
{
    /// <summary>
    /// 無定義的 (設備) DEVICE_UNDEFINED_DEVICE
    /// </summary>
    [EnumDisplayName("DEVICE_UNDEFINED_DEVICE")]
    UNDEFINED_DEVICE = 1000,
    /// <summary>
    /// Android 拍卡 (設備)
    /// </summary>
    [EnumDisplayName("DEVICE_ANDROID_NFC")]
    ANDROID_NFC = 1001,

    [EnumDisplayName("AttendanceMode_DAHUA_CARD_FACE")] //20001為大華設備類型 如何拍卡人臉 DAHUA_CARD_FACE = 1001
    DAHUA_CARD_FACE = 20001,
    /// <summary>
    /// 海康 HIK_DS_KIT341BMW人臉/拍卡 設備類型
    /// </summary>
    [EnumDisplayName("DEVICE_HIK_DS_DEVICE")]
    HIK_DEVICE = 10022,

    /// <summary>
    /// 海康 HIK_DS_KIT341BMW人臉/拍卡 設備類型
    /// </summary>
    [EnumDisplayName("DEVICE_HIK_DS_KIT341BMW")]
    HIK_DS_KIT341BMW = 1002,
    /// <summary>
    /// 海康 HIK_DS_KIT804MF 設備類型
    /// </summary>
    [EnumDisplayName("DEVICE_HIK_DS_KIT804MF")]
    HIK_DS_KIT804MF = 1003,

    /// <summary>
    /// NFC CIC協會的CHECK SITE模式
    /// </summary>
    [EnumDisplayName("DEVICE_TYPE_NFCCIC_CHK")]
    NFCCIC_CHECK = 1004,

    /// <summary>
    /// NFC CIC協會的 拍卡模式 mode=9
    /// </summary>
    [EnumDisplayName("DEVICE_TYPE_NFCCIC_TAP")]
    NFCCIC_TAP = 1005,

    /// <summary>
    /// DESTOP DVR 數字錄像設備/数字录像设备 基於電腦的
    /// </summary>
    [EnumDisplayName("DEVICE_TYPE_DESTOP_DVR")]
    DESTOP_DVR = 1006,

    [EnumDisplayName("AttendanceMode_FACE_RECOGNITION")]
    FACE = -1,
    [EnumDisplayName("AttendanceMode_CAM_GUARD")]
    CAM_GUARD = 0,
    [EnumDisplayName("AttendanceMode_HIK_CARD")]
    HIK_CARD = 1,
    [EnumDisplayName("AttendanceMode_STANDARD_CARD")]
    STANDARD_CARD = 2,
    [EnumDisplayName("AttendanceMode_STANDARD_CARD_AND_FINGERPRINT")]
    STANDARD_CARD_AND_FINGERPRINT = 3,
    [EnumDisplayName("AttendanceMode_CIC_CARD")]
    CIC_CARD = 4,
    [EnumDisplayName("AttendanceMode_CIC_CARD_CHECK")]
    CIC_CARD_CHECK = 44,
    [EnumDisplayName("AttendanceMode_BAIDU_MAP")]
    BAIDU = 5,
    [EnumDisplayName("AttendanceMode_GOOGLE_MAP")]
    GOOGLE = 6,
    [EnumDisplayName("AttendanceMode_QQ_MAP")]
    QQ = 7,
    [EnumDisplayName("AttendanceMode_APPLE_MAP")]
    APPLE_MAP = 77,
    [EnumDisplayName("AttendanceMode_GPS_MAP")]
    GPS = 8,
    [EnumDisplayName("AttendanceMode_FINGERPRINT")]
    FINGERPRINT = 9,
    [EnumDisplayName("AttendanceMode_PASSWORD")]
    PASSWORD = 10,
    [EnumDisplayName("AttendanceMode_COMBINE_VERIFY")]
    COMBINE_VERIFY = 11,
    [EnumDisplayName("AttendanceMode_CARD_FINGERPRINT_PASSWD")]
    CARD_FINGERPRINT_PASSWD = 12,
    [EnumDisplayName("AttendanceMode_FACE_CARD_VERIFY")]
    FACE_CARD_VERIFY = 13
}

/// <summary>
/// 和DataGuardXcore系統一樣 主導表[HistRecognizeRecord].[Mode]的模式的值
///  ATTENDANCE MODE FOR ATTENDANCE_LOG
/// </summary>
public enum AttendanceMode
{
    [EnumDisplayName("AttendanceMode_FACE_RECOGNITION")]
    FACE = -1,
    [EnumDisplayName("AttendanceMode_CAM_GUARD")]
    CAM_GUARD = 0,
    [EnumDisplayName("AttendanceMode_DAHUA_CARD_FACE")] //20001為大華設備類型 如何拍卡人臉 DAHUA_CARD_FACE = 20001
    DAHUA_CARD_FACE = 20001,
    [EnumDisplayName("AttendanceMode_HIK_CARD")]
    HIK_CARD = 1,
    [EnumDisplayName("AttendanceMode_STANDARD_CARD")]
    STANDARD_CARD = 2,
    [EnumDisplayName("AttendanceMode_STANDARD_CARD_AND_FINGERPRINT")]
    STANDARD_CARD_AND_FINGERPRINT = 3,
    [EnumDisplayName("AttendanceMode_CIC_CARD")]
    CIC_CARD = 4,
    [EnumDisplayName("AttendanceMode_BAIDU_MAP")]
    BAIDU = 5,
    [EnumDisplayName("AttendanceMode_GOOGLE_MAP")]
    GOOGLE = 6,
    [EnumDisplayName("AttendanceMode_QQ_MAP")]
    QQ = 7,
    [EnumDisplayName("AttendanceMode_GPS_MAP")]
    GPS = 8,
    [EnumDisplayName("AttendanceMode_FINGERPRINT")]
    FINGERPRINT = 9,
    [EnumDisplayName("AttendanceMode_PASSWORD")]
    PASSWORD = 10,
    [EnumDisplayName("AttendanceMode_COMBINE_VERIFY")]
    COMBINE_VERIFY = 11,
    [EnumDisplayName("AttendanceMode_CARD_FINGERPRINT_PASSWD")]
    CARD_FINGERPRINT_PASSWD = 12,
    [EnumDisplayName("AttendanceMode_FACE_CARD_VERIFY")]
    FACE_CARD_VERIFY = 13,
    //手动调整 MANUAL ADJUSTMENT  FROM [AttRegularMode]
    [EnumDisplayName("AttRegularMode_WORK_ON")]
    WORK_ON = 994,
    [EnumDisplayName("AttRegularMode_WORK_OFF")]
    WORK_OFF = 995,
    [EnumDisplayName("AttRegularMode_LUNCH_START")]
    LUNCH_START = 996,
    [EnumDisplayName("AttRegularMode_LUNCH_END")]
    LUNCH_END = 997,
    [EnumDisplayName("AttRegularMode_OVERTIME_START")]
    OVERTIME_START = 998,
    [EnumDisplayName("AttRegularMode_OVERTIME_END")]
    OVERTIME_END = 999
}
```

