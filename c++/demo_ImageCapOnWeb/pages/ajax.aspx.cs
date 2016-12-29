using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

public partial class _ajax : System.Web.UI.Page 
{
    protected void Page_Load(object sender, EventArgs e)
    {
            if (Request["picData"] != null)
            {
                string base64data = Request["picData"];
                string fileFormat = Request["picExt"];
                byte[] picBytes = Convert.FromBase64String(base64data.Substring(1,base64data.Length-2));
                System.IO.FileStream outputStream = new System.IO.FileStream(Server.MapPath("~/") + "//pages//ajax"+fileFormat, System.IO.FileMode.Create);
                outputStream.Write(picBytes, 0, picBytes.Length);
                outputStream.Close();
                Response.Write("{'savestatus':'ok'}");
                Response.End();
            }

    }
}
