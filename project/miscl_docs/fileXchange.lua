fileXchange_protocol = Proto("fileXchange",  "FileXchange Protocol")

file_id = ProtoField.uint16("fileXchange.file_id", "file_id", base.DEC)

fileXchange_protocol.fields = {file_id}

function fileXchange_protocol.dissector(buffer, pinfo, tree)
  length = buffer:len()
  if length == 0 then return end

  pinfo.cols.protocol = fileXchange_protocol.name

  local subtree = tree:add(fileXchange_protocol, buffer(), "fileXchange Protocol Data")

  subtree:add_le(file_id, buffer(0,2))
end

local udp_port = DissectorTable.get("udp.port")
udp_port:add(7777, fileXchange_protocol)
