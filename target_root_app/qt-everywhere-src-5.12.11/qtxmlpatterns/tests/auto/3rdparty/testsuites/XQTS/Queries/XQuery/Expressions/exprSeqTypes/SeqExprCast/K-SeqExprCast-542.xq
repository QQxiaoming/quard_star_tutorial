(:*******************************************************:)
(: Test: K-SeqExprCast-542                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:decimal to xs:string is allowed and should always succeed. :)
(:*******************************************************:)
xs:decimal("10.01") cast as xs:string
                    ne
                  xs:string("an arbitrary string")