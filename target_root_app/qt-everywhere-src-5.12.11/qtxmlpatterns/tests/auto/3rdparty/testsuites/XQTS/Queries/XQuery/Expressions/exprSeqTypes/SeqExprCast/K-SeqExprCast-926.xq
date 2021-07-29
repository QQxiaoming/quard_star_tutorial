(:*******************************************************:)
(: Test: K-SeqExprCast-926                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:date to xs:date is allowed and should always succeed. :)
(:*******************************************************:)
xs:date("2004-10-13") cast as xs:date
                    eq
                  xs:date("2004-10-13")