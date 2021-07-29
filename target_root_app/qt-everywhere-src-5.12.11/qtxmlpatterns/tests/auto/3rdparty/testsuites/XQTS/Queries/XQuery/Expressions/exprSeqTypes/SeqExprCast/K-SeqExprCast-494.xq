(:*******************************************************:)
(: Test: K-SeqExprCast-494                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:double to xs:string is allowed and should always succeed. :)
(:*******************************************************:)
xs:double("3.3e3") cast as xs:string
                    ne
                  xs:string("an arbitrary string")