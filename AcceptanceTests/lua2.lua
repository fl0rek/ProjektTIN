trivial_proto = Proto("TIN","SIMPLE TIN PROTOCOL")

local tagf_message = {
	[1] = " TAG_INTERNAL",
	[2] = " TAG_GAME",
	[3] = " TAG_CHAT",
	[4] = " TAG_HELLO",
	[5] = " TAG_BYE",
	[6] = " TAG_ECHO",
	[7] = " TAG_OK",
	[0] = " TAG_UNKNOWN"
}

local tagm_message = {
	[0] = " CLIENT_ID",
	[1] = " AUTHENTICATION_CODE",
	[2] = " REQUEST_NAME",
	[3] = " AUTHENTICATION_ERROR",
	[4] = " RESYNC_RESPONSE",
	[5] = " STEP",
	[6] = " INVALID_STEP",
	[7] = " TERMINATE",
	[8] = " ADD_CLIENT",
	[9] = " START_GAME",
	[10] = " MESSAGE",
	[11] = " UNKNOWN"
}
local tin_fields = {
	tag = ProtoField.int8("TAG", "First tag ", base.BYTES),
	sizeM = ProtoField.int8("SIZE", "Size of the rest o message", base.BYTES),
	tagm = ProtoField.int32("TAG_M", "Specific tag: ", base.BYTES),
	f = ProtoField.int8("F", "Flags", base.DEC),
	s = ProtoField.int8("S", "Remaining size to send", base.INT),
	data = ProtoField.int64("DATA", "serialized data", base.BYTES)
}
trivial_proto.fields = tin_fields

function trivial_proto.dissector(buffer, pinfo, tree)
	pinfo.cols.protocol = "TIN"
	
	local subtree = tree:add(trivial_proto, buffer(), "SIMPLE TIN PROTOCOL")
	local t_size = buffer(0, 1):uint()
	local s1_size = buffer(1, 1):uint()
	if t_size == 0x11 then
		subtree:add(tin_fields.tag, t_size):append_text(tagf_message[1])
	elseif t_size == 0x12 then
		subtree:add(tin_fields.tag, t_size):append_text(tagf_message[2])
	elseif t_size == 0x13 then
		subtree:add(tin_fields.tag, t_size):append_text(tagf_message[3])
	elseif t_size == 0xe0 then
		subtree:add(tin_fields.tag, t_size):append_text(tagf_message[4])
	elseif t_size == 0xe1 then
		subtree:add(tin_fields.tag, t_size):append_text(tagf_message[5])
	elseif t_size == 0xe2 then
		subtree:add(tin_fields.tag, t_size):append_text(tagf_message[6])
	elseif t_size == 0xe3 then
		subtree:add(tin_fields.tag, t_size):append_text(tagf_message[7])
	else
		subtree:add(tin_fields.tag, t_size):append_text(tagf_message[0])
	end
	subtree:add(tin_fields.sizeM, s1_size)
	if t_size == 0x11 or t_size == 0x12 or t_size == 0x13 then
		local spec_tag = buffer(2, 4):uint()
		if spec_tag == tonumber("11000001", 16) then
			subtree:add(tin_fields.tagm, spec_tag):append_text(tagm_message[0])
		elseif spec_tag == tonumber("11000002", 16) then
			subtree:add(tin_fields.tagm, spec_tag):append_text(tagm_message[1])
		elseif spec_tag == tonumber("11000003", 16) then
			subtree:add(tin_fields.tagm, spec_tag):append_text(tagm_message[2])
		elseif spec_tag == tonumber("11000004", 16) then
			subtree:add(tin_fields.tagm, spec_tag):append_text(tagm_message[3])
		elseif spec_tag == tonumber("12000001", 16) then
			subtree:add(tin_fields.tagm, spec_tag):append_text(tagm_message[4])
		elseif spec_tag == tonumber("12000002", 16) then
			subtree:add(tin_fields.tagm, spec_tag):append_text(tagm_message[5])
		elseif spec_tag == tonumber("12000003", 16) then
			subtree:add(tin_fields.tagm, spec_tag):append_text(tagm_message[6])
		elseif spec_tag == tonumber("12000004", 16) then
			subtree:add(tin_fields.tagm, spec_tag):append_text(tagm_message[7])
		elseif spec_tag == tonumber("12000005", 16) then
			subtree:add(tin_fields.tagm, spec_tag):append_text(tagm_message[8])
		elseif spec_tag == tonumber("12000006", 16) then
			subtree:add(tin_fields.tagm, spec_tag):append_text(tagm_message[9])
		elseif spec_tag == tonumber("13000002", 16) then
			subtree:add(tin_fields.tagm, spec_tag):append_text(tagm_message[10])
		else
			subtree:add(tin_fields.tagm, spec_tag):append_text(tagm_message[11])		
		end
	end
end
tcp_table = DissectorTable.get("tcp.port")
tcp_table:add(4200, trivial_proto)
