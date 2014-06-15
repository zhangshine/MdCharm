function check_os()
{
    windows = (navigator.userAgent.indexOf("Windows", 0) != -1) ? 1 : 0;
    mac = (navigator.userAgent.indexOf("mac", 0) != -1) ? 1 : 0;
    linux = (navigator.userAgent.indexOf("Linux", 0) != -1) ? 1 : 0;
    unix = (navigator.userAgent.indexOf("Unix", 0) != -1) ? 1 : 0;
    if (windows)
    {
        document.getElementById("os_area").innerHTML =
            "<a href=\"/release/1.1.6/MdCharm-Setup.exe\" class=\"btn btn-primary btn-large\" onclick=\"_gaq.push(['_trackEvent', 'Download-Windows', 'Click', 'IndexPage']);\"><span class=\"large\">Download MdCharm For Windows</span><span class=\"small\">(V1.1.6)</span><br><span class=\"small\" > Free Trial</span><br><span class=\"small\">Windows XP, Vista, and 7</span></a>"
        //do noting
    }
//    } else if(mac)
//    {
//        document.getElementById("os_area").innerHTML =
//            "<a class=\"btn btn-primary btn-large\"><span class=\"large\">Download MdCharm For Windows</span><span class=\"small\">(V0.1.0)</span><br><span class=\"small\">Windows XP, Vista, and 7</span></a>"
//    }
    else {
        document.getElementById("os_area").innerHTML =
            "<a href=\"/release/1.1.6/mdcharm_1.1.6_i386.deb\" class=\"btn btn-primary btn-large\" onclick='_gaq.push(['_trackEvent', 'Download-Ubuntu', 'Click', 'IndexPage']);'><span class=\"large\">Download MdCharm For Ubuntu</span><span class=\"small\">(V1.1.6)Free</span><br><span class=\"small\">Ubuntu 12.04</span></a>"
    }
}