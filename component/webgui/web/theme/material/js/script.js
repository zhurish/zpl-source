/**
 *  Material is a clean HTML5 theme for LuCI. It is based on luci-theme-bootstrap and MUI
 *
 *  luci-theme-material
 *      Copyright 2015 Lutty Yang <lutty@wcan.in>
 *
 *  Have a bug? Please create an issue here on GitHub!
 *      https://github.com/LuttyYang/luci-theme-material/issues
 *
 *  luci-theme-bootstrap:
 *      Copyright 2008 Steven Barth <steven@midlink.org>
 *      Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
 *      Copyright 2012 David Menting <david@nut-bolt.nl>
 *
 *  MUI:
 *      https://github.com/muicss/mui
 *
 *  Licensed to the public under the Apache License 2.0
 */
(function ($) {
    $(".main > .loading").fadeOut();

    /**
     * trim text, Remove spaces, wrap
     * @param text
     * @returns {string}
     */
    function trimText(text) {
        return text.replace(/[ \t\n\r]+/g, " ");
    }


    var lastNode_menu = undefined;
    var lastNode_slide = undefined;
    var lastNode_li = undefined;
    var lastNode_li_a = undefined;
    var lastNode_tab_li_a = undefined;
    /*
    function lastNode_init() {
    	lastNode_menu = $(".main > .main-left > .nav > .slide > .menu");
    	lastNode_slide = lastNode_menu.next(".slide-menu");  	
    	lastNode_li_a = $(".main > .main-left > .nav > .slide > .slide-menu > li > a");
    	lastNode_li = lastNode_li_a.parent();
    }   
    */
    /**
     * get the current node by Burl (primary)
     * @returns {boolean} success?
     */

    /**
     * menu click
     */
    $(".main > .main-left > .nav > .slide > .menu").click(function () {
        var ul = $(this).next(".slide-menu");
        var menu = $(this);
        if (!ul.is(":visible")) {
            if (lastNode_menu != undefined) 
            	lastNode_menu.removeClass("active");
            if (lastNode_slide != undefined)
            {
            	lastNode_slide.removeClass("active");
            	lastNode_slide.css("display", "none");
            }
        
            menu.addClass("active");
            ul.addClass("active");
            ul.stop(true).slideDown("fast");
            ul.css("display", "block");
        } else {
            ul.stop(true).slideUp("fast", function () {
                menu.removeClass("active");
                ul.removeClass("active");
                ul.css("display", "none");
                if (lastNode_menu != undefined) 
                	lastNode_menu.removeClass("active");
                if (lastNode_slide != undefined)
                {
                	lastNode_slide.removeClass("active");
                	lastNode_slide.css("display", "none");
                }
            });
        }
        lastNode_slide=ul;
        lastNode_menu=menu;
        return false;
    });

    /**
     * hook menu click and add the hash
     */
    $(".main > .main-left > .nav > .slide > .slide-menu > li > a").click(function () {
        if (lastNode_li_a != undefined) 
        	lastNode_li_a.removeClass("active");
        $(this).parent().addClass("active");
        $(this).addClass("active");
        $(".main-right > #maincontent > .loading").fadeIn("fast");
        lastNode_li_a = $(this);
        var ulr = $($(this)).attr("href");
        $("#maincontent").load(ulr + " #maincontent");
        return true;
    });

    /**
     * fix menu click
     */
    $(".main > .main-left > .nav > .slide > .slide-menu > li").click(function () {
        if (lastNode_li != undefined) 
        	lastNode_li.removeClass("active");
        lastNode_li = $(this);
        $(this).addClass("active");
        $(".main-right > #maincontent > .loading").fadeIn("fast");
        /*window.location = $($(this).find("a")[0]).attr("href");*/
        var ulr = $($(this).find("a")[0]).attr("href");
        $("#maincontent").load(ulr +" #maincontent");
       
        return false;
    });

    /**
     * get current node and open it
     */

    $(".cbi-button-up").val("");
    $(".cbi-button-down").val("");


    /**
     * hook menu click and add the hash
     */
    $("#maincontent > .container > .cbi-tabmenu > li > a").click(function () {
        if (lastNode_tab_li_a != undefined) 
        {
        	lastNode_tab_li_a.removeClass("cbi-tab");
        	lastNode_tab_li_a.addClass("cbi-tab-disabled");
        }
        $(this).addClass("cbi-tab-disabled");
        $(this).addClass("cbi-tab");
        lastNode_tab_li_a = $(this);
        var ulr = $($(this)).attr("href");
        $("#cbi-tabmenu-maincontent").load(ulr + " #cbi-tabmenu-maincontent");
        return true;
    });
    
    $("#maincontent > .container > .tabs > li > a").click(function () {
        if (lastNode_tab_li_a != undefined) 
        {
        	lastNode_tab_li_a.removeClass("active");
        }
        $(this).parent().addClass("active");
        lastNode_tab_li_a = $(this);
        var ulr = $($(this)).attr("href");
        $("#cbi-tabmenu-maincontent").load(ulr + " #cbi-tabmenu-maincontent");
        return true;
    });
    /**
     * hook other "A Label" and add hash to it.
     */
    $("#maincontent > .container > ").find("a").each(function () {
        var that = $(this);
        var onclick = that.attr("onclick");
        if (onclick == undefined || onclick == "") {
            that.click(function () {
                var href = that.attr("href");
                if (href.indexOf("#") == -1) {
                    $(".main-right > #maincontent > .loading").fadeIn("fast");
                    return /*true*/false;
                }
            });
        }
    });

    /**
     * Sidebar expand
     */
    var showSide = false;
    $(".showSide").click(function () {
        if (showSide) {
            $(".darkMask").stop(true).fadeOut("fast");
            $(".main-left").stop(true).animate({
                width: "0"
            }, "fast");
            $(".main-right").css("overflow", "auto");
            showSide = false;
        } else {
            $(".darkMask").stop(true).fadeIn("fast");
            $(".main-left").stop(true).animate({
                width: "15rem"
            }, "fast");
            $(".main-right").css("overflow", "hidden");
            showSide = true;
        }
    });


    $(".darkMask").click(function () {
        if (showSide) {
            showSide = false;
            $(".darkMask").stop(true).fadeOut("fast");
            $(".main-left").stop(true).animate({
                width: "0"
            }, "fast");
            $(".main-right").css("overflow", "auto");
        }
    });

    $(window).resize(function () {
        if ($(window).width() > 921) {
            $(".main-left").css("width", "");
            $(".darkMask").stop(true);
            $(".darkMask").css("display", "none");
            showSide = false;
        }
    });

    /**
     * fix legend position
     */
    $("legend").each(function () {
        var that = $(this);
        that.after("<span class='panel-title'>" + that.text() + "</span>");
    });

    $(".cbi-section-table-titles, .cbi-section-table-descr, .cbi-section-descr").each(function () {
        var that = $(this);
        if (that.text().trim() == ""){
            that.css("display", "none");
        }
    });


    $(".main-right").focus();
    $(".main-right").blur();
    $("input").attr("size", "0");

})(jQuery);
