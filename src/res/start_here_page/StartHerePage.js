$(document).ready(function()
    {
        try
        {
            browerWebkitHandler.updateRecentFileList.connect(updateRecentFileList);
            browerWebkitHandler.domReady();
        }
        catch(e)
        {
            alert(e);
        }
    }
    );
   
function updateRecentFileList(str)//file path seprate with '|'
{
    htmlStr = '';
    strList = str.split('|');
    var i=0;
    for(; i<strList.length; i++)
    {
        s = strList[i];
        htmlStr += '<a title="click to open this file" onclick="browerWebkitHandler.openRecentFile(\''+s.replace(/\\/g,'\\\\')+'\')">'+s+'</a><br/>';
    }
    document.getElementById("recentFiles").innerHTML = htmlStr;
}