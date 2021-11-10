local redirect

function Link(elem)
  if isLocalLink(elem) then
    if isFolderLink(elem) then
      return pandoc.Link(elem.content, elem.target .. 'index.htm', elem.title, elem.attr)
    elseif isAnchorLink(elem) then
      return pandoc.Link(elem.content, elem.target, elem.title, elem.attr)
    else
      return pandoc.Link(elem.content, elem.target .. '.htm', elem.title, elem.attr)
    end
  end
  return elem
end

function isLocalLink(elem)
  return string.match(elem.target, "://") == nil
end

function isFolderLink(elem)
  return not ( string.match(elem.target, "/$") == nil )
end

function isAnchorLink(elem)
  return not ( string.match(elem.target, "^#") == nil )
end  

-- Support for redirect meta key, adds a meta refresh to the resulting document
function Meta(meta)
  for k, v in pairs(meta) do
    if k == 'redirect' and type(v) == 'table' and v.t == 'MetaInlines' then
      redirect = v[1].c
    end
  end
  return meta
end

function Pandoc(doc)
  if redirect then
    local metaRefresh = pandoc.RawBlock('html', "<meta http-equiv='refresh' content='0;url=" .. redirect .. ".htm'>")
    local redirectText = pandoc.Para({pandoc.Str("This content can be found "), pandoc.Link("here", redirect .. ".htm"), pandoc.Str(".")})
    table.insert(doc.blocks, metaRefresh)
    table.insert(doc.blocks, redirectText)
  end
  return pandoc.Pandoc(doc.blocks, doc.meta)
end
