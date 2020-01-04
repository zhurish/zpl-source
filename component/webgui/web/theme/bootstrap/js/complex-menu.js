// JavaScript Document
    $(function(){
		
		var lastNode = undefined;
		var lastMenu = undefined;
		
		$(".nav > .topslide").each(function () {
			var topNode = $(this);
			if(topNode.hasClass("topactive")) {
				lastNode = topNode;
			}
		});
		
		$(".nav > .slide > .slide-menu > li").each(function () {
			var liNode = $(this);
			if(liNode.hasClass("active")) {
				lastNode = liNode;
			}
		});
		
		$(".nav > .slide > .menu").each(function () {
			var mNode = $(this);
			if(mNode.hasClass("active")) {
				lastMenu = mNode;
			}
		});
		/* 一级菜单 */
		$(".nav > .slide > .menu").click(function () {
        	var ul = $(this).next(".slide-menu");
        	var menu = $(this);
        	if (!ul.is(":visible")) {
				if (lastMenu != undefined){
					var oul = lastMenu.next(".slide-menu");
					lastMenu.removeClass("active");
					oul.css("display", "none");
                	oul.removeClass("active");
				}
				lastMenu = menu;
				
         menu.addClass("active");
				ul.css("display", "block");
            	ul.addClass("active");
            	ul.stop(true).slideDown("fast");
        	} else {
            	ul.stop(true).slideUp("fast", function () {
                	menu.removeClass("active");
					ul.css("display", "none");
                	ul.removeClass("active");
            	});
        	}
    	});
		
    	/*
     	*  二级菜单 
     	*/
		$(".nav > .slide > .slide-menu > li").click(function () {
			//console.log($(this));
        	if (lastNode != undefined) {
				if(lastNode.hasClass("active"))
					lastNode.removeClass("active");
				if(lastNode.hasClass("topactive"))
					lastNode.removeClass("topactive");
			}
        	$(this).addClass("active");
			lastNode = $(this);
    	});
		
		/* 一级菜单 */
		$(".nav > .topslide > a").click(function () {
        	if (lastNode != undefined) {
				if(lastNode.hasClass("active"))
					lastNode.removeClass("active");
				if(lastNode.hasClass("topactive"))
					lastNode.removeClass("topactive");
			}
			if (lastMenu != undefined){
				var oul = lastMenu.next(".slide-menu");
				lastMenu.removeClass("active");
				oul.css("display", "none");
                oul.removeClass("active");
				lastMenu = undefined;
			}			
        	$(this).addClass("topactive");
			lastNode = $(this);
    	});					
		/*
		$(".nav > .slide > .menu > .slide-menu > li").click(function () {
			console.log($(this));
        	if (lastNode != undefined) 
				lastNode.removeClass("active");
        	$(this).addClass("active");
        	return true;
    	});	
		
		$(".nav > .slide > .menu > li").click(function () {
			console.log($(this));
        	if (lastNode != undefined) 
				lastNode.removeClass("active");
        	$(this).addClass("active");
        	return true;
    	});	
		
		$(".nav > .slide > .menu").mouseenter(function () {
        	var ul = $(this).next(".slide-menu");
        	var menu = $(this);
        	if (!ul.is(":visible")) {
            	menu.addClass("active");
				ul.css("display", "block");
            	ul.addClass("active");
            	ul.stop(true).slideDown("fast");
        	} 
    	});
		
		$(".nav > .slide > .menu").mouseleave(function () {
        	var ul = $(this).next(".slide-menu");
        	var menu = $(this);
        	if (ul.is(":visible")) {
            	ul.stop(true).slideUp("fast", function () {
                	menu.removeClass("active");
					ul.css("display", "none");
                	ul.removeClass("active");
            	});
        	}
    	});	
		*/		
    })
	