-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys = require "luci.sys"
local web = require "luci.tools.webadmin"
local fs = require "nixio.fs"


m2 = Map("openconfig", translate("Face Recognition"), translate("Face Recognition parameters"))
-- s = m2:section(TypedSection, "sip", translate("SIP stack configure option"))
s = m2:section(NamedSection, "face", "recognition", translate("Face Recognition option"))
s.anonymous = false
s.addremove = false


-- Face contrast threshold
contrast_threshold = s:option(Value, "contrast_threshold", translate("Face contrast threshold"))
contrast_threshold.datatype = "range(0.0,1.0)"
contrast_threshold.rmempty = true
contrast_threshold.default=0.0

-- Face target size filtering
taget_filtering = s:option(Value, "taget_filtering", translate("Target size filtering"))
taget_filtering.datatype = "range(0,255)"
taget_filtering.rmempty = true
taget_filtering.default=2

-- In vivo detection switch
vivo_detection = s:option(Flag, "vivo_detection", translate("In Vivo Detection"))
vivo_detection.rmempty = true
vivo_detection.default = true

-- In vivo detection threshold
vivo_detection_thr = s:option(Value, "vivo_detection_thr", translate("In Vivo Detection threshold"))
vivo_detection_thr.rmempty = true
vivo_detection_thr.default=false
vivo_detection_thr.datatype = "range(0.0,1.0)"
vivo_detection_thr:depends("vivo_detection", "1")


-- ROI 
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

local apply=luci.http.formvalue("cbi.apply")

if apply then
  local pfd = io.popen("lua-sync -m app -c face-option")
  if pfd then
    pfd:close()
  end 
end

return m2