(:*******************************************************:)
(: Test: K-SeqExprCast-1010                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:gYear to xs:string is allowed and should always succeed. :)
(:*******************************************************:)
xs:gYear("1999") cast as xs:string
                    ne
                  xs:string("an arbitrary string")