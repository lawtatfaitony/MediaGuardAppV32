using LanguageResource;
using System;

namespace DevSynchronize
{
    public class ApiUrlList
    { /// <summary>
      /// Http Picture Upload Request TimeOut = 30 Seconds
      /// </summary>
        public static TimeSpan HttpUploadPictueRequestTimeout { get; set; } = TimeSpan.FromSeconds(30);

        /// <summary>
        /// Http Request TimeOut = 30 Seconds
        /// </summary>
        public static TimeSpan HttpRequestTimeout { get; set; } = TimeSpan.FromSeconds(20);

        public static string GetTokenUrl { get; set; } = "/Authentication/RequestToken";
        public static string GetListBySysModuleIdUrl { get; set; } = string.Format("/{0}/Admin/DeviceManage/GetListBySysModuleId", LangUtilities.LanguageCode); //GetListBySysModuleId
        public static string GetDeviceDetailsUrl { get; set; } = string.Format("/{0}/Admin/DeviceManage/DeviceDetails", LangUtilities.LanguageCode); //GET DEVICE DETAILS BY DEVICE ID
        
        public static string GetDeviceUserListUrl { get; set; } = string.Format("/{0}/Admin/DeviceManage/DeviceUserList", LangUtilities.LanguageCode); //GET DEVICE USERs 
        public static string GetDeviceUserUpdUserProfileIdUrl { get; set; } = string.Format("/{0}/Admin/DeviceManage/DeviceUserUpdUserProfileId", LangUtilities.LanguageCode); //update UserUpdUserProfileId
        public static string GetDeviceUserUpdUserProfileIdClearUrl { get; set; } = string.Format("/{0}/Admin/DeviceManage/DeviceUserUpdUserProfileIdClear", LangUtilities.LanguageCode); //GET DEVICE USERs 
        public static string GetSynchronizeSetFaceDownListUrl { get; set; } = string.Format("/{0}/Admin/DeviceManage/SynchronizeSetFaceDownList", LangUtilities.LanguageCode);

        public static string GetSynchronizeSetCardDownListUrl { get; set; } = string.Format("/{0}/Admin/DeviceManage/SynchronizeSetCardDownList", LangUtilities.LanguageCode);

        public static string GetSetFaceCallBackSuccUrl { get; set; } = string.Format("/{0}/Admin/DeviceManage/SetFaceCallBackSucc", LangUtilities.LanguageCode);

        public static string GetQueryDeviceUserListUrl { get; set; } = string.Format("/{0}/Admin/DeviceManage/QueryDeviceUserList", LangUtilities.LanguageCode);

        //[上行创建用户接口] 暂停使用, 目前仅仅提供单向 下行.
        public static string GetDeviceUserUptoCtreateUrl { get; set; } = string.Format("/{0}/Admin/DeviceManage/DeviceUserUptoCtreate", LangUtilities.LanguageCode);

        //通过自定义编号 获得card的物理编号(十进制)
        public static string GetCardIdDetailsUrl { get; set; } = string.Format("/{0}/Admin/DeviceManage/GetCardIdDetails", LangUtilities.LanguageCode);

        public static string GetSetCardCallBackSuccUrl { get; set; } = string.Format("/{0}/Admin/DeviceManage/SetCardCallBackSucc", LangUtilities.LanguageCode);

        //考勤数据log入口
        public static string GetAttendancePostUrl { get; set; } = string.Format("/{0}/Admin/DeviceManage/AttendancePost", LangUtilities.LanguageCode);

        //GetMainComBySerialNo 通過序列號取得設備信息
        public static string GetMainComBySerialNoUrl { get; set; } = string.Format("/{0}/Device/GetMainComBySerialNo", LangUtilities.LanguageCode);
         //UploadEntriesTemp 上存圖片
        public static string GetUploadEntriesTempPostUrl { get; set; } = string.Format("/{0}/UpFile/UploadEntriesTemp", LangUtilities.LanguageCode);


        //同步模块-------------------------------------------------------------------------------------------------------------------------------------------------------

        //获取 設備配置Config  http://81.71.74.135:5002/zh-HK/Admin/DeviceManage/GetDeviceConfig
        public static string GetDeviceConfigApi { get; set; } = string.Format("/{0}/Admin/DeviceManage/GetDeviceConfig", LangUtilities.LanguageCode);

        //獲取設備用戶的[邏輯型]列表
        public static string GetQueryDeviceUserListApi { get; set; } = $"/{LangUtilities.LanguageCode}/Admin/DeviceManage/QueryDeviceUserList";
 
        //設備用戶相關操作的狀態回發 (回發成功後的狀態結果)
        public static string GetTerminalEquipmentCallBackApi { get; set; } = $"/{LangUtilities.LanguageCode}/Admin/DeviceManage/TerminalEquipmentCallBack";

        //獲取人員資料詳情 返回 PersonDetails
        public static string GetPersonDetailsApi { get; set; } = $"/{LangUtilities.LanguageCode}/Person/GetPersonByEmployeeNoOrId"; //PersonDetails

        //新增人員到雲端 POST JSON
        public static string AddPersonFromDeviceApi { get; set; } = $"/{LangUtilities.LanguageCode}/Person/AddPersonFromDevice";


        public static string AddStandardPersonApi { get; set; } = $"/{LangUtilities.LanguageCode}/Admin/DeviceManage/AddStandardPerson";
        public static string SaveEmployeePassKeyRecordApi { get; set; } = $"/{LangUtilities.LanguageCode}/Admin/DeviceManage/SaveEmployeePassKeyRecord";
        public static string SaveEmployeeCardNoRecordApi { get; set; } = $"/{LangUtilities.LanguageCode}/Admin/DeviceManage/SaveEmployeeCardNoRecord";
        public static string SaveEmployeePictRecordApi { get; set; } = $"/{LangUtilities.LanguageCode} /Admin/DeviceManage/SaveEmployeePictRecord";
        public static string GetUploadPersonFaceFileUrl { get; set; } = string.Format("/{0}/Utility/Uploadprocess?Prefix=P&SubPath=Person", LangUtilities.LanguageCode);

        public static string GetMainComGroupUrl { get; set; } = $"/{LangUtilities.LanguageCode}/Admin/DeviceManage/GetMainComGroup";
    }
}
