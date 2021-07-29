(:*******************************************************:)
(: Test: K-SeqExprCast-1218                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:boolean to xs:string is allowed and should always succeed. :)
(:*******************************************************:)
xs:boolean("true") cast as xs:string
                    ne
                  xs:string("an arbitrary string")