// JavaScript Document

function Int(x) {
	return (/^-?\d+$/.test(x) ? +x : NaN);
}

function Dec(x) {
	return (/^-?\d+(?:\.\d+)?$/.test(x) ? +x : NaN);
}

function is_integer(s) {
	return !!Int(s);
}

function is_uninteger(s) {
	return (Int(s) >= 0);
}

function is_float(s) {
	return !!Dec(s);
}

function is_unfloat(s) {
	return (Dec(s) >= 0);
}


//校验是否全由数字组成
function is_alldigit(s) {
	var patrn = /^\d+$/;
	if (!patrn.exec(s))
		return false
	return true
}

function is_allstring(s) {
	var patrn = /^[a-zA-Z]+$/;
	if (!patrn.test(s)) return false
	return true
}

//校验密码强度
function checkpassword(value) {
	/*
	// 0： 表示第一个级别 1：表示第二个级别 2：表示第三个级别
	// 3： 表示第四个级别 4：表示第五个级别
	var modes = 0;
	if(/\d+$/.test(value)){//如果用户输入的密码 包含了数字
		modes++;
	}
	if(/^[a-zA-Z]+$/.test(value)){//如果用户输入的密码 包含了小写的a到z
		modes++;
	}
	if(/\W+$/.test(value)){//如果是非数字 字母 下划线
		modes++;
	}
	if(modes == 3)
		return true;
	else
		return false;
	*/
	//字母+数字+特殊字符
	var reg = /^(?![a-zA-z]+$)(?!\d+$)(?![!@#$%^&*]+$)(?![a-zA-z\d]+$)(?![a-zA-z!@#$%^&*]+$)(?![\d!@#$%^&*]+$)[a-zA-Z\d!@#$%^&*]+$/;
	return reg.test(value);
}

function checkusername(s) {
	//字母+数字，字母+特殊字符，数字+特殊字符
	//var reg=/^(?![a-zA-z]+$)(?!\d+$)(?![!@#$%^&*]+$)[a-zA-Z\d!@#$%^&*]+$/;
	//字母+数字+特殊字
	var reg = /^(?![a-zA-z]+$)(?!\d+$)(?![!@#$%^&*]+$)(?![a-zA-z\d]+$)(?![a-zA-z!@#$%^&*]+$)(?![\d!@#$%^&*]+$)[a-zA-Z\d!@#$%^&*]+$/;
	return reg.test(s);
}


/*校验电话码格式 */
function is_telphone(str) {
	var reg = /^((0\d{2,3}-\d{7,8})|(1[3584]\d{9}))$/;
	return reg.test(str);
}

/*校验邮件地址是否合法 */
function is_email(str) {
	var reg = /^\w+@[a-zA-Z0-9]{2,10}(?:\.[a-z]{2,4}){1,3}$/;
	return reg.test(str);
}

function is_mac(s) {
	var reg = /^([a-fA-F0-9]{2}:){5}[a-fA-F0-9]{2}$/;
	return reg.test(s);
	//return (s.match(/^([a-fA-F0-9]{2}:){5}[a-fA-F0-9]{2}$/) != null);
}

function hostname(s) {
	if (s.length <= 253)
		return (s.match(/^[a-zA-Z0-9]+$/) != null ||
			(s.match(/^[a-zA-Z0-9_][a-zA-Z0-9_\-.]*[a-zA-Z0-9]$/) &&
				s.match(/[^0-9.]/)));

	return false;
}


function checkipv4(ip) {
	if (ip.match(/^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})(\/(\S+))?$/)) {
		return (RegExp.$1 > 0) && (RegExp.$1 < 255) &&
			(RegExp.$2 >= 0) && (RegExp.$2 <= 255) &&
			(RegExp.$3 >= 0) && (RegExp.$3 <= 255) &&
			(RegExp.$4 > 0) && (RegExp.$4 < 255)
	}
	return false;
}

function checknetmaskv4(ip) {
	//var ip=document.getElementById(obj).value;
	var ipRegExp = /^(254|252|248|240|224|192|128|0)\.0\.0\.0|255\.(254|252|248|240|224|192|128|0)\.0\.0|255\.255\.(254|252|248|240|224|192|128|0)\.0|255\.255\.255\.(254|252|248|240|224|192|128|0)$/;
	if (!ip.match(ipRegExp)) {
		return false;
	}
	return true;
}

/*
function check_ok_show(inobj)
{
	var chkok = inobj.parent().next();
	var chkerr = inobj.parent().next().next();
	chkok.css("display", "block");
	chkerr.css("display", "none");
}

function check_err_show(inobj)
{
	var chkok = inobj.parent().next();
	var chkerr = inobj.parent().next().next();
	chkok.css("display", "none");
	chkerr.css("display", "block");
}

function check_okerr_hiden(inobj)
{
	var chkok = inobj.parent().next();
	var chkerr = inobj.parent().next().next();
	chkok.css("display", "none");
	chkerr.css("display", "none");
}
*/
function check_ok_show(inobj) {
	inobj.css("color", "rgb(85, 85, 85)");
	/*
	if(inobj.hasClass("input_err"))
	{
		inobj.removeClass("input_err");
	}
	*/
}

function check_err_show(inobj) {
	inobj.css("color", "rgb(255, 0, 0)");
	/*
	if(!inobj.hasClass("input_err"))
	{
		inobj.addClass("input_err");
	}
	*/
}

function check_okerr_hiden(inobj) {
	inobj.css("color", "rgb(85, 85, 85)");
	/*
	if(inobj.hasClass("input_err"))
	{
		inobj.removeClass("input_err");
	}
	if(inobj.hasClass("input_ok"))
	{
		inobj.removeClass("input_ok");
	}
	*/
}


function input_address_check(self, obj) {
	var value = self.value;
	if (value != "") {
		if (checkipv4(value) == true) {
			check_ok_show(obj);
			return true;
		}
		else {
			check_err_show(obj);
		}
	}
	return false;
}

/*
oninput 事件在用户输入时触发。

该事件在 <input> 或 <textarea> 元素的值发生改变时触发。

提示： 该事件类似于 onchange 事件。不同之处在于 oninput 事件在元素值发生变化是立即触发， onchange 在元素失去焦点时触发。另外一点不同是 onchange 事件也可以作用于 <keygen> 和 <select> 元素。
*/
function xhr_json_submit(url, json, runobj, warningobj, okcb, errcb) {
	//var argv;
	var stxhr = new XHR();

	var data_json = JSON.stringify(json);

	runobj.css("display", "block");
	stxhr.post_json(url, data_json,
		function (x, json) {
			console.log(x);
			if (x.responseText == "ERROR") {
				warningobj.css("display", "block");
				runobj.css("display", "none");
				setTimeout(function () { warningobj.css("display", "none") }, 3000);
				errcb(x, json);
				return false;
			}
			else if (x.responseText == "OK") {
				runobj.css("display", "none");
				warningobj.css("display", "none");
				okcb(x, json);
				return true;
			}
			else if (x.responseText != "" && json != null) {
				if (json.response == "OK") {
					runobj.css("display", "none");
					warningobj.css("display", "none");
					okcb(x, json);
					return true;
				}
				else if (json.response == "ERROR") {
					warningobj.css("display", "block");
					runobj.css("display", "none");
					setTimeout(function () { warningobj.css("display", "none") }, 3000);
					errcb(x, json);
					return false;
				}
			}
		},
		3
	);
	return false;
}

function xhr_simple_submit(url, json, runobj, warningobj, okcb, errcb) {
	//var argv;
	var stxhr = new XHR();

	var data_json = json;

	runobj.css("display", "block");
	stxhr.post(url, data_json,
		function (x, json) {
			console.log(x);
			if (x.responseText == "ERROR") {
				warningobj.css("display", "block");
				runobj.css("display", "none");
				setTimeout(function () { warningobj.css("display", "none") }, 3000);
				errcb(x, json);
				return false;
			}
			else if (x.responseText == "OK") {
				runobj.css("display", "none");
				warningobj.css("display", "none");
				okcb(x, json);
				return true;
			}
			else if (x.responseText != "" && json != null) {
				if (json.response == "OK") {
					runobj.css("display", "none");
					warningobj.css("display", "none");
					okcb(x, json);
					return true;
				}
				else if (json.response == "ERROR") {
					warningobj.css("display", "block");
					runobj.css("display", "none");
					setTimeout(function () { warningobj.css("display", "none") }, 3000);
					errcb(x, json);
					return false;
				}
			}
		},
		3
	);
	return false;
}

function xhr_simple_load(url, json, jsoncb) {
	//var argv;
	var stxhr = new XHR();

	var data_json = json;

	runobj.css("display", "block");
	stxhr.get(url, data_json,
		function (x, json) {
			console.log(x);
			if (x.responseText != "" && json != null) {
				var data_json = JSON.parse(json);
				$.each(data_json, jsoncb);
			}
		}
	);
	return false;
}

function select_option_active(select, active) {
	for (var i = 0, count = select.options.length; i < count; i++) {
		if (select.options[i].value == active) {
			select.options[i].selected = true;
			break;
		}
		else
			select.options[i].selected = false;
	}
}

function get_select_option_value(select) {
	var value = select.options[select.selectedIndex].value;
	return value;
}

function upload_file(upload_url, filename, fileobj, id, okcb, errcb) {
	var formData = new FormData();
	formData.append(filename, fileobj);
	formData.append("ID", id);
	//formData.append("crowd_desc", crowd_desc);
	//console.log("filename:" + filename);
	console.log("ID:" + id);

	$.ajax({
		type: 'POST',
		//dataType: 'json',
		url: upload_url,//"http://192.168.1.101:8080/springbootdemo/file/upload",
		data: formData,
		async: false,
		cache: false,
		processData: false,
		contentType: false,
		success: function (data) {
			console.log("上传成功:");
			//console.log(data);
			if (okcb)
				okcb(data);
		},
		error: function (response) {
			console.log("上传失败:");
			if (errcb)
				errcb(response);
		}
	});
}

function download_file(url, form) {
	//var argv;
	var stxhr = new XHR();
	stxhr.post(url, form);
}

function get_cookie(cname) {
	var name = cname + "=";
	var ca = document.cookie.split(';');
	for (var i = 0; i < ca.length; i++) {
		var c = ca[i].trim();
		console.log("get_cookie:" + c);
		if (c.indexOf(name) == 0) { return c.substring(name.length, c.length); }
	}
	return "";
}

function del_cookie(cname) {
	document.cookie = cname + "=; expires=Thu, 01 Jan 1970 00:00:00 GMT";
	console.log("del_cookie:" + document.cookie);
}

function add_cookie(cname, cvalue, exmin) {
	var d = new Date();
	d.setTime(d.getTime() + (exmin * 60 * 60 * 1000));
	var expires = "expires=" + d.toGMTString() + "; path=/";
	document.cookie = cname + "=" + cvalue + "; " + expires;
	console.log("add_cookie:" + document.cookie);
}

function set_cookie(cname, cvalue, exmin) {
	var d = new Date();
	d.setTime(d.getTime() + (exmin * 60 * 60 * 1000));
	var expires = "expires=" + d.toGMTString() + "; path=/";
	document.cookie = cname + "=" + cvalue + "; " + expires;
	console.log("set_cookie:" + document.cookie);
}

function check_cookie(cname) {
	var user = get_ookie(cname);
	if (user != "") {
		return true;
	}
	else {
		return false;
	}
}

function view_progress_load(self, act, id, func) {
	var stxhr = new XHR();
	var argv = "VIEW=" + act + "&ID=" + id;
	stxhr.post("/goform/progress_view", argv,
		function (x, json) {
			if (x.responseText != "" && json != null) {
				if (json.response == "OK") {
					func(x, json);
				}
			}
		},
		2000);
}


function view_progress_start(self, act, id, func, timeout) {
	setInterval(function () {
		var stxhr = new XHR();
		var argv = "VIEW=" + act + "&ID=" + id;
		stxhr.post("/goform/progress_view", argv,
			function (x, json) {
				if (x.responseText != "" && json != null) {
					if (json.response == "OK") {
						func(x, json);
					}
				}
			},
			2000);
	}, timeout);
}