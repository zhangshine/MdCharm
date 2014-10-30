function check_os()
{
    windows = (navigator.userAgent.indexOf("Windows", 0) != -1) ? 1 : 0;
    mac = (navigator.userAgent.indexOf("mac", 0) != -1) ? 1 : 0;
    linux = (navigator.userAgent.indexOf("Linux", 0) != -1) ? 1 : 0;
    unix = (navigator.userAgent.indexOf("Unix", 0) != -1) ? 1 : 0;
    if (windows)
    {
        document.getElementById("platform").innerHTML = 'Download MdCharm For Windows';
        document.getElementById("platform_version").innerHTML = 'Windows XP, Vista, and 7';
        //do noting
    } else {
        document.getElementById("platform").innerHTML = 'Download MdCharm For Linux';
        document.getElementById("platform_version").innerHTML = 'Ubuntu 14.04 64bit';
    }
}