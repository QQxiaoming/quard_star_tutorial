(:*******************************************************:)
(: Test: K2-FilterExpr-5                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Subsequent filter expressions, and node tests whose focus depends on where a variable is declared. :)
(:*******************************************************:)
let $d := document {<root><child type=""/></root>}
    return $d//*[let $i := @type
                 return $d//*[$i]],
    (1, 2, 3)[true()],
    (4, 5, 6)[false()]