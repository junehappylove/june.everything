using System;
using System.Collections.Generic;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

public partial class _submit : System.Web.UI.Page 
{
    protected void Page_Load(object sender, EventArgs e)
    {
           if (Request["picData"] != null)
            {
                string base64data = Request["picData"];
                string ext = Request["picExt"];
                byte[] picBytes = Convert.FromBase64String(base64data);
                System.IO.FileStream outputStream = new System.IO.FileStream(Server.MapPath("~/pages/") + "ajax"+ext, System.IO.FileMode.Create);
                outputStream.Write(picBytes, 0, picBytes.Length);
                outputStream.Close();
                Response.Write("<a href='ajax" + ext + "'>click here</a>");
                Response.End();
            }
    }
}
