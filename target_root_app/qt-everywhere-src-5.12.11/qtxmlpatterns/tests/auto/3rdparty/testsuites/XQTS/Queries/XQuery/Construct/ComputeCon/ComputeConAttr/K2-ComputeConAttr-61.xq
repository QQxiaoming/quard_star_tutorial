(:*******************************************************:)
(: Test: K2-ComputeConAttr-61                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Trigger XQTY0024 in a query with a bit of complexity. :)
(:*******************************************************:)
let $x :=
( attribute a { "a" },
  element b { "b" },
  attribute c { "c" } )
return
<foo> { $x } </foo>