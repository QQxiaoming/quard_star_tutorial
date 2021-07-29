(:*******************************************************:)
(: Test: K-SeqExprCast-1222                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:boolean to xs:double is allowed and should always succeed. :)
(:*******************************************************:)
xs:boolean("true") cast as xs:double
                    ne
                  xs:double("3.3e3")