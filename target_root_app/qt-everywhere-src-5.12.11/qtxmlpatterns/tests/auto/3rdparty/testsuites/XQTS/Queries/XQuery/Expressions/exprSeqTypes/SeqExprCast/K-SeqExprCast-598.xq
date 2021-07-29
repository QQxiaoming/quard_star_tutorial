(:*******************************************************:)
(: Test: K-SeqExprCast-598                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:integer to xs:double is allowed and should always succeed. :)
(:*******************************************************:)
xs:integer("6789") cast as xs:double
                    ne
                  xs:double("3.3e3")