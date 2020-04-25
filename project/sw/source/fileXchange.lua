fileXchange_protocol = Proto("fileXchange",  "FileXchange Protocol")

fileXchange_protocol.fields = {}

function fileXchange_protocol.dissector(buffer, pinfo, tree)
  length = buffer:len()
  if length == 0 then return end

  pinfo.cols.protocol = fileXchange_protocol.name

  local subtree = tree:add(fileXchange_protocol, buffer(), "fileXchange Protocol Data")
end

local udp_port = DissectorTable.get("udp.port")
udp_port:add(7777, fileXchange_protocol)
