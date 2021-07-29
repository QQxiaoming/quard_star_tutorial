(:*******************************************************:)
(: Test: K-SeqExprCast-434                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:string to xs:string is allowed and should always succeed. :)
(:*******************************************************:)
xs:string("an arbitrary string") cast as xs:string
                    eq
                  xs:string("an arbitrary string")