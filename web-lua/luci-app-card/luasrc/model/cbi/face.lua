-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys = require "luci.sys"
local web = require "luci.tools.webadmin"
local fs = require "nixio.fs"


m2 = Map("openconfig", translate("Face Recognition"), translate("Face Recognition parameters"))
-- s = m2:section(TypedSection, "sip", translate("SIP stack configure option"))
s = m2:section(NamedSection, "faceopt", "hardware", translate("Face Recognition option"))
s.anonymous = false
s.addremove = false


-- Face Record threshold 录入阈值
record_threshold = s:option(Value, "record_threshold", translate("Face Record threshold"))
record_threshold.datatype = "range(0.0,1.0)"
record_threshold.rmempty = true
record_threshold.default=0.8

-- Face Recognize threshold 识别阈值
recognize_threshold = s:option(Value, "recognize_threshold", translate("Face Recognize threshold"))
recognize_threshold.datatype = "range(0.0,1.0)"
recognize_threshold.rmempty = true
recognize_threshold.default=0.8

-- In vivo detection switch 活体检测开关
living_detection = s:option(Flag, "living_detection", translate("Living Detection"))
living_detection.rmempty = true
living_detection.default = true

-- Face Living threshold 活体阈值
living_threshold = s:option(Value, "living_threshold", translate("Face Living threshold"))
living_threshold.datatype = "range(0.0,1.0)"
living_threshold.rmempty = true
living_threshold.default=0.8
living_threshold:depends("living_detection", "1")


-- Face Successful Recognize intervals 两次成功识别间隔
success_intervals = s:option(Value, "success_intervals", translate("Success Recognize intervals"))
success_intervals.datatype = "range(0,30)"
success_intervals.rmempty = true
success_intervals.default=2

-- Face failure Recognize intervals 两次失败识别间隔
failure_intervals = s:option(Value, "failure_intervals", translate("Failure Recognize intervals"))
failure_intervals.datatype = "range(0,30)"
failure_intervals.rmempty = true
failure_intervals.default=3

--[[
-- Face target size filtering 目标尺寸过滤
taget_filtering = s:option(Value, "taget_filtering", translate("Target size filtering"))
taget_filtering.datatype = "range(0,255)"
taget_filtering.rmempty = true
taget_filtering.default=2
]]

-- yaw角和pitch角
face_yaw_left = s:option(Value, "face_yaw_left", translate("Face Yaw Left"))
face_yaw_left.datatype = "range(-180,180)"
face_yaw_left.rmempty = true
face_yaw_left.default=-20

face_yaw_right = s:option(Value, "face_yaw_right", translate("Face Yaw Right"))
face_yaw_right.datatype = "range(-180,180)"
face_yaw_right.rmempty = true
face_yaw_right.default=20

face_pitch_up = s:option(Value, "face_pitch_up", translate("Face Pitch Up"))
face_pitch_up.datatype = "range(-180,180)"
face_pitch_up.rmempty = true
face_pitch_up.default=-20

face_pitch_down = s:option(Value, "face_pitch_down", translate("Face Pitch Down"))
face_pitch_down.datatype = "range(-180,180)"
face_pitch_down.rmempty = true
face_pitch_down.default=20


-- 人脸大小参数
face_roi_width = s:option(Value, "face_roi_width", translate("ROI Width"))
face_roi_width.datatype = "range(0,255)"
face_roi_width.rmempty = true
face_roi_width.default=150

face_roi_height = s:option(Value, "face_roi_height", translate("ROI Height"))
face_roi_height.datatype = "range(0,255)"
face_roi_height.rmempty = true
face_roi_height.default=150


-- 录入人脸大小参数
face_record_width = s:option(Value, "face_record_width", translate("Record Width"))
face_record_width.datatype = "range(0,255)"
face_record_width.rmempty = true
face_record_width.default=90

face_record_height = s:option(Value, "face_record_height", translate("Record Height"))
face_record_height.datatype = "range(0,255)"
face_record_height.rmempty = true
face_record_height.default=90


-- 识别人脸大小参数
face_recognize_width = s:option(Value, "face_recognize_width", translate("Recognize Width"))
face_recognize_width.datatype = "range(0,255)"
face_recognize_width.rmempty = true
face_recognize_width.default=120

face_recognize_height = s:option(Value, "face_recognize_height", translate("Recognize Height"))
face_recognize_height.datatype = "range(0,255)"
face_recognize_height.rmempty = true
face_recognize_height.default=120






--[[
-- ROI 人脸大小参数
face_roi = s:option(Flag, "face_roi", translate("ROI Configure"))
face_roi.rmempty = true
face_roi.default = true

-- ROI left, right, top bottom
face_roi_left = s:option(Value, "face_roi_left", translate("ROI Left"))
face_roi_left.datatype = "range(0,255)"
face_roi_left.rmempty = true
face_roi_left.default=15
face_roi_left:depends("face_roi", "1")

face_roi_right = s:option(Value, "face_roi_right", translate("ROI Right"))
face_roi_right.datatype = "range(0,255)"
face_roi_right.rmempty = true
face_roi_right.default=15
face_roi_right:depends("face_roi", "1")

face_roi_top = s:option(Value, "face_roi_top", translate("ROI Top"))
face_roi_top.datatype = "range(0,255)"
face_roi_top.rmempty = true
face_roi_top.default=15
face_roi_top:depends("face_roi", "1")

face_roi_bottom = s:option(Value, "face_roi_bottom", translate("ROI Bottom"))
face_roi_bottom.datatype = "range(0,255)"
face_roi_bottom.rmempty = true
face_roi_bottom.default=15
face_roi_bottom:depends("face_roi", "1")
]]

local apply=luci.http.formvalue("cbi.apply")

if apply then
  local pfd = io.popen("lua-sync -m app -c face-option")
  if pfd then
    pfd:close()
  end 
end

return m2