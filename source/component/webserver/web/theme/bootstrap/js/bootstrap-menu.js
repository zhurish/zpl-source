// JavaScript Document
/*
function load_main_page(url, id) {
	if(url == "logout" || url == "action/logout" || 
		url == "form/logout" || url == "action/logout"){
		return false;
	}
	//把 url文件中 id="maincontent" 的元素的内容，加载到指定的 id="main_content" 元素中
	$("#main_content").load(url + ".html" + id, function (response, status) {
		if (status == "success") {
			if ($(id).hasClass("container"))
				$(id).removeClass("container");
		}
		return true;
	});
	return false;
}
*/
function load_main_page(url, id) {
	if(url == "logout" || url == "action/logout" || 
		url == "form/logout" || url == "action/logout"){
		return false;
	}
	var aa = url + ".html" + id;
	console.log(aa);
	document.getElementById("main_content").src = url + ".html";
	return true;
	/*
	$("#main_content").load(url + ".html" + id, function (response, status) {
		if (status == "success") {
			if ($(id).hasClass("container"))
				$(id).removeClass("container");
		}
		return true;
	});
	return false;
	*/
}

$(function () {
	var lastNode = undefined;
	var lastMenu = undefined;

	$(".dropdown-menu>li").each(function () {
		var liNode = $(this);
		if (liNode.hasClass("active")) {
			lastNode = liNode;
		}
	});

	$(".nav > li").each(function () {
		var mNode = $(this);
		if (mNode.hasClass("active")) {
			lastMenu = mNode;
		}
	});

	/*
	 *  二级菜单 
	 */
	$(".dropdown-menu>li").click(function () {
		var menuhrefvalue = undefined;
		if (lastNode != undefined) {
			if (lastNode.hasClass("active"))
				lastNode.removeClass("active");
			//if (lastNode.hasClass("topactive"))
			//	lastNode.removeClass("topactive");
		}
		$(this).addClass("active");
		lastNode = $(this);

		menuhrefvalue = lastNode.children().attr("href");
		//console.log("lastNode:" + hrefvalue.slice(1));
		if (menuhrefvalue != undefined && menuhrefvalue != null && menuhrefvalue != "")
		{
			console.log("lastNode:" + menuhrefvalue.slice(1));
			load_main_page(menuhrefvalue.slice(1), "#maincontent");
		}
		//else
		//	load_main_page("about", " #maincontent");
	});

	/* 一级菜单 */
	$(".nav > li").click(function () {
		var hrefvalue = undefined;
		if (lastMenu != undefined) {
			if (lastMenu.hasClass("active"))
				lastMenu.removeClass("active");
		}
		$(this).addClass("active");
		lastMenu = $(this);

		hrefvalue = lastMenu.children().attr("href");
		
		if (hrefvalue != undefined && hrefvalue != null && hrefvalue != "")
		{
			console.log("lastMenu:" + hrefvalue.slice(1));
			load_main_page(hrefvalue.slice(1), "#maincontent");
		}
		//else
		//	load_main_page("about", " #maincontent");
	});
	
	$(".nav > li").mouseout(function () {
		if($(this).hasClass("open"))
			$(this).removeClass("open");
	});
	$(".nav > li").mouseover(function () {
		$(this).addClass("open");
	});
})