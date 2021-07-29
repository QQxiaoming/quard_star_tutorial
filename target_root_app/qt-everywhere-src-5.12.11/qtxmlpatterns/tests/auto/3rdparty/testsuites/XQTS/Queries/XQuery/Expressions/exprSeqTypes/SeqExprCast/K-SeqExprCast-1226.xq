(:*******************************************************:)
(: Test: K-SeqExprCast-1226                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:boolean to xs:integer is allowed and should always succeed. :)
(:*******************************************************:)
xs:boolean("true") cast as xs:integer
                    ne
                  xs:integer("6789")