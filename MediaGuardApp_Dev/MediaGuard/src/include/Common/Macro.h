/**************************************************************
* @brief:       macros of project used 
* @copyright:
*
***************************************************************/
#pragma once



/**************************************************************
*                           json macro                        *
***************************************************************/
#define JS_PARSE_OBJECT_VOID(json, para)  \
    {   \
        json.Parse(para.c_str());   \
        if(json.HasParseError() || !json.IsObject())    \
        {   \
            return; \
        }   \
    }

#define JS_PARSE_OBJECT_RETURN(json, para, ret)  \
    {   \
        json.Parse(para.c_str());   \
        if(json.HasParseError() || !json.IsObject())    \
        {   \
            return ret;\
        }   \
    }

#define JS_CHECK_FIELD_RETURN(json, field, type, ret) \
    {   \
        if(!json.HasMember(#field) || !json[#field].Is##type()) \
            return ret;\
    }

#define JS_CHECK_TYPE_VOID(type) \
    {   \
        if(!json.Is##type()) \
            return; \
    }

#define JS_CHECK_TYPE_RETURN(type, ret) \
    {   \
        if(!json.Is##type()) \
            return ret;\
    }

#define JS_PARSE_REQUIRED_RETURN(to, json, type, field, result,  ret) \
    {   \
        if(json.HasMember(#field) && json[#field].Is##type())  \
        {   \
            to = json[#field].Get##type();  \
        }   \
        else \
        {   \
            result = "Invalid para: "#field"" ;    \
            return ret;    \
        }   \
    }

#define JS_PARSE_REQUIRED_VOID(to, json, type, field, result) \
    {   \
        if(json.HasMember(#field) && json[#field].Is##type())  \
        {   \
            to = json[#field].Get##type();  \
        }   \
        else \
        {   \
            result = "Invalid para: #field" ;    \
            return;    \
        }   \
    }

#define JS_PARSE_OPTION(to, json, type, field) \
    {   \
        if(json.HasMember(#field) && json[#field].Is##type())  \
        {   \
            to = json[#field].Get##type();  \
        }   \
    }

#define JS_PARSE_OPTION_VOID(to, json, type, field) \
    {   \
        if(json.HasMember(#field) && json[#field].Is##type())  \
        {   \
            to = json[#field].Get##type();  \
        }   \
        else \
        {   \
            return;    \
        }   \
    }

#define JS_PARSE_OPTION_RETURN(to, json, type, field, ret) \
    {   \
        if(json.HasMember(#field) && json[#field].Is##type())  \
        {   \
            to = json[#field].Get##type();  \
        }   \
        else \
        {   \
            return ret;    \
        }   \
    }

#define JS_PARSE_OPTION_DEFAULT(to, json, type, field, def) \
    {   \
        if(json.HasMember(#field) && json[#field].Is##type())  \
        {   \
            to = json[#field].Get##type();  \
        }   \
        else \
        {   \
            to = def;   \
        }   \
    }

#define JS_PARSE_OPTION_UPDATE_INT(to, json, type, field, ss) \
    {   \
        if(json.HasMember(#field) && json[#field].Is##type())  \
        {   \
            to = json[#field].Get##type();  \
            ss << ",`"#field"` = " << to;     \
        }   \
    }

#define JS_PARSE_OPTION_UPDATE_STRING(to, json, type, field, ss) \
    {   \
        if(json.HasMember(#field) && json[#field].Is##type())  \
        {   \
            to = json[#field].Get##type();  \
            ss << ","#field" = '" << to << "'";     \
        }   \
    }

#define JS_PARSE_NUMBER_REQUIRED(to, json, type, field, ret) \
    {   \
        if(json.HasMember(#field) && json[#field].IsNumber())  \
        {   \
            to = json[#field].Get##type();  \
        }   \
        else \
        {   \
            return ret;    \
        }   \
    }

#define JS_PARSE_NUMBER_OPTION(to, json, type, field) \
    {   \
        if(json.HasMember(#field) && json[#field].IsNumber())  \
        {   \
            to = json[#field].Get##type();  \
        }   \
    }

/**************************************************************
*                           common macro                      *
***************************************************************/
#define VERIFY_EXPR_RETURN_VOID(expr) \
    if(!expr) \
    {   \
        return; \
    }

#define VERIFY_EXPR_RETURN(expr, code) \
    do{   \
        bool success = expr;    \
        if (!success) \
        {   \
            return code; \
        }   \
    }while(false);

#define VERIFY_CODE_RETURN(func) \
    do{   \
        int rst = func;    \
        if (rst != 0) \
        {   \
            return rst; \
        }   \
    }while(false);

#define VERIFY_CODE_RETURN_VOID(func) \
    do{   \
        int rst = func;    \
        if (rst != 0) \
        {   \
            return; \
        }   \
    }while(false);

#define VERIFY_RETURN_OPTION(func, ret) \
    do{   \
        int rst = func;    \
        if (rst != 0) \
        {   \
            return ret; \
        }   \
    }while(false);




/**************************************************************
*                           exception macro                   *
***************************************************************/
#define CATCH_EXCEPTION_BEGIN \
    try{

#define CATCH_EXCEPTION_END(identity) \
    } \
    catch(const std::exception& e) \
    { \
        LOG(ERROR) << "#identity, call exception: " << e.what(); \
    }


#define SCOPE_LOCK(lock) \
    std::lock_guard<std::mutex> locker(lock)

// database
#define CHANGE_DATABASE(conn, base) \
    conn<<"use "<< base

#define DB_BEGIN(conn) \
    conn.begin()

#define DB_COMMIT(conn) \
    conn.commit()

#define DB_ROLLBACK(conn) \
    conn.rollback()