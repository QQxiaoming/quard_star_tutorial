(:*******************************************************:)
(: Test: K-SeqExprCast-522                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:double to xs:boolean is allowed and should always succeed. :)
(:*******************************************************:)
xs:double("3.3e3") cast as xs:boolean
                    eq
                  xs:boolean("true")