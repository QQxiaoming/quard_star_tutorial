(:*******************************************************:)
(: Test: K-SeqExprCast-626                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:integer to xs:boolean is allowed and should always succeed. :)
(:*******************************************************:)
xs:integer("6789") cast as xs:boolean
                    eq
                  xs:boolean("true")